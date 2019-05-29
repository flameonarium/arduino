#include "lionEEPROM.h"

lionEEPROM props;

void setup()
{
    Serial.begin(115200);
    props.load();
    Serial.println("init: " + (props.initialized ? "true" : "false"));
    Serial.println("initDate: " + props.initDate);
    Serial.println("ver: " + itoa(props.ver));
    Serial.println("netIP: " + props.netIP);
    Serial.println("netMAC: " + props.netMAC);
    Serial.println("switchers: " + props.switchers);

    if(!props.initialized){
        println("Save...");
        props.setIP(192, 168, 111, 111);
        props.setMAC(1, 1, 1, 1, 1, 1);
        props.setSwitch(9, 1);

        props.save(19, 06, 01); //meaning 2019 jun 01
        Serial.println("init: " + (props.initialized ? "true" : "false"));
        Serial.println("initDate: " + props.initDate);
        Serial.println("ver: " + itoa(props.ver));
        Serial.println("netIP: " + props.netIP);
        Serial.println("netMAC: " + props.netMAC);
        Serial.println("switchers: " + props.switchers);
    }
}

void loop()
{
  while(true);
}
