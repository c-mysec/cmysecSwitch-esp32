/*
 * globals.cpp
 *
 *  Created on: 31 de jul de 2020
 *      Author: user
 */

#include <Arduino.h>
#include <FS.h>
#ifdef ARDUINO_ARCH_ESP32
#include "SPIFFS.h"
#endif
#ifdef ARDUINO_ARCH_ESP8266
#endif

#include "mySwitchConfig.h"

String webBuffer1;
String webBuffer2;
bool writeInWebBuffer1 = true;
long serialBuf = 0;

ConfigValues configValues;
CloudValues cloudValues;
ConfigButtonValues configButtonValues[MAX_BUTTONS];
ConfigActuatorValues configActuatorValues[MAX_ACTUATORS];


bool loadAllConfigs() {
	loadButtonActuatorsConfig();
	if (!loadConfig()) return false;
	loadCloudConfig(); // se retornar false n�o tem cloud mas tudo funciona normal.
	return true;
}
const char _CLOUDDAT[] PROGMEM = "/data/cloud.dat";
const char _PASSWORDCONFIGDAT[] PROGMEM = "/pconfig.dat";
const char _PASSWORDCONFIGDAT2[] PROGMEM = "config.dat";
const char _CONFIGDAT[] PROGMEM = "/data/config.dat";
const char _CONFIG_BUTTONS[] PROGMEM = "/data/ConfigButton.dat";
const char _CONFIG_ATUADORES[] PROGMEM = "/data/ConfigAtuador.dat";

const char _CLOUDDAT1[] PROGMEM = "/cloud.dat";
const char _PASSWORDCONFIGDAT1[] PROGMEM = "/pconfig.dat";
const char _CONFIGDAT1[] PROGMEM = "/config.dat";
const char _CONFIG_BUTTONS1[] PROGMEM = "/ConfigButton.dat";
const char _CONFIG_ATUADORES1[] PROGMEM = "/ConfigAtuador.dat";

