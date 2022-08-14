/*
 * NTPSync.cpp
 *
 *  Created on: Oct 2, 2021
 *      Author: clovis
 */
#include <Arduino.h>
#include <time.h>
#include "NTPSync.h"
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setupNtp() {
	timeClient.begin();
	int retries = 10;
	while (!timeClient.update()) {
		if (!timeClient.forceUpdate()) {
			if (retries-- == 0)	ESP.restart();
		}
	}
}
int NTPgetYear() {
  time_t rawtime = timeClient.getEpochTime();
  struct tm * ti;
  ti = localtime (&rawtime);
  int year = ti->tm_year + 1900;

  return year;
}

int NTPgetMonth() {
  time_t rawtime = timeClient.getEpochTime();
  struct tm * ti;
  ti = localtime (&rawtime);
  //int month = (ti->tm_mon + 1) < 10 ? 0 + (ti->tm_mon + 1) : (ti->tm_mon + 1);

  return ti->tm_mon + 1;
}

int NTPgetMDay() {
  time_t rawtime = timeClient.getEpochTime();
  struct tm * ti;
  ti = localtime (&rawtime);
  //int month = (ti->tm_mday) < 10 ? 0 + (ti->tm_mday) : (ti->tm_mday);

  return ti->tm_mday;
}

int NTPgetYDay() {
  time_t rawtime = timeClient.getEpochTime();
  struct tm * ti;
  ti = localtime (&rawtime);

  return ti->tm_yday;
}
