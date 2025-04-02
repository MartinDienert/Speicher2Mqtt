#include <Arduino.h>
#include "LittleFS.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <time.h>
#include <arduino-timer.h>
#include <Main.h>
#include <Einstellungen.h>
#include <SpeicherLib.h>

// allgemeine Einstellungen ----------------------------
const char* ssidap = "AP-Speicher";
IPAddress ip(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);

// Objekte ---------------------------------------------
ESP8266WebServer server(80);
WiFiClient wifiClient;
PubSubClient mqttClient = PubSubClient(wifiClient);
//auto timer = timer_create_default();
Timer<> timer;
Einstellungen einst = Einstellungen(&server);
Daten daten = Daten();
Speicher speicher = Speicher(&daten);

// Wifi Client -----------------------------------------
boolean apModus = false;

void setupWifi(){
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
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
    apModus = true;
  }
}

// Webserver -------------------------------------------
void dateiSenden(String dn, String typ = "text/html"){
  File datei = LittleFS.open(dn, "r");
  if(!datei){
    server.send(404, "text/plain", "Datei nicht gefunden.");
    return;
  }
  server.streamFile(datei, typ);
  datei.close();

}

void fehlerseite(){
  server.send(404, "text/plain", "Link wurde nicht gefunden!");
}

void hauptseite(){
  dateiSenden("/index.html");
}

void einstellungsmenue(){
  boolean master_alt = einst.master;
  einst.setEinst();
  if(!master_alt && einst.master){
    // Mastermodus einschalten
  }else if(master_alt && !einst.master){
    // Mastermodus ausschalten
  }
  dateiSenden("/einst.html");
}

void einstellungAll(){
  dateiSenden("/einstAll.html");
}

void einstellungWl(){
  dateiSenden("/einstWl.html");
}

void einstellungMq(){
  dateiSenden("/einstMq.html");
}

void einstellungCSS(){
  dateiSenden("/einst.css", "text/css");
}

void sendeDaten(){
  server.send(200, "application/json", daten.json);
}

void sendeEinst(){
  server.send(200, "application/json", einst.json);
}

void mehrdaten(){
  hauptseite();
  speicher.sendeTel(telMD);
}

void befehle(){
  boolean b = false;
  if(server.hasArg("bef")){
    if(server.arg("bef") == "lad"){
      speicher.sendeTel(telLa, true);
      b = true;
    }else if(server.arg("bef") == "ent"){
      speicher.sendeTel(telEl, true);
      b = true;
    }else if(server.arg("bef") == "spa"){
      speicher.sendeTel(telSa);
      b = true;
    }else if(server.arg("bef") == "laden"){
        speicher.sendeTel(telLa, true);
    }else if(server.arg("bef") == "entladen"){
      speicher.sendeTel(telEl, true);
    }else if(server.arg("bef") == "speicher_aus"){
      speicher.sendeTel(telSa);
    }else if(server.arg("bef") == "laden_aus"){
      speicher.sendeTel(telLa);
    }else if(server.arg("bef") == "laden_ein"){
      speicher.sendeTel(telLa + 1);
    }else if(server.arg("bef") == "entladen_aus"){
      speicher.sendeTel(telEl);
    }else if(server.arg("bef") == "entladen_ein"){
      speicher.sendeTel(telEl + 1);
    }else if(server.arg("bef") == "mehr_daten"){
      speicher.sendeTel(telMD);
    }
  }
  if(b)
    dateiSenden("/index.html");
  else
    server.send(200, "text/html", "Ok");
}

void softreset(){
  server.send(200, "text/html", "Restart_Ok");
  delay(500);
  ESP.restart();
}

void setupWS(){
  server.onNotFound(fehlerseite);
  server.on("/", hauptseite);
  server.on("/einst", einstellungsmenue);
  server.on("/einstAll", einstellungAll);
  server.on("/einstWl", einstellungWl);
  server.on("/einstMq", einstellungMq);
  server.on("/einst.css", einstellungCSS);
  server.on("/daten.json", sendeDaten);
  server.on("/einst.json", sendeEinst);
  server.on("/md4", mehrdaten);
  server.on("/befehle", befehle);
  server.on("/rst", softreset);
  server.begin();
}

