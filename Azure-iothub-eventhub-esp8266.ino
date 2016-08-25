#include <ESP8266WiFi.h>
#include <TimeLib.h>    
#include "src/AzureIoTHub.h"
   
IPAddress timeServer(203, 56, 27, 253); // NTP Server au.pool.ntp.org

void setup() {
 Azure.begin(IoTHub,"Your Key");
}

void loop() {

  if (WiFi.status() == WL_CONNECTED) {
    getCurrentTime();
    Serial.println("push");
    DataElement a=DataElement();
    a.setValue("set",120);
    Azure.push(&a);
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


