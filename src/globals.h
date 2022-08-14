/*
 * globals.h
 *
 *  Created on: 4 de ago de 2020
 *      Author: user
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_
#include <Arduino.h>

#define MAX_ROUTER_STR 32
#define MAX_NAME_LEN 16
#define SERIAL_BAUDRATE     115200

void printBlock(const uint8_t* block, int length);
void printBlockLn(const uint8_t* block, int length);
void println(const __FlashStringHelper* parname, uint8_t num);
void println(const __FlashStringHelper* parname, uint8_t num1, uint8_t num2);
void println(const __FlashStringHelper* parname, uint8_t num1, uint8_t num2, uint8_t num3);
void println(const __FlashStringHelper* parname, const char* buf);
void println(const __FlashStringHelper* parname, const char* buf1, const char* buf2);
void println(const __FlashStringHelper* parname, const char* buf1, const char* buf2, const char* buf3);
void println(const __FlashStringHelper* parname, const char* buf, uint16_t num);
uint32_t initChipId();

#endif /* GLOBALS_H_ */
