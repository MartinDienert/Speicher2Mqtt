#ifndef SPEICHER_LIB_H_
#define SPEICHER_LIB_H_
#include <Arduino.h>

class Daten  // Class Declaration
{
    public:
        Daten();
        String json;
        void setDaten(byte t, float s, int so, float sa, float sp, int tp);
        void setDaten(byte t, float s, int so, float sa);
    private:
        byte typ = 0;
        float spannung = 0;
        int soc = 0;
        float stromakku = 0;
        float strompv = 0;
        int temperatur = 0;
        String datum = "";
        String zeit = "";
        void genJson();
};

class Speicher  // Class Declaration
{
    public: 
        Speicher(Daten*);  // Constructor
    
        void run();
        void sendeTel(int);
        void master();
        void zeit();
        void setMaster(boolean);
                 
    private:
        Daten* daten;
        int tele = -1;
        int startTele = 0;
        byte t0[7] = {0x55,0xAA,0,0,0,0,0xFF};                          // Lebenszeichen
        byte t1[7] = {0x55,0xAA,0,1,0,0,0};                             // Abfrage Produkt Information
        byte t2[7] = {0x55,0xAA,0,2,0,0,1};                             // Abfrage Arbeitsmodus
        byte t3[7] = {0x55,0xAA,0,8,0,0,7};                             // Datensynchronisation
        byte t4[8] = {0x55,0xAA,0,3,0,1,3,6};                           // Status 3 (verbunden mit Router)
        byte t5[8] = {0x55,0xAA,0,3,0,1,4,7};                           // Status 4 (verbunden mit der Cloud)
        byte t6[16] = {0x55,0xAA,0,0x34,0,9,2,1,0,0,0,0,0,0,4,0};        // Zeit setzen (ab Byte 9: YY MM DD HH MM SS)
        byte t7[16] = {0x55,0xAA,0,6,0,9,1,0,0,5,1,1,1,1,1,0x19};       // mehr Daten (App aktiv)
        byte* telegramme[8] = {t0,t1,t2,t3,t4,t5,t6,t7};
        byte teleGroesse[8] = {7,7,7,7,8,8,16,16};
        int warteZeiten[6] = {20,20,1400,50,600,13000};
        void senden();
        boolean pruefsumme(byte*, int, byte*, int);
        void decodieren1(byte*);
        void decodieren2(byte*);
};

const int telMD = 7;                // Telegramm mehr Daten im Array auf Position 7

#endif