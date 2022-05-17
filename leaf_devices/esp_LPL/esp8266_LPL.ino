

// Linux for Embedded Objects 1
// University of Southern Denmark

// 2022-03-24, Kjeld Jensen, First version

// Configuration
#define WIFI_SSID       "LEO1_TEAM_06"
#define WIFI_PASSWORD    "embeddedlinux"

#define MQTT_SERVER      "192.168.10.1"
#define MQTT_SERVERPORT  1883 
#define MQTT_USERNAME    "my_user"
#define MQTT_KEY         "bendevictor"
#define MQTT_TOPIC        "mqtt"
#define VOLTAGE_TOPIC        "volt"
#define CO2_TOPIC       "/co2"  
#define TEMPERATURE_TOPIC    "/temperature"
#define HUMIDITY_TOPIC    "/humidity"
//servo
#include <Servo.h>


Servo myservo;  // create servo object to control a servo

//MotorPin
int motorPin = 12; // for ESP8266

// wifi
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
ESP8266WiFiMulti WiFiMulti;
const uint32_t conn_tout_ms = 5000;

// counter
#define GPIO_INTERRUPT_PIN 4
#define DEBOUNCE_TIME 100 
volatile unsigned long count_prev_time;
volatile unsigned long count;

// mqtt
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
WiFiClient wifi_client;
Adafruit_MQTT_Client mqtt(&wifi_client, MQTT_SERVER, MQTT_SERVERPORT, MQTT_USERNAME, MQTT_KEY);

//subscribes
const char ONOFF_FEED[] PROGMEM = TEMPERATURE_TOPIC;
Adafruit_MQTT_Subscribe temp_sub = Adafruit_MQTT_Subscribe(&mqtt, "tempActuator");
Adafruit_MQTT_Subscribe co2_sub = Adafruit_MQTT_Subscribe(&mqtt, "windowActuator");


//Adafruit_MQTT_Publish count_mqtt_publish = Adafruit_MQTT_Publish(&mqtt, MQTT_TOPIC);
//Adafruit_MQTT_Publish voltage_mqtt_publish = Adafruit_MQTT_Publish(&mqtt, VOLTAGE_TOPIC);

// Sensor
#include <Arduino.h>
#include <SensirionI2CScd4x.h>
#include <Wire.h>
SensirionI2CScd4x scd4x;

// publish
#define PUBLISH_INTERVAL 2000
unsigned long prev_post_time;

// debug
#define DEBUG_INTERVAL 2000
unsigned long prev_debug_time;

ICACHE_RAM_ATTR void count_isr()
{
  if (count_prev_time + DEBOUNCE_TIME < millis() || count_prev_time > millis())
  {
    count_prev_time = millis(); 
    count++;
  }
}

// Used by sensor
void printUint16Hex(uint16_t value) {
    Serial.print(value < 4096 ? "0" : "");
    Serial.print(value < 256 ? "0" : "");
    Serial.print(value < 16 ? "0" : "");
    Serial.print(value, HEX);
}

// Used by sensor
void printSerialNumber(uint16_t serial0, uint16_t serial1, uint16_t serial2) {
    Serial.print("Serial: 0x");
    printUint16Hex(serial0);
    printUint16Hex(serial1);
    printUint16Hex(serial2);
    Serial.println();
}

void debug(const char *s)
{
  Serial.print (millis());
  Serial.print (" ");
  Serial.println(s);
}

void mqtt_connect()
{
  int8_t ret;

  // Stop if already connected.
  if (! mqtt.connected())
  {
    debug("Connecting to MQTT... ");
    while ((ret = mqtt.connect()) != 0)
    { // connect will return 0 for connected
         Serial.println(mqtt.connectErrorString(ret));
         debug("Retrying MQTT connection in 5 seconds...");
         mqtt.disconnect();
         delay(5000);  // wait 5 seconds
    }
    debug("MQTT Connected");
  }
}

void print_wifi_status()
{
  Serial.print (millis());
  Serial.print(" WiFi connected: ");
  Serial.print(WiFi.SSID());
  Serial.print(" ");
  Serial.print(WiFi.localIP());
  Serial.print(" RSSI: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
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

    Serial.println("Waiting for first measurement... (5 sec)");
}



void setupServo(){
  
   myservo.attach(15);  // attaches the servo on GIO2 to the servo object
}

void setupMotor(){
  pinMode(motorPin, OUTPUT);
}

void setup()
{
  // count
  count_prev_time = millis();
  count = 0;
  pinMode(GPIO_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(GPIO_INTERRUPT_PIN), count_isr, RISING);

  // serial
  Serial.begin(115200);
  while (!Serial) {
        delay(100);
  }
  delay(10);
  debug("Boot");
  
  connectToWifi();
  
  setupSensor();
  setupServo();
  setupMotor();
  mqtt.subscribe(&temp_sub);
  mqtt.subscribe(&co2_sub);
}

void connectToWifi(){
  // wifi
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  if(WiFiMulti.run(conn_tout_ms) == WL_CONNECTED)
  {
    print_wifi_status();
  }
  else
  {
    debug("Unable to connect");
  }
  
}

void publish_data(const char* topic, const char* content){
  if((WiFiMulti.run(conn_tout_ms) == WL_CONNECTED))
  {
    print_wifi_status();

    mqtt_connect();
    char charBuf[50];
    Adafruit_MQTT_Publish publish_topic = Adafruit_MQTT_Publish(&mqtt, topic);
    Serial.println("connect success");
    if (! publish_topic.publish(content))
    {
      debug("MQTT failed");
    }
    else
    {
      debug("MQTT ok");
    }
  }
}



void loop()
{
    readSensorValues();
    debugHearthbeat();
    recievedMessage();
    ESP.deepSleep(30e6);
}




  
void debugHearthbeat(){
  if (millis() - prev_debug_time >= DEBUG_INTERVAL)
    {
      prev_debug_time = millis();
      Serial.print(millis());
      Serial.print(" ");
      Serial.println(count);
    }
}

void readSensorValues(){
  if (millis() - prev_post_time >= PUBLISH_INTERVAL)
    {
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
        publish_data("co2", String(co2).c_str());
        publish_data("humidity", String(humidity).c_str());
        publish_data("temp", String(temperature).c_str());
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

void recievedMessage(){
   Adafruit_MQTT_Subscribe *subscription;
   while ((subscription = mqtt.readSubscription(15000))) {
    if (subscription == &temp_sub) {
      int duty_cycle = atoi((char *)temp_sub.lastread);
      analogWrite(motorPin, duty_cycle);
      
    } else if (subscription == &co2_sub){
        Serial.print("received the following from co2 topic: ");
        Serial.println((char *)co2_sub.lastread);
        String co2_message = (char *)co2_sub.lastread;
        
        if(co2_message == "open"){ //set the value so it fits
          int pos;
          myservo.write(180);
        }
        else{
          myservo.write(0);
        }
    }

  }
}
