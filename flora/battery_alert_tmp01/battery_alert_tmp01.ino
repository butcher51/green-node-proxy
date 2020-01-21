#include "BLEDevice.h"
#include <WiFi.h>
#include <PubSubClient.h>

#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>


#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

#include "config.h"

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  
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

} // End of setup.


// This is the Arduino main loop function.
void loop() {
  
  float temp = bme.readTemperature();

  String topic = "flora/C4:7C:8D:62:D2:86/";

  char buffer[64];
  Serial.print("Temperature: ");
  Serial.println(temp);
  snprintf(buffer, 64, "%f", temp);
  client.publish(setMQTTTopic("temperature", topic), buffer);
  
  float conductivity = bme.readHumidity();
  
  Serial.print("Conductivity: ");
  Serial.println(conductivity);
  snprintf(buffer, 64, "%f", conductivity);
  client.publish(setMQTTTopic("conductivity", topic), buffer);
   
  delay(5000);
} // End of loop


char message_buff[150];
char* setMQTTTopic(char* var, String topicBase) {
  String myBigArray = "";
  myBigArray = topicBase + var;
  myBigArray.toCharArray(message_buff, myBigArray.length() + 1);
  return message_buff;
}
