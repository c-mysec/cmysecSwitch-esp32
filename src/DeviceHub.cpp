/*
 * RfDeviceControl.cpp
 *
 *  Created on: Jan 23, 2022
 *      Author: clovis
 */

#include "globals.h"
#include "mySwitchConfig.h"
#include "RfDeviceControl.h"
#include "DeviceHub.h"
#include "IotCore.h"

void deviceHubSetup() {
	rfDeviceControlSetup();
}

void buttonRfPressed(uint8_t buttonRead) {
	// para cada atuador, envia o sinal de acordo com o estado
	for (uint8_t index = 0; index < MAX_BUTTON_ACTUATORS; index++) {
		uint8_t atuador = configButtonValues[buttonRead].actuator[index];
		uint8_t command = configButtonValues[buttonRead].valor[index];
		uint8_t flags = configButtonValues[buttonRead].flags[index];
		if (atuador != NOT_CONFIGURED && command != NOT_CONFIGURED) {
			Serial.print(F("Atuador "));
			Serial.print(configActuatorValues[atuador].actuatorName);
			Serial.print(F("; command "));
			Serial.println(command);
			rfDeviceControlSend(atuador, command, (flags & 1) == 1);
			logln(F("DH AltSt "), atuador, command, (flags & 1) == 1);
			yield();
		}
	}
}
bool controlStatusChanged(char * controlName, uint32_t state) {
	// verifica o botão
	uint8_t control;
	bool found = false;
	for (uint8_t i = 0; i < MAX_BUTTONS; i++) {
		if (strncmp(configActuatorValues[i].actuatorName, controlName, MAX_NAME_LEN) == 0) {
			found = true;
			control = i;
			break;
		}
	}
	if (!found) return false;
	return sendRelayChangeMessageIotCore(configActuatorValues[control].actuatorName, state);
}
uint8_t mapButtonRead = NOT_CONFIGURED;
long mapButtonReadTime = 0;

bool deviceHubLoopRF() {
	uint8_t buttonRead = rfDeviceControlReceive();
	if (buttonRead != NOT_CONFIGURED && mapButtonRead == NOT_CONFIGURED) {
		mapButtonReadTime = millis();
		mapButtonRead = buttonRead;
		//buttonRfPressed(buttonRead);
		return true;
	}
	return false;
}
bool deviceHubLoop() {
	bool received = false;
	if (deviceHubLoopRF()) received = true;
	return received;
}
