#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <MPU6050_tockn.h>

const char* mqtt_server = "192.168.2.1";
const char* ssid = "b9stuart";
const char* password = "b9stuart";

int value = 0;
long lastMsg = 0;
char msg[50];

const int led = 4;
float angle;

MPU6050 mpu6050(Wire);
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  
  pinMode(led, OUTPUT);
}

void setup_wifi() {
  delay(10);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);

    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }

  Serial.println();

  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");

    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(led, HIGH);
    }
    else if(messageTemp == "off") {
      Serial.println("off");
      digitalWrite(led, LOW);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESP32Client")) {
      Serial.println("connected");

      client.subscribe("esp32/Angle");
      client.subscribe("esp32/output");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      
      delay(5000);
    }
  }
}

void loop() {
  mpu6050.update();
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    
    angle = mpu6050.getAngleZ();

    char tempString[8];
    dtostrf(angle, 1, 2, tempString);
    Serial.print("Angle: ");
    Serial.println(tempString);
    client.publish("esp32/Angle", tempString);
  }
}
