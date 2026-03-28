void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("ESP32-S3 GPIO Monitor - Reading pins 0-21");
  Serial.println("GPIO\tState");
  
  // Set pins as input
  for (int pin = 0; pin <= 21; pin++) {
    pinMode(pin, INPUT);
  }
}

void loop() {
  for (int pin = 0; pin <= 21; pin++) {
    bool state = digitalRead(pin);
    Serial.print(pin);
    Serial.print("\t");
    Serial.println(state ? "HIGH" : "LOW");
  }
  Serial.println("---");
  delay(2000);
}
