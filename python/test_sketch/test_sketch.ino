#define PIN_LED 13
#define PIN_BTN 2 // must have interrupts on pin
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(PIN_BTN, INPUT);
  pinMode(PIN_LED,OUTPUT);
  digitalWrite(PIN_LED,LOW);    
  //attachInterrupt(digitalPinToInterrupt(PIN_BTN), teste, RISING);
}

unsigned long int lastTime = 0;
byte currentValue = 0;
byte prevValue = currentValue;


void teste() {
  unsigned long int currentTime = millis();
  
    if((currentTime - lastTime) > 1000){
       Serial.println("cenas");
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
  unsigned long int currentTime = millis();
  prevValue = currentValue;
  currentValue = digitalRead(PIN_BTN);

  boolean timeHasPassed = (currentTime - lastTime) > 200;

  if(prevValue == 1 && currentValue == 0 && timeHasPassed){
     Serial.println("0");
     lastTime = millis();  
  } 
  delay(100);
  
}
