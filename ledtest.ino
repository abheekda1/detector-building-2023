void testLeds() {
  analogWrite(2, 255);
  delay(1000);
  analogWrite(2, 0);
  delay(1000);
  analogWrite(4, 255);
  delay(1000);
  analogWrite(4, 0);
  delay(1000);
  analogWrite(7, 255);
  delay(1000);
  analogWrite(7, 0);
  delay(1000);
}
