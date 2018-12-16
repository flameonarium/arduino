#include <EEPROM.h>
#define SMS_SECURED_PHONES 2

void setup()
{
  /*digitalWrite(13, HIGH);
  char number[14];
  int romLen = EEPROM.length();
  byte numbPh = SMS_SECURED_PHONES;
  String smsSecuredPhones[SMS_SECURED_PHONES];
  smsSecuredPhones[0] = "+79999999999";
  smsSecuredPhones[1] = "+79232471964";
  int eeAddress = 0;

  
  EEPROM.put(eeAddress, numbPh);
  eeAddress += sizeof(numbPh);
  for(int i=0; i<SMS_SECURED_PHONES; i++)
    {
      smsSecuredPhones[i].toCharArray(number, 14);
      number[13] = 0;
      EEPROM.put(eeAddress, number);
      eeAddress += sizeof(number);
    }
  digitalWrite(13, LOW);*/

  Serial.begin(115200);
  Serial.println(EEPROM.read(0));
  for(int i=1; i<40; i++)
  {
    Serial.print(char(EEPROM.read(i)));
  }
  Serial.println("");
}

void loop()
{
  while(true);
}
