char* FLORA_ADDR[]={"C4:7C:8D:64:35:C1", "C4:7C:8D:62:D2:86","C4:7C:8D:62:6A:39","C4:7C:8D:62:75:B9","C4:7C:8D:64:2D:C6"}; //1-2-3-4-5 array of different xiaomi flora MAC addresses

String outTopic = "flora/"; //MQTT base topic - data gets published to "basetopic/MAC_address/property", where MAC_address is one of the values from FLORA_ADDR array, property is either temperature, moisture, conductivity, light or battery
 
#define SLEEP_DURATION 240    // duration of sleep between flora connection attempts in seconds
#define SLEEP_WAIT 60         // time until esp32 is put into deep sleep mode. must be sufficient to connect to wlan, connect to xiaomi flora device & push measurement data to MQTT

#define BATTERY_INTERVAL 12   // Retrieve battery level every n wakeups

const char* wifi_ssid     = "Drakula";
const char* wifi_password = "drazsimazsi95";

const char* MQTT_SERVER = "192.168.1.5";
const char* MQTT_CLIENTID = "miflora-client";