// Mqtt Client
void callback(char* topic, byte* payload, unsigned int length){
  if(strcmp(topic, (einst.mqttTp + "/Befehl").c_str()) == 0){
    char pl[length + 1] = {'\0'};
    memcpy(pl, payload, length);
    if(strcmp(pl, "laden") == 0){
      speicher.sendeTel(telLa, true);
    }else if(strcmp(pl, "entladen") == 0){
      speicher.sendeTel(telEl, true);
    }else if(strcmp(pl, "speicher_aus") == 0){
      speicher.sendeTel(telSa);
    }else if(strcmp(pl, "laden_aus") == 0){
      speicher.sendeTel(telLa);
    }else if(strcmp(pl, "laden_ein") == 0){
      speicher.sendeTel(telLa + 1);
    }else if(strcmp(pl, "entladen_aus") == 0){
      speicher.sendeTel(telEl);
    }else if(strcmp(pl, "entladen_ein") == 0){
      speicher.sendeTel(telEl + 1);
    }
  }
}

void setupMqtt(){
  mqttClient.setServer(einst.mqttIp.c_str(), 1883); // Mqtt Server Ip
  mqttClient.setCallback(callback);
  reconnectMqtt();
}

void reconnectMqtt(){
  if(!apModus){
    int i = 0;
    while(i < 2 && !mqttClient.connected()){
      String clientId = "ESP8266Client-";
      clientId += String(random(0xffff), HEX);
      if(mqttClient.connect(clientId.c_str())){
        mqttClient.subscribe((einst.mqttTp + "/Befehl").c_str());
      }else{
        delay(500);
      }
      i++;
    }
  }
}

void mqttPub(){
  if(einst.mqtt && !apModus){
    if(!mqttClient.connected()){
      reconnectMqtt();
    }
    if(mqttClient.connected()){
      mqttClient.publish((einst.mqttTp + "/Daten").c_str(), daten.json.c_str());
    }
  }
}

// NTP Zeitserver
#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03"
tm dat;

void setupNTP(){
  if(!apModus)
    configTime(MY_TZ, einst.ntzIp);
}

void getZeit(){
  if(!apModus){
    time_t now;
    time(&now);
    localtime_r(&now, &dat);
  }
}

String getDatumStr(){
  if(!apModus){
    getZeit();
    char d[11];
    sprintf(d, "%.2d.%.2d.%.4d", dat.tm_mday, dat.tm_mon + 1, dat.tm_year + 1900);
    return (String)d;
  }
  return "";
}

String getZeitStr(){
  if(!apModus){
    char z[9];
    sprintf(z, "%.2d:%.2d:%.2d", dat.tm_hour, dat.tm_min, dat.tm_sec);
    return (String)z;
  }
  return "";
}

// Arduino-Timer
bool mqttPubTimer(void *){
  if(einst.mqtt)
    mqttPub();
  return true;
}

bool mDatenTimer(void *){
  if(einst.mDaten)
    speicher.sendeTel(telMD);
  return true;
}

bool masterTimer(void *){
  speicher.master();
  return true;
}

bool zeitTimer(void *){
  speicher.zeit();
  return true;
}

void setupTimer(){
  timer.every(30000, mqttPubTimer);
  timer.every(240000, mDatenTimer);
  if(einst.master)
    timer.in(5000, masterTimer);
}

// Arduino
void setup(){
  pinMode(D1, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(D1, LOW);
  Serial.setTimeout(40);
  Serial.begin(115200, SERIAL_8N1);
  if(LittleFS.begin()){
    einst.alle_einst_laden();
    setupWifi();
  }
  setupAP();
  setupWS();
  setupMqtt();
  setupNTP();
  setupTimer();
  while(Serial.available()){          // Puffer leeren
    Serial.read();
    delay(5);
  }
}


void loop(){
  digitalWrite(LED_BUILTIN,LOW);      // LED leuchtet
  timer.tick();
  yield();
  speicher.run();
  yield();
  server.handleClient();
  mqttClient.loop();
  digitalWrite(LED_BUILTIN,HIGH);     // LED ist in der Pause aus
  delay(300);  
}
