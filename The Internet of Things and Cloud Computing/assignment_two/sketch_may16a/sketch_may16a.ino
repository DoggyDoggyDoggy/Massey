const unsigned long INTERVAL = 30UL * 60UL * 1000UL; // 30 minutes
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial) ;
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= INTERVAL) {
    previousMillis = currentMillis;
    int moisture = analogRead(A0);
    Serial.println(moisture);
  }

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "read") {
      int moisture = analogRead(A0);
      Serial.println(moisture);
    }
  }
}
