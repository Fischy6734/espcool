#include <Arduino.h>   // <-- Required for all Arduino core definitions

void setup() {
    // Configure the built‑in LED pin as an output
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    // Turn the LED on
    digitalWrite(
