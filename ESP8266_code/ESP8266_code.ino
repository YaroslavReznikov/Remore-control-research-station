// ======================== Includes ========================
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// ======================== Configuration ========================
// WiFi Settings
#define WIFI_SSID "Redmi Note 12"
#define WIFI_PASSWORD "yaros5lav"

// Pin Definitions
#define COMMAND_SEND_PIN 18
#define RX_PIN 4
#define TX_PIN 5

// Communication Settings
#define WIFI_RECONNECT_DELAY 4000  // 4 seconds
#define WIFI_CONNECT_TIMEOUT 10000  // 10 seconds
#define SERIAL_BAUD_RATE 115200
#define UART_BAUD_RATE 9600

// ======================== Global Variables ========================
// Hardware Instances
HardwareSerial SerialCommunication(1);  // Use UART1
WebServer server(80);

// Sensor Data
struct SensorData {
    float temperature;
    float lightLevel;
    float gasLevel;
    float distance;
} sensorData;

// Communication
String inputString = "";

// ======================== Function Declarations ========================
// Web Server Handlers
void handleGet();
void handlePost();
void handleOptions();

// Communication Functions
void getData();
void sendCommand(uint8_t command);
void parseData(String data);

// ======================== Setup Functions ========================
void setupCommunication() {
    Serial.begin(SERIAL_BAUD_RATE);
    SerialCommunication.begin(UART_BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
    pinMode(COMMAND_SEND_PIN, OUTPUT);
    digitalWrite(COMMAND_SEND_PIN, HIGH);
}

void setupWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void setupWebServer() {
    server.enableCORS(true);
    server.on("/control", HTTP_OPTIONS, handleOptions);
    server.on("/control", HTTP_POST, handlePost);
    server.on("/control", HTTP_GET, handleGet);
    server.begin();
    Serial.println("HTTP server started");
}

// ======================== Web Server Handlers ========================
void handleGet() {
    getData();

    StaticJsonDocument<200> doc;
    doc["temperature"] = sensorData.temperature;
    doc["light"] = sensorData.lightLevel;
    doc["gas"] = sensorData.gasLevel;
    doc["distance"] = sensorData.distance;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void handlePost() {
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Body not found");
        return;
    }

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    if (error) {
        server.send(400, "text/plain", "Invalid JSON");
        return;
    }

    String mode = doc["mode"];
    server.send(200, "application/json", "{\"status\":\"ok\"}");

    // Handle different command modes
    if (mode == "sendCommand") {
        processSingleCommand(doc["command"].as<String>());
    } 
    else if (mode == "sendQueue") {
        processCommandQueue(doc["queue"].as<JsonArray>());
    }
}

void handleOptions() {
    server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(204);
}

// ======================== Command Processing ========================
void processSingleCommand(String command) {
    Serial.println("Processing command: " + command);
    if (command == "forward") sendCommand(0x20);
    else if (command == "backward") sendCommand(0x21);
    else if (command == "left") sendCommand(0x22);
    else if (command == "right") sendCommand(0x23);
    else if (command == "stop") sendCommand(0x24);
    else Serial.println("Unknown command: " + command);
}

void processCommandQueue(JsonArray queue) {
    for (JsonVariant command : queue) {
        String cmd = command.as<String>();
        Serial.println("Queue command: " + cmd);
        
        if (cmd == "forward") sendCommand(0x30);
        else if (cmd == "backward") sendCommand(0x31);
        else if (cmd == "left") sendCommand(0x32);
        else if (cmd == "right") sendCommand(0x33);
        else if (cmd == "stop") sendCommand(0x34);
        else Serial.println("Unknown queue command: " + cmd);
        
        delay(100);
    }
}

// ======================== Communication Functions ========================
void sendCommand(uint8_t command) {
    SerialCommunication.write(command);
    SerialCommunication.flush();
    delay(2);
    digitalWrite(COMMAND_SEND_PIN, HIGH);
    delay(10);
    digitalWrite(COMMAND_SEND_PIN, LOW);
}

void getData() {
    sendCommand(0x00);
    unsigned long timeout = millis();
    while (!SerialCommunication.available()) {
        if (millis() - timeout > 500) {
            sendCommand(0x23);
            return;
        }
    }

    inputString = SerialCommunication.readStringUntil('\n');
    inputString.trim();
    parseData(inputString);
}

void parseData(String data) {
    int idx1 = data.indexOf(',');
    int idx2 = data.indexOf(',', idx1 + 1);
    int idx3 = data.indexOf(',', idx2 + 1);

    if (idx1 > 0 && idx2 > idx1 && idx3 > idx2) {
        sensorData.temperature = data.substring(0, idx1).toFloat();
        sensorData.lightLevel = data.substring(idx1 + 1, idx2).toFloat();
        sensorData.gasLevel = data.substring(idx2 + 1, idx3).toFloat();
        sensorData.distance = data.substring(idx3 + 1).toFloat();
    } else {
        sendCommand(0x23);  // notify arduino about data error
    }
}

// ======================== WiFi Management ========================
void checkWiFiConnection() {
    static unsigned long lastWiFiCheck = 0;
    if (millis() - lastWiFiCheck > WIFI_RECONNECT_DELAY) {
        lastWiFiCheck = millis();

        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi disconnected. Reconnecting...");
            sendCommand(0x21);  // Notify Arduino about disconnection
            WiFi.disconnect();
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

            unsigned long startAttempt = millis();
            while (WiFi.status() != WL_CONNECTED && 
                   millis() - startAttempt < WIFI_CONNECT_TIMEOUT) {
                delay(1);
            }

            if (WiFi.status() == WL_CONNECTED) {
                Serial.println("WiFi reconnected!");
                sendCommand(0x20);  // Notify Arduino about connection
            }
        }
    }
}

// ======================== Main Functions ========================
void setup() {
    setupCommunication();
    setupWiFi();
    setupWebServer();
    sendCommand(0x20);  // Notify Arduino about initial connection
}

void loop() {
    server.handleClient();
    checkWiFiConnection();
    delay(1);
}