/*
 * UdpStatusMessage.cpp
 *
 *  Created on: 4 de ago de 2020
 *      Author: user
 */

#include <mySwitchConfig.h>
#include "Messages.h"


size_t maxMessageSize = sizeof(MsgStatusRes);

void buildMsgStatusRes(MsgStatusRes * msg) {
	memset(msg, 0, sizeof(MsgStatusRes));
	msg->header[0] = MSG_STATUSRES;
	strncpy(msg->nodeName, configValues.nodeName, MAX_NAME_LEN);
	memcpy(msg->configActuatorValues, configActuatorValues, sizeof(ConfigActuatorValues) * MAX_ACTUATORS);
	Serial.print(F("Buildmsg status"));
	printBlockLn((uint8_t *)msg, sizeof(MsgStatusRes));
}
/**
void buildMsgButtonClick(MsgButtonClick * msg, uint8_t btnNum, bool notifyCloud) {
	memset(msg, 0, sizeof(msg));
	msg->header[0] = MSG_BTNCLICK;
	msg->header[1] = notifyCloud ? MSG_NOTIFY_CLOUD : MSG_DONT_NOTIFY_CLOUD;
	strncpy(msg->nodeName, configValues.nodeName, MAX_NAME_LEN);
	strncpy(msg->button, getConfigButtonName(btnNum), MAX_NAME_LEN);
}
 */
/**
 */
void buildMsgRelayStateChange(MsgRelayStateChange * msg, uint8_t relayNum, uint8_t state) {
	memset(msg, 0, sizeof(MsgRelayStateChange));
	msg->header[0] = MSG_RELAYSTATECHANGE;
	strncpy(msg->nodeName, configValues.nodeName, MAX_NAME_LEN);
	strncpy(msg->relay, configActuatorValues[relayNum].actuatorName, MAX_NAME_LEN);
	msg->state = state;
}
void buildMsgRelayStateChange(MsgRelayStateChange * msg, const char * relayName, uint8_t state) {
	memset(msg, 0, sizeof(MsgRelayStateChange));
	msg->header[0] = MSG_RELAYSTATECHANGE;
	strncpy(msg->nodeName, configValues.nodeName, MAX_NAME_LEN);
	strncpy(msg->relay, relayName, MAX_NAME_LEN);
	msg->state = state;
}
