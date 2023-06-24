#include <dht.h>
#include <ArduinoJson.h>

dht DHT11;

void setup() {
  Serial.begin(115200);
  
  pinMode(A0, INPUT); // MQ2
  pinMode(A1, INPUT); // MQ135
}

void loop() {
  DHT11.read11(7); // DHT11
  float temperature = DHT11.temperature;
  float humidity = DHT11.humidity;
  delay(250);

  float mq2 = analogRead(A0);
  delay(250);

  float mq135 = analogRead(A1);
  delay(250);

  // Create a JSON object
  StaticJsonDocument<1024> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["mq2"] = mq2;
  doc["mq135"] = mq135;

  // Serialize the JSON object to a string
  String sensorData;
  serializeJson(doc, sensorData);

  Serial.println(sensorData);

  delay(250);
}
