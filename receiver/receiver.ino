// Receptor improved

#include <IRremote.hpp>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// I2C address 0x27 is used for the LCD, with dimensions 20x4
LiquidCrystal_I2C lcd(0x27, 20, 4);  

// Temperatura
const int pin_temp = 8;

// Servos
Servo lateral;
const int pin_servo_lateral = 9;
const int pin_servo_superior = 10;
Servo superior;

// Pines de conexión
const int irRec = 2; // Pin del receptor IR
const int salida = 3; // Pin de abanico ???
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

const unsigned long codeSwingLado = 0xFFFF20;
bool swingLadoEnabled = false;

const unsigned long codeSwingArriba = 0xFFFF30;
bool swingArribaEnabled = false;

String cmd = ""; // Initialize cmd properly

const unsigned long debounceDelay = 200; // Delay to filter out noise
unsigned long lastDebounceTime = 0;
unsigned long lastValidCodeTime = 0;

void setup() {
  pinMode(salida, OUTPUT);
  config_fan();
  lateral.attach(pin_servo_lateral);
  superior.attach(pin_servo_superior);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1,0);
  Serial.begin(9600);
  lcd.print("Waiting...");
  Serial.println("Waiting...");
  IrReceiver.begin(irRec);  // Inicializa el receptor IR
}

void loop() {

  if (IrReceiver.decode()) {
    unsigned long receivedCode = IrReceiver.decodedIRData.decodedRawData;
    Serial.print("0x");
    Serial.println(receivedCode, HEX);
    unsigned long currentTime = millis();

    if (currentTime - lastDebounceTime > debounceDelay) {
      if (receivedCode != lastValidCodeTime) {
        lastValidCodeTime = receivedCode;
        lastDebounceTime = currentTime;

        Serial.print("0x");
        Serial.println(receivedCode, HEX);

        if (receivedCode == codeON) {
          isOn = !isOn;
          if (isOn){
            digitalWrite(fan, HIGH);
            setSpeed(speed)
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
            case codeSwingLado:
              swingLadoEnabled = !swingLadoEnabled;

              cmd = String("Swing lado: ") + String(swingLadoEnabled ? "enabled" : "disabled");
              break;
            case codeSwingArriba:
              swingArribaEnabled = !swingArribaEnabled;
              cmd = String("Swing arriba: ") + String(swingArribaEnabled ? "enabled" : "disabled");
              break;
            default:
              break;
          }
        }
      }
    }

    IrReceiver.resume();
  }

  if (cmd != "") {
    Serial.println(cmd);
    lcd.clear(); 
    lcd.setCursor(0, 0);
    lcd.print(cmd);
    cmd = "";
  }
}

void config_fan(){
 TCCR2A=0X23;
  TCCR2B=0X0A;
  OCR2A=100;
  OCR2B=0;
}

void setSpeed(byte ocrb){
  OCR2B=ocrb;
}


/*
  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15 ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15 ms for the servo to reach the position
  }
*/