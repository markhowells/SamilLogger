#include <Arduino.h>


#include <TimeLib.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "SamilCommunicator.h"
#include "SettingsManager.h"
#include "MQTTPublisher.h"
#include "HAPublisher.h"
#include "PVOutputPublisher.h"
#include "ESP8266mDNS.h"
#include <MqttLogger.h>
#include "Settings.h"			//change and then rename Settings.example.h to Settings.h to compile


SettingsManager settingsManager;
SamilCommunicator samilComms(&settingsManager, true);
MQTTPublisher mqqtPublisher(&settingsManager, &samilComms, true);
HAPublisher haPublisher(&settingsManager);
PVOutputPublisher pvoutputPublisher(&settingsManager, &samilComms, true);
WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "pool.ntp.org");
bool validTimeSet =false;
MqttLogger logger(mqqtPublisher.getClient(),"mqttlogger/log");

String getMacAddress(void) {
    uint8_t mac[6];
	char macStr[13];
    wifi_get_macaddr(STATION_IF, mac);
    sprintf(macStr, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return String (macStr);
}

void setup()
{
	//debug settings
	auto settings = settingsManager.GetSettings();
	//set settings from heade file
	settings->mqttHostName = MQTT_HOST_NAME;	
	settings->mqttPort = MQTT_PORT;	
	settings->mqttUserName = MQTT_USER_NAME;
	settings->mqttPassword = MQTT_PASSWORD;	
	settings->mqttQuickUpdateInterval = MQTT_QUICK_UPDATE_INTERVAL;	
	settings->mqttRegularUpdateInterval = MQTT_REGULAR_UPDATE_INTERVAL;	
	settings->pvoutputApiKey = PVOUTPUT_API_KEY;	
	settings->pvoutputSystemId = PVOUTPUT_SYSTEM_ID;	
	settings->pvoutputUpdateInterval = PVOUTPUT_UPDATE_INTERVAL;	
	settings->wifiHostname = WIFI_HOSTNAME;
	settings->wifiSSID = WIFI_SSID;
	settings->wifiPassword = WIFI_PASSWORD;
	settings->timezone = TIMEZONE;
	settings->RS485Rx = RS485_RX;
	settings->RS485Tx = RS485_TX;
	settings->haDiscoveryTopic = HA_DISCOVERY_TOPIC;
	settings->inverterName = INVERTER_NAME;
	settings->haStateTopicRoot = HA_STATE_TOPIC_ROOT;


	//Init our compononents
	Serial.begin(9600);
	Serial.println("Booting");
	WiFi.mode(WIFI_STA);
	WiFi.hostname(settings->wifiHostname.c_str());
	WiFi.begin(settings->wifiSSID.c_str(), settings->wifiPassword.c_str());

	Serial.print("Connecting to WiFi");
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("Connected!");

	timeClient.begin();

	ArduinoOTA.setHostname("settings->wifiHostname.c_str()");

	ArduinoOTA.onStart([]() {
		Serial.println("Start Ota");
	});
	ArduinoOTA.onEnd([]() {
		Serial.println("\nEnd Ota");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("OTA Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("End Failed");
	});
	ArduinoOTA.begin();
	Serial.println("Ready");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

	String mac=getMacAddress();
	settings->unique_id = mac;
	
	mqqtPublisher.start();
logger.println("Hello World\n");

	//ntp client
	samilComms.start(logger);


	Serial.println("Registering MQTT device");
	if (!haPublisher.HARegister(mqqtPublisher)) {
		Serial.println("Failed to register device with MQTT server");
	} else {
		Serial.println("Device registered at HA MQTT");
	}

	validTimeSet = timeClient.update();
	timeClient.setTimeOffset(settings->timezone * 60 * 60);
}


void loop()
{
	//update the time if set correctly
	auto timeUpdate = timeClient.update();

	//if the time is updated, we need to set it in the time lib so we can use the helper functions from timelib
	if (timeUpdate)
	{
		if (!validTimeSet)
			validTimeSet = true; //pvoutput is started after the time is set
	
		//sync time to time lib
		setTime(timeClient.getEpochTime());
	}

	ArduinoOTA.handle();
	yield();
	samilComms.handle();
	yield();
	mqqtPublisher.handle();
	yield();
	//start the pvoutput publisher after the time has been set if it is configured to start
	if (validTimeSet && pvoutputPublisher.canStart() && !pvoutputPublisher.getIsStarted())
		pvoutputPublisher.start();

	pvoutputPublisher.handle();
	yield(); //prevent wathcdog timeout
}

