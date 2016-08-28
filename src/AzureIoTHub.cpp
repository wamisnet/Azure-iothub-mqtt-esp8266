#include "AzureIoTHub.h"

WiFiClientSecure tlsClient;

CloudConfig cloud;

// Azure IoT Hub Settings
const char* TARGET_URL = "/devices/";
const char* IOT_HUB_END_POINT = "/messages/events?api-version=2015-08-15-preview";

// Azure Event Hub settings
const char* EVENT_HUB_END_POINT = "/ehdevices/publishers/nodemcu/messages";

char buffer[256];

void AzureIoTHub::begin(CloudMode _mode, String cs)
{
	cloud.cloudMode = _mode;
	cloud.host = GetStringValue(splitStringByIndex(splitStringByIndex(cs, ';', 0), '=', 1));
	cloud.id = GetStringValue(splitStringByIndex(splitStringByIndex(cs, ';', 1), '=', 1));
	cloud.key = (char*)GetStringValue(splitStringByIndex(splitStringByIndex(cs, ';', 2), '=', 1));
	switch (cloud.cloudMode) {
	case IoTHub:
		initialiseIotHub();
		break;
	case EventHub:
		initialiseEventHub();
		break;
	}
}


bool AzureIoTHub::push(DataElement *data)
{
	int bytesWritten = 0;

	// https://msdn.microsoft.com/en-us/library/azure/dn790664.aspx  


	if (!tlsClient.connected()) { connect(); }
	if (!tlsClient.connected()) { return false; }

	char* sendData = data->toCharArray();
	tlsClient.flush();
	//Serial.println( buildHttpRequest(sendData));
	bytesWritten = tlsClient.print(buildHttpRequest(sendData));
	free(sendData);

	String response = "";
	String chunk = "";
	int limit = 1;

	do {
		if (tlsClient.connected()) {
			yield();
			chunk = tlsClient.readStringUntil('\n');
			response += chunk;
		}
	} while (chunk.length() > 0 && ++limit < 100);

	Serial.print("Bytes sent ");
	Serial.print(bytesWritten);
	Serial.print(", Memory ");
	Serial.print(ESP.getFreeHeap());
	Serial.print(", Response chunks ");
	Serial.print(limit);
	Serial.print(", Response code: ");

	if (response.length() > 12) {
		String code = response.substring(9, 12);
		Serial.println(code);
		if (code.equals("204"))return true;
	}
	else {
		Serial.println("unknown");
		return false;
	}

}



String AzureIoTHub::buildHttpRequest(String data)
{
	return "POST " + cloud.endPoint + " HTTP/1.1\r\n" +
		"Host: " + cloud.host + "\r\n" +
		"Authorization: SharedAccessSignature " + cloud.fullSas + "\r\n" +
		"Content-Type: application/atom+xml;type=entry;charset=utf-8\r\n" +
		"Content-Length: " + data.length() + "\r\n\r\n" + data;
}



const char * AzureIoTHub::GetStringValue(String value)
{
	int len = value.length() + 1;
	char *temp = new char[len];
	value.toCharArray(temp, len);
	return temp;
}

