//Hostname of ESP8266
#define WIFI_HOSTNAME "SamilLogger-1"

// InverterName
#define INVERTER_NAME "House_PV_Inverter"

//Wifi SSID to connect to
#define WIFI_SSID "Mulberry_IOT"

//Passowrd for WIFI
#define WIFI_PASSWORD "tUPjyANBsn2p"

// Discovery topic for HomeAssistant
#define HA_DISCOVERY_TOPIC "homeassistant/sensor"

// State topic for values
#define HA_STATE_TOPIC_ROOT "samil"

//set the mqqt host name or ip address to your mqqt host. Leave empty to disable mqtt.
#define MQTT_HOST_NAME  "192.168.1.174"

//mqtt port for the above host
#define MQTT_PORT       1883

//if authentication is enabled for mqtt, set the username below. Leave empty to disable authentication
#define MQTT_USER_NAME  "hass"

//password for above user
#define MQTT_PASSWORD   "6WcEu&e3gmCKY3Dz"

//update interval for fast changing values in milliseconds for mqtt
#define MQTT_QUICK_UPDATE_INTERVAL  1000

//update interval for slow changing values in milliseconds for mqtt
#define MQTT_REGULAR_UPDATE_INTERVAL  120000

//set to your pvoutput api key (must have write rights). Leave empty to disable pvoutput publishing
#define PVOUTPUT_API_KEY  ""

//set to the pvoutput system id to update. Only the first inverter is supported for now
#define PVOUTPUT_SYSTEM_ID  ""

//how long between pvoutput updates. Pvoutput specifies a minimum of 5 minutes (5*60*1000)
#define PVOUTPUT_UPDATE_INTERVAL   5 * 60 * 1000

//timezone offset in hours (amsterdam is +2 in summertime). Used for posting to pvoutput
#define TIMEZONE 2

//rs485 receive pin
#define RS485_RX D1

//rs485 transmit pin
#define RS485_TX D2
