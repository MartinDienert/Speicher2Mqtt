#ifndef MAIN_H_
#define MAIN_H_

#include <PubSubClient.h>

extern PubSubClient mqttClient;

void reconnectMqtt();
void mqttPub();
void getZeitStr(char* d, char* z);

#endif
