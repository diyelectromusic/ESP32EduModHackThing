void setup() {
  pinMode (36, INPUT);
  pinMode (39, INPUT);
  pinMode (34, INPUT);
  pinMode (35, INPUT);
  Serial.begin(9600);
}

void loop() {
  Serial.print("1T: ");
  Serial.print(digitalRead(35));
  Serial.print("\t1G: ");
  Serial.print(digitalRead(34));
  Serial.print("\t2T: ");
  Serial.print(digitalRead(39));
  Serial.print("\t2G: ");
  Serial.print(digitalRead(36));
  Serial.print("\n");
  delay(200);
}
