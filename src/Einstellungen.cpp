#include <ESP8266WebServer.h>
#include "LittleFS.h"
#include <Einstellungen.h>
#include <Main.h>



Einstellungen::Einstellungen(ESP8266WebServer* s){
  server = s;
  genJson();
}

void Einstellungen::einst_speichern(const char* dateiname, String daten[], int l){
  File datei = LittleFS.open(dateiname, "w");
  if(datei){
    for (int i = 0; i < l; i++){
      datei.print(daten[i]);
      datei.print(";");
    }
    datei.close();
  }
}

boolean Einstellungen::einst_lesen(const char* dateiname, String daten[], int l){
  File datei = LittleFS.open(dateiname, "r");
  if(datei){
    for (int i = 0; i < l; i++){
      daten[i] = datei.readStringUntil(';');
    }
    datei.close();
    return true;
  }
  return false;
}

void Einstellungen::genJson(){
    json  = "{\"Master\":" + (String)master + ",\"mDaten\":" + (String)mDaten + ",\"NtzIp\":\"" + ntzIp + "\"";
    json += ",\"Wlan\":" + (String)wlan + ",\"SSId\":\"" + ssid + "\",\"PWD\":\"" + pwd + "\"";
    json += ",\"Mqtt\":" + (String)mqtt + ",\"MqttIp\":\"" + mqttIp + "\",\"MqttTp\":\"" + mqttTp + "\"}";
}

void Einstellungen::alle_einst_laden(){
    String tmp[3];
    if(einst_lesen("einst", tmp, 3)){
        master = (tmp[0] == "on")? true: false;
        mDaten = (tmp[1] == "on")? true: false;
        ntzIp = tmp[2];
    }
    if(einst_lesen("wlan", tmp, 3)){
        wlan = (tmp[0] == "on")? true: false;
        ssid = tmp[1];
        pwd = tmp[2];
    }
    if(einst_lesen("mqtt", tmp, 3)){
        mqtt = (tmp[0] == "on")? true: false;
        mqttIp = tmp[1];
        mqttTp = tmp[2];
    }
    genJson();
}

void Einstellungen::setEinst(){
    if(server->hasArg("save")){
        if(server->arg("save") == "ei" && server->hasArg("ntip")){
            boolean m = master;
            master = (server->hasArg("ma") && server->arg("ma") == "on")? true: false;
            if(m != master) speicher.setMaster(master);
            mDaten = (server->hasArg("md") && server->arg("md") == "on")? true: false;
            ntzIp = server->arg("ntip");
            String daten[] = {(master)? "on": "off", (mDaten)? "on": "off", ntzIp};
            einst_speichern("einst", daten, 3);
        }else if(server->arg("save") == "wl" && server->hasArg("ssid") && server->hasArg("pwd")){
            wlan = (server->hasArg("wl") && server->arg("wl") == "on")? true: false;
            ssid = server->arg("ssid");
            if(server->arg("pwd") != "" || (ssid == "" && server->arg("pwd") == "")) pwd = server->arg("pwd");
            String daten[] = {(wlan)? "on": "off", ssid, pwd};
            einst_speichern("wlan", daten, 3);
        }else if(server->arg("save") == "mq" && server->hasArg("mqip") && server->hasArg("mqtp")){
            mqtt = (server->hasArg("mq") && server->arg("mq") == "on")? true: false;
            mqttIp = server->arg("mqip");
            mqttTp = server->arg("mqtp");
            String daten[] = {(mqtt)? "on": "off", mqttIp, mqttTp};
            einst_speichern("mqtt", daten, 3);
        }
        genJson();
    }
}
