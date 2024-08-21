// Receptor improved

#include <IRremote.hpp>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <Arduino.h>
#include "DHT_Async.h"

// I2C address 0x27 is used for the LCD, with dimensions 20x4
LiquidCrystal_I2C lcd(0x27, 20, 4);  

// Temperatura
#define DHT_SENSOR_PIN 10
#define DHT_SENSOR_TYPE DHT_TYPE_11
DHT_Async dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
float temperature, humidity;

// Servos
Servo lateralServo;
Servo superiorServo;
int lateralServoPin = 9; // Change to your servo pin
int superiorServoPin = 10; // Change to your servo pin

int lateralServoPos = 0; // Initial position for lateral servo
int superiorServoPos = 0; // Initial position for superior servo

int lateralServoSpeed = 1; // Speed of the lateral servo movement
int superiorServoSpeed = 1; // Speed of the superior servo movement

bool swingLateralEnabled = false;
bool swingArribaEnabled = false;

unsigned long lastLateralUpdateTime = 0;
unsigned long lastSuperiorUpdateTime = 0;

unsigned long updateInterval = 20; // Update interval in milliseconds

// Pines de conexión
const int salida = 2; // Pin de abanico ???
const int irRec = 3; // Pin del receptor IR
const int fan = 7; // Actual abanico pin
const int fanIncrease = 25;
int fanSpeed = 0;

// Códigos IR
bool isOn = false;
const unsigned long codeON = 0xFFFF00;

const unsigned long codeMoreFan = 0xFFFF10;
const unsigned long codeLessFan = 0xFFFF11;
const unsigned long codePlusUltra = 0xFFFF1F;
int speed = 1; // Initialize speed to 1

const unsigned long codeSwingLateral = 0xFFFF20;
const unsigned long codeSwingArriba = 0xFFFF30;

String cmd = ""; // Initialize cmd properly

const unsigned long debounceDelay = 200; // Delay to filter out noise
unsigned long lastDebounceTime = 0;
unsigned long lastValidCodeTime = 0;

void setup() {
  pinMode(salida, OUTPUT);
  config_fan();
  lateralServo.attach(lateralServoPin);
  superiorServo.attach(superiorServoPin);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  Serial.begin(115200);
  lcd.print("Waiting...");
  Serial.println("Waiting...");
  IrReceiver.begin(irRec);  // Inicializa el receptor IR
}

void loop() {
  
  /* Measure temperature and humidity.  If the functions returns
    true, then a measurement is available. */
  if (measure_environment(&temperature, &humidity)) {
    Serial.print("T = ");
    Serial.print(temperature, 1);
    Serial.print(" deg. C");
    Serial.println();
    lcd.setCursor(5, 3);
    lcd.print(temperature);
    lcd.setCursor(10, 3);
    lcd.print("C");
  }
  if (IrReceiver.decode()) {
    unsigned long receivedCode = IrReceiver.decodedIRData.decodedRawData;
    Serial.print("Received code: 0x");
    Serial.println(receivedCode, HEX);
    unsigned long currentTime = millis();

    if (currentTime - lastDebounceTime > debounceDelay) {
      if (receivedCode != lastValidCodeTime) {
        lastValidCodeTime = receivedCode;
        lastDebounceTime = currentTime;

        Serial.print("Valid code: 0x");
        Serial.println(receivedCode, HEX);

        if (receivedCode == codeON) {
          isOn = !isOn;
          if (isOn){
            digitalWrite(fan, HIGH);
            setSpeed(speed);
          } else {
            digitalWrite(fan, LOW);
          }
          cmd = String("Turned ") + String(isOn ? "on" : "off");
        } else if (isOn) {
          switch (receivedCode) {
            case codeMoreFan:
              if (speed <= 50) { speed+=fanIncrease; }
              cmd = String("Intensidad: ") + String(speed);
              setSpeed(speed);
              break;
            case codeLessFan:
              if (speed >= 25) { speed--; }
              cmd = String("Intensidad: ") + String(speed);
              setSpeed(speed);
              break;
            case codePlusUltra:
              speed = 100;
              cmd = String("Intensidad: ") + String(speed);
              setSpeed(speed);
              break;
            case codeSwingLateral:
              swingLateralEnabled = !swingLateralEnabled;
              cmd = String("Swing lado: ") + String(swingLateralEnabled ? "enabled" : "disabled");
              break;
            case codeSwingArriba:
              swingArribaEnabled = !swingArribaEnabled;
              cmd = String("Swing arriba: ") + String(swingArribaEnabled ? "enabled" : "disabled");
              break;
            default:
              break;
          }
        }
      } else {
        Serial.println("Duplicate code received, ignoring.");
      }
    }

    IrReceiver.resume();
  }

  if (cmd != "") {
    Serial.println(cmd);
    //lcd.clear(); 
    lcd.setCursor(0, 0);
    lcd.print(cmd);
    cmd = "";
  }

  // Non-blocking lateral servo movement
  if (swingLateralEnabled) {
    unsigned long currentTime = millis();
    if (currentTime - lastLateralUpdateTime >= updateInterval) {
      lastLateralUpdateTime = currentTime;
      lateralServoPos += lateralServoSpeed;
      if (lateralServoPos >= 180 || lateralServoPos <= 0) {
        lateralServoSpeed = -lateralServoSpeed; // Reverse direction
      }
      lateralServo.write(lateralServoPos);
    }
  }

  // Non-blocking superior servo movement
  if (swingArribaEnabled) {
    unsigned long currentTime = millis();
    if (currentTime - lastSuperiorUpdateTime >= updateInterval) {
      lastSuperiorUpdateTime = currentTime;
      superiorServoPos += superiorServoSpeed;
      if (superiorServoPos >= 180 || superiorServoPos <= 0) {
        superiorServoSpeed = -superiorServoSpeed; // Reverse direction
      }
      superiorServo.write(superiorServoPos);
    }
  }
}

void setSpeed(byte ocrb){
  OCR2B=ocrb;
}

static bool measure_environment(float *temperature, float *humidity) {
  static unsigned long measurement_timestamp = millis();
  /* Measure once every four seconds. */
  if (millis() - measurement_timestamp > 4000ul) {
      if (dht_sensor.measure(temperature, humidity)) {
        measurement_timestamp = millis();
        return (true);
      }
  }

  return (false);
}

void config_fan(){
  TCCR2A=0X23;
  TCCR2B=0X0A;
  OCR2A=100;
  OCR2B=0;
}