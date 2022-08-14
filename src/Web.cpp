/*
 * Web.cpp
 *
 *  Created on: 7 de ago de 2020
 *      Author: user
 */

#include "Web.h"
#include "globals.h"
#include "FS.h"
#ifdef ARDUINO_ARCH_ESP32
#include "SPIFFS.h"
#include <WiFi.h>
#endif
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#endif
#ifdef ARDUINO_ARCH_ESP32
#include <AsyncElegantOTA.h>
#endif

#include "mySwitchConfig.h"
#include "UdpBroadcast.h"
#include "ProgMemConsts.h"
#include "RfDeviceControl.h"
#include "DeviceHub.h"
#include "iotCore.h"

int fwupdt = 0;

const char *update_path = "/firmware";
const char *update_username = "admin";
//const char *update_password = "lockheed";
bool reset = false;
bool format = false;
long rfSendingCode;
int rfSendingProtocol;
int rfSendingBitlen;
int rfSendingQtd = 0;
bool rfSending = false;
AsyncWebServer httpServer(80);

String getContentType(String filename) { // convert the file extension to the MIME type
	if (filename.endsWith(FPSTR(_DOT_HMTL)))
		return FPSTR(_TEXTHTML);
	else if (filename.endsWith(FPSTR(_DOT_CSS)))
		return FPSTR(_TEXTCSS);
	else if (filename.endsWith(FPSTR(_DOT_JS)))
		return FPSTR(_JAVASCRIPT);
	else if (filename.endsWith(FPSTR(_DOT_ICO)))
		return FPSTR(_IMAGEICON);
	else if (filename.endsWith(FPSTR(_DOT_GZ)))
		return FPSTR(_APPGZIP);
	return String(FPSTR(_TEXTPLAIN));
}

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index,
		uint8_t *data, size_t len, bool final) {
	if (!request->authenticate(update_username, configValues.passwd))
		return;
	if (!index) {
		println(F("UploadStart: "), filename.c_str());
		// open the file on first call and store the file handle in the request object
		if (filename.endsWith(FPSTR(_PASSWORDCONFIGDAT2))) {
			request->_tempFile = SPIFFS.open(_PASSWORDCONFIGDAT, _W);
		} else if (filename.endsWith(FPSTR(_DOT_DAT))) {
			request->_tempFile = SPIFFS.open(_DATA + filename, _W);
		} else if (filename.endsWith(FPSTR(_DOT_DER))) {
			request->_tempFile = SPIFFS.open('/'+filename, _W);
		} else {
			request->_tempFile = SPIFFS.open(_WWW + filename, _W);
		}
	}
	if (len) {
		// stream the incoming chunk to the opened file
		request->_tempFile.write(data, len);
	}
	if (final) {
		println(F("UploadEnd: "),
				filename.c_str(), index + len);
		// close the file handle as the upload is now done
		request->_tempFile.close();
		request->send(200, FPSTR(_TEXTHTML), F("File Uploaded !. <a href=\"fupload\">voltar</a>"));
		if (filename.equals(FPSTR(_CONFIG_DAT))) loadConfig();
	}
}
#ifdef ARDUINO_ARCH_ESP8266
void handle_update_progress_cb(AsyncWebServerRequest *request, String filename,
		size_t index, uint8_t *data, size_t len, bool final) {
	if (!request->authenticate(update_username, configValues.passwd))
		return request->requestAuthentication();
	uint32_t free_space = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
	if (!index) {
		fwupdt = 1;
		Serial.println("Update");
		Update.runAsync(true);
		if (!Update.begin(free_space)) {
			Update.printError(Serial);
		}
	}

	if (Update.write(data, len) != len) {
		Update.printError(Serial);
	} else {
		println(F("Progress: "),
				(Update.progress() * 100) / Update.size());
	}

	if (final) {
		if (!Update.end(true)) {
			Update.printError(Serial);
		} else {
			fwupdt = 2; //Set flag so main loop can issue restart call
			Serial.println(F("Update complete"));
		}
	}
}
#endif
void webSetup() {
	httpServer.serveStatic(_SLASH, SPIFFS, _WWW).setCacheControl("max-age=600").setDefaultFile("index.html").setAuthentication(
			update_username, configValues.passwd);

//	httpServer.on(_SLASH, HTTP_GET, [](AsyncWebServerRequest *request) {
//		request->redirect(F("/index.html"));
//		delay(100);
//	});
	httpServer.on("/ver", HTTP_GET, [](AsyncWebServerRequest *request) {
		String t(F("1.0.1"));
		request->send(200, FPSTR(_TEXTHTML),t);
	});
#ifdef ARDUINO_ARCH_ESP8266
	httpServer.on("/dir", HTTP_GET, [](AsyncWebServerRequest *request) {
		Dir dir = SPIFFS.openDir(_SLASH);
		String t;
		t.reserve(500);
		while (dir.next()) {
			t.concat(F("<br><a href=\"/delete?f="));
		    t.concat(dir.fileName());
		    t.concat(F("\">"));
		    t.concat(dir.fileName());
		    t.concat(F("</a>"));
		    t.concat(F("<br>"));
		}
		t.concat(F("<br><a href=\"/format\">format</a>"));
		t.concat(F("<br><a href=\"/\">home1</a>"));
		request->send(200, FPSTR(_TEXTHTML),t);
	});
#endif
#ifdef ARDUINO_ARCH_ESP32
	httpServer.on("/dir", HTTP_GET, [](AsyncWebServerRequest *request) {
		String fn = request->arg("f");
		File dir = SPIFFS.open(_SLASH);
		if (fn.length() > 0) {
			dir = SPIFFS.open(fn);
		}
		String t;
		t.reserve(500);
		File f = dir.openNextFile();
		while (f) {
			t.concat(F("<br><a href=\"/delete?f="));
		    t.concat(f.name());
		    t.concat(F("\">"));
		    t.concat(f.name());
		    t.concat(F("</a>"));
		    t.concat(F("<br>"));
			f = dir.openNextFile();
		}
		t.concat(F("<br><a href=\"/format\">format</a>"));
		t.concat(F("<br><a href=\"/\">home1</a>"));
		request->send(200, FPSTR(_TEXTHTML),t);
	});
#endif
	httpServer.on(String(F("/date")).c_str(), HTTP_GET,
			[](AsyncWebServerRequest *request) {
		String t;
		t.reserve(500);
		t.concat(__DATE__);
		t.concat(' ');
		t.concat(__TIME__);
		t.concat(FPSTR(_BRHOME));
		request->send(200, FPSTR(_TEXTHTML),
				t);
			}).setAuthentication(update_username, configValues.passwd);
	httpServer.on(String(F("/getLog1")).c_str(), HTTP_GET,
			[](AsyncWebServerRequest *request) {
		String f = request->arg("f");
		if (f.equals(F("1"))) request->send(200, FPSTR(_TEXTPLAIN), webBuffer1);
		else request->send(200, FPSTR(_TEXTPLAIN), webBuffer2);
			}).setAuthentication(update_username, configValues.passwd);

	httpServer.on(String(F("/freemem")).c_str(), HTTP_GET,
			[](AsyncWebServerRequest *request) {
		String s = String(F("Free heap: "));
		s.concat(ESP.getFreeHeap());
		s.concat(F("bytes wifi: "));
		s.concat(WiFi.status());
		request->send(200, FPSTR(_TEXTPLAIN), s);
	}).setAuthentication(update_username, configValues.passwd);

	httpServer.on(String(F("/delete")).c_str(), HTTP_GET,
			[](AsyncWebServerRequest *request) {
				if (!request->authenticate(update_username, configValues.passwd))
					return request->requestAuthentication();
				String f = request->arg("f");
				bool deleted;
				if (f.endsWith(FPSTR(_DOT_DAT)) ||
						f.endsWith(FPSTR(_DOT_DER))) {
					deleted = SPIFFS.remove(_DATA + f);
					if(!deleted){
						deleted = SPIFFS.remove(f);
					}
				} else {
					deleted = SPIFFS.remove(_WWW + f);
				}
				String t;
				t.reserve(500);
				if(deleted){
					t.concat(F("Success deleting "));
					t.concat(f);
				}else{
					t.concat(F("Error deleting "));
					t.concat(f);
				}
				t.concat(FPSTR(_BRHOME));
				request->send(200, FPSTR(_TEXTHTML),
						t);
			}).setAuthentication(update_username, configValues.passwd);
	httpServer.on(String(F("/format")).c_str(), HTTP_GET,
			[](AsyncWebServerRequest *request) {
				if (!request->authenticate(update_username, configValues.passwd))
					return request->requestAuthentication();
				format = true;
				String t;
				t.reserve(500);
				t.concat(F("Format started"));
				t.concat(FPSTR(_BRHOME));
				request->send(200, FPSTR(_TEXTHTML),
						t);
				Serial.println(F("Pedido de formata��o"));
			}).setAuthentication(update_username, configValues.passwd);

	httpServer.on(String(F("/heap")).c_str(), HTTP_GET,
			[](AsyncWebServerRequest *request) {
				if (!request->authenticate(update_username, configValues.passwd))
					return request->requestAuthentication();
				String t;
				t.reserve(500);
				t.concat(ESP.getFreeHeap());
				t.concat(FPSTR(_BRHOME));
				request->send(200, FPSTR(_TEXTHTML),
						t);
			}).setAuthentication(update_username, configValues.passwd);

	//used to force a reset of the device
	httpServer.on(String(F("/reset")).c_str(), HTTP_GET,
			[](AsyncWebServerRequest *request) {
				if (!request->authenticate(update_username, configValues.passwd))
					return request->requestAuthentication();
				request->send(200, FPSTR(_TEXTHTML), F("resetting device. <script>setTimeout(function(){ location.href='/index.html' }, 3000)</script>"));
				reset = true;
			}).setAuthentication(update_username, configValues.passwd);

	httpServer.on(String(F("/resetwifi")).c_str(), HTTP_GET,
			[](AsyncWebServerRequest *request) {
				if (!request->authenticate(update_username, configValues.passwd))
					return request->requestAuthentication();
				request->send(200, FPSTR(_TEXTPLAIN),
						F("resetting wifi settings ..."));
				delay(1000);
				WiFi.disconnect(true);
				ESP.restart();
			}).setAuthentication(update_username, configValues.passwd);

	httpServer.on(String(F("/reloadRfDevicesConfig")).c_str(), HTTP_GET,
			[](AsyncWebServerRequest *request) {
				if (!request->authenticate(update_username, configValues.passwd))
					return request->requestAuthentication();
				bool b =loadButtonActuatorsConfig();
				String s;
				if (b) {
					s.concat(F("Config reloaded"));
				} else {
					s.concat(F("Error reloading config"));
				}
				request->send(200, FPSTR(_TEXTPLAIN), s.c_str());
			}).setAuthentication(update_username, configValues.passwd);
	httpServer.on(String(F("/getLastRfDetected")).c_str(), HTTP_GET,
			[](AsyncWebServerRequest *request) {
				if (!request->authenticate(update_username, configValues.passwd))
					return request->requestAuthentication();
				String s;
				s.concat(rFDeviceControlLastCode);
				s.concat(';');
				s.concat(rFDeviceControlLastLen);
				s.concat(';');
				s.concat(rFDeviceControlLastProto);
				request->send(200, FPSTR(_TEXTPLAIN), s.c_str());
			}).setAuthentication(update_username, configValues.passwd);

	httpServer.on(String(F("/sendRfTest")).c_str(), HTTP_GET,
			[](AsyncWebServerRequest *request) {
				if (!request->authenticate(update_username, configValues.passwd))
					return request->requestAuthentication();

				// GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
				if (request->hasParam(FPSTR(_ARG_PROTOCOL)) && request->hasParam(FPSTR(_ARG_BITLEN)) && request->hasParam(FPSTR(_ARG_CODE))) {
					rfSendingCode = request->getParam(_ARG_CODE)->value().toInt();
					rfSendingProtocol = request->getParam(_ARG_PROTOCOL)->value().toInt();
					rfSendingBitlen = request->getParam(_ARG_BITLEN)->value().toInt();
					rfSendingQtd = 4;
					rfSending = true;
					rfDeviceControlTestSend(rfSendingProtocol, rfSendingBitlen, rfSendingCode);
					rfSending = false;
					String s;
					s.concat(F("Sent to "));
					s.concat(rfSendingCode);
					s.concat(';');
					s.concat(rfSendingProtocol);
					s.concat(';');
					s.concat(rfSendingBitlen);
					request->send(200, FPSTR(_TEXTPLAIN), s.c_str());
				} else {
					String s;
					s.concat(F("Invalid parameters"));
					request->send(400, FPSTR(_TEXTPLAIN), s.c_str());
				}
			}).setAuthentication(update_username, configValues.passwd);
	httpServer.on(String(F("/simulateButton")).c_str(), HTTP_GET,
			[](AsyncWebServerRequest *request) {
				if (!request->authenticate(update_username, configValues.passwd))
					return request->requestAuthentication();

				// GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
				if (request->hasParam(FPSTR(_ARG_BUTTON))) {
					uint8_t button = request->getParam(_ARG_BUTTON)->value().toInt();
					buttonRfPressed(button);
					String s;
					s.concat(F("Simulate button "));
					s.concat(button);
					request->send(200, FPSTR(_TEXTPLAIN), s.c_str());
				} else {
					String s;
					s.concat(F("Invalid parameters"));
					request->send(400, FPSTR(_TEXTPLAIN), s.c_str());
				}
			}).setAuthentication(update_username, configValues.passwd);

	httpServer.on(String(F("/controlActuatorName")).c_str(), HTTP_GET,
			[](AsyncWebServerRequest *request) {
				if (!request->authenticate(update_username, configValues.passwd))
					return request->requestAuthentication();
				// GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
				if (request->hasParam(FPSTR(_ARG_ACTUATOR)) && request->hasParam(FPSTR(_ARG_STATE)) && request->hasParam(FPSTR(_ARG_SENDTOACTUATOR))) {
					String actuator = request->getParam(_ARG_ACTUATOR)->value();
					uint8_t state = request->getParam(_ARG_STATE)->value().toInt();
					bool send = request->getParam(_ARG_SENDTOACTUATOR)->value().toInt() == 1;
					rfDeviceControlSend(actuator.c_str(), state, send);
					String s;
					s.concat(F("Sent to actuator "));
					s.concat(actuator);
					s.concat(';');
					s.concat(state);
					s.concat(';');
					s.concat(send);
					request->send(200, FPSTR(_TEXTPLAIN), s.c_str());
				} else {
					String s;
					s.concat(F("Invalid parameters"));
					request->send(400, FPSTR(_TEXTPLAIN), s.c_str());
				}
			}).setAuthentication(update_username, configValues.passwd);

	httpServer.on(String(F("/controlActuator")).c_str(), HTTP_GET,
			[](AsyncWebServerRequest *request) {
				if (!request->authenticate(update_username, configValues.passwd))
					return request->requestAuthentication();
				// GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
				if (request->hasParam(FPSTR(_ARG_ACTUATOR)) && request->hasParam(FPSTR(_ARG_STATE)) && request->hasParam(FPSTR(_ARG_SENDTOACTUATOR))) {
					uint8_t actuator= request->getParam(_ARG_ACTUATOR)->value().toInt();
					uint8_t state = request->getParam(_ARG_STATE)->value().toInt();
					bool send = request->getParam(_ARG_SENDTOACTUATOR)->value().toInt() == 1;
					rfDeviceControlSend(actuator, state, send);
					String s;
					s.concat(F("Sent to actuator "));
					s.concat(actuator);
					s.concat(';');
					s.concat(state);
					s.concat(';');
					s.concat(send);
					request->send(200, FPSTR(_TEXTPLAIN), s.c_str());
				} else {
					String s;
					s.concat(F("Invalid parameters"));
					request->send(400, FPSTR(_TEXTPLAIN), s.c_str());
				}
			}).setAuthentication(update_username, configValues.passwd);


	httpServer.on(String(F("/fupload")).c_str(), HTTP_GET,
			[](AsyncWebServerRequest *request) {
				if (!request->authenticate(update_username, configValues.passwd))
					return request->requestAuthentication();
				request->send(200, FPSTR(_TEXTHTML), FPSTR(_UPLOADFORM));
			});
	httpServer.on(String(F("/fupload")).c_str(), HTTP_POST,
			[](AsyncWebServerRequest *request) {
				if (!request->authenticate(update_username, configValues.passwd))
					return request->requestAuthentication();
				request->send(200, FPSTR(_TEXTPLAIN), FPSTR(_OK));
			},handleUpload).setAuthentication(update_username, configValues.passwd);

	// Simple Firmware Update Form
	httpServer.on(update_path, HTTP_GET,
			[](AsyncWebServerRequest *request) {
				if (!request->authenticate(update_username, configValues.passwd))
					return request->requestAuthentication();
				request->send(200, FPSTR(_TEXTHTML),
						"<form method='POST' action='/firmware' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form><br><a href=\"/\">home</a>");
			}).setAuthentication(update_username, configValues.passwd);
#ifdef ARDUINO_ARCH_ESP8266
	httpServer.on(update_path, HTTP_POST, [](AsyncWebServerRequest *request) {
		fwupdt = Update.hasError() ? 0 : 2;
		AsyncWebServerResponse *response = request->beginResponse(200,FPSTR(_TEXTHTML) , fwupdt==2?"OK<script>setTimeout(function(){ location='/'; }, 3000);</script>":"FAIL");
		fwupdt = fwupdt==2?3:0;
		response->addHeader("Connection", "close");
		request->send(response);
	}, handle_update_progress_cb);
#endif

	httpServer.on(String(FPSTR(_CONFIGDAT1)).c_str(), HTTP_GET, [](AsyncWebServerRequest *request) {
		if (!request->authenticate(update_username, configValues.passwd))
			return request->requestAuthentication();

		ConfigValues cv;
		memset(&cv, 0, sizeof(ConfigValues));
		cv.version = 1;
		strncpy(cv.nodeName, configValues.nodeName, MAX_NAME_LEN);
		strncpy(cv.userName, configValues.userName, MAX_NAME_LEN);
		strncpy(cv.passwd, configValues.passwd, MAX_NAME_LEN);
		AsyncWebServerResponse *response = request->beginResponse_P(200, _BINARY, (uint8_t*) &cv,sizeof(ConfigValues));
		request->send(response);
	}).setAuthentication(update_username, configValues.passwd);

	httpServer.on(String(FPSTR(_CLOUDDAT1)).c_str(), HTTP_GET, [](AsyncWebServerRequest *request) {
		if (!request->authenticate(update_username, configValues.passwd))
			return request->requestAuthentication();
		request->send(SPIFFS, String(FPSTR(_CLOUDDAT)), _BINARY);
	}).setAuthentication(update_username, configValues.passwd);

	httpServer.on(String(FPSTR(_CONFIG_BUTTONS1)).c_str(), HTTP_GET, [](AsyncWebServerRequest *request) {
		if (!request->authenticate(update_username, configValues.passwd))
			return request->requestAuthentication();
		request->send(SPIFFS, String(FPSTR(_CONFIG_BUTTONS)), _BINARY);
	}).setAuthentication(update_username, configValues.passwd);

	httpServer.on(String(FPSTR(_CONFIG_ATUADORES1)).c_str(), HTTP_GET, [](AsyncWebServerRequest *request) {
		if (!request->authenticate(update_username, configValues.passwd))
			return request->requestAuthentication();
		request->send(SPIFFS, String(FPSTR(_CONFIG_ATUADORES)), _BINARY);
	}).setAuthentication(update_username, configValues.passwd);

	httpServer.on(String(FPSTR(IOT_CERT_CA_PATH)).c_str(), HTTP_GET, [](AsyncWebServerRequest *request) {
		if (!request->authenticate(update_username, configValues.passwd))
			return request->requestAuthentication();
		request->send(SPIFFS, String(FPSTR(IOT_CERT_CA_PATH)), _BINARY);
	}).setAuthentication(update_username, configValues.passwd);

	httpServer.on(String(FPSTR(IOT_CERT_PATH)).c_str(), HTTP_GET, [](AsyncWebServerRequest *request) {
		if (!request->authenticate(update_username, configValues.passwd))
			return request->requestAuthentication();
		request->send(SPIFFS, String(FPSTR(IOT_CERT_PATH)), _BINARY);
	}).setAuthentication(update_username, configValues.passwd);

	httpServer.on(String(FPSTR(IOT_PRIVKEY_PATH)).c_str(), HTTP_GET, [](AsyncWebServerRequest *request) {
		if (!request->authenticate(update_username, configValues.passwd))
			return request->requestAuthentication();
		request->send(SPIFFS, String(FPSTR(IOT_PRIVKEY_PATH)), _BINARY);
	}).setAuthentication(update_username, configValues.passwd);

	httpServer.onNotFound([](AsyncWebServerRequest *request) {
		  if (request->method() == HTTP_OPTIONS) {
		    request->send(200);
		  } else {
			  request->send(404, FPSTR(_TEXTPLAIN), F("404: Not Found")); // otherwise, respond with a 404 (Not Found) error
		  }
	});


	DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Origin"), F("*"));
	//DefaultHeaders::Instance().addHeader(F("Access-Control-Max-Age"), F("600"));
	DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Methods"), F("PUT,POST,GET,OPTIONS"));
	DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Headers"), F("*"));
	DefaultHeaders::Instance().addHeader(F("Access-Control-Expose-Headers"), F("www-authenticate, WWW-Authenticate"));
}
void webBegin() {
#ifdef ARDUINO_ARCH_ESP32
	AsyncElegantOTA.begin(&httpServer); 
#endif
	httpServer.begin(); // a linha abaixo faz isto
}
void verifyReset() {
	if (reset || fwupdt == 3) {
		delay(1000);
		ESP.restart();
	}
}
void verifyFormat() {
	if (format) {
		format = false;
		Serial.println(F("Formatando"));
		bool formatted = SPIFFS.format();
		if(formatted){
			Serial.println(F("Formatado"));
		}else{
			Serial.println(F("Erro formatando"));
		}
	}
}
void webLoop() {
}