const char _FILEOPENFAILED[] PROGMEM = "file open failed";
const char _FILEREADFAILED[] PROGMEM = "file read failed";
const char _FILEREAD[] PROGMEM = "file read";
const char * _FILE_READ = FILE_READ;
const char * _FILE_WRITE = FILE_WRITE;
void savePasswordConfig(const char * nodeName, const char * userName, const char * password) {
	File f1 = SPIFFS.open(FPSTR(_PASSWORDCONFIGDAT), _FILE_WRITE);
	memset(&configValues, 0, sizeof(ConfigValues));
	configValues.version = 1;
	strncpy(configValues.nodeName, nodeName, MAX_NAME_LEN);
	strncpy(configValues.userName, userName, MAX_NAME_LEN);
	strncpy(configValues.passwd, password, MAX_NAME_LEN);
	f1.write((const uint8_t *)&configValues, sizeof(ConfigValues));
	f1.close();
}
bool loadFile(const String &path, void * buffer, size_t size) {
	File f1 = SPIFFS.open(path, _FILE_READ);
	memset(buffer, 0, size);
	Serial.print(path);
	if (!f1) {
		Serial.println(FPSTR(_FILEOPENFAILED));
		return false;
	}
	Serial.println(f1.available());
	if (f1.available() < size) return false;
	Serial.println(FPSTR(_FILEREAD));
	uint8_t ver[4];
	if (f1.available() == size+4) {
		f1.readBytes((char*)ver, 4);
	}
	// ver[0] == 1
	f1.readBytes((char*)buffer, size);
	return true;
}
bool saveFile(const String &path, void * buffer, size_t size) {
	File f1 = SPIFFS.open(path, _FILE_WRITE);
	Serial.print(path);
	if (!f1) {
		Serial.println(FPSTR(_FILEOPENFAILED));
		return false;
	}
	uint8_t ver[4];
	ver[0] = 1; ver[1] = ver[2] = ver[3] = 0;
	f1.write(ver, 4);
	f1.write((uint8_t*)buffer, size);
	return true;
}
bool loadConfig() {
	bool res = true;
	if (!loadFile(FPSTR(_PASSWORDCONFIGDAT), &configValues, sizeof(ConfigValues))) {
		strcpy(configValues.passwd, "12345678");
		strcpy(configValues.nodeName, "ESP32");
		strcpy(configValues.userName, "admin");
	}
	println(F("Node name: "), configValues.nodeName);
	println(F("Node User name: "), configValues.userName);
	println(F("Node passwd: "), configValues.passwd);
	return res;
}
bool saveCloudConfig() {
	return saveFile(FPSTR(_CLOUDDAT), &cloudValues, sizeof(cloudValues));	
}
bool loadCloudConfig() {
	if (!loadFile(FPSTR(_CLOUDDAT), &cloudValues, sizeof(cloudValues))) {
		cloudValues.userName[0] = 0;
		cloudValues.passwd[0] = 0;
		cloudValues.endpoint[0] = 0; // a2g89lg5mw80c9-ats.iot.us-east-1.amazonaws.com
		cloudValues.iotOutTopic[0] = 0; // $aws/rules/MySwitchStatusChangeMySwitchStatusChangeIoTRule_uxqN0kOhCUf9
		cloudValues.email[0] = 0;
		// this is a slave
		Serial.println(F("cloud not loaded"));
		return false;
	}
	// this is a master
	println(F("uname: "), cloudValues.userName);
	println(F("pass: "), cloudValues.passwd);
	println(F("endpoint: "), cloudValues.endpoint);
	println(F("iotOutTopic: "), cloudValues.iotOutTopic);
	println(F("email: "), cloudValues.email);
	return true;
}
bool loadButtonActuatorsConfig() {
	if (!loadFile(FPSTR(_CONFIG_BUTTONS), configButtonValues, sizeof(configButtonValues))) {
		return false;
	}
	if (!loadFile(FPSTR(_CONFIG_ATUADORES), configActuatorValues, sizeof(configActuatorValues))) {
		return false;
	}
	return true;
}
void log(const __FlashStringHelper* buffer) {
	if (writeInWebBuffer1) {
		if (webBuffer1.length() < __MAX_WEBBUF_LEN - 100) {
			webBuffer1.concat(buffer);
		} else {
			writeInWebBuffer1 = false;
			webBuffer2.clear();
			webBuffer2.concat(serialBuf);
			webBuffer2.concat('\n');
			webBuffer2.concat(buffer);
		}
	} else {
		if (webBuffer2.length() < __MAX_WEBBUF_LEN - 100) {
			webBuffer2.concat(buffer);
		} else {
			writeInWebBuffer1 = true;
			webBuffer1.clear();
			webBuffer1.concat(serialBuf);
			webBuffer1.concat('\n');
			webBuffer1.concat(buffer);
		}
	}
}
void log(const char * buffer) {
	if (writeInWebBuffer1) {
		if (webBuffer1.length() + strlen(buffer) < __MAX_WEBBUF_LEN - 5) {
			webBuffer1.concat(buffer);
		} else {
			writeInWebBuffer1 = false;
			webBuffer2.clear();
			webBuffer2.concat(buffer);
		}
	} else {
		if (webBuffer2.length() + strlen(buffer) < __MAX_WEBBUF_LEN - 5) {
			webBuffer2.concat(buffer);
		} else {
			writeInWebBuffer1 = true;
			webBuffer1.clear();
			webBuffer1.concat(buffer);
		}
	}
}
void log(String& buffer) {
	if (writeInWebBuffer1) {
		if (webBuffer1.length() + buffer.length() < __MAX_WEBBUF_LEN - 5) {
			webBuffer1.concat(buffer);
		} else {
			writeInWebBuffer1 = false;
			webBuffer2.clear();
			webBuffer2.concat(buffer);
		}
	} else {
		if (webBuffer2.length() + buffer.length() < __MAX_WEBBUF_LEN - 5) {
			webBuffer2.concat(buffer);
		} else {
			writeInWebBuffer1 = true;
			webBuffer1.clear();
			webBuffer1.concat(buffer);
		}
	}
}
void log(char c) {
	if (writeInWebBuffer1) {
		webBuffer1.concat(c);
	} else {
		webBuffer2.concat(c);
	}
}
void log(uint8_t c) {
	if (writeInWebBuffer1) {
		webBuffer1.concat(c);
	} else {
		webBuffer2.concat(c);
	}
}
void log(uint16_t c) {
	if (writeInWebBuffer1) {
		webBuffer1.concat(c);
	} else {
		webBuffer2.concat(c);
	}
}
void logln(uint16_t c) {
	log(c);
	log('\n');
}
void log(uint32_t c) {
	if (writeInWebBuffer1) {
		webBuffer1.concat(c);
	} else {
		webBuffer2.concat(c);
	}
}
void logln(const __FlashStringHelper* buf) {
	log(buf);
	log('\n');
}
void logln(const char* buf) {
	log(buf);
	log('\n');
}
void logln(String& buf) {
	log(buf);
	log('\n');
}
void log(const __FlashStringHelper* buf, uint8_t num) {
	log(buf);
	log(' ');
	log(num);
}
void logln(const __FlashStringHelper* buf, uint8_t num) {
	log(buf, num);
	log('\n');
}
void logln(const __FlashStringHelper* buf, uint16_t num) {
	log(buf);
	log(' ');
	log(num);
	log('\n');
}
void logln(const __FlashStringHelper* buf, uint32_t num) {
	log(buf);
	log(' ');
	log(num);
	log('\n');
}
void logln(const __FlashStringHelper* buf, uint8_t num1, uint8_t num2) {
	log(buf);
	log(' ');
	log(num1);
	log(' ');
	log(num2);
	log('\n');
}
void logln(const __FlashStringHelper* buf, uint8_t num1, uint8_t num2, bool b) {
	log(buf);
	log(' ');
	log(num1);
	log(' ');
	log(num2);
	log(' ');
	log(b ? F("true") : F("false"));
	log('\n');
}
void logln(const __FlashStringHelper* buf, uint32_t num1, uint8_t num2) {
	log(buf);
	log(' ');
	log(num1);
	log(' ');
	log(num2);
	log('\n');
}
void log(const __FlashStringHelper* buf, uint32_t num1, uint8_t num2, uint8_t num3) {
	log(buf);
	log(' ');
	log(num1);
	log(' ');
	log(num2);
	log(' ');
	log(num3);
}
void logln(const __FlashStringHelper* buf, uint32_t num1, uint8_t num2, uint8_t num3) {
	log(buf, num1, num2, num3);
	log('\n');
}