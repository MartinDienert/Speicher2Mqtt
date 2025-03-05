#include <Arduino.h>
#include "LittleFS.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <Einstellungen.h>
#include <SpeicherLib.h>
//#include <MqttClient.h>

// allgemeine Einstellungen ----------------------------
const char* ssidap = "AP-Speicher";
IPAddress ip(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);
unsigned long jetzt = millis();
unsigned long letztesTel1 = jetzt;
unsigned long letztePub = jetzt;

// Objekte ---------------------------------------------
ESP8266WebServer server(80);
WiFiClient wifiClient;
PubSubClient mqttClient = PubSubClient(wifiClient);
Einstellungen einst = Einstellungen(&server);
Daten daten = Daten();
Speicher speicher = Speicher(&daten);

// Wifi Client -----------------------------------------
void setupWifi(){
  if(einst.wlan){
    WiFi.begin(einst.ssid, einst.pwd);
    int i = 0;
    while(i < 60 && WiFi.status() != WL_CONNECTED){   //warten auf Wlan connect, langsames flackern der LED
      digitalWrite(LED_BUILTIN,HIGH);
      delay(500);
      digitalWrite(LED_BUILTIN,LOW);
      delay(500);
      i++;
    }
  }
}

void setupAP(){
  if(WiFi.status() != WL_CONNECTED){
    WiFi.softAPConfig(ip, gateway, subnet);
    WiFi.softAP(ssidap);
    for(int i = 0; i < 10; i++){                        //Accesspoint aktiviert, 10 mal schnelles flackern der LED
      digitalWrite(LED_BUILTIN,HIGH);
      delay(50);
      digitalWrite(LED_BUILTIN,LOW);
      delay(50);
    }
  }
}

// Webserver -------------------------------------------
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

void setupWS(){
  server.onNotFound(fehlerseite);
  server.on("/", hauptseite);
  server.on("/einst", einstellungsseite);
  server.on("/daten.json", sendeDaten);
  server.on("/einst.json", sendeEinst);
  server.on("/md4", mehrdaten);
  server.on("/.", softreset);
  server.begin();
}

// Mqtt Client
//void callback(char* topic, byte* payload, unsigned int length){
//}

void setupMqtt(){
  mqttClient.setServer(einst.mqttIP.c_str(), 1883); // Mqtt Server Ip
//  mqttClient.setCallback(callback);
}

void reconnectMqtt(){
  while(!mqttClient.connected()){
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if(mqttClient.connect(clientId.c_str())){
//      mqttClient.subscribe("inTopic");
    }else{
      delay(5000);
    }
  }
}

void mqttPub(){
  if(einst.mqtt){
    if(!mqttClient.connected()){
        reconnectMqtt();
    }
    mqttClient.publish(einst.mqttTp.c_str(), daten.json.c_str());
    letztePub = millis();
  }
}

// Arduino
void setup(){
  pinMode(D1, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(D1, LOW);
  Serial.setTimeout(40);
  Serial.begin(115200);
  if(LittleFS.begin()){
    einst.alle_einst_laden();
    setupWifi();
  }
  setupAP();
  setupWS();
  setupMqtt();
  while(Serial.available()){      // Puffer leeren
    Serial.read();
    delay(5);
  }
}

void loop(){
  digitalWrite(LED_BUILTIN,HIGH);
  speicher.run();
  jetzt = millis();
  if(einst.mqtt){
    if(jetzt - letztePub > 30000){
      mqttPub();
    }
  }
  if(einst.mDaten){
    if(jetzt - letztesTel1 > 240000){
      letztesTel1 = jetzt;
      speicher.sendeTel(1);
    }
  }
  server.handleClient();
  digitalWrite(LED_BUILTIN,LOW);
  delay(500);  
}
