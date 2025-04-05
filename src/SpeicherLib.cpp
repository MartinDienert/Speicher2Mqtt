#include <time.h>
#include <Main.h>
#include <SpeicherLib.h>

Daten::Daten(){
    genJson();
}

void Daten::genJson(){
    json = "{\"Spannung\":" + (String)spannung;
    json += ",\"Ladezustand\":" + (String)soc;
    json += ",\"StromAkku\":" + (String)stromakku;
    json += ",\"Typ\":" + (String)typ;
    if(typ == 2){
        json += ",\"StromPV\":" + (String)strompv;
        json += ",\"Temperatur\":" + (String)temperatur;
    }else{
        json += ",\"StromPV\":0,\"Temperatur\":0";
    }
    json += ",\"Datum\":" + ("\"" + datum + "\"");
    json += ",\"Zeit\":" + ("\"" + zeit + "\"");
    String s = (laden)? "ein": "aus";
    json += ",\"Laden\":" + ("\"" + s + "\"");
    s = (entladen)? "ein": "aus";
    json += ",\"Entladen\":" + ("\"" + s + "\"");
    json += "}";
}

void Daten::setDaten(byte t, float s, int so, float sa){
    typ = t;
    spannung = s;
    soc = so;
    stromakku = sa;
    datum = getDatumStr();
    zeit = getZeitStr();
    genJson();
    mqttPub();
}

void Daten::setDaten(byte t, float s, int so, float sa, float sp, int tp){
    strompv = sp;
    temperatur = tp;
    setDaten(t, s, so, sa);
}

void Daten::setLaden(boolean l){
    laden = l;
    genJson();
}

void Daten::setEntladen(boolean l){
    entladen = l;
    genJson();
}

boolean Daten::getLaden(){
    return laden;
}

boolean Daten::getEntladen(){
    return entladen;
}

Speicher::Speicher(Daten* d){
    daten = d;
}

void Speicher::sendeTel(int t, boolean to){
    if(tele != -1){                                 // das Telegramm wurde noch nicht gesendet
        if(to)
            t += 0x100;
        timer.in(1000, teleTimer, (void *)t);       // Telegramm in 1 Sekunde noch einmal senden
        return;
    }
    if(to){
        if(t == telLa){
            if(!daten->getLaden())
                t++;
        }else if(t == telEl){
            if(!daten->getEntladen())
                t++;
        }
    }
    tele = t;
}

void Speicher::sendeTel(int t){
    sendeTel(t, false);
}

void Speicher::sendeZeit(){
    if(einst.master && getZeit()){
        byte *t = telegramme[6];
        t[8] = dat.tm_year - 100;
        t[9] = dat.tm_mon + 1;
        t[10] = dat.tm_mday;
        t[11] = dat.tm_hour;
        t[12] = dat.tm_min;
        t[13] = dat.tm_sec;
        t[14] = dat.tm_wday;
        t[15] = pruefsummeBer(t, 16);
        sendeTel(telZe);
    }
}

void Speicher::setMaster(boolean m){
    if(m) master();
    else startTele = 0;
}

void Speicher::master(){
    if(einst.master){
        if(startTele < 6){
            sendeTel(startTele);
            timer.in(warteZeiten[startTele], masterTimer);
            startTele++;
        }else{
            sendeTel(0);
            totmanRun();
            timer.in(warteZeiten[5], masterTimer);
        }
        if(startTele == 5)
            timer.in(15000, zeitTimer);
    }
}

void Speicher::run(){
    int l;
    byte bp[6]; 
    l = Serial.readBytes(bp, 6);
    while(l == 6){
        if(bp[0] == 0x55 && bp[1] == 0xAA){
            l = bp[5];
            byte bp2[l + 1];
            l = Serial.readBytes(bp2, l + 1);
            if(bp[3] == 0x07){
                if(l == 156){
                    if(pruefsumme(bp, 6, bp2, 156))
                        decodieren1(bp2);
                    totmanReset();
                }
                if(l == 153){
                    if(pruefsumme(bp, 6, bp2, 153))
                        decodieren2(bp2);
                }
                if(l == 6){
                    if(pruefsumme(bp, 6, bp2, 6))
                        decodieren3(bp2);
                }
            }
        }
        l = Serial.readBytes(bp, 6);
    }
    senden();
}

void Speicher::senden(){
    if(tele >= 0){
        digitalWrite(D1, HIGH);
        delay(10);
        Serial.write(telegramme[tele], teleGroesse[tele]);
//        Serial.print("Senden: ");
//        Serial.println(tele);
        delay(5);
        digitalWrite(D1, LOW);
        tele = -1;
    }
}

byte Speicher::pruefsummeBer(byte* bp, int l1, byte* bp2, int l2){
    int pfsumme = 0;
    for(int i = 2; i < l1; i++)
        pfsumme += bp[i];
    for(int i = 0; i < l2 - 1; i++)
        pfsumme += bp2[i];
    return (byte)pfsumme - 1;
}

byte Speicher::pruefsummeBer(byte* bp, int l){
    int pfsumme = 0;
    for(int i = 2; i < l - 1; i++)
        pfsumme += bp[i];
    return (byte)pfsumme - 1;
}

boolean Speicher::pruefsumme(byte* bp, int l1, byte* bp2, int l2){
    return bp2[l2 - 1] == pruefsummeBer(bp, l1, bp2, l2);
}

void Speicher::decodieren1(byte* bp){
    daten->setDaten(1, (float)((sint16)(bp[62] * 256 + bp[63])) / 10, bp[59], (float)((sint16)(bp[66] * 256 + bp[67])) / 10);
}

void Speicher::decodieren2(byte* bp){
    daten->setDaten(2, (float)((sint16)(bp[123] * 256 + bp[124])) / 10, bp[122],(float)((sint16)(bp[125] * 256 + bp[126])) / 10,
                       (float)((sint16)(bp[150] * 256 + bp[151])) / 10, bp[136]);
}

void Speicher::decodieren3(byte* bp){
    if(bp[0] == 0x65)
        daten->setLaden(bp[4] == 1);
    else if(bp[0] == 0x66)
        daten->setEntladen(bp[4] == 1);
}

void Speicher::totmanReset(){
    if(einst.master){
        totman = millis();
    }
}

void Speicher::totmanRun(){
    if(totman == 0)
        return;
    if(millis() - totman > totmanZeit * 1000){
        startTele = 0;
        totman = 0;
        addLog("Verbindung resettet.");
    }
}
