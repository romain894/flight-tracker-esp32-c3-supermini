#include "Arduino.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>

#include <secrets.h>
#include <index.h>

#define MDNS_HOSTNAME "flight-tracker"

// WiFiServer server(80); /* Instance of WiFiServer with port number 80 */
AsyncWebServer server(80);

String request;


WiFiClient client;

// #ifndef LED_BUILTIN
#define LED_BUILTIN 8
// #endif


// Create empty vectors of size 1000
std::vector<int> x(5000, 0);
std::vector<int> y(5000, 0);
std::vector<int> z(5000, 0);

String generateCSV(const std::vector<int>& list_x, const std::vector<int>& list_y, const std::vector<int>& list_z) {
  String csvData = "x,y,z\n";
  for (size_t i = 0; i < list_x.size(); i++) {
    csvData += String(list_x[i]) + "," + String(list_y[i]) + "," + String(list_z[i]) + "\n";
  }
  return csvData;
}

void setup()
{
  // initialize LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);

  Serial.print("Connecting to: ");
  Serial.println(WIFI_SSID);
  // WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }
  Serial.print("\n");
  Serial.print("Connected to Wi-Fi ");
  Serial.println(WiFi.SSID());
  delay(1000);
  server.begin(); /* Start the HTTP web Server */
  Serial.print("Connect to IP Address: ");
  Serial.print("http://");
  Serial.println(WiFi.localIP());
  // Initialize mDNS
  if (!MDNS.begin(MDNS_HOSTNAME)) {   // Set the hostname to "esp32.local"
    while(1) {
      Serial.println("Error setting up MDNS responder! Please restart the controller");
      delay(1000);
    }
  }
  Serial.print("mDNS responder started: http://");
  Serial.print(MDNS_HOSTNAME);
  Serial.print(".local");

  // Serve the HTML page from the file
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("ESP32 Web Server: New request received:");  // for debugging
    Serial.println("GET /");                                    // for debugging

    request->send(200, "text/html", webpage);
  });

  server.on("/download", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("ESP32 Web Server: Download request received");
    String csvData = generateCSV(x, y, z);
    request->send(200, "text/csv", csvData);
  });


  // Start the server
  server.begin();
}

void loop()
{

}
