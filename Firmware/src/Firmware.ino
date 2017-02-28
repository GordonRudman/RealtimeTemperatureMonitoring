/*
 * Project What's Brewing
 * Description: Realtime IoT Project to monitor our beer brewing process.
 * Author: Gordon Rudman
 * Date: 2017-02-28
 */
#include <string>
#include <map>
#include "spark-dallas-temperature.h"
#include "HttpClient/HttpClient.h"
using namespace std;


int _led1 = D0;
int _inputPinForDallasTemperatureSensors = D5;
DallasTemperature _dallasSensors(new OneWire(_inputPinForDallasTemperatureSensors));
HttpClient _httpClient;


String convertToJson(std::map<String, float> dictionary)
{
  String json = "{";

    for (auto it = dictionary.begin(); it != dictionary.end(); ++it)
    {
      // get the current index
      auto i = std::distance(dictionary.begin(), it);

      // access elements as *it
      String key = it->first;
      float value = it->second;
      String formattedValue = String(value, 4);

      if (i > 0)
      {
        //Add , delimiter
        json = json + ",";
      }

      json = json + "\"" + key + "\":" + formattedValue;
    }

  json = json + "}";
  return json;
}



float collectTemperatureFromDallasSensor(int sensorIndex)
{
  _dallasSensors.requestTemperatures();
  float temperature = _dallasSensors.getTempCByIndex(sensorIndex);

  int retryCount = 0;
  while ((temperature < -100) && (retryCount < 10))
  {
      retryCount++;
      delay(10);
      temperature = _dallasSensors.getTempCByIndex(sensorIndex);
  }

  return temperature;
}


void performHttpGet(String hostname, int port, String path, String queryParameters) //TODO: send the queryParameters
{
  http_header_t httpRequestHeaders[] =
  {
      //  { "Content-Type", "application/json" },
      //  { "Accept" , "application/json" },
      { "Accept" , "*/*"},
      { NULL, NULL } // NOTE: Always terminate headers with NULL
  };

  http_request_t httpRequest;
  httpRequest.hostname = hostname;
  httpRequest.port = port;
  httpRequest.path = path;

  http_response_t httpResponse;

  Serial.println("HTTP GET...");
  Serial.println(httpRequest.hostname + ":" + httpRequest.port + httpRequest.path);
  Serial.println("");

  _httpClient.get(httpRequest, httpResponse, httpRequestHeaders);
}

void sendDataToPubNub(String jsonData)
{
  String publishKey = "demo";
  String subscriptionKey = "demo";
  String basePath = "/publish/" + publishKey + "/" + subscriptionKey + "/0/";
  String channel = "3ssldkfjglkdsjfkgl23028374yihas23";
  String eonJson = "{\"eon\":" + jsonData + "}";

  String path = basePath + channel + "/0/" + eonJson;
  String queryParamers = "";

  performHttpGet("ps7.pubnub.com", 80, path, queryParamers);
}
void sendDataToAzure(String jsonData)
{
  //TODO
}

void turnOnSendingDataIndicator()
{
    digitalWrite(_led1, HIGH);
}

void turnOffSendingDataIndicator()
{
    digitalWrite(_led1, LOW);
}

void setup()
{
  Serial.begin(9600);
  pinMode(_led1, OUTPUT);
  _dallasSensors.begin();

  Serial.println("Setup complete");
}

void loop()
{
  std::map<String, float> sensorReadings;
  sensorReadings["T1"] = collectTemperatureFromDallasSensor(0);
  sensorReadings["T2"] = collectTemperatureFromDallasSensor(1);
  sensorReadings["T3"] = collectTemperatureFromDallasSensor(2);

  String jsonData = convertToJson(sensorReadings);

  turnOnSendingDataIndicator();

  sendDataToPubNub(jsonData);
//  sendDataToAzure(jsonData);

  turnOffSendingDataIndicator();

//  delay(delayBetweenSends);
//  if (nextTime > millis()) { return; }
//  else { nextTime = millis() + delayBetweenSends; }
}
