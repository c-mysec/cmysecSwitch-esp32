/*
 * NTPSync.h
 *
 *  Created on: Oct 2, 2021
 *      Author: clovis
 */

#ifndef NTPSYNC_H_
#define NTPSYNC_H_
#include <NTPClient.h>
extern NTPClient timeClient;

void setupNtp();
int NTPgetYear();
int NTPgetMonth();
int NTPgetMDay();
int NTPgetYDay();

#endif /* NTPSYNC_H_ */
