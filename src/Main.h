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
void getZeit();
String getDatumStr();
String getZeitStr();
bool masterTimer(void *);
bool zeitTimer(void *);

#endif
