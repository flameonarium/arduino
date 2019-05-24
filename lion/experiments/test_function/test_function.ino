#include "lionEEPROM.h"

lionEEPROM props;

void setup()
{
  Serial.begin(115200);
  props.load();
  Serial.println("init: " + (props.initialized ? "true" : "false"));

  for(int i = 0; i < 4; i++)
  {
    Serial.println(char(EEPROM.read(i)));
  }
  Serial.println("");

  props.setIP(192,168,111,111);
  props.setMAC(1,1,1,1,1,1);
  props.set

}

void loop()
{
  while(true);
}
