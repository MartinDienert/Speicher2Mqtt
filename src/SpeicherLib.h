#ifndef SPEICHER_LIB_H_
#define SPEICHER_LIB_H_
#include <Arduino.h>

const int telZe = 6;                    // Telegramm Zeit, im Array auf Position 6
const int telMD = 7;                    // Telegramm mehr Daten, im Array auf Position 7
const int telLa = 8;                    // Telegramm Laden aus
const int telEl = 10;                   // Telegramm Entladen aus
const int telSa = 12;                   // Telegramm Speicher aus
const long totman_Interval = 330000;    // Totmanintervall in Millisekunden, (05:30 Minuten)
const long mDaten_Interval = 240000;    // Telegrammintervall für mehr Daten, (4 Minuten)

struct Daten{
    byte typ = 0;
    double spannung = 0;
    int soc = 0;
    double stromakku = 0;
    double strompv = 0;
    int temperatur = 0;
    boolean laden = true;
    boolean entladen = true;
};

struct Zeit{
    byte jahr = 0;                  // Jahr, 0-99
    byte monat = 0;                 // Monat, 1-12
    byte tag = 0;                   // Tag, 1-31
    byte stunde = 0;                // Stunde, 0-59
    byte minute = 0;                // Minute, 0-59
    byte sekunde = 0;               // Sekunde, 0-59
    byte tagWoche = 0;              // Tag der Woche, 1-7
};

class Speicher{                     // Class Declaration
    protected:
        using Lesen = int (*)(byte *, int);
        Lesen lesen = nullptr;
        using Schreiben = void (*)(byte *, int);
        Schreiben schreiben = nullptr;
        using NeueDaten = void (*)();
        NeueDaten neueDaten = nullptr;
        using GetDatumZeit = void (*)(Zeit *);
        GetDatumZeit getDatumZeit = nullptr;
        using Logeintrag = void (*)(const char *);
        Logeintrag logeintrag = nullptr;

    public:
        Speicher();  // Constructor
        void callbackLesen(Lesen);
        void callbackSchreiben(Schreiben);
        void callbackNeueDaten(NeueDaten);
        void callbackGetDatumZeit(GetDatumZeit);
        void callbackLogeintrag(Logeintrag);
        void run();
        void sendeTel(int, boolean);
        void sendeTel(int);
        void master();
        void sendeZeit();
        void startMaster(unsigned long);
        void setMaster(boolean);
        void startMDaten(unsigned long);
        void setMDaten(boolean);
        Daten getDaten();

    private:
        Daten daten;
        Zeit zeit;
        byte bp[6];
        byte bp2[200];
        int tele = -1;
        int startTele = 0;
        byte t0[7] = {0x55,0xAA,0,0,0,0,0xFF};                          // Lebenszeichen
        byte t1[7] = {0x55,0xAA,0,1,0,0,0};                             // Abfrage Produkt Information
        byte t2[7] = {0x55,0xAA,0,2,0,0,1};                             // Abfrage Arbeitsmodus
        byte t3[7] = {0x55,0xAA,0,8,0,0,7};                             // Datensynchronisation
        byte t4[8] = {0x55,0xAA,0,3,0,1,3,6};                           // Status 3 (verbunden mit Router)
        byte t5[8] = {0x55,0xAA,0,3,0,1,4,7};                           // Status 4 (verbunden mit der Cloud)
        byte t6[16] = {0x55,0xAA,0,0x34,0,9,2,1,0,0,0,0,0,0,0,0};       // Zeit setzen (ab Byte 9: YY MM DD HH MM SS DW)
        byte t7[16] = {0x55,0xAA,0,6,0,9,1,0,0,5,1,1,1,1,1,0x19};       // mehr Daten (App aktiv)
        byte t8[12] = {0x55,0xAA,0,6,0,5,0x65,1,0,1,0,0x71};            // Laden aus
        byte t9[12] = {0x55,0xAA,0,6,0,5,0x65,1,0,1,1,0x72};            // Laden ein
        byte t10[12] = {0x55,0xAA,0,6,0,5,0x66,1,0,1,0,0x72};           // Entladen aus
        byte t11[12] = {0x55,0xAA,0,6,0,5,0x66,1,0,1,1,0x73};           // Entladen ein
        byte t12[12] = {0x55,0xAA,0,6,0,5,0x67,0,0,1,0,0x72};           // Speicher aus
        byte* telegramme[13] = {t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12};
        byte teleGroesse[13] = {7,7,7,7,8,8,16,16,12,12,12,12,12};
        const int warteZeiten[7] = {40,40,1400,50,600,13000,15000};
        void empfangen();
        void senden();
        byte pruefsummeBer(byte*, int);
        byte pruefsummeBer(byte*, int, byte*, int);
        boolean pruefsumme(byte*, int, byte*, int);
        void decodieren1(byte*);
        void decodieren2(byte*);
        void decodieren3(byte*);
        // ------ Timer -------
        unsigned long masterZeit = 0;
        unsigned long masterInterval;
        unsigned long mDatenZeit = 0;
        unsigned long mDatenInterval;
        unsigned long zeitTelZeit = 0;
        unsigned long zeitTelInterval;
        unsigned long teleZeit = 0;
        unsigned long teleInterval;
        int teleParam;
        unsigned long totmanZeit = 0;
        unsigned long totmanInterval = totman_Interval;
        void timerRun();
        void setMasterTimer(unsigned long);
        void setMDatenTimer(unsigned long);
        void setZeitTelTimer(unsigned long);
        void setTeleTimer(unsigned long, int);
        void totmanReset();    
};

#endif
