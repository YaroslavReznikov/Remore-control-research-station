// ======================== Includes and Defines ========================
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

#define DHT_PIN 13
#define DHT_TYPE DHT11
#define TRIG_PIN 12
#define ECHO_PIN 10
#define GAS_SENSOR_PIN A2
#define LIGHT_SENSOR_PIN A3
#define LIGHT_REF_RESISTOR 250

#define RX_PIN 4
#define TX_PIN 2
#define REQUEST_PIN 3

#define RIGHT_ENGINE_PWM 5
#define RIGHT_PIN1 8
#define RIGHT_PIN2 7

#define LEFT_ENGINE_PWM 6
#define LEFT_PIN1 A0
#define LEFT_PIN2 A1

#define MAXPWM 255
#define MINPWM 150
#define MAX_QUEUE_SIZE 10
#define ENGINE_DURATION 10000

// Gas sensor threshold (based on MQ-135 typical values)
#define GAS_CRITICAL_THRESHOLD 950  // Immediately dangerous (>5% CO2)

// ======================== Structures ========================
struct SensorData {
    float temperature;
    float distance;
    float lightLevel;
    float gasLevel;
    
// ======================== Sensor Functions ========================
    void readTemperature(DHT& dht) {
        // Read temperature from DHT sensor
        temperature = dht.readTemperature();
    }
    void readDistance() {
        //make sure the trig pin is low
        digitalWrite(TRIG_PIN, LOW); 
        delayMicroseconds(2);
        // make the trig pin high for 10 microseconds so it sends a sound wave
        digitalWrite(TRIG_PIN, HIGH); 
        delayMicroseconds(10);
        digitalWrite(TRIG_PIN, LOW);
        // wait for the echo pin to go high
        float duration = pulseIn(ECHO_PIN, HIGH);
        // Convert duration to distance (speed of sound = 0.0343 cm/us)
        distance = (duration * 0.0343) / 2;
    }
    
    void readLightLevel() {
        float adcValue = analogRead(LIGHT_SENSOR_PIN);
        float voltage = adcValue * 5.0 / 1023;
        float resistance = LIGHT_REF_RESISTOR * (voltage / (5.0 - voltage));
        // Calculate light level based on resistance
        lightLevel = (resistance != 0) ? 500 * pow(resistance, -1.4) : 0;
    }
    
    void readGasLevel() {
        float rawValue = analogRead(GAS_SENSOR_PIN);
        float percentage = (rawValue / GAS_CRITICAL_THRESHOLD) * 100;
        // Update gas level
        gasLevel = percentage;
    }
    
    void readAllSensors(DHT& dht) {
        readTemperature(dht);
        readDistance();
        readLightLevel();
        readGasLevel();
    }
    
    String toString() {
      // Return a string with all sensor data separated by commas
        return String(temperature) + "," +
               String(lightLevel) + "," +
               String(gasLevel) + "," +
               String(distance) + "\n";
    }
};

// ======================== Global Instances ========================
// Sensor Instances and Variables
DHT dht(DHT_PIN, DHT_TYPE);
SensorData sensorData;

// LCD Instance
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Communication Instances and Flags
SoftwareSerial SerialCommunication(RX_PIN, TX_PIN); 
bool commandReceived = false;

// Motor Control Variables
bool engineBusy = false;
unsigned long engineStartTime = 0;
uint8_t currentEngineCommand;

// Queue Management
uint8_t engineCommandQueue[MAX_QUEUE_SIZE];
int queueStart = 0;
int queueEnd = 0;


// ======================== Queue Functions ========================
void setupQueue() {
  // initialize the queue
  for(int i = 0; i < MAX_QUEUE_SIZE; i++) {
    engineCommandQueue[i] = 0;
  }
  Serial.println("Queue initialized");
}

