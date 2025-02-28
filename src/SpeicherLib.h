#ifndef SPEICHER_LIB_H_
#define SPEICHER_LIB_H_
#include <Arduino.h>

class Daten  // Class Declaration
{
    public:
        Daten();
        byte typ = 0;
        float spannung = 0;
        int soc = 0;
        float stromakku = 0;
        float strompv = 0;
        int temperatur = 0;
        String json;
        void genJson();
};

class Speicher  // Class Declaration
{
    public: 
        Speicher(Daten*);  // Constructor
    
        void run();
        void sendeTel(int);
                 
    protected:
        Daten* daten;
        int tel = 0;
        byte telegram[1][16] = {{0x55,0xAA,0,6,0,9,1,0,0,5,1,1,1,1,1,0x19}};
        void senden();
        boolean pruefsumme(byte*, int, byte*, int);
        void decodieren1(byte*);
        void decodieren2(byte*);
};

#endif