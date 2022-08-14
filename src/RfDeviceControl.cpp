/*
 * RfDeviceControl.cpp
 *
 *  Created on: Jan 23, 2022
 *      Author: clovis
 */

#include <RCSwitch.h>
#include "mySwitchConfig.h"
#include "RfDeviceControl.h"
#include "IotCore.h"

RCSwitch mySwitch = RCSwitch();
uint8_t lastButton = 255; // 255 nenhum
uint32_t lastDetected = 0;

uint32_t rFDeviceControlLastCode;
uint8_t rFDeviceControlLastLen;
uint8_t rFDeviceControlLastProto;

void rfDeviceControlSetup() {
	mySwitch.enableReceive(GPIO_NUM_18);
	mySwitch.enableTransmit(GPIO_NUM_16);
	mySwitch.setRepeatTransmit(10);
	// Received 100100010000011111110010 / 9504754 / 24 bit;Protocol: 1
	// Received 100000011011011110010001 / 8501137 / 24 bit;Protocol: 1
	// Received 100000011011011110010100 / 8501140 / 24 bit;Protocol: 1
	// Received 100000011011011110010010 / 8501138 / 24 bit;Protocol: 1
	/*
	strcpy(configButtonValues[0].buttonName, "button1");
	configButtonValues[0].type = TYPE_RF;
	configButtonValues[0].estado = 0; // próximo estado do grupo
	configButtonValues[0].group = 0;
	configButtonValues[0].data.rf.bitlen = 24;
	configButtonValues[0].data.rf.code = 8501137;
	configButtonValues[0].data.rf.protocol = 1;
	strcpy(configButtonValues[1].buttonName, "button2");
	configButtonValues[1].type = TYPE_RF;
	configButtonValues[1].estado = 1; // próximo estado do grupo
	configButtonValues[1].group = 0;
	configButtonValues[1].data.rf.bitlen = 24;
	configButtonValues[1].data.rf.code = 8501140;
	configButtonValues[1].data.rf.protocol = 1;
	strcpy(configButtonValues[2].buttonName, "button3");
	configButtonValues[2].type = TYPE_RF;
	configButtonValues[2].estado = NEXT_STATE; // próximo estado do grupo
	configButtonValues[2].group = 0;
	configButtonValues[2].data.rf.bitlen = 24;
	configButtonValues[2].data.rf.code = 8501138;
	configButtonValues[2].data.rf.protocol = 1;

	strcpy(configActuatorValues[0].actuatorName, "act1");
	configActuatorValues[0].data.rf.estadoAtual = 0;
	configActuatorValues[0].type = TYPE_RF;
	configActuatorValues[0].data.rf.numStates = 2;
	configActuatorValues[0].data.rf.bitlen = 24;
	configActuatorValues[0].data.rf.protocol = 1;
	configActuatorValues[0].data.rf.code[0] = 0b101010100000000000000001;
	configActuatorValues[0].data.rf.code[1] = 0b101010100000000000000010;
	strcpy(configGroupValues[0].groupName, "grp1");
	configGroupValues[0].currentState = 0;
	configGroupValues[0].numStates = 2;
	for (int i = 0; i < MAX_GROUP_ACTUATORS; i++) {
		configGroupValues[0].actuator[i] = NOT_CONFIGURED;
		for (int j = 0; j < MAX_GROUP_STATES; j++) {
			configGroupValues[0].valor[j][i] = NOT_CONFIGURED;
		}
	}
	configGroupValues[0].actuator[0] = 0;
	configGroupValues[0].valor[0][0] = 0;
	configGroupValues[0].valor[1][0] = 1;
	*/
}

