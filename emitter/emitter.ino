#include <IRremote.hpp>

const int irSendPin = 2;   // Pin del emisor IR

// Button pins
const int turboBtnPin = 4;
const int swingLadoBtnPin = 5;
const int increaseIntensityBtnPin = 6;
const int swingArribaBtnPin = 7;
const int onOffBtnPin = 8;
const int decreaseIntensityBtnPin = 9;

const int numBtn = 6;
int btn[numBtn] = {turboBtnPin, swingLadoBtnPin, increaseIntensityBtnPin, swingArribaBtnPin, onOffBtnPin, decreaseIntensityBtnPin};

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

const unsigned long debounceDelay = 200; // Debounce delay in milliseconds

void setup() {
  for (int i = 0; i < numBtn; ++i) {
    pinMode(btn[i], INPUT_PULLUP);
  }
  IrSender.begin(irSendPin);  // Inicializa el emisor IR
  Serial.begin(9600);
}

void handleButtonPress(int pin, unsigned long code, const char* message, bool* toggle = nullptr) {
  if (digitalRead(pin) == LOW) {
    if (toggle) {
      *toggle = !*toggle;
      Serial.println(String(message) + (*toggle ? " enabled" : " disabled"));
    } else {
      Serial.println(message);
    }
    IrSender.sendNEC(code, 32); // Enviar código IR
    delay(debounceDelay); // Anti-rebote
  }
}

void changeSpeed(int delta) {
  speed += delta;
  if (speed < 1) speed = 1;
  if (speed > 4) speed = 4;
  Serial.print("Intensidad: ");
  Serial.println(speed);
}

void loop() {
  // Envío de códigos IR cuando se presionan los botones
  handleButtonPress(onOffBtnPin, codeON, isOn ? "Turned off" : "Turned on", &isOn);
  if (isOn) {
    handleButtonPress(turboBtnPin, codePlusUltra, "TURBO ENGAGED");
    if (digitalRead(turboBtnPin) == LOW) {
      speed = 5;
      Serial.println("Intensidad: 5");
    }
    handleButtonPress(swingLadoBtnPin, codeSwingLado, "Swing lateral", &swingLadoEnabled);
    if (digitalRead(increaseIntensityBtnPin) == LOW) {
      changeSpeed(1);
      IrSender.sendNEC(codeMoreFan, 32); // Enviar código IR
      delay(debounceDelay); // Anti-rebote
    }
    handleButtonPress(swingArribaBtnPin, codeSwingArriba, "Swing arriba abajo", &swingArribaEnabled);
    if (digitalRead(decreaseIntensityBtnPin) == LOW) {
      changeSpeed(-1);
      IrSender.sendNEC(codeLessFan, 32); // Enviar código IR
      delay(debounceDelay); // Anti-rebote
    }
  }
}