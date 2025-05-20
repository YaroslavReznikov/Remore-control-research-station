#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#define Interval 1000  // 1 second
#define DATA_REQUEST_PIN 16  // D0 (GPIO16)

SoftwareSerial arduinoSerial(13, 15); // RX, TX (connect to Arduino TX, RX)
String inputString = "";
unsigned long lastMillis = 0;

const char* ssid = "Redmi Note 12";
const char* password = "yaros5lav";

float temperature = 0;
float lightLevel = 0;
float gasLevel = 0;
float distance = 0;

ESP8266WebServer server(80);

void move(String direction) {
  Serial.println("Moving: " + direction);
}

// POST /control to send command
void handlePost() {
  if (!server.hasArg("plain")) {
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

// GET /control to get sensor data
void handleGet() {
  getData(); // Request and receive data from Arduino

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

// Handle preflight request
void handleOptions() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(204);
}

void getData() {
    pinMode(DATA_REQUEST_PIN, OUTPUT);
  digitalWrite(DATA_REQUEST_PIN, HIGH);  // 🔼 Ask for data

  unsigned long timeout = millis();
  while (!arduinoSerial.available()) {
    if (millis() - timeout > 500){
      Serial.println("Got canceled");
        digitalWrite(DATA_REQUEST_PIN, LOW); 
      return;
    } // Timeout after 500ms
  }
  inputString = arduinoSerial.readStringUntil('\n');
  inputString.trim();
  Serial.println(inputString);
  digitalWrite(DATA_REQUEST_PIN, LOW);  // 🔼 Ask for data

  parseData(inputString);
}

void parseData(String data) {
  Serial.println(data);
  int idx1 = data.indexOf(',');
  int idx2 = data.indexOf(',', idx1 + 1);
  int idx3 = data.indexOf(',', idx2 + 1);

  if (idx1 > 0 && idx2 > idx1 && idx3 > idx2) {
    temperature = data.substring(0, idx1).toFloat();
    lightLevel = data.substring(idx1 + 1, idx2).toFloat();
    gasLevel = data.substring(idx2 + 1, idx3).toFloat();
    distance = data.substring(idx3 + 1).toFloat();

    Serial.println("Received from Arduino:");
    Serial.println("Temperature: " + String(temperature));
    Serial.println("Light Level: " + String(lightLevel));
    Serial.println("Gas Level: " + String(gasLevel));
    Serial.println("Distance: " + String(distance));
  } else {
    Serial.println("Invalid data format: " + data);
  }
}

void setup() {
  Serial.begin(115200 );
  arduinoSerial.begin(9600);

  WiFi.begin(ssid, password);
  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected! IP address:");
  Serial.println(WiFi.localIP());

  server.on("/control", HTTP_OPTIONS, handleOptions);
  server.on("/control", HTTP_POST, handlePost);
  server.on("/control", HTTP_GET, handleGet);
  server.begin();

  Serial.println("HTTP server started");
}

void loop() {
  unsigned long currentMillis = millis();
  if (millis() - lastMillis > Interval) {
    lastMillis = millis();
    getData();
  }
}
