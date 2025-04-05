#ifndef MAIN_H_
#define MAIN_H_

#include <time.h>
#include <PubSubClient.h>
#include <arduino-timer.h>
#include <SpeicherLib.h>
#include <Einstellungen.h>

extern Einstellungen einst;
extern Speicher speicher;
extern PubSubClient mqttClient;
extern Timer<> timer;

extern boolean apModus;
extern tm dat;

void reconnectMqtt();
void mqttPub();
boolean getZeit();
String getDatumStr();
String getZeitStr();
char* getDatumCStr();
char* getZeitCStr();
bool masterTimer(void *);
bool zeitTimer(void *);
bool teleTimer(void *);
void addLog(char *);

#endif
