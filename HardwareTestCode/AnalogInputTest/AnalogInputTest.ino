void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.print("V1: ");
  Serial.print(analogRead(15));
  Serial.print("\tA1: ");
  Serial.print(analogRead(4));
  Serial.print("\tV2: ");
  Serial.print(analogRead(2));
  Serial.print("\n");
  delay(100);
}
