#ifndef   MQTT_SOCKET_TIMEOUT
#define MQTT_SOCKET_TIMEOUT 5
#endif
#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "SettingsManager.h"
#include "SamilCommunicator.h"
#include <vector>
#include "PubSubClient.h"
#include "WiFiClient.h"

#define RECONNECT_TIMEOUT 15000


class MQTTPublisher
{
private:
	SettingsManager::Settings * mqttSettings;
	SettingsManager * mqttSettingsManager;
	SamilCommunicator * samilCommunicator;
	bool debugMode;
	bool isStarted;

	unsigned long lastConnectionAttempt = 0;		//last reconnect
	unsigned long lastSentQuickUpdate = 0;			//last update of the fast changing info
	unsigned long lastSentRegularUpdate = 0;		//last update of the regular update info

	bool publishOnMQTT(String prepend, String topic, String value);
	bool reconnect();
public:
	MQTTPublisher(SettingsManager * settingsManager, SamilCommunicator *samiL, bool inDebugMode = false);
	~MQTTPublisher();

	bool publish(String topic, String data);

	void start();
	void stop();
	
	void handle();

	PubSubClient & getClient();
};
