#pragma once
#include <Arduino.h>
#include "SettingsManager.h"
#include "MQTTPublisher.h"
#include <ArduinoJson.h>

class HAPublisher
{   
private:
    /* data */
    SettingsManager * settings;

public:
    HAPublisher(SettingsManager * settingsManager);
    bool HARegister(MQTTPublisher mqtt);
    // ~HAPublisher();
};

