void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(2, INPUT);
  pinMode(3,OUTPUT);
  digitalWrite(3,HIGH); //RELAY
  attachInterrupt(digitalPinToInterrupt(2), teste, FALLING);
}

unsigned long int lastTime = 0;
void teste() {
  unsigned long int currentTime = millis();
    if((currentTime - lastTime) > 200){
       Serial.print(0);
       lastTime = millis();
    }
    
}

char buf[1];
void loop() {
  if(Serial.available()){
    byte statusByte = Serial.readBytes(buf,1);
    if(buf[0] == '1'){
      digitalWrite(3,LOW);
     
    }

    if(buf[0] == '0'){
      digitalWrite(3,HIGH); //RELAY
     }
  }
}
