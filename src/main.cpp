#include "FS.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#define BUFFERSIZE 1024

const char *ssid = "";
const char *password = "";

WiFiServer httpserver(80);
WiFiClient wifiClient;

void sendFile(WiFiClient c, String path) {
  if (SPIFFS.exists(path)) {
    File f = SPIFFS.open(path, "r");
    while (f.available()) {
      Serial.println("[http] Sending " + path);
      Serial.println("[http] bytes:" + String((int)f.size()));
      int max = f.size() / (BUFFERSIZE - 1);
      int leftover = f.size() % (BUFFERSIZE - 1);
      char buff[BUFFERSIZE];
      Serial.print("[http]");
      for (int i = 0; i < max; i++) {
        Serial.print(".");
        f.readBytes(buff, (BUFFERSIZE - 1));
        buff[(BUFFERSIZE - 1)] = 0;
        c.print(buff);
      }
      Serial.println();
      Serial.println("[http] done.");
      f.readBytes(buff, leftover);
      buff[leftover] = 0;
      c.print(buff);
    }
    f.close();
  } else {
    c.print("404");
  }
}

void setup() {
  SPIFFS.begin();
  Serial.begin(9600);

  Serial.println("[WiFi] Set up connection..");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("[WiFi] connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("[WiFi] connected");
  Serial.println(WiFi.localIP().toString());

  httpserver.begin();

  if (!MDNS.begin("test"))
    Serial.println("mDNS failed.");
}

void loop() {
  WiFiClient httpclient = httpserver.available();
  if (!httpclient) {
    return;
  }

  Serial.println("[Server] new client");
  while (!httpclient.available()) {
    delay(1);
  }

  // Read the first line of the request
  String req = httpclient.readStringUntil('\r');
  Serial.println("[Server] " + req);
  httpclient.flush();

  if (req.indexOf("/index.html") != -1) {
    sendFile(httpclient, "/index.html");
  } else {
    httpclient.print("404");
    return;
  }
}
