/*
 * IotCore.cpp
 *
 *  Created on: Sep 29, 2021
 *      Author: clovis
 */

#include "IotCore.h"
#include "FS.h"
#ifdef ARDUINO_ARCH_ESP32
#include "SPIFFS.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#endif
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#endif

#include "ProgMemConsts.h"

#include <mySwitchConfig.h>
#include <PubSubClient.h>

#include "UdpBroadcast.h"
#include "NTPSync.h"

#include "RfDeviceControl.h"
String outTopic;
String outMsg;

//const char *AWS_endpoint = "a2g89lg5mw80c9-ats.iot.us-east-1.amazonaws.com";
//"xxxxxxxxxxxxxx-ats.iot.us-east-1.amazonaws.com"; //MQTT broker ip

void callbackIotCore(char *topic, byte *payload, unsigned int length) {
	String f;
	f.reserve(40);
	f.concat(F("Message arrived ["));
	f.concat(topic);
	f.concat(' ');
	f.concat(length);
	f.concat(']');
	f.concat(' ');
	for (unsigned int i = 0; i < length; i++) {
		f.concat((char) payload[i]);
	}
	payload[length - 1] = 0;
	Serial.println(f.c_str());
	logln(f.c_str());
	//1;4;0;home;1;niche -> 0 - off, 1 - on, 2 - toggle
	char *ptr = NULL;
	ptr = strtok((char *)payload, ";");
	// ptr -> 1 ver
	ptr = strtok(NULL, ";");
	// ptr -> 4 msgType
	int msgType = atoi(ptr);
	ptr = strtok(NULL, ";");
	// ptr -> home   user
	char* user = ptr;
	while (ptr = strtok(NULL, ";")) {
		// ptr -> 0   nextState
		int nextState = atoi(ptr);
		ptr = strtok(NULL, ";");
		if (ptr) {
			// ptr -> niche   actuator
			f.clear();
			f.concat(F("Control : "));
			f.concat(msgType);
			f.concat(',');
			f.concat(user);
			f.concat(',');
			f.concat(nextState);
			f.concat(',');
			f.concat(ptr);
			f.concat('.');
			Serial.println(f.c_str());
			logln(f.c_str());
			bool found = false;
			for (int i = 0; i < MAX_ACTUATORS; i++) {
				if (strncmp(configActuatorValues[i].actuatorName, ptr, MAX_NAME_LEN) == 0) {
					// send RF
					Serial.print(F("Sending to device :"));
					Serial.println(configActuatorValues[i].actuatorName);
					rfDeviceControlSend(ptr, nextState, true);
					// send UDP
					found = true;
					break;
				}
			}
			if (!found) Serial.println("not found!");
		} else break;
	}
}
WiFiClientSecure wifiSecureClient;
PubSubClient *pubSubClient;
long lastReconnect = 0;
bool configured = false;

long lastMsg = 0;
char msg[50];
int value = 0;
long lastConnectRetry = -1;
void reconnect() {
// Loop until we're reconnected
	if ((((long)(millis() - lastConnectRetry)) > 30000 || lastConnectRetry == -1) && !pubSubClient->connected()) {
		Serial.print(FPSTR(_MQTT_RECCONECT_ATTEMPT));
		logln(FPSTR(_MQTT_RECCONECT_ATTEMPT));
		// Attempt to connect
		if (pubSubClient->connect(cloudValues.userName)) {
			Serial.println(FPSTR(_CONNECTED));
			logln(FPSTR(_CONNECTED));
// Once connected, publish an announcement...
			//client.publish("outTopic", "hello world");
// ... and resubscribe
			String inTopic(F("myswitch/"));
			inTopic.concat(cloudValues.userName);
			inTopic.concat('.');
			inTopic.concat(configValues.nodeName);
			inTopic.concat(F("/control"));
			pubSubClient->subscribe(inTopic.c_str());
			Serial.print(F("Subscribing to: "));
			Serial.print(inTopic.c_str());
			logln(inTopic.c_str());

			// topicos para cada usu�rio
			// topic/myswitch/userid/data
			// topic/myswitch/userid/control
			// topic/myswitch/userid/config

		} else {
			Serial.print(FPSTR(_MQTT_FAILED_CONN));
			Serial.print(pubSubClient->state());
			log(FPSTR(_MQTT_FAILED_CONN));
			logln(pubSubClient->state());
			Serial.println(F(" try again in 5 seconds"));
			lastConnectRetry = millis();
			char buf[256];
#ifdef ARDUINO_ARCH_ESP8266
			wifiSecureClient.getLastSSLError(buf, 256);
#endif
#ifdef ARDUINO_ARCH_ESP32
			wifiSecureClient.lastError(buf, 256);
#endif
			Serial.print(F("WiFiClientSecure SSL error: "));
			Serial.println(buf);

// Wait 5 seconds before retrying
		}
	}
}
const char IOT_CERT_CA_PATH[] PROGMEM = "/ca.der";;
const char IOT_CERT_PATH[] PROGMEM = "/cert.der";;
const char IOT_PRIVKEY_PATH[] PROGMEM = "/private.der";;

