#include <ESP8266WebServer.h>
#include "LittleFS.h"
#include <Einstellungen.h>

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
    json  = "{\"Master\":" + (String)master + ",\"mDaten\":" + (String)mDaten;
    json += ",\"Wlan\":" + (String)wlan + ",\"SSId\":\"" + ssid + "\",\"PWD\":\"" + pwd + "\"";
    json += ",\"Mqtt\":" + (String)mqtt + ",\"MqttIp\":\"" + mqttIP + "\",\"MqttTp\":\"" + mqttTp + "\"}";
}

void Einstellungen::alle_einst_laden(){
    String tmp[3];
    if(einst_lesen("einst", tmp, 2)){
        master = (tmp[0] == "on")? true: false;
        mDaten = (tmp[1] == "on")? true: false;
    }
    if(einst_lesen("wlan", tmp, 3)){
        wlan = (tmp[0] == "on")? true: false;
        ssid = tmp[1];
        pwd = tmp[2];
    }
    if(einst_lesen("mqtt", tmp, 3)){
        mqtt = (tmp[0] == "on")? true: false;
        mqttIP = tmp[1];
        mqttTp = tmp[2];
    }
    genJson();
}

void Einstellungen::setEinst(){
    if(server->hasArg("save")){
        if(server->arg("save") == "ei"){
            master = (server->hasArg("ma") && server->arg("ma") == "on")? true: false;
            mDaten = (server->hasArg("md") && server->arg("md") == "on")? true: false;
            String daten[] = {(master)? "on": "off", (mDaten)? "on": "off"};
            einst_speichern("einst", daten, 2);
        }else if(server->arg("save") == "wl" && server->hasArg("ssid") && server->hasArg("pwd")){
            wlan = (server->hasArg("wl") && server->arg("wl") == "on")? true: false;
            ssid = server->arg("ssid");
            if(server->arg("pwd") != "" || (ssid == "" && server->arg("pwd") == "")) pwd = server->arg("pwd");
            String daten[] = {(wlan)? "on": "off", ssid, pwd};
            einst_speichern("wlan", daten, 3);
        }else if(server->arg("save") == "mq" && server->hasArg("mqip") && server->hasArg("mqtp")){
            mqtt = (server->hasArg("mq") && server->arg("mq") == "on")? true: false;
            mqttIP = server->arg("mqip");
            mqttTp = server->arg("mqtp");
            String daten[] = {(mqtt)? "on": "off", mqttIP, mqttTp};
            einst_speichern("mqtt", daten, 3);
        }
        genJson();
    }
}
