#ifndef MAIN_H_
#define MAIN_H_

#include <SpeicherLib.h>

extern Speicher speicher;
extern tm dat;

void reconnectMqtt();
void getZeit();
void getDatumZeit(Zeit *);
void getDatumZeitStr(char *, char *);

#endif
