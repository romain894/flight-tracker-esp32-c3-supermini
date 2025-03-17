#include "Arduino.h"
#include <WiFi.h>
#include <ESPmDNS.h>

#include <secrets.h>

#define MDNS_HOSTNAME "flight-tracker"

WiFiServer server(80); /* Instance of WiFiServer with port number 80 */
String request;


WiFiClient client;

// #ifndef LED_BUILTIN
#define LED_BUILTIN 8
// #endif

void setup()
{
  // initialize LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  Serial.print("Connecting to: ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
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
}

void loop()
{
  client = server.available();
    if(!client)
    {
      return;
    }

    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        request += c;

        if (c == '\n')
        {
          if (request.indexOf("GET /LED_ON") != -1)
          {
            Serial.println("LED in ON");
            digitalWrite(LED_BUILTIN, LOW);
          }

          if (request.indexOf("GET /LED_OFF") != -1)
          {
            Serial.println("LED in OFF");
            digitalWrite(LED_BUILTIN, HIGH);
          }
          break;
        }
      }
    }
    request="";
    client.stop();
}
