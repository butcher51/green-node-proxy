#include "BLEDevice.h"
#include <WiFi.h>
#include <PubSubClient.h>

#include "config.h"

RTC_DATA_ATTR int bootCount = 0;
static int differentMACs = sizeof FLORA_ADDR;

// The remote service we wish to connect to.
static BLEUUID serviceUUID("00001204-0000-1000-8000-00805f9b34fb");

// The characteristic of the remote service we are interested in.
static BLEUUID uuid_version_battery("00001a02-0000-1000-8000-00805f9b34fb");
static BLEUUID uuid_sensor_data("00001a01-0000-1000-8000-00805f9b34fb");
static BLEUUID uuid_write_mode("00001a00-0000-1000-8000-00805f9b34fb");

static int doConnect = 0;
static boolean connected = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;

WiFiClient espClient;
PubSubClient client(espClient);


bool getSensorData(BLEAddress pAddress, bool getBattery, int floraDevice) {

  String topic = outTopic + FLORA_ADDR[floraDevice] + "/";
  
  Serial.print("Forming a connection to Flora device at ");
  Serial.println(pAddress.toString().c_str());

  BLEClient*  pClient  = BLEDevice::createClient();

  // Connect to the remove BLE Server.
  if (!pClient->connect(pAddress)) {
      return false;
  }
  Serial.println(" - Connected to Flora");

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    return false;
  }
  Serial.println(" - Found our service");

  pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_write_mode);
  uint8_t buf[2] = {0xA0, 0x1F};
  pRemoteCharacteristic->writeValue(buf, 2, true);

  delay(500);

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_sensor_data);
  Serial.println(pRemoteService->toString().c_str());
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(uuid_sensor_data.toString().c_str());
    return false;
  }
  Serial.println(" - Found our characteristic");


  // Read the value of the characteristic.
  std::string value = pRemoteCharacteristic->readValue();
  Serial.print("The characteristic value was: ");
  const char *val = value.c_str();

  Serial.print("Hex: ");
  for (int i = 0; i < 16; i++) {
    Serial.print((int)val[i], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");

  float temp = (val[0] + val[1] * 256) / ((float)10.0);
  int moisture = val[7];
  int light = val[3] + val[4] * 256;
  int conductivity = val[8] + val[9] * 256;
  
  char buffer[64];
  
  Serial.print("Temperature: ");
  Serial.println(temp);
  snprintf(buffer, 64, "%f", temp);
  client.publish(setMQTTTopic("temperature", topic), buffer);
  

  Serial.print("Moisture: ");
  Serial.println(moisture);
  snprintf(buffer, 64, "%d", moisture);
  client.publish(setMQTTTopic("moisture", topic), buffer);

  Serial.print("Light: ");
  Serial.println(light);
  snprintf(buffer, 64, "%d", light);
  client.publish(setMQTTTopic("light", topic), buffer);

  Serial.print("Conductivity: ");
  Serial.println(conductivity);
  snprintf(buffer, 64, "%d", conductivity);
  client.publish(setMQTTTopic("conductivity", topic), buffer);

  if (getBattery) {
    Serial.println("Trying to retrieve battery level...");
    pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_version_battery);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(uuid_sensor_data.toString().c_str()); 
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic...
    value = pRemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    const char *val2 = value.c_str();
    Serial.print("Hex: ");
    for (int i = 0; i < 16; i++) {
      Serial.print((int)val2[i], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");

    int battery = val2[0];
    Serial.print("Battery: ");
    Serial.println(battery);
    snprintf(buffer, 64, "%d", battery);
    client.publish(setMQTTTopic("battery", topic), buffer);
  }

  pClient->disconnect();

}

void taskDeepSleep( void * parameter )
{
  delay(SLEEP_WAIT * 1000);
  Serial.println("Going to sleep now.");
  delay(1000);
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION * 1000000);  
  esp_deep_sleep_start();
}


void setup() {
  Serial.begin(115200);
  delay(1000);
  
  xTaskCreate(      taskDeepSleep,          /* Task function. */
                    "TaskDeepSleep",        /* String with name of task. */
                    10000,                  /* Stack size in words. */
                    NULL,                   /* Parameter passed as input of the task */
                    1,                      /* Priority of the task. */
                    NULL);                  /* Task handle. */
  
  
  Serial.println("Starting Flora client...");
  BLEDevice::init("");

  Serial.println("Connecting WiFi...");
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");

  client.setServer(MQTT_SERVER, 1883);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENTID)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }

  delay(1000);

/*
  for(int i=0;i<differentMACs;i++){
    BLEAddress floraAddress(FLORA_ADDR[i]);
    getSensorData(floraAddress, ((bootCount % BATTERY_INTERVAL) == 0), i);
    delay(1000);
  }
*/
  int i = bootCount % 5;
  BLEAddress floraAddress(FLORA_ADDR[i]);
  getSensorData(floraAddress, ((bootCount % BATTERY_INTERVAL) == 0), i);
  
  bootCount++;
} // End of setup.


// This is the Arduino main loop function.
void loop() {
  delay(10000);
} // End of loop


char message_buff[150];
char* setMQTTTopic(char* var, String topicBase) {
  String myBigArray = "";
  myBigArray = topicBase + var;
  myBigArray.toCharArray(message_buff, myBigArray.length() + 1);
  return message_buff;
}

