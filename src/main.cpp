#include <Arduino.h>
#include "LittleFS.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <Einstellungen.h>
#include <SpeicherLib.h>


ESP8266WebServer server(80);
const char* ssidap = "AP-Speicher";
IPAddress ip(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);

Einstellungen einst = Einstellungen(&server);
Daten daten = Daten();
Speicher speicher = Speicher(&daten);

void fehlerseite(){
  server.send(404, "text/plain", "Link wurde nicht gefunden!");
}

void hauptseite(){
  File datei = LittleFS.open("/index.html", "r");
  if(!datei){
    server.send(404, "text/plain", "Datei nicht gefunden.");
    return;
  }
  server.streamFile(datei, "text/html");
  datei.close();
}

void einstellungsseite(){
  einst.setEinst();
  File datei = LittleFS.open("/einst.html", "r");
  if(!datei){
    server.send(404, "text/plain", "Datei nicht gefunden.");
    return;
  }
  server.streamFile(datei, "text/html");
  datei.close();
}

void sendeDaten(){
  server.send(200, "application/json", daten.json);
}

void sendeEinst(){
  server.send(200, "application/json", einst.json);
}

void mehrdaten(){
  hauptseite();
  speicher.sendeTel(1);
}

void softreset(){
  server.send(200, "text/html", "Restart_Ok");
  delay(500);
  ESP.restart();
}

void setup(){
  pinMode(D1, OUTPUT);
  digitalWrite(D1, LOW);
  Serial.setTimeout(40);
  Serial.begin(115200);
  if(LittleFS.begin()){
    einst.alle_einst_laden();
    if(einst.wlan){
      WiFi.begin(einst.ssid, einst.pwd);
      int i = 0;
      while(i < 60 && WiFi.status() != WL_CONNECTED){
        delay(500);
        i++;
      }
    }
  }
  if(WiFi.status() != WL_CONNECTED){
    WiFi.softAPConfig(ip, gateway, subnet);
    WiFi.softAP(ssidap);
    delay(500);
  }

  server.onNotFound(fehlerseite);
  server.on("/", hauptseite);
  server.on("/einst", einstellungsseite);
  server.on("/daten.json", sendeDaten);
  server.on("/einst.json", sendeEinst);
  server.on("/md4", mehrdaten);
  server.on("/.", softreset);
  server.begin();

  while(Serial.available()){      // Puffer leeren
    Serial.read();
    delay(5);
  }
}

void loop(){
  speicher.run();
  server.handleClient();  
}
