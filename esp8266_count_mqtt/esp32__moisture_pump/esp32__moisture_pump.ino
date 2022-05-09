#include <PubSubClient.h>
#include <analogWrite.h>
#include <WiFi.h>
// network credentials (STATION)
const char* ssid = "LEO1_TEAM_06";
const char* password = "embeddedlinux";

//MQTT
const char* mqtt_server =  "192.168.10.1";
WiFiClient espClient;
PubSubClient client(espClient);
#define pumpPin 4
#define moisturePin 34
#define topicMoisture "moisture"
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
void pumpPWMExecute(int output, int duration){
  analogWrite(pumpPin, output);
  delay(duration);
  analogWrite(pumpPin, 0);
}
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  String msg = "";
  for(int i = 0; i< length; i++){
    msg = msg+(char)message[i];
  }
  char* pumpTopic = "pumpActuator";
  if(String(topic) == String(pumpTopic)){
    Serial.print("pump: " + msg);
    pumpPWMExecute(msg.toInt(), 10000);
  } 
 }
void setup() {
  Serial.begin(115200);
  initWiFi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  float sensorValue = analogRead(moisturePin);
  char result[9];
  client.publish(topicMoisture, dtostrf(sensorValue,6,3,result));
  
  //Serial.print("Temperature: ");
  
  // Convert raw temperature in F to Celsius degrees
  //Serial.print((temprature_sens_read() - 32) / 1.8);
  //Serial.println(" C");
  delay(5000);
}
