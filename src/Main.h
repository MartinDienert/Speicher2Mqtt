#ifndef MAIN_H_
#define MAIN_H_

#include <PubSubClient.h>
#include <arduino-timer.h>

extern PubSubClient mqttClient;
extern Timer timer;

void reconnectMqtt();
void mqttPub();
String getDatumStr();
String getZeitStr();
bool masterTimer(void *);

#endif