void setupIotCore() {
	configured = false;
	if (cloudValues.config[0] != 1) return;
	// Load certificate file
	File cert = SPIFFS.open(String(FPSTR(IOT_CERT_PATH)), _R); //replace cert.crt eith your uploaded file name
	if (!cert) {
		Serial.print(FPSTR(_FAILED));
		log(FPSTR(_FAILED));
		Serial.println(F("Failed to open cert file"));
		return;
	} else {
		Serial.print(FPSTR(_SUCCESS));
		log(FPSTR(_SUCCESS));
		Serial.print(FPSTR(_MQTT_TO_OPEN_CERT));
		log(FPSTR(_MQTT_TO_OPEN_CERT));
	}
	// Load private key file
	File private_key = SPIFFS.open(String(FPSTR(IOT_PRIVKEY_PATH)), _R); //replace private eith your uploaded file name
	if (!private_key) {
		Serial.print(FPSTR(_FAILED));
		log(FPSTR(_FAILED));
		Serial.print(FPSTR(_MQTT_TO_OPEN_KEY));
		log(FPSTR(_MQTT_TO_OPEN_KEY));
		return;
	} else {
		Serial.print(FPSTR(_SUCCESS));
		log(FPSTR(_SUCCESS));
		Serial.print(FPSTR(_MQTT_TO_OPEN_KEY));
		log(FPSTR(_MQTT_TO_OPEN_KEY));
	}
	// Load CA file
	File ca = SPIFFS.open(String(FPSTR(IOT_CERT_CA_PATH)), _R); //replace ca eith your uploaded file name
	if (!ca) {
		Serial.print(FPSTR(_FAILED));
		log(FPSTR(_FAILED));
		Serial.print(FPSTR(_MQTT_TO_OPEN_CACERT));
		log(FPSTR(_MQTT_TO_OPEN_CACERT));
		return;
	} else {
		Serial.print(FPSTR(_SUCCESS));
		log(FPSTR(_SUCCESS));
		Serial.print(FPSTR(_MQTT_TO_OPEN_CACERT));
		log(FPSTR(_MQTT_TO_OPEN_CACERT));
	}
#ifdef ARDUINO_ARCH_ESP8266
	wifiSecureClient.setBufferSizes(512, 512);

	wifiSecureClient.setX509Time(timeClient.getEpochTime());
	Serial.print(F("Heap: "));
	Serial.println(ESP.getFreeHeap());

	if (wifiSecureClient.loadCertificate(cert)) {
#endif
#ifdef ARDUINO_ARCH_ESP32
	Serial.print(F("Heap: "));
	Serial.println(ESP.getFreeHeap());

	if (wifiSecureClient.loadCertificate(cert, cert.size())) {
#endif
		Serial.print(FPSTR(_LOADED));
		log(FPSTR(_LOADED));
		Serial.println(FPSTR(_MQTT_TO_OPEN_CERT));
		logln(FPSTR(_MQTT_TO_OPEN_CERT));
	} else {
		Serial.print(FPSTR(_NOT_LOADED));
		log(FPSTR(_NOT_LOADED));
		Serial.println(FPSTR(_MQTT_TO_OPEN_CERT));
		logln(FPSTR(_MQTT_TO_OPEN_CERT));
		return;
	}
#ifdef ARDUINO_ARCH_ESP8266
	if (wifiSecureClient.loadPrivateKey(private_key)) {
#endif
#ifdef ARDUINO_ARCH_ESP32
	if (wifiSecureClient.loadPrivateKey(private_key, private_key.size())) {
#endif
		Serial.print(FPSTR(_LOADED));
		log(FPSTR(_LOADED));
		Serial.println(FPSTR(_MQTT_TO_OPEN_KEY));
		logln(FPSTR(_MQTT_TO_OPEN_KEY));
	} else {
		Serial.print(FPSTR(_NOT_LOADED));
		log(FPSTR(_NOT_LOADED));
		Serial.println(FPSTR(_MQTT_TO_OPEN_KEY));
		logln(FPSTR(_MQTT_TO_OPEN_KEY));
		return;
	}

#ifdef ARDUINO_ARCH_ESP8266
	if (wifiSecureClient.loadCACert(ca)) {
#endif
#ifdef ARDUINO_ARCH_ESP32
	if (wifiSecureClient.loadCACert(ca, ca.size())) {
#endif
		Serial.print(FPSTR(_LOADED));
		log(FPSTR(_LOADED));
		Serial.println(FPSTR(_MQTT_TO_OPEN_CACERT));
		logln(FPSTR(_MQTT_TO_OPEN_CACERT));
	} else {
		Serial.print(FPSTR(_NOT_LOADED));
		log(FPSTR(_NOT_LOADED));
		Serial.println(FPSTR(_MQTT_TO_OPEN_CACERT));
		logln(FPSTR(_MQTT_TO_OPEN_CACERT));
		return;
	}
	pubSubClient = new PubSubClient(cloudValues.endpoint, 8883, callbackIotCore, wifiSecureClient); //set MQTT port number to 8883 as per //standard
	Serial.print(F("Heap: "));
	Serial.println(ESP.getFreeHeap());
	outTopic.clear();
	if (strlen(cloudValues.iotOutTopic) > 0) {
		outTopic.concat(cloudValues.iotOutTopic);
	} else {
		outTopic.concat(F("myswitch/"));
		outTopic.concat(cloudValues.userName);
		outTopic.concat('.');
		outTopic.concat(configValues.nodeName);
		outTopic.concat(F("/data"));
	}
	logln(outTopic.c_str());
	configured = true;
}
void loopIotCore() {
	if (!configured) return;
	if (!pubSubClient->connected()) {
		long now = millis();
		if (now - lastReconnect > 5000) {
			reconnect();
		}
	} else {
		pubSubClient->loop();
		long now = millis();
		if (now - lastMsg > 0 && false) {
			lastMsg = now + 36000000;
			++value;
			snprintf(msg, 75, "{\"message\": \"hello world #%ld\"}", value);
			Serial.print(F("Publish message: "));
			Serial.println(msg);
			log(F("Publish message: "));
			logln(msg);
			pubSubClient->publish("outTopic", msg);
			Serial.print(F("Heap: "));
			Serial.println(ESP.getFreeHeap()); //Low heap can cause problems
		}
	}
}
bool sendRelayChangeMessageIotCore(const char * name, uint8_t state) {
	if (!configured) return false;
	outMsg.clear();
	outMsg.reserve(100);
	outMsg.concat(F("{\"account\":\""));
	outMsg.concat(cloudValues.userName);
	outMsg.concat(F("\",\"control\":\""));
	outMsg.concat(name);
	outMsg.concat(F("\",\"state\":"));
	outMsg.concat(state);
	outMsg.concat(F(",\"password\":\""));
	outMsg.concat(cloudValues.passwd);
	outMsg.concat(F("\"}"));
	pubSubClient->publish(outTopic.c_str(), outMsg.c_str());
	Serial.print(F("outTopic: "));
	Serial.print(outTopic.c_str());
	Serial.print(F("outMsg: "));
	Serial.print(outMsg.c_str());
	log(F("outTopic: "));
	log(outTopic.c_str());
	log(F(" outMsg: "));
	logln(outMsg.c_str());
	Serial.print(F("Heap: "));
	Serial.println(ESP.getFreeHeap()); //Low heap can cause problems
	return true;
}
/* Mensagem envio para IOT CORE
{
     "username" : "home",
     "relay" : "niche",
     "state" : 1,
     "password" : "Lockheed!"
}
*/
/* Mensagem que vem do IOT CORE ao acionar alexa
    // protocol 1 ; relay state change; powerNextState; home; niche
     * 1;4;0;home;niche
    var buffer = "1;4;" + powerNextState + ";" + request.directive.endpoint.cookie.account + ";" + request.directive.endpoint.cookie.name;
    var params = {
        topic: 'myswitch/'+request.directive.endpoint.cookie.account+'/control',
        payload: buffer,
        qos: 0
    }
 Provavelmente s� chega o campo payload
*/

