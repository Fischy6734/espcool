#include <Arduino.h>   // <-- brings in all Arduino core symbols

void setup() {
    // Configure the built‑in LED pin as an output
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    // Turn the LED on
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);   // wait 500 ms

    // Turn the LED off
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);   // wait 500 ms
}