// ======================== Setup ========================
void setupPins() {
  // initialize sensor pins
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  pinMode(GAS_SENSOR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(REQUEST_PIN, INPUT);
  // initialize engine pins
  pinMode(RIGHT_PIN1, OUTPUT);
  pinMode(RIGHT_PIN2, OUTPUT);
  pinMode(LEFT_PIN1, OUTPUT);
  pinMode(LEFT_PIN2, OUTPUT);
  pinMode(RIGHT_ENGINE_PWM, OUTPUT);
  pinMode(LEFT_ENGINE_PWM, OUTPUT);
  // attach interrupt to the request pin
  attachInterrupt(digitalPinToInterrupt(REQUEST_PIN), commandHandlerFlag, RISING);
}

void setupLCD() {
  // initialize the lcd
  lcd.init();
  // turn on the backlight
  lcd.backlight();
  // clear the lcd
  lcd.clear();
  // show the internet disconnected message
  showInternetDisconnected();
}

void setup() {
  // initialize the pins
  setupPins();
  // initialize the lcd
  setupLCD();
  // initialize the queue
  setupQueue();
  // initialize the dht sensor
  dht.begin();
  // initialize the serial communication
  Serial.begin(4800);
  SerialCommunication.begin(9600);
  Serial.println("Program Started");
  moveForward();
}

// ======================== Main Loop ========================
void loop() {
  // check if a command from ESP has been received
  if (commandReceived) {
    // set the command received flag to false
    commandReceived = false;
    // handle the command
    commandHandler();
  }
  // check if engine is currently running
  if (engineBusy) {
    if (millis() - engineStartTime >= ENGINE_DURATION) {
      stopEngines();
      engineBusy = false;
    }
  }
  // check if there is a command in the queue and engine is not currently running
  if (!engineBusy && queueStart != queueEnd) {
    // get the next command from the queue
    currentEngineCommand = engineCommandQueue[queueStart];
    // increment the queue start index
    queueStart = (queueStart + 1) % MAX_QUEUE_SIZE;
    // execute the command
    Serial.println("Started doing new engine command");
    executeEngineCommand(currentEngineCommand);
    // set the engine start time
    engineStartTime = millis();
    // set the engine busy flag
    engineBusy = true;
  }
  delay(5);
}

// ======================== Interrupt Handler ========================
void commandHandlerFlag() {
  // set the command received flag
  commandReceived = true;
}

void commandHandler() {
  Serial.println("Interrupt Received");
  delay(1);
  // read the command from the serial communication
  int command = SerialCommunication.read();
  Serial.print("Command: "); 
  Serial.println(command, HEX);
  // use switch with and operation to get the first 4 bits of the command
  switch (command & 0xF0) {
    case 0x00: sendSensorData(); break;
    case 0x20: // Immediate execution with engine stop
      stopEngines();
      delay(1);
      executeEngineCommand(command);
      break;
    case 0x30: handleEngineCommand(command); break; // Queue mode
    case 0x40: executeLCDCommand(command); break;
    default: Serial.println("Unknown command");
  }
}


// ======================== Communication Handlers ========================
void sendSensorData() {
  // read all sensors using the sensorData structure
    sensorData.readAllSensors(dht);
    // send the sensor data to the esp
    SerialCommunication.print(sensorData.toString());
}

void handleEngineCommand(uint8_t command) {
  // check if the queue is full
  if ((queueEnd + 1) % MAX_QUEUE_SIZE == queueStart) {
    Serial.println("Too many commands");
    return;
  } 
  engineCommandQueue[queueEnd] = command;
  queueEnd = (queueEnd + 1) % MAX_QUEUE_SIZE;
}

void executeLCDCommand(uint8_t command) {
  //use swith with and operator 
  switch (command & 0x0F) {
    case 0x00: showInternetConnected(); break;
    case 0x01: showInternetDisconnected(); break;
    case 0x02: showErrorOnLCD(); break;
    case 0x03: showDataErrorOnLCD(); break;
    default: Serial.println("Unknown LCD command");
  }
}

// ======================== Motor Control ========================
void executeEngineCommand(uint8_t command) {
  switch (command & 0x0F) {
    case 0x00: moveForward(); break;
    case 0x01: moveBackward(); break;
    case 0x02: moveLeft(); break;
    case 0x03: moveRight(); break;
    case 0x04: stopEngines(); break;
  }
}

void moveForward() {
  digitalWrite(RIGHT_PIN1, HIGH); digitalWrite(RIGHT_PIN2, LOW);
  digitalWrite(LEFT_PIN1, HIGH);  digitalWrite(LEFT_PIN2, LOW);
  analogWrite(RIGHT_ENGINE_PWM, MAXPWM);
  analogWrite(LEFT_ENGINE_PWM, MAXPWM);
}

void moveBackward() {
  digitalWrite(RIGHT_PIN1, LOW); digitalWrite(RIGHT_PIN2, HIGH);
  digitalWrite(LEFT_PIN1, LOW);  digitalWrite(LEFT_PIN2, HIGH);
  analogWrite(RIGHT_ENGINE_PWM, MAXPWM);
  analogWrite(LEFT_ENGINE_PWM, MAXPWM);
}

void moveLeft() {
  digitalWrite(RIGHT_PIN1, HIGH); digitalWrite(RIGHT_PIN2, LOW);
  digitalWrite(LEFT_PIN1, LOW);  digitalWrite(LEFT_PIN2, HIGH);
  analogWrite(RIGHT_ENGINE_PWM, MINPWM);
  analogWrite(LEFT_ENGINE_PWM, MAXPWM);
}

void moveRight() {
  digitalWrite(RIGHT_PIN1, LOW); digitalWrite(RIGHT_PIN2, HIGH);
  digitalWrite(LEFT_PIN1, HIGH); digitalWrite(LEFT_PIN2, LOW);
  analogWrite(RIGHT_ENGINE_PWM, MAXPWM);
  analogWrite(LEFT_ENGINE_PWM, MINPWM);
}

void stopEngines() {
  digitalWrite(RIGHT_PIN1, LOW); digitalWrite(RIGHT_PIN2, LOW);
  digitalWrite(LEFT_PIN1, LOW);  digitalWrite(LEFT_PIN2, LOW);
  analogWrite(RIGHT_ENGINE_PWM, 0);
  analogWrite(LEFT_ENGINE_PWM, 0);
}

// ======================== LCD Status Functions ========================
void showInternetConnected() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Internet Status:");
  lcd.setCursor(0, 1);
  lcd.print("Connected");
}

void showInternetDisconnected() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Internet Status:");
  lcd.setCursor(0, 1);
  lcd.print("Disconnected");
}

void showErrorOnLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ERROR:");
  lcd.setCursor(0, 1);
  lcd.print("Check sensors!");
}
void showDataErrorOnLCD(){
    lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ERROR:");
  lcd.setCursor(0, 1);
  lcd.print("Getting data");
}