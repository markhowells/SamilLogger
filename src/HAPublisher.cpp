#include "HAPublisher.h"

HAPublisher::HAPublisher(SettingsManager * settingsManager)
{
    settings = settingsManager;
};

bool HAPublisher::HARegister(MQTTPublisher mqtt)
{
    SettingsManager::Settings *s = settings->GetSettings();
    
    DynamicJsonDocument discoverydoc(1024);
    DynamicJsonDocument devicedoc(512);
    DynamicJsonDocument origindoc(512);
    char buffer[1024];

    devicedoc["ids"]=s->unique_id;
    devicedoc["name"]=s->inverterName;
    devicedoc["mf"]="Howells";
    devicedoc["sw"]="0.0";
    devicedoc["hw"]="0.0rev 1";

    origindoc["name"]="blamqtt";
    origindoc["url"]="https://www.howells.org";
    origindoc["sw"]="00.00";

    discoverydoc["device"] = devicedoc;
    discoverydoc["origin"] = origindoc;
    discoverydoc["device_class"] = "power";
    discoverydoc["unit_of_measurement"]="Watts";
    discoverydoc["name"] = s->inverterName + " Power";
    discoverydoc["state_topic"] = s->haStateTopicRoot+"/power";
    discoverydoc["unique_id"] = s->unique_id;
    discoverydoc["frc_upd"] = true;
    discoverydoc["val_tpl"] = "{{ value_json.power|default(100) }}";

    serializeJson(discoverydoc, buffer);
Serial.printf("Data is %d bytes\n", strlen(buffer));
Serial.println(String("About to publish on ")+(s->haDiscoveryTopic+"/"+s->inverterName+"/config"));
Serial.println(String("Data published:")+"hello World");

    auto retVal =  mqtt.publish((s->haDiscoveryTopic+"/"+s->inverterName+"/config").c_str(),buffer);
		yield();
		return retVal;
}