uint8_t rfDeviceControlReceive() {
	if (mySwitch.available()) {
		rFDeviceControlLastCode = mySwitch.getReceivedValue();
		rFDeviceControlLastLen = mySwitch.getReceivedBitlength();
		rFDeviceControlLastProto = mySwitch.getReceivedProtocol();
		mySwitch.resetAvailable();
/*		Serial.print(F("Received "));
		Serial.print(rFDeviceControlLastCode, BIN);
		Serial.print(F(" / "));
		Serial.print(rFDeviceControlLastCode);
		Serial.print(F(" / "));
		Serial.print(rFDeviceControlLastLen);
		Serial.print(F(" bit;Protocol: "));
		Serial.println(rFDeviceControlLastProto);
*/
		log(F("RfDC Rec "), rFDeviceControlLastCode, rFDeviceControlLastProto, rFDeviceControlLastLen);
//		for (uint8_t i = 0; i < 20; i++) {
		for (uint8_t i = 0; i < MAX_BUTTONS; i++) {
			if (configButtonValues[i].type == TYPE_RF &&
				configButtonValues[i].data.rf.code == rFDeviceControlLastCode &&
					configButtonValues[i].data.rf.bitlen == rFDeviceControlLastLen &&
					configButtonValues[i].data.rf.protocol == rFDeviceControlLastProto) {
				log(F(" #"), i);
				if (lastButton != i ||
					millis() - lastDetected >= 1000) { // não detecta o mesmo código em menos de 1 segundo e meio
					lastDetected = millis();
				logln(F(" Det"));
		Serial.print(F("New button press: "));
		Serial.println(i);
					return lastButton = i;
				}
				logln(F(" Dup"));
				return NOT_CONFIGURED;
			}
		}
		logln(F("NF"));
	}
	return NOT_CONFIGURED;
}

bool rfDeviceControlSend(const char * name, uint8_t command, bool send) {
	if (name == NULL) return false;
	Serial.print(F("RF sending name : "));
	Serial.print(name);
	Serial.print(',');
	Serial.print(command);
	Serial.print(',');
	Serial.println(send);
	for (int i = 0; i < MAX_ACTUATORS; i++) {
		if (strncmp(configActuatorValues[i].actuatorName, name, MAX_NAME_LEN) == 0) {
			return rfDeviceControlSend(i, command, send);
		}
	}
	return false;
}
bool rfDeviceControlSend(uint8_t actuator, uint8_t command, bool send) {
	String log;
	log.reserve(100);
	log.concat(F("RF sending : "));
	log.concat(actuator);
	log.concat(',');
	log.concat(command);
	log.concat(',');
	log.concat(send);
	log.concat(',');
	if (actuator >= MAX_ACTUATORS) {
		log.concat(F("ABOUVE MAX"));
		logln(log.c_str());
		return false;
	}
	// o próximo estado é o valor do comando (on uo off) ou é a inversão do estado quando o comando for NEXT_STATE
	// independete se for toggle ou latch
	if (configActuatorValues[actuator].type == 1 || configActuatorValues[actuator].type == 2) {
		// se for do tipo latch, o command 2 significa NEXT_STATE
		if (command == 2 && configActuatorValues[actuator].type == 1) command = NEXT_STATE;
		uint8_t nextState = command == NEXT_STATE ? 1 - configActuatorValues[actuator].data.rf.estadoAtual : command;
		log.concat(nextState);
		log.concat(',');
		log.concat(configActuatorValues[actuator].type);
		configActuatorValues[actuator].data.rf.estadoAtual = nextState;

		// atualiza o status na cloud
		bool resp = sendRelayChangeMessageIotCore(configActuatorValues[actuator].actuatorName, nextState);
		if (send) {
			// envia toggle ou, no caso de latch, envia off ou on
			uint8_t codePos = 0; // Envia toggle ou off
			if (configActuatorValues[actuator].type == 1 /*latch*/ && nextState == 1 /*on*/) codePos = 1; // envia On
			mySwitch.setProtocol(configActuatorValues[actuator].data.rf.protocol);
			mySwitch.send(configActuatorValues[actuator].data.rf.code[codePos], configActuatorValues[actuator].data.rf.bitlen);
			log.concat(F("\nRF send: "));
			// envia para o device
			log.concat(actuator);
			log.concat(';');
			log.concat(configActuatorValues[actuator].type);
			log.concat(';');
			log.concat(codePos);
			log.concat(';');
			log.concat(configActuatorValues[actuator].data.rf.code[codePos]);
			log.concat(';');
			log.concat(configActuatorValues[actuator].actuatorName);
		} else {
			log.concat(F("RF set but not send: "));
		}
	}
	logln(log.c_str());
	Serial.println(log.c_str());
	return true;
}
void rfDeviceControlTestSend(int protocol, int bitlen, long code) {
	mySwitch.setProtocol(protocol);
	mySwitch.send(code, bitlen);
	Serial.print(protocol);
	Serial.print(';');
	Serial.print(bitlen);
	Serial.print(';');
	Serial.println(code);
}

