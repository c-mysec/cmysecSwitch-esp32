/*
 * globals.cpp
 *
 *  Created on: 4 de ago de 2020
 *      Author: user
 */

#include "globals.h"

void printHex(uint32_t num) {
	uint8_t byte = (num >> 24) && 0xff;
	if (byte < 16) Serial.print('0');
	Serial.print(byte, HEX);
	byte = (num >> 16) && 0xff;
	if (byte < 16) Serial.print('0');
	Serial.print(byte, HEX);
	byte = (num >> 8) && 0xff;
	if (byte < 16) Serial.print('0');
	Serial.print(byte, HEX);
	byte = num && 0xff;
	if (byte < 16) Serial.print('0');
	Serial.print(num, HEX);
}
// prints given block of given length in HEX
void printBlock(const uint8_t* block, int length) {
  Serial.print(" { ");
  for (int i=0; i<length; i++) {
		if (block[i] < 16) Serial.print('0');
		Serial.print(block[i], HEX);
  }
  Serial.print("}");
}
void printBlockLn(const uint8_t* block, int length) {
  printBlock(block, length);
  Serial.println();
}

void println(const __FlashStringHelper* parname, const char* buf) {
	Serial.print(parname);
	Serial.println(buf);
}
void println(const __FlashStringHelper* parname, uint8_t num) {
	Serial.print(parname);
	Serial.println(num);
}
void println(const __FlashStringHelper* parname, uint8_t num1, uint8_t num2) {
	Serial.print(parname);
	Serial.print(num1);
	Serial.print(',');
	Serial.println(num2);
}
void println(const __FlashStringHelper* parname, uint8_t num1, uint8_t num2, uint8_t num3) {
	Serial.print(parname);
	Serial.print(num1);
	Serial.print(',');
	Serial.print(num2);
	Serial.print(',');
	Serial.println(num3);
}
void println(const __FlashStringHelper* parname, const char* buf, uint16_t num) {
	Serial.print(parname);
	Serial.print(buf);
	Serial.print(',');
	Serial.println(num);
}
void println(const __FlashStringHelper* parname, const char* buf1, const char* buf2) {
	Serial.print(parname);
	Serial.print(buf1);
	Serial.print(',');
	Serial.println(buf2);
}
void println(const __FlashStringHelper* parname, const char* buf1, const char* buf2, const char* buf3) {
	Serial.print(parname);
	Serial.print(buf1);
	Serial.print(',');
	Serial.print(buf2);
	Serial.print(',');
	Serial.println(buf3);
}
uint64_t macAddress;
uint32_t chipID;
uint32_t initChipId() {
	macAddress = ESP.getEfuseMac();
	uint64_t macAddressTrunc = macAddress << 40;
	chipID = macAddressTrunc >> 40;
	return chipID;
}
