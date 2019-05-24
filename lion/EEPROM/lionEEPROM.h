#ifndef lioneeprom_h
#define lioneeprom_h

/*
Note: if you alter something with set* changes will apply after ::save() only
*/
#include "WProgram.h"
#include "Arduino.h"
#include <EEPROM.h>

#define EEPROM_VERSION 0
class lionEEPROM
{
    public:
        lionEEPROM();
        //yeah... i know about public/private things, but we should preserve as much memory in progmem/ram as possible.
        //i don't think you will try to make worse for yourself. It will be your fault if you alter info with no
        //  particular purpose
        bool initialized;

        lionModuleMemory memory;

        //and i do afraid of memory leaks, so i use as much predetermined memory as suitable
        char initDate[11]; //01.12.2012 + \0
        byte ver;
        char netIP[16];//255.255.255.255 + \0
        char netMAC[18];//E1:E2:E3:E4:E5:E6 + \0
        char switchers[12];//--tff--ff-- + \0

        //setters
        void setIP(byte ip1, byte ip2, byte ip3, byte ip4);
        void setMAC(byte mac1, byte mac2, byte mac3, byte mac4, byte mac5, byte mac6);
        void setSwitch(byte indx, byte value);

        //apply+get
        void save(byte dateDD, byte dateMM, byte dateYY);
        bool load();
};

struct lionModuleMemory{
    char init[4];
    byte date[3];
    byte ver;
    byte ip[4];
    byte mac[6];
    byte switchers[11];
}

#endif
