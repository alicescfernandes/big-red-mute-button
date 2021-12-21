#define PIN_LED 9
#define PIN_BTN 2 // must have interrupts on pin
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(PIN_BTN, INPUT);
  pinMode(PIN_LED,OUTPUT);
  digitalWrite(PIN_LED,LOW);    
  attachInterrupt(digitalPinToInterrupt(PIN_BTN), teste, FALLING);
}

unsigned long int lastTime = 0;
void teste() {
  unsigned long int currentTime = millis();
    if((currentTime - lastTime) > 200){
       Serial.print(0);
       lastTime = millis();
    }
    
}


void loop() {

  if(Serial.available()){
    
    byte byteChar = Serial.read();
    if(byteChar == 48){
      digitalWrite(PIN_LED,LOW);
     
    }

    if(byteChar == 49){
      digitalWrite(PIN_LED,HIGH);
     }
  }
}
