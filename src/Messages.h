/*
 * UdpStatusMessage.h
 *
 *  Created on: 4 de ago de 2020
 *      Author: user
 */

#ifndef MESSAGES_H_
#define MESSAGES_H_

#include "globals.h"
#include "mySwitchConfig.h"

extern size_t maxMessageSize;

// message status reply - para responder ao App o status
// message button - para indicar que um botão foi pressionado ()
// message relay change
//

// header[1]:	0 - don't notify to cloud
//				1 - notify to cloud
#define MSG_DONT_NOTIFY_CLOUD 0
#define MSG_NOTIFY_CLOUD 1
// header[0]:	0 - status request
//				1 - status reply
//				2 - button change
#define MSG_STATUSREQ 0
#define MSG_STATUSRES 1
#define MSG_NEOPIXEL_STATUSRES 2

#define MSG_BTNCLICK 3
#define MSG_RELAYSTATECHANGE 4
#define MSG_NEOPIXELSTATECHANGE 5

typedef struct {
	uint8_t header[4];
} MsgStatusReq;
typedef struct {
	uint8_t header[4];
    char nodeName[MAX_NAME_LEN];
    ConfigActuatorValues configActuatorValues[MAX_ACTUATORS];
    // uint8 nodetype
    // np status, alarm status, etc.
} MsgStatusRes;
typedef struct {
	uint8_t header[4];
    char nodeName[MAX_NAME_LEN];
    uint8_t nodetype;
    // np status, alarm status, etc.
    uint8_t npProgram;
    uint8_t colorR;
    uint8_t colorG;
    uint8_t colorB;
    uint8_t filler1;
    uint8_t filler2;
    uint8_t filler3;
} MsgStatusNeoPixel;

typedef struct {
	uint8_t header[4];
    char nodeName[MAX_NAME_LEN];
    char button[MAX_NAME_LEN];
} MsgButtonClick;

typedef struct {
	uint8_t header[4];
    char nodeName[MAX_NAME_LEN];
    char relay[MAX_NAME_LEN];
    uint8_t state;
} MsgRelayStateChange;

typedef struct {
	uint8_t header[4];
    char nodeName[MAX_NAME_LEN];
	uint8_t flags[4]; // byte0 - program
	uint32_t  startTime; // quantidade de minutos ap�s meia noite OU a data de in�cio UnixEpoch seconds
	uint16_t duration; // seconds
	uint8_t intensityR;
	uint8_t intensityG;
	uint8_t intensityB;
} MsgNeoPixelStateChange;


void buildMsgStatusRes(MsgStatusRes * msg);
void buildMsgRelayStateChange(MsgRelayStateChange * msg, uint8_t relayNum, uint8_t state);
void buildMsgRelayStateChange(MsgRelayStateChange * msg, const char * relayName, uint8_t state);
/**
 * mensagens vindas de controles UDP, logo, o controlador não constroi este tipo de mensagem
 */
//void buildMsgButtonClick(MsgButtonClick * msg, uint8_t btnNum);

#endif /* MESSAGES_H_ */
