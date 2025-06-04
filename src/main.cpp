#include <Arduino.h>
#include "LittleFS.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <time.h>
#include <Main.h>
#include <Einstellungen.h>
#include <SpeicherLib.h>
#include <ESP8266HTTPUpdateServer.h>

// allgemeine Einstellungen ----------------------------
const char* ssidap = "AP-Speicher";
IPAddress ip(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);

// Objekte ---------------------------------------------
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiClient wifiClient;
PubSubClient mqttClient = PubSubClient(wifiClient);
Speicher speicher = Speicher();
Einstellungen einst = Einstellungen(&server);

// Log -------------------------------------------------
const uint16 ll = 1000;                   // Loglänge
uint16 pos = 0;                           // Position erstes freies Zeichen
char log1[ll + 1] = {'\0'};

void logloeschen(int a){                  // a = benötigter Platz im Puffer, -1 = Puffer löschen, -2 = halben Puffer löschen
  if(a == -1){
      log1[0] = '\0';
      pos = 0;
  }else{
      if(a == -2) a = ll / 2;
      char* x = strstr(log1 + a, "\r\n") + 2;
      log1[0] = '\0';
      strcpy(log1, x);
      pos = strlen(log1);
  }
}

void addLog(const char *lo){
  char datum[36];
  char zeit[36];
  getDatumZeitStr(datum, zeit);
  uint16 ld = strlen(datum);
  uint16 lz = strlen(zeit);
  uint16 le = strlen(lo);
  uint16 lg = le + ld + lz + 4;
  if(pos + lg > ll)
    logloeschen(-2);                  // löscht die Hälfte des Puffers
  if(pos + lg < ll + 1){
    strcat(log1, datum);
    strcat(log1, " ");
    strcat(log1, zeit);
    strcat(log1, " ");
    strcat(log1, lo);
    strcat(log1, "\r\n");
    pos += lg;
  }
}

char* getLog(){
  return log1;
}

// Json -----------------------------------------
char json[200] = {'\0'};

void generiereJson(Daten daten){
  char datum[36];
  char zeit[36];
  getDatumZeitStr(datum, zeit);
  sprintf(json, "{\"Spannung\":%.1f,\"Ladezustand\":%d,\"StromAkku\":%.1f,\"Typ\":%d",
      daten.spannung, daten.soc, daten.stromakku, daten.typ);
  char s[150] = {'\0'};
  if(daten.typ == 2){
      sprintf(s, ",\"StromPV\":%.1f,\"Temperatur\":%d", daten.strompv, daten.temperatur);
  }else{
      strcat(s, ",\"StromPV\":0,\"Temperatur\":0");
  }
  strcat(json, s);
  sprintf(s, ",\"Datum\":\"%s\",\"Zeit\":\"%s\",\"Laden\":%s,\"Entladen\":%s",
      datum, zeit, (daten.laden)? "\"ein\"": "\"aus\"", (daten.entladen)? "\"ein\"": "\"aus\"");
  strcat(json, s);
  strcat(json, "}");
}

