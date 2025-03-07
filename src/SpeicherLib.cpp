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
    json += "}";
    typ = 0;
}

void Daten::setDaten(byte t, float s, int so, float sa){
    typ = t;
    // spannung_ = spannung;
    // soc_ = soc;
    // stromakku_ = stromakku;
    if(spannung != s){
        spannung = s;
        geaendert = true;
    }
    if(soc != so){
        soc = so;
        geaendert = true;
    }
    if(stromakku != sa){
        stromakku = sa;
        geaendert = true;
    }
    if(geaendert){
        char d[11];
        char z[9];
        getZeitStr(d, z);
        datum = d;
        zeit = z;
        genJson();
        mqttPub();
        geaendert = false;
    }
}

void Daten::setDaten(byte t, float s, int so, float sa, float sp, int tp){
    // strompv_ = strompv;
    // temperatur_ = temperatur;
    if(strompv != sp){
        strompv = sp;
        geaendert = true;
    }
    if(temperatur != tp){
        temperatur = tp;
        geaendert = true;
    }
    setDaten(t, s, so, sa);
}

Speicher::Speicher(Daten* d){
    daten = d;
}

void Speicher::sendeTel(int t){
    tel = t;
//    Serial.print("Senden: ");
//    Serial.println(tel);
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
            if(l == 156){
                if(pruefsumme(bp, 6, bp2, 156))
                    decodieren1(bp2);
            }
            if(l == 153){
                if(pruefsumme(bp, 6, bp2, 153))
                    decodieren2(bp2);
            }
//            Serial.println(l);
        }
        l = Serial.readBytes(bp, 6);
    }
    senden();
}

void Speicher::senden(){
    if(tel > 0){
        digitalWrite(D1, HIGH);
        delay(10);
        Serial.write(telegram[tel - 1], 16);
        delay(50);
        digitalWrite(D1, LOW);
        tel = 0;
    }
}

boolean Speicher::pruefsumme(byte* bp, int l1, byte* bp2, int l2){
    int pfsumme = 0;
    for(int i = 2; i < l1; i++)
        pfsumme += bp[i];
    for(int i = 0; i < l2 - 1; i++)
        pfsumme += bp2[i];
    byte pfs = (byte)pfsumme - 1;
    return bp2[l2 - 1] == pfs;
}

void Speicher::decodieren1(byte* bp){
    daten->setDaten(1, (float)((sint16)(bp[62] * 256 + bp[63])) / 10, bp[59], (float)((sint16)(bp[66] * 256 + bp[67])) / 10);
    // daten->spannung = (float)((sint16)(bp[62] * 256 + bp[63])) / 10;         // -> = auf Variablen über den Objektpointer zugreifen
    // daten->soc = bp[59];
    // daten->stromakku = (float)((sint16)(bp[66] * 256 + bp[67])) / 10;
    // daten->typ = 1;
    // daten->genJson();
}

void Speicher::decodieren2(byte* bp){
    daten->setDaten(2, (float)((sint16)(bp[123] * 256 + bp[124])) / 10, bp[122],(float)((sint16)(bp[125] * 256 + bp[126])) / 10,
                       (float)((sint16)(bp[150] * 256 + bp[151])) / 10, bp[136]);
    // daten->spannung = (float)((sint16)(bp[123] * 256 + bp[124])) / 10;         // -> = auf Variablen über den Objektpointer zugreifen
    // daten->soc = bp[122];
    // daten->stromakku = (float)((sint16)(bp[125] * 256 + bp[126])) / 10;
    // daten->strompv = (float)((sint16)(bp[150] * 256 + bp[151])) / 10;
    // daten->temperatur = bp[136];
    // daten->typ = 2;
    // daten->genJson();
}
