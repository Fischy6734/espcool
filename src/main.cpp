#include <Arduino.h> // <-- Required for all Arduino core functions

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // Configure the LED pin as an output
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH); // Turn the LED on
  delay(500);                      // Wait for 0.5 seconds

  digitalWrite(LED_BUILTIN, LOW);  // Turn the LED off
  delay(500);                      // Wait for 0.5 seconds
}
