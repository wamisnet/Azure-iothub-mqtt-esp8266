
#ifndef AzureIoTHub_h
#define AzureIoTHub_h
#include <TimeLib.h>           // http://playground.arduino.cc/code/time - installed via library manager
  // https://github.com/bblanchon/ArduinoJson - installed via library manager
#include <WiFiClientSecure.h>
#include "sha256.h"
#include "Base64.h"
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include "aJson/aJSON.h"
enum CloudMode {
	IoTHub,
	EventHub
};


struct CloudConfig {
	CloudMode cloudMode = IoTHub;
	unsigned int publishRateInSeconds = 90; // defaults to once a minute
	unsigned int sasExpiryDate = 1737504000;  // Expires Wed, 22 Jan 2025 00:00:00 GMT
	const char *host;
	char *key;
	const char *id;
	unsigned long lastPublishTime = 0;
	String fullSas;
	String endPoint;
};

class DataElement {
public:
	DataElement();
	DataElement(char *json_string);
	~DataElement();
	void setValue(const char *key, const char *v);
	void setValue(const char *key, int v);
	void setValue(const char *key, double v);
	char *toCharArray();
	char *getString(const char *key);
	int getInt(const char *key);
	float getFloat(const char *key);

private:
	aJsonObject *params;
	aJsonObject *paJsonObj;
};

class AzureIoTHub
{
public:
	int senddata = 0;
	bool connect(),
		push(DataElement *data);
	void begin(CloudMode _mode, String cs, const char *geo);
	char* GetISODateTime();
private:
	const char *GetStringValue(String value);
	String splitStringByIndex(String data, char separator, int index),
		urlEncode(const char* msg),
		buildHttpRequest(String data),
		createIotHubSas(char *key, String url),
		createEventHubSas(char *key, String url);
	void initialiseIotHub(),
		initialiseEventHub();
};

extern AzureIoTHub Azure;
#endif
