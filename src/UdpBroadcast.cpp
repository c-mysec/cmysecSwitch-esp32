/*
 * UdpBroadcast.cpp
 *
 *  Created on: 5 de ago de 2020
 *      Author: user
 */
#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#endif
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#endif
#include <WiFiUdp.h>
#include "Messages.h"
#include "UdpBroadcast.h"
#include "KeyManager.h"

unsigned int brodcastPort = 5577;
IPAddress broadcastIp = 0xFFFFFFFF;
WiFiUDP udp;
uint32_t hbNext;
void sendPackage(const uint8_t* packet, size_t packetSize) {
	Serial.print(F("Sending "));
	printBlockLn((uint8_t*)packet, packetSize);
	udp.beginPacket(broadcastIp, brodcastPort);
	udp.write(packet, packetSize);
	udp.endPacket();
}
void sendHeartBeat() {
	IPAddress ip = WiFi.localIP();
	uint8_t packet[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, ip[0], ip[1], ip[2], ip[3]};
	udp.beginPacket(broadcastIp, brodcastPort);
	udp.write(packet, 14);
	udp.endPacket();
	hbNext = millis() + 60000; // 1 minuto
}
uint32_t  calcBroadcastAddress(uint32_t ip, uint32_t subnet) {
	uint32_t broadcastAddress = ip | (~ subnet);
	return broadcastAddress;
}
void udpSetup() {
	Serial.println(F("Starting UDP"));
	udp.begin(brodcastPort);
	IPAddress ip = WiFi.localIP();
	uint32_t lip = ip;
	ip = WiFi.subnetMask();
	uint32_t netmask = ip;
	broadcastIp = calcBroadcastAddress(lip, netmask);
}
void udpLoop() {
	if ((long) (millis() - hbNext) >= 0) {
		//sendHeartBeat();
		hbNext = millis() + 60000; // envia a cada minuto
	}
	int cb = udp.parsePacket();
	if (cb)	{
		int maxMessageSize = 128;
		uint8_t packetBuffer[maxMessageSize];
		// We've received a UDP packet, send it to serial
		memset(packetBuffer, 0, sizeof(maxMessageSize));
		int bytesRead = udp.read(packetBuffer, maxMessageSize);
		if (bytesRead == 14) {
			if (packetBuffer[0] == 0 && packetBuffer[1] == 1 && packetBuffer[2] == 2 && packetBuffer[3] == 3
					 && packetBuffer[4] == 4 && packetBuffer[5] == 5 && packetBuffer[6] == 6 && packetBuffer[7] == 7
					 && packetBuffer[8] == 8 && packetBuffer[9] == 9) {
				// received a heartbeat
				return;
			}
		}
		if (bytesRead > 0) {
			Serial.printf("Received %d bytes", bytesRead);
			printBlockLn((unsigned char *) packetBuffer, bytesRead);
			CryptoClass cc;
			size_t recpacketSize = cc.calcMaxDecryptedSize(bytesRead);
			Serial.printf("recpacketSize %d\n", recpacketSize);
			uint8_t *recpacket = new uint8_t[recpacketSize];
			size_t recpacketSize2 = cc.openEnvelop((const uint8_t*)packetBuffer, bytesRead, recpacket, recpacketSize);
			if (recpacketSize2 <= 0) {
				delete [] recpacket;
				return;
			}
			Serial.printf("recpacketSize2 %d\n", recpacketSize2);

			uint8_t msgType = recpacket[0];


/*#define MSG_STATUSREQ 0
#define MSG_STATUSRES 1
#define MSG_NEOPIXEL_STATUSRES 2

#define MSG_BTNCLICK 3
#define MSG_RELAYSTATECHANGE 4
#define MSG_NEOPIXELSTATECHANGE 5
*/
			switch (msgType) {
				case MSG_STATUSREQ: {
					// send statusres
					MsgStatusRes res;
					buildMsgStatusRes(&res);
					sendPackage((const uint8_t *)&res, sizeof(MsgStatusRes));
					break;
				}
			}
			delete [] recpacket;
			return;
		}
	}
}
