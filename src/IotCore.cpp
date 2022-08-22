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

#include <ArduinoJson.h>
#include "ProgMemConsts.h"

#include <mySwitchConfig.h>
#include <PubSubClient.h>

#include "UdpBroadcast.h"
#include "NTPSync.h"

#include "RfDeviceControl.h"
String outTopic;
String outMsg;
String certificateOwnershipToken;

WiFiClientSecure wifiSecureClient;
PubSubClient *pubSubClient;
long lastReconnect = 0;
bool configured = false;
bool hasTCerts = false;
bool waitRegister = false;

long lastMsg = 0;
char msg[50];
int value = 0;
long lastConnectRetry = -1;

const char IOT_CERT_CA_PATH[] PROGMEM = "/ca.der";
const char IOT_CERT_PATH[] PROGMEM = "/cert.der";
const char IOT_PRIVKEY_PATH[] PROGMEM = "/private.der";

const char IOT_TCERT_PATH[] PROGMEM = "/tcert.der";
const char IOT_TPRIVKEY_PATH[] PROGMEM = "/tprivate.der";

void callbackIotCore(char* topic, byte* payload, unsigned int length);

//const char *AWS_endpoint = "a2g89lg5mw80c9-ats.iot.us-east-1.amazonaws.com";
//"xxxxxxxxxxxxxx-ats.iot.us-east-1.amazonaws.com"; //MQTT broker ip
void _log(const char s[]) {
	Serial.print(FPSTR(s));
	log(FPSTR(s));
}
void _logln(const char s[]) {
	Serial.println(FPSTR(s));
	logln(FPSTR(s));
}
void _logln(String & s) {
	Serial.println(s);
	logln(s);
}
void _logln(const __FlashStringHelper* buf) {
	Serial.println(buf);
	log(buf);
	log('\n');
}
void _log(const __FlashStringHelper* buf) {
	Serial.print(buf);
	log(buf);
}
void _log(String & s) {
	Serial.print(s);
	log(s);
}
bool parseCreateKeysAndCertificate(const char * payload) {
	// mensagem de geração de certificado
	StaticJsonDocument<256> doc;
	/*
		{
			"certificateId": "string",
			"certificatePem": "string",
			"privateKey": "string",
			"certificateOwnershipToken": "string"
		}
	*/
	deserializeJson(doc, payload);
	if (doc.containsKey("statusCode")) {
		// erro
		_log(_FAILED);
		_log(F("exchange certs. "));
		String temp;
		temp.concat((const char*)doc["statusCode"]); _log(temp);
		temp.clear();
		temp.concat((const char*)doc["errorCode"]); _log(temp);
		temp.clear();
		temp.concat((const char*)doc["errorMessage"]); _log(temp);
		doc.clear();
		return false;
	}
	File cert = SPIFFS.open(IOT_CERT_PATH, _W);
	cert.write(doc["certificatePem"]);
	cert.close();
	File pk = SPIFFS.open(IOT_PRIVKEY_PATH, _W);
	cert.write(doc["privateKey"]);
	cert.close();
	certificateOwnershipToken.clear();
	certificateOwnershipToken.concat((const char *)doc["certificateOwnershipToken"]);
	return true;
}
bool parseRegisterThing(const char * payload) {
	// mensagem de geração de certificado
	StaticJsonDocument<256> doc;
	/*
{
    "deviceConfiguration": {
        "deviceKey": "string"
    },
    "thingName": "string"
}
	*/
	deserializeJson(doc, payload);
	if (doc.containsKey("statusCode")) {
		// erro
		_log(_FAILED);
		_log(F("exchange certs. "));
		String temp;
		temp.concat((const char*)doc["statusCode"]); _log(temp);
		temp.clear();
		temp.concat((const char*)doc["errorCode"]); _log(temp);
		temp.clear();
		temp.concat((const char*)doc["errorMessage"]); _log(temp);
		doc.clear();
		return false;
	}
	JsonObject deviceConfiguration = doc["deviceConfiguration"];
	strncpy(cloudValues.passwd, deviceConfiguration["deviceKey"], MAX_ROUTER_STR);
	// delete tempkeys
	bool deleted = SPIFFS.remove(String(FPSTR(IOT_TCERT_PATH)));
	deleted = SPIFFS.remove(String(FPSTR(IOT_TPRIVKEY_PATH)));
	return saveCloudConfig();
}
void sendRegisterThing() {
	String topic(F("$aws/provisioning-templates/mysecSwitch1/provision/json/accepted"));
	pubSubClient->subscribe(topic.c_str());
	topic.clear();
	topic.concat(F("$aws/provisioning-templates/mysecSwitch1/provision/json"));
	StaticJsonDocument<256> doc;
	/*
		{
			"certificateOwnershipToken": "string",
			"parameters": {
				"userName": "string",
				"deviceName": "string",
			}
		}
	*/
	doc["certificateOwnershipToken"] = certificateOwnershipToken;
	JsonObject parameters = doc.createNestedObject("parameters");
	parameters["userName"] = cloudValues.userName;
	parameters["userName"] = configValues.nodeName;
	String result;
	result.reserve(200);
	serializeJson(doc, result);
	pubSubClient->publish(topic.c_str(), result.c_str());
}
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
	if (hasTCerts) {
		bool r = parseCreateKeysAndCertificate((const char *) payload);
		if (!r) return;
		sendRegisterThing();
		waitRegister = true;
		hasTCerts = false;
	} else if (waitRegister) {
		bool r = parseRegisterThing((const char *) payload);
		if (!r) return;
		// reset
		ESP.restart();
	} else {
		// mensagem normal
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
}
void reconnect() {
// Loop until we're reconnected
	if ((((long)(millis() - lastConnectRetry)) > 30000 || lastConnectRetry == -1) && !pubSubClient->connected()) {
		Serial.print(FPSTR(_MQTT_RECCONECT_ATTEMPT));
		logln(FPSTR(_MQTT_RECCONECT_ATTEMPT));
		// Attempt to connect
		if (pubSubClient->connect(cloudValues.userName)) {
			Serial.println(FPSTR(_CONNECTED));
			logln(FPSTR(_CONNECTED));
			if (hasTCerts) {
				String inTopic(F("$aws/certificates/create/json/accepted"));
				pubSubClient->subscribe(inTopic.c_str());
				_log(F("Subscribing to: "));
				_logln(inTopic.c_str());
				// envia mensagem de pedido de certificado
				pubSubClient->publish(outTopic.c_str(), "");
			} else {
				String inTopic(F("myswitch/"));
				inTopic.concat(cloudValues.userName);
				inTopic.concat('.');
				inTopic.concat(configValues.nodeName);
				inTopic.concat(F("/control"));
				pubSubClient->subscribe(inTopic.c_str());
				Serial.print(F("Subscribing to: "));
				Serial.print(inTopic.c_str());
				logln(inTopic.c_str());
			}
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
File openFile(String filename) {
	File file = SPIFFS.open(filename, _R); //replace cert.crt eith your uploaded file name
	if (!file) {
		_log(_FAILED_TO_OPEN);
		_logln(filename);
	} else {
		_log(_SUCCESS_OPEN);
		_logln(filename);
	}
	return file;
}
void setupIotCore() {
	configured = false;
	hasTCerts = false;
	if (cloudValues.config[0] != 1) return;
	File ca = openFile(String(FPSTR(IOT_CERT_CA_PATH))); //replace ca eith your uploaded file name
	if (!ca) return;
#ifdef ARDUINO_ARCH_ESP8266
	if (wifiSecureClient.loadCACert(ca)) {
#endif
#ifdef ARDUINO_ARCH_ESP32
	if (wifiSecureClient.loadCACert(ca, ca.size())) {
#endif
		_log(_LOADED);
		_logln(IOT_CERT_CA_PATH);
	} else {
		_log(_NOT_LOADED);
		_logln(IOT_CERT_CA_PATH);
		return;
	}
#ifdef ARDUINO_ARCH_ESP8266
	wifiSecureClient.setBufferSizes(512, 512);
	wifiSecureClient.setX509Time(timeClient.getEpochTime());
#endif

	File tcert = openFile(String(FPSTR(IOT_TCERT_PATH)));
	File tkey = openFile(String(FPSTR(IOT_TPRIVKEY_PATH)));
	if (tcert && tkey) {
		hasTCerts = true; // se tem tcerts, da subscribe no topic de certificação e envia para topico de certificação
#ifdef ARDUINO_ARCH_ESP8266
		if (wifiSecureClient.loadCertificate(cert)) {
#endif
#ifdef ARDUINO_ARCH_ESP32
		if (wifiSecureClient.loadCertificate(tcert, tcert.size())) {
#endif
			_log(_LOADED);
			_logln(IOT_CERT_PATH);
		} else {
			_log(_NOT_LOADED);
			_logln(IOT_CERT_PATH);
			return;
		}
#ifdef ARDUINO_ARCH_ESP8266
		if (wifiSecureClient.loadPrivateKey(private_key)) {
#endif
#ifdef ARDUINO_ARCH_ESP32
		if (wifiSecureClient.loadPrivateKey(tkey, tkey.size())) {
#endif
			_log(_LOADED);
			_logln(IOT_PRIVKEY_PATH);
		} else {
			_log(_NOT_LOADED);
			_logln(IOT_PRIVKEY_PATH);
		}
	} else {
		File cert = openFile(String(FPSTR(IOT_CERT_PATH))); //replace cert.crt eith your uploaded file name
		if (!cert) return;
		File private_key = openFile(String(FPSTR(IOT_PRIVKEY_PATH))); //replace private eith your uploaded file name
		if (!private_key) return;
#ifdef ARDUINO_ARCH_ESP8266
		if (wifiSecureClient.loadCertificate(cert)) {
#endif
#ifdef ARDUINO_ARCH_ESP32
		if (wifiSecureClient.loadCertificate(cert, cert.size())) {
#endif
			_log(_LOADED);
			_logln(IOT_CERT_PATH);
		} else {
			_log(_NOT_LOADED);
			_logln(IOT_CERT_PATH);
			return;
		}
#ifdef ARDUINO_ARCH_ESP8266
		if (wifiSecureClient.loadPrivateKey(private_key)) {
#endif
#ifdef ARDUINO_ARCH_ESP32
		if (wifiSecureClient.loadPrivateKey(private_key, private_key.size())) {
#endif
			_log(_LOADED);
			_logln(IOT_PRIVKEY_PATH);
		} else {
			_log(_NOT_LOADED);
			_logln(IOT_PRIVKEY_PATH);
			return;
		}
	}
	pubSubClient = new PubSubClient(cloudValues.endpoint, 8883, callbackIotCore, wifiSecureClient); //set MQTT port number to 8883 as per //standard
	outTopic.clear();
	if (hasTCerts) {
		outTopic.concat("$aws/certificates/create/json");
	} else if (strlen(cloudValues.iotOutTopic) > 0) {
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
		/*
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
		*/
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

