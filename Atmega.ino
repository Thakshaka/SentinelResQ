// Including the libraries
#include <dht.h>
#include <MQ2.h>
#include "MQ135.h"
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <ArduinoJson.h>

// Defining pins and creating objects
#define DHT22PIN 7
dht DHT;

MQ2 mq2(A0);

MQ135 mq135(A1);

Adafruit_BMP085 bmp180;

// Setup function
void setup() {
  Serial.begin(115200);
  mq2.begin();
  bmp180.begin();
}

// Declaring variables
float temperature, humidity, co2, lpg, pressure;

// Loop function
void loop() {

  DHT.read22(DHT22PIN);

  // Calling functions
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
  String dataString;
  serializeJson(jsonDoc, dataString);

  // Print the JSON string
  Serial.println(dataString);

  // 1 second delay
  delay(1000);
}

// Functions to read data from the sensors
float readTemperature() {
  return DHT.temperature;
}

float readHumidity() {
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
