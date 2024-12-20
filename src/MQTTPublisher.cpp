#include "MQTTPublisher.h"

WiFiClient espClient;
PubSubClient client(espClient);
MqttLogger mqttLogger(client,"mqttlogger/log",MqttLoggerMode::MqttOnly);

MQTTPublisher::MQTTPublisher(SettingsManager * settingsManager, SamilCommunicator * samiL, bool inDebugMode)
{
	randomSeed(micros());
	mqttSettingsManager = settingsManager;
	samilCommunicator = samiL;
	debugMode = inDebugMode;
}

MQTTPublisher::~MQTTPublisher()
{
	client.publish("samil", "offline");
	client.disconnect();
}


bool MQTTPublisher::reconnect()
{
	lastConnectionAttempt = millis();
	if (debugMode)
	{
		Serial.print("Attempting MQTT connection to server: ");
		Serial.print(mqttSettings->mqttHostName);
		Serial.println("...");
	}

	// Create a random client ID
	String clientId = "SamilLogger-";
	clientId += String(random(0xffff), HEX);

	// Attempt to connect
	bool clientConnected;
	if (mqttSettings->mqttUserName.length())
	{
		Serial.println("Using user credientials for authentication.");
		clientConnected = client.connect(clientId.c_str(), mqttSettings->mqttUserName.c_str(), mqttSettings->mqttPassword.c_str());
	}
	else
	{
		Serial.println("Connecting without user credentials.");
		clientConnected = client.connect(clientId.c_str());
	}
		
	if (clientConnected)
	{
		if (debugMode)
			Serial.println("connected");
		// Once connected, publish an announcement...
		client.publish("samil", "online");

		return true;
	}
	else {
		if (debugMode)
		{
			Serial.print("failed, rc=");
			Serial.print(client.state());
		}
	}

	return false;
}


void MQTTPublisher::start()
{
	mqttSettings = mqttSettingsManager->GetSettings();
	if (mqttSettings->mqttHostName.length() == 0 || mqttSettings->mqttPort == 0)
	{
		Serial.println("MQTT disabled. No hostname or port set.");
		return; //not configured
	}
	Serial.println("MQTT enabled. Connecting.");
	client.setServer(mqttSettings->mqttHostName.c_str(), mqttSettings->mqttPort);
	client.setBufferSize(1024);
	reconnect(); //connect right away
	isStarted = true;
}

void MQTTPublisher::stop()
{
	isStarted = false;
}

void MQTTPublisher::handle()
{
	if (!isStarted)
		return;

	if (!client.connected() && millis() - lastConnectionAttempt > RECONNECT_TIMEOUT) {
		if (!reconnect()) return;
	}
	//got a valid mqtt connection. Loop through the inverts and send out the data if needed
	client.loop();

	bool sendRegular = millis() - lastSentRegularUpdate > mqttSettings->mqttRegularUpdateInterval;
	bool sendQuick = millis() - lastSentQuickUpdate > mqttSettings->mqttQuickUpdateInterval;

	if (sendRegular || sendQuick)
	{
		bool sendOk = true; //if a mqtt message fails, wait for retransmit at a later time
		auto inverters = samilCommunicator->getInvertersInfo();
		for (char cnt = 0; cnt < inverters.size(); cnt++)
		{
			if (inverters[cnt].addressConfirmed)
			{
				auto prependTopic = (String("samil/") + String(inverters[cnt].serialNumber));
				if (debugMode)
				{
					Serial.print("Publishing prepend topic for this inverter is: ");
					Serial.println(prependTopic);
				}

				if (inverters[cnt].isOnline)
				{
					if (sendQuick)
					{
						//send out fast changing values
						if (sendOk) sendOk = publishOnMQTT(prependTopic, "/online", "1");
						if (sendOk) sendOk = publishOnMQTT(prependTopic, "/vpv1", String(inverters[cnt].vpv1, 1));
						if (sendOk) sendOk = publishOnMQTT(prependTopic, "/vpv2", String(inverters[cnt].vpv2, 1));
						if (sendOk) sendOk = publishOnMQTT(prependTopic, "/ipv1", String(inverters[cnt].ipv1, 1));
						if (sendOk) sendOk = publishOnMQTT(prependTopic, "/ipv2", String(inverters[cnt].ipv2, 1));
						//publishing sometimes cuases the wdt to reset the ESP. 
						//On the github page of the pubsubclient it was suggested to add extra client.loop().
						client.loop();
						if (sendOk) sendOk = publishOnMQTT(prependTopic, "/vac1", String(inverters[cnt].vac1, 1));
						if (sendOk) sendOk = publishOnMQTT(prependTopic, "/iac1", String(inverters[cnt].iac1, 1));
						if (sendOk) sendOk = publishOnMQTT(prependTopic, "/fac1", String(inverters[cnt].fac1, 2));
						if (sendOk) sendOk = publishOnMQTT(prependTopic, "/pac", String(inverters[cnt].pac));
						if (sendOk) sendOk = publishOnMQTT(prependTopic, "/temp", String(inverters[cnt].temp, 1));

						if (inverters[cnt].isDTSeries)
						{
							//On the github page of the pubsubclient it was suggested to add extra client.loop().
							client.loop();
							//also send tri fase info
							if (sendOk) sendOk = publishOnMQTT(prependTopic, "/vac2", String(inverters[cnt].vac2, 1));
							if (sendOk) sendOk = publishOnMQTT(prependTopic, "/iac2", String(inverters[cnt].iac2, 1));
							if (sendOk) sendOk = publishOnMQTT(prependTopic, "/fac2", String(inverters[cnt].fac2, 2));
							if (sendOk) sendOk = publishOnMQTT(prependTopic, "/vac3", String(inverters[cnt].vac3, 1));
							if (sendOk) sendOk = publishOnMQTT(prependTopic, "/iac3", String(inverters[cnt].iac3, 1));
							if (sendOk) sendOk = publishOnMQTT(prependTopic, "/fac3", String(inverters[cnt].fac3, 2));
						}
					}
					else
					{
						//regular
						if (sendOk) sendOk = publishOnMQTT(prependTopic, "/workmode", String(inverters[cnt].workMode));
						if (sendOk) sendOk = publishOnMQTT(prependTopic, "/eday", String(inverters[cnt].eDay));
						//TODO: Rest of data 
					}
				}
				else if (sendRegular) //only send offline info on regular basis
					if (sendOk) sendOk = publishOnMQTT(prependTopic, "/online", "0");
			}

			//On the github page of the pubsubclient it was suggested to add extra client.loop().
			client.loop();
		}

		if (sendQuick)
			lastSentQuickUpdate = millis();
		if (sendRegular)
			lastSentRegularUpdate = millis();

		if (debugMode)
		{
			Serial.print("MQTT send status: ");
			Serial.println(sendOk);
		}
	}
}

PubSubClient & MQTTPublisher::getClient()
{
    return client;
}


    bool MQTTPublisher::publishOnMQTT(String prepend, String topic, String value)
	{
		String fullTopic = (prepend.c_str() + topic).c_str();
		
		auto retVal =  this->publish(fullTopic, value);
		return retVal;
	}

	bool MQTTPublisher::publish(String topic, String data) {
		auto retVal =  client.publish(topic.c_str(), data.c_str());
		yield();
		return retVal;

	}

