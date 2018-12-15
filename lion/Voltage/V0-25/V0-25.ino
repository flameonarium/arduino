void setup()
{
  Serial.begin(115200);
}

void loop()
{
  float val2;
  val2 = (analogRead(0) / 40.92);
  Serial.println(String(val2) + 'V');
  delay(1000);
}
