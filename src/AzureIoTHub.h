
#ifndef AzureIoTHub_h
#define AzureIoTHub_h
#include <WiFiClientSecure.h>
#include "sha256.h"
#include "Base64.h"

#ifdef ESP32
#include <WiFi.h>
#elif ESP8266
#include <ESP8266WiFi.h>
#endif

#include "aJson/aJSON.h"
#include "pubsubclient/PubSubClient.h"
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
	const char* fullSas;
	const char * postUrl;
	const char * hubUser;
	const char * getUrl;
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

typedef void(*GeneralFunction) (String AzureData);

class AzureIoTHub
{
public:
	int senddata = 0;
	bool connect(),
		push(DataElement *data);
	void begin(String cs);
	void setCallback(GeneralFunction _az);
private:
	static GeneralFunction az;
	static void callback(char* topic, byte* payload, unsigned int length);
	const char *GetStringValue(String value);
	String splitStringByIndex(String data, char separator, int index),
		urlEncode(const char* msg),
		createIotHubSas(char *key, String url);
};

extern AzureIoTHub Azure;
#endif
