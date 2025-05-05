#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
const char* ssid = "Redmi Note 12";
const char* password = "yaros5lav";

ESP8266WebServer server(80);

// Motor control function (placeholder)
void move(String direction) {
  Serial.println("Moving: " + direction);
  // Add your motor control code here (e.g., digitalWrite to control H-Bridge)
}

void handleControl() {
  if (server.hasArg("plain") == false) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, "text/plain", "Body not found");
    return;
  }

  String body = server.arg("plain");
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, body);

  if (error) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  String command = doc["command"];
  move(command);

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}
void handleOptions() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(204);  // No Content
}

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected! IP address: ");
  Serial.println(WiFi.localIP());

  // Define route to accept commands
  server.on("/control", HTTP_POST, handleControl);

  server.begin();
  server.on("/control", HTTP_OPTIONS, handleOptions);  // CORS preflight
  server.on("/control", HTTP_POST, handleControl);     // Actual POST
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