String AzureIoTHub::splitStringByIndex(String data, char separator, int index)
{
	int found = 0;
	int strIndex[] = { 0, -1 };
	int maxIndex = data.length() - 1;

	for (int i = 0; i <= maxIndex && found <= index; i++) {
		if (data.charAt(i) == separator || i == maxIndex) {
			found++;
			strIndex[0] = strIndex[1] + 1;
			strIndex[1] = (i == maxIndex) ? i + 1 : i;
		}
	}
	return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

String AzureIoTHub::urlEncode(const char * msg)
{
	const char *hex = "0123456789abcdef";
	String encodedMsg = "";

	while (*msg != '\0') {
		if (('a' <= *msg && *msg <= 'z')
			|| ('A' <= *msg && *msg <= 'Z')
			|| ('0' <= *msg && *msg <= '9')) {
			encodedMsg += *msg;
		}
		else {
			encodedMsg += '%';
			encodedMsg += hex[*msg >> 4];
			encodedMsg += hex[*msg & 15];
		}
		msg++;
	}
	return encodedMsg;
}

bool AzureIoTHub::connect()
{
	if (tlsClient.connected()) { return true; }
	Serial.print(cloud.id);
	Serial.print(" connecting to ");
	Serial.println(cloud.host);
	if (!tlsClient.connect(cloud.host, 443)) {      // Use WiFiClientSecure class to create TLS connection
		Serial.println("Azure connection failed. ");
		return false;
	}
}
void AzureIoTHub::initialiseEventHub()
{
	String url = urlEncode("https://") + urlEncode(cloud.host) + urlEncode(EVENT_HUB_END_POINT);
	cloud.endPoint = EVENT_HUB_END_POINT;
	cloud.fullSas = createEventHubSas(cloud.key, url);
}

void AzureIoTHub::initialiseIotHub()
{
	String url = urlEncode(cloud.host) + urlEncode(TARGET_URL) + (String)cloud.id;
	cloud.endPoint = (String)TARGET_URL + (String)cloud.id + (String)IOT_HUB_END_POINT;
	cloud.fullSas = createIotHubSas(cloud.key, url);
}

String AzureIoTHub::createIotHubSas(char * key, String url)
{
	String stringToSign = url + "\n" + cloud.sasExpiryDate;

	// START: Create signature
	// https://raw.githubusercontent.com/adamvr/arduino-base64/master/examples/base64/base64.ino

	int keyLength = strlen(key);

	int decodedKeyLength = base64_dec_len(key, keyLength);
	char decodedKey[decodedKeyLength];  //allocate char array big enough for the base64 decoded key

	base64_decode(decodedKey, key, keyLength);  //decode key

	Sha256.initHmac((const uint8_t*)decodedKey, decodedKeyLength);
	Sha256.print(stringToSign);
	char* sign = (char*)Sha256.resultHmac();
	// END: Create signature

	// START: Get base64 of signature
	int encodedSignLen = base64_enc_len(HASH_LENGTH);
	char encodedSign[encodedSignLen];
	base64_encode(encodedSign, sign, HASH_LENGTH);

	// SharedAccessSignature
	return "sr=" + url + "&sig=" + urlEncode(encodedSign) + "&se=" + cloud.sasExpiryDate;
	// END: create SAS  
}

String AzureIoTHub::createEventHubSas(char * key, String url)
{
	// START: Create SAS  
	// https://azure.microsoft.com/en-us/documentation/articles/service-bus-sas-overview/
	// Where to get seconds since the epoch: local service, SNTP, RTC

	String stringToSign = url + "\n" + cloud.sasExpiryDate;

	// START: Create signature
	Sha256.initHmac((const uint8_t*)key, 44);
	Sha256.print(stringToSign);

	char* sign = (char*)Sha256.resultHmac();
	int signLen = 32;
	// END: Create signature

	// START: Get base64 of signature
	int encodedSignLen = base64_enc_len(signLen);
	char encodedSign[encodedSignLen];
	base64_encode(encodedSign, sign, signLen);
	// END: Get base64 of signature

	// SharedAccessSignature
	return "sr=" + url + "&sig=" + urlEncode(encodedSign) + "&se=" + cloud.sasExpiryDate + "&skn=" + cloud.id;
	// END: create SAS
}


DataElement::DataElement() {
	params = aJson.createObject();
	paJsonObj = aJson.createObject();
	aJson.addItemToObject(paJsonObj, "params", params);
	aJson.addStringToObject(paJsonObj, "Dev", cloud.id);
	aJson.addNumberToObject(paJsonObj, "Id",++(Azure.senddata));
}

DataElement::DataElement(char *json_string) {
	paJsonObj = aJson.parse(json_string);
	params = aJson.getObjectItem(paJsonObj, "params");
}

DataElement::~DataElement() {
	aJson.deleteItem(paJsonObj);
	paJsonObj = NULL;
	params = NULL;
}

void DataElement::setValue(const char *key, const char *v) {
	aJson.addStringToObject(params, key, v);
}

void DataElement::setValue(const char *key, int v) {
	aJson.addNumberToObject(params, key, v);
}

void DataElement::setValue(const char *key, double v) {
	aJson.addNumberToObject(params, key, v);
}

char *DataElement::getString(const char *key) {
	aJsonObject* obj = aJson.getObjectItem(params, key);
	if (obj == NULL) {
		Serial.println("obj is NULL");
		return (char*)"";
	}
	return obj->valuestring;
}

int DataElement::getInt(const char *key) {
	aJsonObject* obj = aJson.getObjectItem(params, key);
	if (obj == NULL) {
		Serial.println("obj is NULL");
		return 0;
	}
	return obj->valueint;
}

float DataElement::getFloat(const char *key) {
	aJsonObject* obj = aJson.getObjectItem(params, key);
	if (obj == NULL) {
		Serial.println("obj is NULL");
		return 0;
	}
	return obj->valuefloat;
}


char *DataElement::toCharArray() {
	return aJson.print(paJsonObj);
}

AzureIoTHub Azure;