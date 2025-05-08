#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#define FORWARD_LEFT_WHEEL 4
#define FORWARD_RIGHT_WHEEL 4
#define BACK_LEFT_WHEEL 4
#define BACK_RIGHT_WHEEL 4
#define MAX_PWM 255
#define MIN_PWM 100
#define DHT11PIN 8
#define trigPin 2
#define echoPin 4
#define temperaturePin 8
#define gasSensorPin A0
#define LightSensorPin A3
#define LightConstantResistor 250
DHT dht(temperaturePin, DHT11);
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial espSerial(12, 14); // RX, TX – match this to your wiring
bool direction = 1;
float duration, distance;
float temperature;
float lightLevel = 0;
void setup() {
  pinMode(LightSensorPin, INPUT);
  pinMode(gasSensorPin, INPUT);
  pinMode(trigPin, OUTPUT);
  dht.begin();
  pinMode(echoPin, INPUT);
  Serial.begin(9600);
  Serial.println("start of the program");
  lcd.init();           
  lcd.backlight();       
  lcd.setCursor(0, 0);   
  lcd.print("");     
  lcd.setCursor(0, 1);   
  lcd.print("");

}


void loop() {
  CalculateDistance();
  temperature = dht.readTemperature();
  CalculateLight();

  if (espSerial.available()) {
    String inputString = espSerial.readStringUntil('\n');
    inputString.trim();

    if (inputString == "SEND") {
      espSerial.print(temperature);
      espSerial.print(",");
      espSerial.print(lightLevel);
      espSerial.print(",");
      espSerial.print(2); // replace 2 with actual gasLevel if needed
      espSerial.print(",");
      espSerial.println(distance);
    }
  }

  delay(200);  // Optional: allows time for commands to come in
}

void CalculateLight(){
  lightLevel = 0;
  float adcValue = analogRead(LightSensorPin);

  float voltage = adcValue * 5/1023;
  float R_sensor = LightConstantResistor * (voltage / (5.0 - voltage));
  Serial.println(voltage);
    Serial.println(adcValue);
  if(R_sensor !=0)  lightLevel = 500 * pow(R_sensor, -1.4); 

}

void CalculateDistance(){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration*.0343)/2;
}

void left(){
  analogWrite(FORWARD_LEFT_WHEEL, MAX_PWM);
  analogWrite(BACK_LEFT_WHEEL, MAX_PWM);
  analogWrite(FORWARD_RIGHT_WHEEL, MIN_PWM);
  analogWrite(BACK_RIGHT_WHEEL, MIN_PWM);
}
void right(){
  analogWrite(FORWARD_LEFT_WHEEL, MIN_PWM);
  analogWrite(BACK_LEFT_WHEEL, MIN_PWM);
  analogWrite(FORWARD_RIGHT_WHEEL, MAX_PWM);
  analogWrite(BACK_RIGHT_WHEEL, MAX_PWM);
}
void forward(){
  analogWrite(FORWARD_LEFT_WHEEL, MAX_PWM);
  analogWrite(BACK_LEFT_WHEEL, MAX_PWM);
  analogWrite(FORWARD_RIGHT_WHEEL, MAX_PWM);
  analogWrite(BACK_RIGHT_WHEEL, MAX_PWM);
}
void back(){
  direction = 0;
  analogWrite(FORWARD_LEFT_WHEEL, MAX_PWM);
  analogWrite(BACK_LEFT_WHEEL, MAX_PWM);
  analogWrite(FORWARD_RIGHT_WHEEL, MAX_PWM);
  analogWrite(BACK_RIGHT_WHEEL, MAX_PWM);
}