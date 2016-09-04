#include <AzureIoTHub.h>

void setup() {
  Serial.begin(115200);
  WiFi.begin("SSID", "PASS");
  Azure.begin("Your Key"); //YourKey Example:"HostName=YourHost.azure-devices.net;DeviceId=YourDevice;SharedAccessKey="
  Azure.setCallback(azureCallback);
}

void azureCallback(String s) {
  Serial.print("azure Message arrived [");
  Serial.print(s);
  Serial.println("] ");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Azure.connect();
    DataElement a = DataElement();
    a.setValue("EspAnalog", analogRead(A0));
    Azure.push(&a);
    Serial.println("pushed");
    delay(2000);
  } else {
    Serial.println("Not connected to the Internet");
    delay(250);
  }
}