char* getJson(){
  return json;
}

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
void dateiSenden(const char *dn, const char *typ = "text/html"){
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
  einst.setEinst();
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

void einstellungOt(){
  dateiSenden("/einstOt.html");
}

void einstellungCSS(){
  dateiSenden("/einst.css", "text/css");
}

void sendeDaten(){
  server.send(200, "application/json", getJson());
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
    if(server.arg("bef").equals("lad")){
      speicher.sendeTel(telLa, true);
      b = true;
    }else if(server.arg("bef").equals("ent")){
      speicher.sendeTel(telEl, true);
      b = true;
    }else if(server.arg("bef").equals("spa")){
      speicher.sendeTel(telSa);
      b = true;
    }else if(server.arg("bef").equals("laden")){
        speicher.sendeTel(telLa, true);
    }else if(server.arg("bef").equals("entladen")){
      speicher.sendeTel(telEl, true);
    }else if(server.arg("bef").equals("speicher_aus")){
      speicher.sendeTel(telSa);
    }else if(server.arg("bef").equals("laden_aus")){
      speicher.sendeTel(telLa);
    }else if(server.arg("bef").equals("laden_ein")){
      speicher.sendeTel(telLa + 1);
    }else if(server.arg("bef").equals("entladen_aus")){
      speicher.sendeTel(telEl);
    }else if(server.arg("bef").equals("entladen_ein")){
      speicher.sendeTel(telEl + 1);
    }else if(server.arg("bef").equals("mehr_daten")){
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

void logdaten(){
  server.send(200, "text/plain", getLog());
}

void setupWS(){
  server.onNotFound(fehlerseite);
  server.on("/", hauptseite);
  server.on("/einst", einstellungsmenue);
  server.on("/einstAll", einstellungAll);
  server.on("/einstWl", einstellungWl);
  server.on("/einstMq", einstellungMq);
  server.on("/einstOt", einstellungOt);
  server.on("/einst.css", einstellungCSS);
  server.on("/daten.json", sendeDaten);
  server.on("/einst.json", sendeEinst);
  server.on("/md4", mehrdaten);
  server.on("/befehle", befehle);
  server.on("/rst", softreset);
  server.on("/log", logdaten);
  server.begin();
}

// Mqtt Client -------------------------------------------
void callback(char* topic, byte* payload, unsigned int length){
  int i = strlen(einst.mqttTp.c_str());
  if(strncmp(topic, einst.mqttTp.c_str(), i) == 0 && strcmp(topic + i, "/Befehl") == 0){
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
  mqttClient.setCallback(callback);
  reconnectMqtt();
}

void reconnectMqtt(){
  if(!apModus){
    mqttClient.setServer(einst.mqttIp.c_str(), einst.mqttPo.toInt());       // Mqtt Server Ip, Port
    int i = 0;
    while(i < 2 && !mqttClient.connected()){
      char id[19] = "ESP8266Client-";
      ltoa(random(0xffff), id + 14, HEX);
      boolean c = false;
      if(einst.mqttBe != "" && einst.mqttPw != "")
        c = mqttClient.connect(id, einst.mqttBe.c_str(), einst.mqttPw.c_str());
      else
        c = mqttClient.connect(id);
      if(c){
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
      mqttClient.publish((einst.mqttTp + "/Daten").c_str(), getJson());
    }
  }
}

// NTP Zeitserver -------------------------------------------
#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03"
tm dat;

void setupNTP(){
  if(!apModus){
    configTime(MY_TZ, einst.ntzIp);
    speicher.callbackGetDatumZeit(getDatumZeit);
  }
}

void getZeit(){
  if(!apModus){
    time_t now;
    time(&now);
    localtime_r(&now, &dat);
  }
}

void getDatumZeit(Zeit *z){
  getZeit();
  z->jahr     = dat.tm_year - 100;
  z->monat    = dat.tm_mon + 1;
  z->tag      = dat.tm_mday;
  z->stunde   = dat.tm_hour;
  z->minute   = dat.tm_min;
  z->sekunde  = dat.tm_sec;
  z->tagWoche = dat.tm_wday;
}

void getDatumZeitStr(char *datum, char *zeit){
  getZeit();
  sprintf(datum, "%02d.%02d.%04d", dat.tm_mday, dat.tm_mon + 1, dat.tm_year + 1900);
  sprintf(zeit, "%02d:%02d:%02d", dat.tm_hour, dat.tm_min, dat.tm_sec);
}

// Timer -------------------------------------------
unsigned long mqttPubZeit = 0;
const unsigned long mqttPubInterval = 30000;              // 30 Sekunden
unsigned long mDatenZeit = 0;
const unsigned long mDatenInterval = 240000;              // 4 Minuten
unsigned long testZeit = 0;
unsigned long testInterval = 30000;
boolean testB = false;
int testI = 0;

void timerRun(){
  unsigned long zeit = millis();
  if(mqttPubZeit > 0 && zeit - mqttPubZeit > mqttPubInterval){
    mqttPubZeit = zeit;
    if(mqttPubZeit == 0) mqttPubZeit = 1;
    if(einst.mqtt)
      mqttPub();
  }
  if(testZeit > 0 && zeit - testZeit > testInterval){
    testZeit = zeit;
    if(testZeit == 0) testZeit = 1;
    char t[] = "SpeicherM02/Befehl";
    byte p[5] = {'l','a','d','e','n'};
    callback(t, p, 5);
  }
}

void setMqttPubTimer(){
  mqttPubZeit = millis();
  if(mqttPubZeit == 0) mqttPubZeit = 1;
}

void setTestTimer(){
  testZeit = millis();
  if(testZeit == 0) testZeit = 1;
}

void setupTimer(){
  setMqttPubTimer();
//  setTestTimer();
}

// Speicher -------------------------------------------
int lesen(byte *b, int l){
  return Serial.readBytes(b, l);
}

void schreiben(byte *b, int l){
  digitalWrite(D1, HIGH);
  delay(10);
  Serial.write(b, l);
  delay(5);
  digitalWrite(D1, LOW);
}

void neueDaten(){
  generiereJson(speicher.getDaten());
  mqttPub();
}

void logEintrag(const char *s){
  addLog(s);
}

void setupSpeicher(){
  speicher.callbackLesen(lesen);
  speicher.callbackSchreiben(schreiben);
  speicher.callbackNeueDaten(neueDaten);
  speicher.callbackLogeintrag(logEintrag);
  if(einst.master) speicher.startMaster(5000);
  if(einst.mDaten) speicher.startMDaten(30000);
  generiereJson(speicher.getDaten());
}

// OTA Update ----------------------------------------
void setupOta(){
  httpUpdater.setup(&server);
}

// Arduino -------------------------------------------
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
  setupSpeicher();
  setupOta();
  while(Serial.available()){          // Puffer leeren
    Serial.read();
    delay(5);
  }
}


void loop(){
  digitalWrite(LED_BUILTIN,LOW);      // LED leuchtet
  timerRun();
  yield();
  speicher.run();
  yield();
  server.handleClient();
  yield();
  mqttClient.loop();
  yield();
  digitalWrite(LED_BUILTIN,HIGH);     // LED ist in der Pause aus
  delay(300);  
}
