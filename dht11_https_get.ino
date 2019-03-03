/*
 * DHT Temperature and humidity monitoring using ESP8266 and the askSensors 
 * Description: This examples connects the ESP to wifi, and sends Temperature and humidity to askSensors IoT platfom over HTTPS GET Request.
 *  Author: https://asksensors.com, 2018
 *  github: https://github.com/asksensors
 * InstructableS: https://www.instructables.com/id/DHT11-Temperature-and-Humidity-Monitoring-Using-th/
 */

// includes
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// user config: TODO
const char* wifi_ssid = "..........";             // SSID
const char* wifi_password = ".........";         // WIFI
const char* apiKeyIn = ".........";      // API KEY IN

// ASKSENSORS config.
const char* https_host = "asksensors.com";         // ASKSENSORS host name
const int https_port = 443;                        // https port
const char* https_fingerprint =  "B5 C3 1B 2C 0D 5D 9B E5 D6 7C B6 EF 50 3A AD 3F 9F 1E 44 75";     // ASKSENSORS HTTPS SHA1 certificate
// DHT config.
#define DHTPIN            2         // Pin which is connected to the DHT sensor.
// Uncomment the type of sensor in use:
#define DHTTYPE           DHT11     // DHT 11 
//#define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;
int status = WL_IDLE_STATUS;
float myTemperature = 0, myHumidity = 0; 
// create ASKSENSORS client
WiFiClientSecure client;
// 
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("********** connecting to WIFI : ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("-> WiFi connected");
  Serial.println("-> IP address: ");
  Serial.println(WiFi.localIP());
  // Initialize device.
  dht.begin();
  Serial.println("DHTxx Unified Sensor Example");
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Temperature");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");  
  Serial.println("------------------------------------");
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Humidity");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println("------------------------------------");
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;
}

void loop() {
// Read data from DHT
// Delay between measurements.
  delay(delayMS);
  // Get temperature event and print its value.
  sensors_event_t event;  
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
  }
  else {
    // Update temperature and humidity
    myTemperature = (float)event.temperature;
    Serial.print("Temperature: ");
    Serial.print(myTemperature);
    Serial.println(" C");
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
  }
  else {
     myHumidity = (float)event.relative_humidity;
    Serial.print("Humidity: ");
    Serial.print(myHumidity);
    Serial.println("%");
  }
  
// Use WiFiClientSecure class to create TLS connection
  Serial.print("********** connecting to HOST : ");
  Serial.println(https_host);
  if (!client.connect(https_host, https_port)) {
    Serial.println("-> connection failed");
    //return;
  }
  /* ignore certificate verification 
  if (client.verify(https_fingerprint, https_host)) {
    Serial.println("-> certificate matches");
  } else {
    Serial.println("-> certificate doesn't match");
  }
  */
  // Create a URL for the request
  String url = "/api.asksensors/write/";
  url += apiKeyIn;
    url += "?module1=";
  url += myTemperature;
  url += "&module2=";
  url += myHumidity;
  
  Serial.print("********** requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + https_host + "\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("> Request sent to ASKSENSORS");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
        String line = client.readStringUntil('\n');
        Serial.println("********** ASKSENSORS replay:");
        Serial.println(line);
        Serial.println("********** closing connection");
      
        break;
    }
  }

  delay(25000);     // delay in msec
}
