#include <PubSubClient.h>
#include <analogWrite.h>
#include <WiFi.h>
#include <SensirionI2CScd4x.h>
#include <Wire.h>
SensirionI2CScd4x scd4x;
// publish
#define PUBLISH_INTERVAL 2000

unsigned long prev_post_time;
// network credentials (STATION)
const char* ssid = "LEO1_TEAM_06";
const char* password = "embeddedlinux";
boolean ActuatorActive = "false";
//MQTT
const char* mqtt_server =  "192.168.10.1";
WiFiClient espClient;
PubSubClient client(espClient);
#define topicCo2 "co2LPL"
#define topicHumidity "humidityLPL"
#define topicTemp "tempLPL"

boolean activeActuator = "False";

long int t1 = millis();
#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32", "my_user", "bendevictor")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("pumpActuator");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  String msg = "";
  for(int i = 0; i< length; i++){
    msg = msg+(char)message[i];
  }
  char* pumpTopic = "pumpActuator";
}
void setup() {
  Serial.begin(115200);
  initWiFi();
  setupSensor();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}


void setupSensor(){
  Wire.begin();

    uint16_t error;
    char errorMessage[256];

    scd4x.begin(Wire);

    // stop potentially previously started measurement
    error = scd4x.stopPeriodicMeasurement();
    if (error) {
        Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }

    uint16_t serial0;
    uint16_t serial1;
    uint16_t serial2;
    error = scd4x.getSerialNumber(serial0, serial1, serial2);
    if (error) {
        Serial.print("Error trying to execute getSerialNumber(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        printSerialNumber(serial0, serial1, serial2);
    }

    // Start Measurement
    error = scd4x.startPeriodicMeasurement();
    if (error) {
        Serial.print("Error trying to execute startPeriodicMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }

    Serial.println("Waiting for firs t measurement... (5 sec)");
}

void printSerialNumber(uint16_t serial0, uint16_t serial1, uint16_t serial2) {
    Serial.print("Serial: 0x");
    printUint16Hex(serial0);
    printUint16Hex(serial1);
    printUint16Hex(serial2);
    Serial.println();
}
void printUint16Hex(uint16_t value) {
    Serial.print(value < 4096 ? "0" : "");
    Serial.print(value < 256 ? "0" : "");
    Serial.print(value < 16 ? "0" : "");
    Serial.print(value, HEX);
}

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  readSensorValues();
}


void readSensorValues(){
  if (millis() - prev_post_time >= PUBLISH_INTERVAL){
      prev_post_time = millis();
      uint16_t co2;
      uint16_t error;
      char errorMessage[256];
      float temperature;
      float humidity;
      error = scd4x.readMeasurement(co2, temperature, humidity);
    if (error) {
        Serial.print("Error trying to execute readMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else if (co2 == 0) {
        Serial.println("Invalid sample detected, skipping.");
    } else {
        char co2_char = co2;
        char temp_char = temperature;
        char hum_char = humidity;
        client.publish(topicCo2, String(co2).c_str());
        client.publish(topicHumidity, String(humidity).c_str());
        client.publish(topicTemp, String(temperature).c_str());
        Serial.print("Co2:");
        Serial.print(co2);
        Serial.print("\t");
        Serial.print("Temperature:");
        Serial.print(temperature);
        Serial.print("\t");
        Serial.print("Humidity:");
        Serial.println(humidity);
    }
    
  }
  
}
