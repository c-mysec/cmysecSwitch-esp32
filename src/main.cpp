#include <Arduino.h>
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#ifdef ESP32
  #include <SPIFFS.h>
#endif
#include "mySwitchConfig.h"
#include "ProgMemConsts.h"
#include "NTPSync.h"
#include "UdpBroadcast.h"
#include "web.h"
#include "DeviceHub.h"
#include "KeyManager.h"
#include "IotCore.h"
bool wifiConnected = false;
//flag for saving data
bool shouldSavePasswordConfig = false;

//callback notifying us of the need to save config
void wm_saveConfigCallback() {
  Serial.println(F("shouldSavePasswordConfig"));
  shouldSavePasswordConfig = true;
}
void setup()
{
  webBuffer1.reserve(__MAX_WEBBUF_LEN);
  webBuffer2.reserve(__MAX_WEBBUF_LEN);
  webBuffer1.concat(F("Init\n"));
  uint32_t chipID = initChipId();
  Serial.begin(115200);
	SPIFFS.begin(true);
  WiFiManager wifiManager;

  WiFiManagerParameter nodeName("nodeName", "Node name", "", MAX_NAME_LEN);
  wifiManager.addParameter(&nodeName);
  WiFiManagerParameter nodeUser("nodeUserName", "Node username", "", MAX_NAME_LEN);
  wifiManager.addParameter(&nodeUser);
  WiFiManagerParameter nodePassword("nodePassword", "Node password", "", MAX_NAME_LEN);
  wifiManager.addParameter(&nodePassword);

  wifiManager.setConfigPortalTimeout(180);
  String ap(FPSTR(_DEVICE_WIFI_AP_NAME));
  ap.concat(chipID);
  wifiConnected = wifiManager.autoConnect(ap.c_str(), "123mySwitch");
  if(!wifiConnected) {
    Serial.println(F("Failed to connect"));
    ESP.restart();
  } 
  else {
    //if you get here you have connected to the WiFi    
    Serial.println(F("connected...yeey :)"));
    if (shouldSavePasswordConfig) {
      savePasswordConfig(nodeName.getValue(), nodeUser.getValue(), nodePassword.getValue());
    }
    loadAllConfigs();
    makeKeys(configValues.userName, configValues.passwd);

    webSetup();
    setupNtp();
    udpSetup();
    deviceHubSetup();
    setupIotCore();
    webBegin();
  }
}

void loop()
{
  delay(100);
  udpLoop();
	verifyReset();
	verifyFormat();
  deviceHubLoop();
  loopIotCore();
	// This is a cheap way to detect memory leaks
	static unsigned long last = millis();
  if (mapButtonRead != NOT_CONFIGURED && ((long)(millis() - mapButtonReadTime)) > 150) {
    buttonRfPressed(mapButtonRead);
    mapButtonRead = NOT_CONFIGURED;
  }
/*
  if (rfSendingQtd > 0 && !rfSending) {
      rfSending = true;
      rfDeviceControlTestSend(rfSendingProtocol, rfSendingBitlen, rfSendingCode);
      rfSendingQtd--;
      rfSending = false;
  }*/
	if (((long)(millis() - last)) > 60000) {
    //testEnc();

    Serial.print(NTPgetMDay());
    Serial.print('/');
    Serial.print(NTPgetMonth());
    Serial.print('/');
    Serial.println(NTPgetYear());

		last = millis();
		Serial.printf(
				String(F("[MAIN] Free heap: %d bytes after %lu, wifi: %d\n")).c_str(),
				ESP.getFreeHeap(), last, WiFi.status());
		// Received 9504756 / 24bit Protocol: 1
		//rfDeviceControlTestSend(1, 24, 9504756);
	}}