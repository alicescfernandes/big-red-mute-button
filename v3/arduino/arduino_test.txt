#include <Adafruit_NeoPixel.h>

#define BUTTON D1
#define PIN_MODE INPUT_PULLUP // D1 can use the internal pull up resistor

#define PIN D9
#define NUMPIXELS 12

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 175

// Sketch to setup the individual components before bringing in the bluetooth code
// it's also a good small scale test for the button and light components

void setup() {
  // Setup the pushbutton as an input and enable internal pullup
  pinMode(BUTTON, PIN_MODE);

  // Setup Serial Port
  Serial.begin(9600);

  pixels.begin();

}

void loop() {
  // Print pushbutton state to serial monitor
  int buttonRead = 1 - digitalRead(BUTTON); 
  Serial.println(buttonRead);
  
  // Neopixel code
  pixels.clear();

  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 255, 255)); // RGB
  }
  
  pixels.show();
}