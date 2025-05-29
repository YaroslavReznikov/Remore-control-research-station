// ======================== Includes and Defines ========================
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

#define DHT_PIN 8
#define DHT_TYPE DHT11
#define TRIG_PIN 12
#define ECHO_PIN 13
#define GAS_SENSOR_PIN A2
#define LIGHT_SENSOR_PIN A3
#define LIGHT_REF_RESISTOR 250

#define RX_PIN 4
#define TX_PIN 2
#define REQUEST_PIN 3

#define RIGHT_WHEEL_PWM 5
#define RIGHT_PIN1 7
#define RIGHT_PIN2 13

#define LEFT_WHEEL_PWM 6
#define LEFT_PIN1 A1
#define LEFT_PIN2 A0

#define MAXPWM 255
#define MINPWM 150
#define MAX_QUEUE_SIZE 10
#define MOTOR_DURATION 10000




// ======================== Global Instances ========================
DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial SerialCommunication(RX_PIN, TX_PIN); 

float temperature, distance, lightLevel;
bool direction = true, commandReceived = false;
uint8_t motorCommandQueue[MAX_QUEUE_SIZE];
int queueStart = 0;
int queueEnd = 0;
bool motorBusy = false;
unsigned long motorStartTime = 0;
uint8_t currentMotorCommand;

// ======================== Setup ========================
void setupPins() {
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  pinMode(GAS_SENSOR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(REQUEST_PIN, INPUT);
  pinMode(RIGHT_PIN1, OUTPUT);
  pinMode(RIGHT_PIN2, OUTPUT);
  pinMode(LEFT_PIN1, OUTPUT);
  pinMode(LEFT_PIN2, OUTPUT);
  pinMode(RIGHT_WHEEL_PWM, OUTPUT);
  pinMode(LEFT_WHEEL_PWM, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(REQUEST_PIN), commandHandlerFlag, RISING);
}

void setupLCD() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  showInternetDisconnected();
}

void setup() {
  setupPins();
  setupLCD();
  dht.begin();
  Serial.begin(4800);
  SerialCommunication.begin(9600);
  Serial.println("Program Started");
  
}

// ======================== Main Loop ========================
void loop() {
  if (commandReceived) {
    commandReceived = false;
    commandHandler();
  }

  if (motorBusy && millis() - motorStartTime >= MOTOR_DURATION && queueStart == queueEnd) {
    Serial.println("Finished doing motor command");
    stopMotors();
    motorBusy = false;
  }

  if ( queueStart != queueEnd && motorBusy && millis() - motorStartTime >= MOTOR_DURATION) {
    currentMotorCommand = motorCommandQueue[queueStart];
    queueStart = (queueStart + 1) % MAX_QUEUE_SIZE;
    Serial.println("Staered doing new motor command");
    executeMotorCommand(currentMotorCommand);
  }


  delay(1);
}

// ======================== Interrupt Handler ========================
void commandHandlerFlag() {
  commandReceived = true;
}

void commandHandler() {
  Serial.println("Interrupt Received");
  delay(1);
  int command = SerialCommunication.read();
  Serial.print("Command: "); 
  Serial.println(command, HEX);
  switch (command & 0xF0) {
    case 0x00: sendSensorData(); break;
    case 0x10: handleMotorCommand(command); break;
    case 0x20: handleLCDCommand(command); break;
    default: Serial.println("Unknown command");
  }
}

// ======================== Sensor Functions ========================
void readDistance() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  float duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration * 0.0343) / 2;
}

void readLightLevel() {
  float adcValue = analogRead(LIGHT_SENSOR_PIN);
  float voltage = adcValue * 5.0 / 1023;
  float resistance = LIGHT_REF_RESISTOR * (voltage / (5.0 - voltage));
  lightLevel = (resistance != 0) ? 500 * pow(resistance, -1.4) : 0;
}

float readGasLevel() {
  return 2;
}

// ======================== Communication Handlers ========================
void sendSensorData() {
  readDistance();
  temperature = dht.readTemperature();
  readLightLevel();
  float gasLevel = readGasLevel();
  String dataString = String(temperature) + "," +
                      String(lightLevel) + "," +
                      String(gasLevel) + "," +
                      String(distance) + "\n";
  SerialCommunication.print(dataString);
}

void handleMotorCommand(uint8_t command) {
  if ((queueEnd + 1) % MAX_QUEUE_SIZE == queueStart) return;
  motorCommandQueue[queueEnd] = command;
  queueEnd = (queueEnd + 1) % MAX_QUEUE_SIZE;
}

void handleLCDCommand(uint8_t command) {
  switch (command 0x0F) {
    case 0x0: showInternetConnected(); break;
    case 0x1: showInternetDisconnected(); break;
    case 0x2: showErrorOnLCD(); break;
    case 0x3: showDataErrorOnLCD(); break;
    default: Serial.println("Unknown LCD command");
  }
}

// ======================== Motor Control ========================
void executeMotorCommand(uint8_t command) {
  motorStartTime = millis();
  motorBusy = true;
  switch (command & 0x0F) {
    case 0x0: moveForward(); break;
    case 0x1: moveBackward(); break;
    case 0x2: moveLeft(); break;
    case 0x3: moveRight(); break;
    case 0x4: stopMotors(); break;
  }
}

void moveForward() {
  digitalWrite(RIGHT_PIN1, HIGH); digitalWrite(RIGHT_PIN2, LOW);
  digitalWrite(LEFT_PIN1, HIGH);  digitalWrite(LEFT_PIN2, LOW);
  analogWrite(RIGHT_WHEEL_PWM, MAXPWM);
  analogWrite(LEFT_WHEEL_PWM, MAXPWM);
}

void moveBackward() {
  digitalWrite(RIGHT_PIN1, LOW); digitalWrite(RIGHT_PIN2, HIGH);
  digitalWrite(LEFT_PIN1, LOW);  digitalWrite(LEFT_PIN2, HIGH);
  analogWrite(RIGHT_WHEEL_PWM, MAXPWM);
  analogWrite(LEFT_WHEEL_PWM, MAXPWM);
}

void moveLeft() {
  digitalWrite(RIGHT_PIN1, HIGH); digitalWrite(RIGHT_PIN2, LOW);
  digitalWrite(LEFT_PIN1, LOW);  digitalWrite(LEFT_PIN2, HIGH);
  analogWrite(RIGHT_WHEEL_PWM, MINPWM);
  analogWrite(LEFT_WHEEL_PWM, MAXPWM);
}

void moveRight() {
  digitalWrite(RIGHT_PIN1, LOW); digitalWrite(RIGHT_PIN2, HIGH);
  digitalWrite(LEFT_PIN1, HIGH); digitalWrite(LEFT_PIN2, LOW);
  analogWrite(RIGHT_WHEEL_PWM, MAXPWM);
  analogWrite(LEFT_WHEEL_PWM, MINPWM);
}

void stopMotors() {
  digitalWrite(RIGHT_PIN1, LOW); digitalWrite(RIGHT_PIN2, LOW);
  digitalWrite(LEFT_PIN1, LOW);  digitalWrite(LEFT_PIN2, LOW);
  analogWrite(RIGHT_WHEEL_PWM, 0);
  analogWrite(LEFT_WHEEL_PWM, 0);
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