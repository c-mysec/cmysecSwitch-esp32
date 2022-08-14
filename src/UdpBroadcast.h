/*
 * UdpBroadcast.h
 *
 *  Created on: 5 de ago de 2020
 *      Author: user
 */

#ifndef UDPBROADCAST_H_
#define UDPBROADCAST_H_
#include "globals.h"

void udpSetup();
void udpLoop();
void sendPackage(const uint8_t* packet, size_t packetSize);
#endif /* UDPBROADCAST_H_ */
