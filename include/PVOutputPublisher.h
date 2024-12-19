#pragma once
#include <Arduino.h>
#include "TimeLib.h"
#include <ESP8266WiFi.h>
#include "SettingsManager.h"
#include "SamilCommunicator.h"
#include "ESP8266HTTPClient.h"

#define MAX_EDAY_DIFF 100.0f

class PVOutputPublisher
{
public:
	PVOutputPublisher(SettingsManager * settingsManager, SamilCommunicator *samiL, bool inDebugMode = false);
	~PVOutputPublisher();

	void start();
	void stop();
	bool canStart();
	bool getIsStarted();
	void sendToPvOutput(SamilCommunicator::SamilInverterInformation info);

	void handle();

	void ResetAverage();

private:
	SettingsManager::Settings * pvoutputSettings;
	SettingsManager * pvOutputSettingsManager;
	SamilCommunicator * samilCommunicator;
	bool debugMode;
	unsigned long lastUpdated;
	bool isStarted = false;	 
	unsigned long currentPacSum = 0;
	unsigned int lastPac = 0;
	float lastVoltage = 0;
	double currentVoltageSum = 0;
	float lastTemp;
	double currentTemp = 0;
	double currentTempSum = 0;
	unsigned long avgCounter = 0;
	bool wasOnline = false;
	float prevEday = 0.0f;
	String getZeroFilled(int num);
};

