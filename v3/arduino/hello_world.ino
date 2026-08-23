#define BUTTON D9


// Sketch to setup the individual components before bringing in the bluetooth code
// it's also a good small scale test for the button and light components

void setup() {
  // Setup the pushbutton as an input and enable internal pullup
  pinMode(BUTTON, INPUT_PULLUP);

  // Setup Serial Port
  Serial.begin(9600);
}

void loop() {
  // Print pushbutton state to serial monitor
  Serial.println(digitalRead(BUTTON));
}