// ======================== Includes and Config ========================
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

// WiFi Credentials
#define  ssid  "Redmi Note 12"
#define  password  "yaros5lav"

// GPIO Assignments
#define COMMAND_SEND_PIN 16  // D0
#define DATA_REQUEST_PIN 5   // D1

// Communication
SoftwareSerial arduinoEsp(12, 14); // RX, TX
SoftwareSerial espArduino(3, 1); // TX, RX

void getData();
void sendCommand(uint8_t command);
void parseData(String data);
void handleGet();
void handlePost();
void handleOptions();

// Sensor Readings
float temperature = 0;
float lightLevel = 0;
float gasLevel = 0;
float distance = 0;

// Web Server
ESP8266WebServer server(80);

// Raw Serial Input
String inputString = "";



// ======================== Web Server Handlers ========================
void handleGet() {
  getData();
  
  StaticJsonDocument<200> doc;
  doc["temperature"] = temperature;
  doc["light"] = lightLevel;
  doc["gas"] = gasLevel;
  doc["distance"] = distance;

  String response;
  serializeJson(doc, response);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", response);
}

void handlePost() {
  if (!server.hasArg("plain")) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, "text/plain", "Body not found");
    return;
  }

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  if (error) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  String command = doc["command"];
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", "{\"status\":\"ok\"}");

  if (command == "forward") {
    sendCommand(0x10);
  } else if (command == "backward") {
    sendCommand(0x11);
  } else if (command == "left") {
    sendCommand(0x12);
  } else if (command == "right") {
    sendCommand(0x13);
  } else if (command == "stop") {
    sendCommand(0x14);
  } else {
    Serial.println("Unknown command received via HTTP POST: " + command);
  }
}



void handleOptions() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(204);
}



// ======================== Communication with Arduino ========================
void sendCommand(uint8_t command) {
  espArduino.write(command);
  espArduino.flush();
  delay(2); 
  digitalWrite(COMMAND_SEND_PIN, HIGH);
  delay(10);
  digitalWrite(COMMAND_SEND_PIN, LOW);
}

void getData() {
  sendCommand(0x00);
  unsigned long timeout = millis();
  while (!arduinoEsp.available()) {
    if (millis() - timeout > 500) {
      sendCommand(0x23);
      return;
    }
  }
  Serial.println("Started recieving the coomand");
  inputString = arduinoEsp.readStringUntil('\n');
  Serial.println("Raw input: " + inputString);

  inputString.trim();

  parseData(inputString);
}

void parseData(String data) {
  int idx1 = data.indexOf(',');
  int idx2 = data.indexOf(',', idx1 + 1);
  int idx3 = data.indexOf(',', idx2 + 1);

  if (idx1 > 0 && idx2 > idx1 && idx3 > idx2) {
    temperature = data.substring(0, idx1).toFloat();
    lightLevel = data.substring(idx1 + 1, idx2).toFloat();
    gasLevel = data.substring(idx2 + 1, idx3).toFloat();
    distance = data.substring(idx3 + 1).toFloat();
  } else {
    sendCommand(0x23);
  }
}

// ======================== Setup ========================
void setup() {
  Serial.begin(115200);
  arduinoEsp.begin(9600);
  espArduino.begin(9600);

  pinMode(COMMAND_SEND_PIN, OUTPUT);
  pinMode(DATA_REQUEST_PIN, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  sendCommand(0x20);

  server.on("/control", HTTP_OPTIONS, handleOptions);
  server.on("/control", HTTP_POST, handlePost);
  server.on("/control", HTTP_GET, handleGet);
  server.begin();

  Serial.println("HTTP server started");
}

// ======================== Loop ========================
void loop() {
  static unsigned long lastWiFiCheck = 0;
  server.handleClient();

  if (millis() - lastWiFiCheck > 4000) {
    lastWiFiCheck = millis();

    if (WiFi.status() != WL_CONNECTED) {
      sendCommand(0x21);
      WiFi.disconnect();
      WiFi.begin(ssid, password);

      unsigned long startAttempt = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
        delay(1);
      }

      if (WiFi.status() == WL_CONNECTED) {
        sendCommand(0x20);
      }
    }
  }

  yield();
}
