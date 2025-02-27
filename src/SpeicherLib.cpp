#include <SpeicherLib.h>

Daten::Daten(){
    json = "{\"Spannung\":0,\"Ladezustand\":0,\"StromAkku\":0,\"Typ\":0,\"StromPV\":0,\"Temperatur\":0}";
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
    json += "}";
    typ = 0;
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
    daten->spannung = (float)(bp[62] * 256 + bp[63]) / 10;         // -> = auf Variablen über den Objektpointer zugreifen
    daten->soc = bp[59];
    daten->stromakku = (float)(bp[66] * 256 + bp[67]) / 10;
    daten->typ = 1;
    daten->genJson();
}

void Speicher::decodieren2(byte* bp){
    daten->spannung = (float)(bp[123] * 256 + bp[124]) / 10;         // -> = auf Variablen über den Objektpointer zugreifen
    daten->soc = bp[122];
    daten->stromakku = (float)(bp[125] * 256 + bp[126]) / 10;
    daten->strompv = (float)(bp[150] * 256 + bp[151]) / 10;
    daten->temperatur = bp[136];
    daten->typ = 2;
    daten->genJson();
}
