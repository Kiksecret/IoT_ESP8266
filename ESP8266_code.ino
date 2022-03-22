#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#ifndef STASSID
#define STASSID "TP-Link_580B"
#define STAPSK  "41083150"
#endif

#define DHTPIN D2
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

const char* ssid     = STASSID;
const char* password = STAPSK; 
const char* mqtt_broker = "192.168.43.16";

#define BUFFER_SIZE 100
char tem[BUFFER_SIZE];
char hum[BUFFER_SIZE];

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // DHT
  dht.begin();

  // MQTT
  client.setServer(mqtt_broker, 1883);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  Serial.printf("Humidity: %f\n", h);
  Serial.printf("Temperature: %f\n", t);

  // publish MQTT message
  snprintf(tem, BUFFER_SIZE, "insert into sensor(sensorid, value, unit) values (%d, %f, %s)",
    1, t, "\"degree\""); 
  snprintf(hum, BUFFER_SIZE, "insert into sensor(sensorid, value, unit) values (%d, %f, %s)",
    2, h, "\"percent\"");   
  client.publish(tem, "temperature");
  Serial.printf("Published temperature: %s\n", tem);
  
  client.publish(hum, "humidity");
  Serial.printf("Published humidity: %s\n", hum);
  
  delay(5000);
}
