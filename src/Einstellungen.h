#ifndef EINSTELLUNGEN_H_
#define EINSTELLUNGEN_H_
#include <Arduino.h>

class Einstellungen  // Class Declaration
{
    public:
        Einstellungen(ESP8266WebServer*);
        boolean master = false;
        boolean mDaten = false;
        boolean wlan = false;
        String ssid = "";
        String pwd = "";
        boolean mqtt = false;
        String mqttIP = "";
        String mqttTp = "";
        String json = "";
        void genJson();
        void alle_einst_laden();
        void setEinst();
    
    private:
        ESP8266WebServer* server;
        void einst_speichern(const char* dateiname, String daten[], int l);
        boolean einst_lesen(const char* dateiname, String daten[], int l);
};

#endif