#define notdebug

void setup() {
#ifdef debug
  Serial.begin(115200);
#endif
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, false);
}

void loop() {
  int val = 0;
  bool doLed = false;
  // put your main code here, to run repeatedly:
  for(int i=A0; i<A15; i++){
    val = analogRead(i);
    #ifdef debug
    Serial.print(val);
    #endif
    if(val > 980){
      #ifdef debug
      Serial.print("!!!,");
      #endif
      doLed = true;
    }
    #ifdef debug
    else
      Serial.print(",   ");
    #endif
  }
  Serial.println();
  digitalWrite(LED_BUILTIN, doLed);
  delay(100);
}
