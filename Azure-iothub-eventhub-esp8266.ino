#include "src/AzureIoTHub.h"

void setup() {
  Serial.begin(115200);
  WiFi.begin("SSID", "PASS");
  Azure.begin(IoTHub, "Your Key"); //YourKey Example:"HostName=YourHost.azure-devices.net;DeviceId=YourDevice;SharedAccessKey="
}

void loop() {

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("push");
    DataElement a = DataElement();
    a.setValue("set", 120);
    Azure.push(&a);
    Serial.println("pushed");
    delay(5000);
  } else {
    Serial.println("Not connected to the Internet");
    delay(250);
  }
}



