

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

// publish
#define SLEEP_INTERVAL 15000
unsigned long prev_sleep_time;

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

void setupServo(){
  
   myservo.attach(15);  // attaches the servo on GIO2 to the servo object
}

void setupMotor(){
  pinMode(motorPin, OUTPUT);
}

void setup()
{
  prev_sleep_time = millis();

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
    debugHearthbeat();
    recievedMessage();
    Serial.println("Active");
    

    if (millis() - prev_sleep_time >= SLEEP_INTERVAL){
      prev_sleep_time = millis();
      analogWrite(motorPin, 0);
      Serial.println("sleepy sleepy ESP ZZZzzzz");
      publish_data("ESP8266", "Offline");
      delay(2000);
      ESP.deepSleep(30000000);
    }
    publish_data("ESP8266", "Active");
    delay(1000);
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
