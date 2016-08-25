#include <Wire.h>
#include <ESP8266WiFi.h>
#include <TimeLib.h>           // http://playground.arduino.cc/code/time - installed via library manager
#include <ArduinoJson.h>    // https://github.com/bblanchon/ArduinoJson - installed via library manager
#include "globals.h"        // global structures and enums used by the applocation

CloudConfig cloud;
SensorData data;

IPAddress timeServer(203, 56, 27, 253); // NTP Server au.pool.ntp.org

void initDeviceConfig() { // Example device configuration
  cloud.cloudMode = IoTHub;            // CloudMode enumeration: IoTHub and EventHub (default is IoTHub)
  cloud.publishRateInSeconds = 90;     // limits publishing rate to specified seconds (default is 90 seconds).  Connectivity problems may result if number too small eg 2
  cloud.sasExpiryDate = 1737504000;    // Expires Wed, 22 Jan 2025 00:00:00 GMT (defaults to Expires Wed, 22 Jan 2025 00:00:00 GMT)
}

void setup() {
  initDeviceConfig();
  initCloudConfig("Your Key", "Sydney");
}

void loop() {
  getFakeWeatherReadings();

  if (WiFi.status() == WL_CONNECTED) {
    getCurrentTime();
    Serial.println("push");
    publishToAzure();
    Serial.println("pushed");
    delay(5000);
  } else {
    Serial.println("Not connected to the Internet");
    delay(250);
  }
}

void getCurrentTime() {
  int ntpRetryCount = 0;
  while (timeStatus() == timeNotSet && ++ntpRetryCount < 10) { // get NTP time
    Serial.println(WiFi.localIP());
    setSyncProvider(getNtpTime);
    setSyncInterval(60 * 60);
  }
}

void getFakeWeatherReadings() {
  data.temperature = 25;
  data.humidity = 50;
  data.pressure = 1000;
}
