void setup() {
  // Initialize the built-in LED pin as an output
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // Turn the LED on
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500); // Wait for 500 milliseconds

  // Turn the LED off
  digitalWrite(LED_BUILTIN, LOW);
  delay(500); // Wait for 500 milliseconds
}
