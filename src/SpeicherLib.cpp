#include <SpeicherLib.h>

Speicher::Speicher(){
}

void Speicher::callbackLesen(Lesen l){
    lesen = l;
} 

void Speicher::callbackSchreiben(Schreiben s){
    schreiben = s;
} 

void Speicher::callbackNeueDaten(NeueDaten d){
    neueDaten = d;
}

void Speicher::callbackGetDatumZeit(GetDatumZeit gDZ){
    getDatumZeit = gDZ;
}

void Speicher::callbackLogeintrag(Logeintrag l){
    logeintrag = l;
}

void Speicher::sendeTel(int t, boolean to){
    if(tele != -1){                                 // das Telegramm wurde noch nicht gesendet
        if(to)
            t += 0x100;
        setTeleTimer(1000, t);                      // Telegramm in 1 Sekunde noch einmal senden
        return;
    }
    if(to){
        if(t == telLa){
            if(!daten.laden)
                t++;
        }else if(t == telEl){
            if(!daten.entladen)
                t++;
        }
    }
    tele = t;
}

void Speicher::sendeTel(int t){
    sendeTel(t, false);
}

void Speicher::sendeZeit(){
    if(masterZeit != 0){
        if(getDatumZeit){
            getDatumZeit(&zeit);
            byte *t = telegramme[6];
            t[8]  = zeit.jahr;
            t[9]  = zeit.monat;
            t[10] = zeit.tag;
            t[11] = zeit.stunde;
            t[12] = zeit.minute;
            t[13] = zeit.sekunde;
            t[14] = zeit.tagWoche;
            t[15] = pruefsummeBer(t, 16);
            sendeTel(telZe);
        }
    }
}

void Speicher::startMaster(unsigned long wz){
    setMasterTimer(wz);
}

void Speicher::setMaster(boolean m){
    if(m){
      if(masterZeit == 0)
        master();
    }else{
        startTele = 0;
        masterZeit = 0;
    }
}

void Speicher::startMDaten(unsigned long wz){
    setMDatenTimer(wz);
}

void Speicher::setMDaten(boolean m){
    if(m){
        if(mDatenZeit == 0)
            setMDatenTimer(1000);
    }else{
        mDatenZeit = 0;
    }
}

void Speicher::master(){
    if(startTele < 6){
        sendeTel(startTele);
        setMasterTimer(warteZeiten[startTele]);
        startTele++;
    }else{
        sendeTel(0);
        setMasterTimer(warteZeiten[5]);
    }
    if(startTele == 5){
        setZeitTelTimer(warteZeiten[6]);
        totmanReset();
    }
}

void Speicher::run(){
    timerRun();
    empfangen();
    senden();
}

void Speicher::empfangen(){
    int l = 0;
    if(lesen) l = lesen(bp, 6);
    while(l == 6){
        if(bp[0] == 0x55 && bp[1] == 0xAA){
            l = bp[5];
            if(lesen) l = lesen(bp2, l + 1);
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
        if(lesen) l = lesen(bp, 6); else l = 0;
    }
}

void Speicher::senden(){
    if(tele >= 0){
        if(schreiben) schreiben(telegramme[tele], teleGroesse[tele]);
//Serial.print("Senden: ");
//Serial.println(tele);
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
    daten.typ = 1;
    daten.spannung = (float)((short)(bp[62] * 256 + bp[63])) / 10;
    daten.soc = bp[59];
    daten.stromakku = (float)((short)(bp[66] * 256 + bp[67])) / 10;
    daten.strompv = 0.0;
    daten.temperatur = 0;
    if(neueDaten) neueDaten();
}

void Speicher::decodieren2(byte* bp){
    daten.typ = 2;
    daten.spannung = (float)((short)(bp[123] * 256 + bp[124])) / 10;
    daten.soc = bp[122];
    daten.stromakku = (float)((short)(bp[125] * 256 + bp[126])) / 10;
    daten.strompv = (float)((short)(bp[150] * 256 + bp[151])) / 10;
    daten.temperatur = bp[136];
    if(neueDaten) neueDaten();
}

void Speicher::decodieren3(byte* bp){
    if(bp[0] == 0x65){
        daten.laden = (bp[4] == 1);
    }else if(bp[0] == 0x66){
        daten.entladen = (bp[4] == 1);
    }else return;
    if(neueDaten) neueDaten();
}

Daten Speicher::getDaten(){
    return daten;
}

// --------------------- Timer ---------------------
void Speicher::timerRun(){
    unsigned long zeit = millis();
    if(masterZeit > 0 && zeit - masterZeit > masterInterval){
        masterZeit = 0;
        master();
    }
    if(mDatenZeit > 0 && zeit - mDatenZeit > mDatenInterval){
        mDatenZeit = 0;
        setMDatenTimer(mDaten_Interval);
        sendeTel(telMD);
    }
    if(teleZeit > 0 && zeit - teleZeit > teleInterval){
        teleZeit = 0;
        sendeTel(teleParam & 0x0FF, (teleParam & 0x100) == 0x100);
    }
    if(zeitTelZeit > 0 && zeit - zeitTelZeit > zeitTelInterval){
        zeitTelZeit = 0;
        sendeZeit();
    }
    if(totmanZeit > 0 && zeit - totmanZeit > totmanInterval){
        totmanZeit = 0;
        startTele = 0;
        if(logeintrag) logeintrag("Verbindung resettet.");
    }
}

void Speicher::setMasterTimer(unsigned long wz){
    masterZeit = millis();
    if(masterZeit == 0) masterZeit = 1;
    masterInterval = wz;
}

void Speicher::setMDatenTimer(unsigned long wz){
    mDatenZeit = millis();
    if(mDatenZeit == 0) mDatenZeit = 1;
    mDatenInterval = wz;
}

void Speicher::setTeleTimer(unsigned long wz, int param){
    teleZeit = millis();
    if(teleZeit == 0) teleZeit = 1;
    teleInterval = wz;
    teleParam = param;
}

void Speicher::setZeitTelTimer(unsigned long wz){
    zeitTelZeit = millis();
    if(zeitTelZeit == 0) zeitTelZeit = 1;
    zeitTelInterval = wz;
}

void Speicher::totmanReset(){
    if(masterZeit != 0){                                // läuft als Master
        totmanZeit = millis();
        if(totmanZeit == 0) totmanZeit = 1;
    }
}
