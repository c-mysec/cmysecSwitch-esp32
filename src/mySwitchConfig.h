/*
 * globals.h
 *
 *  Created on: 31 de jul de 2020
 *      Author: user
 *
 *      https://c-mysec.github.io/
 *      myswitch/home/data
 *      myswitch/home/control
 *
{
      	"username": "home",
      	"relay":"niche",
      	"state":0
}
 */
#ifndef CONFIG_H_
#define CONFIG_H_
#include <time.h>
#include "globals.h"

#define MAX_ACTUATORS 64
#define MAX_RF_ACTUATOR_CODES 8
#define MAX_BUTTONS 128
#define MAX_BUTTON_ACTUATORS 16
#define TYPE_RF 1
#define TYPE_RF_TOGGLE 2
#define TYPE_UDP 5
#define NOT_CONFIGURED 255
#define NEXT_STATE 254

extern const char _CLOUDDAT[];
extern const char _PASSWORDCONFIGDAT[];
extern const char _CONFIGDAT[];
extern const char _CONFIG_BUTTONS[];
extern const char _CONFIG_ATUADORES[];

extern const char _CLOUDDAT1[];
extern const char _PASSWORDCONFIGDAT1[];
extern const char _PASSWORDCONFIGDAT2[];
extern const char _CONFIGDAT1[];
extern const char _CONFIG_BUTTONS1[];
extern const char _CONFIG_ATUADORES1[];

typedef struct ConfigValues {
	uint32_t version;
    char nodeName[MAX_NAME_LEN];
	char userName[MAX_NAME_LEN];
	char passwd[MAX_NAME_LEN];
} ConfigValues;
typedef struct CloudValues {
	uint8_t config[4]; // 1 0 0 0 => this node is the main, else, this is a slave
	char userName[MAX_ROUTER_STR];
	char passwd[MAX_ROUTER_STR];
	char endpoint[MAX_ROUTER_STR * 4];
	char iotOutTopic[MAX_ROUTER_STR * 4];
	char email[MAX_ROUTER_STR * 2];
} CloudValues;
typedef struct ConfigRfButtonValues { // 8
	uint32_t code; // 0 - não configurado
	uint8_t protocol;
	uint8_t bitlen;
	uint8_t filler1;
	uint8_t filler2;
} ConfigRfButtonValues;
typedef struct ConfigUdpButtonValues {
	uint8_t device;
	uint8_t button;
	uint8_t filler1;
	uint8_t filler2;
} ConfigUdpButtonValues;
typedef struct ConfigButtonValues { // 44
	uint8_t type; // 1 - RF, 2 - Udp
	uint8_t filler1;
	uint8_t filler2;
	uint8_t filler3;
    char buttonName[MAX_NAME_LEN];
	uint8_t actuator[MAX_BUTTON_ACTUATORS]; // # sequencia do atuador - NOT_CONFIGURED: inexistente
	uint8_t valor[MAX_BUTTON_ACTUATORS]; // estado atuador. Se valor == NOT_CONFIGURED, não altera o estado,
										// RF pode ter os comandos: 0: off, 1: on, 2: toggle
	uint8_t flags[MAX_BUTTON_ACTUATORS]; // bit 0: 0=propaga, 1=não propaga
	union {
		struct ConfigRfButtonValues rf; // 8
		struct ConfigUdpButtonValues udp; // 4
	} data; // access with some_info_object.data.a or some_info_object.data.b
} ConfigButtonValues;
typedef struct ConfigRfActuatorValues { //36
	uint32_t code[MAX_RF_ACTUATOR_CODES]; // 0 - não configurado
	uint8_t protocol;
	uint8_t bitlen;
	uint8_t estadoAtual; // estados 0 a 7 enviam code, estados reservados: 255: prox 254: não muda
	uint8_t numStates;
} ConfigRfActuatorValues;
typedef struct ConfigUdpActuatorValues { // 24
    char nodeName[MAX_NAME_LEN];
	uint32_t ip;
	uint8_t estadoAtual; // estados 0 a 7 enviam code, estados reservados: 255: prox 254: não muda
	uint8_t numStates;
    uint8_t filler2;
    uint8_t filler3;
} ConfigUdpActuatorValues;
typedef struct ConfigUdpNeoPixelActuatorValues { // 32
    char nodeName[MAX_NAME_LEN];
	uint32_t ip;
	union {
		time_t  startDate; // long (int32_t)
		uint16_t startMinute;
	} start;
	uint16_t duration; // seconds
    uint8_t npProgram;
    uint8_t colorR;
    uint8_t colorG;
    uint8_t colorB;
	uint8_t filler1;
	uint8_t filler2;
} ConfigUdpNeoPixelActuatorValues;
typedef struct ConfigActuatorValues { // 56
 	// 1 - RF Toggle & Latch -> Codes: 0=off, 1=on, 2=toggle
	// 2 - RF Toggle -> Codes: 0=toggle
	// 5 - Udp
	uint8_t type;
	uint8_t filler1;
	uint8_t filler2;
	uint8_t filler3;
    char actuatorName[MAX_NAME_LEN];
	union {
		struct ConfigRfActuatorValues rf; //36
		struct ConfigUdpActuatorValues udp;//24
		struct ConfigUdpNeoPixelActuatorValues np;//32
	} data; // access with some_info_object.data.a or some_info_object.data.b
} ConfigActuatorValues;

extern ConfigValues configValues;
extern CloudValues cloudValues;
extern ConfigButtonValues configButtonValues[MAX_BUTTONS];
extern ConfigActuatorValues configActuatorValues[MAX_ACTUATORS];

extern String webBuffer1;
extern String webBuffer2;
#define __MAX_WEBBUF_LEN 2040

extern const char _FILEOPENFAILED[];
extern const char _FILEREADFAILED[];
extern const char _FILEREAD[];
void savePasswordConfig(const char * nodeName, const char * userName, const char * password);
bool loadAllConfigs();
bool loadConfig();
bool loadCloudConfig();
bool loadButtonActuatorsConfig();

void log(const __FlashStringHelper* buffer);
void log(const char * buffer);
void log(String& buffer);
void log(char c);
void log(uint8_t c);
void log(uint16_t c);
void log(uint32_t c);
void logln(uint16_t c);
void logln(const __FlashStringHelper* buf);
void logln(const char* buf);
void logln(String& buf);
void logln(const __FlashStringHelper* buf, uint8_t num);
void log(const __FlashStringHelper* buf, uint8_t num);
void logln(const __FlashStringHelper* buf, uint16_t num);
void logln(const __FlashStringHelper* buf, uint32_t num);
void logln(const __FlashStringHelper* buf, uint8_t num1, uint8_t num2);
void logln(const __FlashStringHelper* buf, uint8_t num1, uint8_t num2, bool b);
void logln(const __FlashStringHelper* buf, uint32_t num1, uint8_t num2);
void log(const __FlashStringHelper* buf, uint32_t num1, uint8_t num2, uint8_t num3);
void logln(const __FlashStringHelper* buf, uint32_t num1, uint8_t num2, uint8_t num3);

#endif /* GLOBALS_H_ */
