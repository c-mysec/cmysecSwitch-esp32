/*
 * ProgMemConsts.cpp
 *
 *  Created on: Jan 30, 2022
 *      Author: clovis
 */

#include "ProgMemConsts.h"

char _DEVICE_WIFI_AP_NAME[] PROGMEM = "MySwitch-";

char _TEXTPLAIN[] PROGMEM = "text/plain";
char _TEXTHTML[] PROGMEM = "text/html";
char _TEXTCSS[] PROGMEM = "text/css";

char _JAVASCRIPT[] PROGMEM = "application/javascript";
char _BINARY[] PROGMEM = "binary/octet-stream";
char _IMAGEICON[] PROGMEM = "image/x-icon";
char _APPGZIP[] PROGMEM = "application/x-gzip";

char _DOT_DAT[] PROGMEM = ".dat";
char _DOT_DER[] PROGMEM = ".der";
char _DOT_HMTL[] PROGMEM = ".html";
char _DOT_CSS[] PROGMEM = ".css";
char _DOT_JS[] PROGMEM = ".js";
char _DOT_ICO[] PROGMEM = ".ico";
char _DOT_GZ[] PROGMEM = ".gz";

char _CONFIG_DAT[] PROGMEM = "config.dat";

char _OK[] PROGMEM = "Ok";
char _CONNECTED[] PROGMEM = " connected ";
char _FAILED[] PROGMEM = " failed ";
char _SUCCESS[] PROGMEM = " success ";

char _BRHOME[] PROGMEM = "<br><a href=\"/\">home</a>";
char _UPLOADFORM[] PROGMEM
		= "<form method=\"POST\" action=\"/fupload\" enctype=\"multipart/form-data\"><input type=\"file\" name=\"data\" /><input type=\"submit\" name=\"upload\" value=\"Upload\" title=\"Upload Files\"></form><br><a href=\"/\">home</a>";
char _RELAYNAME[] PROGMEM = "relayName";
char _BUTTONNUM[] PROGMEM = "buttonNum";
char _STATE[] PROGMEM = "state";

// N�o pode ser progmem
char _R[] = "r";
char _W[] = "w";
char _WWW[] = "/www/";
char _DATA[] = "/data/";
char _SLASH[] = "/";

char _ARG_PROTOCOL[] PROGMEM = "protocol";
char _ARG_BITLEN[] PROGMEM = "bitlen";
char _ARG_CODE[] PROGMEM = "code";

char _ARG_BUTTON[] PROGMEM = "button";
char _ARG_ACTUATOR[] PROGMEM = "actuator";
char _ARG_STATE[] PROGMEM = "state";
char _ARG_SENDTOACTUATOR[] PROGMEM = "sendtoactuator";

char _MQTT_RECCONECT_ATTEMPT[] PROGMEM = "Attempting MQTT connection...";
char _MQTT_FAILED_CONN[] PROGMEM = "failed, rc=";
char _MQTT_TO_OPEN_CERT[] PROGMEM = "to open cert file";
char _MQTT_TO_OPEN_KEY[] PROGMEM = "to open private key file";
char _MQTT_TO_OPEN_CACERT[] PROGMEM = "to open ca file";

char _LOADED[] PROGMEM = "loaded ...";
char _NOT_LOADED[] PROGMEM = "not loaded ...";
