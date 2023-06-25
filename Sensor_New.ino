#include <dht.h>
#include <MQ2.h>
#include <ArduinoJson.h>

dht DHT11;

MQ2 mq2(A0);
float mq2_lpg, mq2_co, mq2_smoke;

void setup() {
  Serial.begin(115200);
  
  mq2.begin(); // MQ2
  pinMode(A1, INPUT); // MQ135
}

void loop() {
  DHT11.read11(7); // DHT11
  float temperature = DHT11.temperature;
  float humidity = DHT11.humidity;
  delay(250);

  mq2_lpg = mq2.readLPG();
  mq2_co = mq2.readCO();
  mq2_smoke = mq2.readSmoke();
  delay(250);

  float mq135 = analogRead(A1);
  delay(250);

  // Create a JSON object
  StaticJsonDocument<1024> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["lpg"] = mq2_lpg;
  doc["co"] = mq2_co;
  doc["smoke"] = mq2_smoke;
  doc["air_quality"] = mq135;

  // Serialize the JSON object to a string
  String sensorData;
  serializeJson(doc, sensorData);

  Serial.println(sensorData);

  delay(250);
}
