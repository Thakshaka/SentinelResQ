#include <dht.h>
#include <MQ2.h>
#include "MQ135.h"
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <ArduinoJson.h>

// DHT22
#define DHT22PIN 7
dht DHT;

// MQ2
MQ2 mq2(A0);

// MQ135
MQ135 mq135(A1);

// BMP180
Adafruit_BMP085 bmp180;

void setup() {
  Serial.begin(115200);
  mq2.begin();
  bmp180.begin();
  warmupSensors();
}

float temperature, humidity, co2, lpg, pressure;

void loop() {
  temperature = readTemperature();
  humidity = readHumidity();
  co2 = readCO2();
  lpg = readLPG();
  pressure = readPressure();

  // Create a StaticJsonDocument
  StaticJsonDocument<40> jsonDoc;

  // Assign values to the JSON document
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;
  jsonDoc["co2"] = co2;
  jsonDoc["lpg"] = lpg;
  jsonDoc["pressure"] = pressure;

  // Serialize the JSON document to a string
  String jsonString;
  serializeJson(jsonDoc, jsonString);

  // Print the JSON string
  Serial.println(jsonString);

  delay(1000);
}

void warmupSensors() {
  delay(20000);
}

float readTemperature() {
  DHT.read22(DHT22PIN);
  return DHT.temperature;
}

float readHumidity() {
  DHT.read22(DHT22PIN);
  return DHT.humidity;
}

float readCO2() {
  return mq135.getPPM();
}

float readLPG() {
  return mq2.readLPG();
}

float readPressure() {
  return bmp180.readPressure();
}
