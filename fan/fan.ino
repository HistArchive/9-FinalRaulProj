int salida=3;
int valorpwn=0;
double pulso=0;
double frecuencia=0;
int tiempomuestra=1000;
long tiempo=millis();

void setup() {
  pinMode(salida, OUTPUT);
  configuracionpwn();
  //ciclopwn(valorpwn);
  Serial.begin(9600);
  pinMode(7, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()>0){
    valorpwn=Serial.parseInt();
    if (valorpwn == 1){
      Serial.println("a");
      digitalWrite(7, HIGH);
    } else if (valorpwn == -1) {
      Serial.println("b");
      digitalWrite(7, LOW);
    }
    if (valorpwn > 0 && valorpwn < 101) {
      Serial.println(valorpwn);
      ciclopwn(valorpwn);
    }
  }
}

void configuracionpwn(){
 TCCR2A=0X23;
  TCCR2B=0X0A;
  OCR2A=100;
  OCR2B=0;
}

void ciclopwn(byte ocrb){
  OCR2B=ocrb;
}
