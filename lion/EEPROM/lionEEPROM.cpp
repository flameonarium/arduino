#include "WProgram.h"
#include "lionEEPROM.h"

lionEEPROM::lionEEPROM(){

}

void lionEEPROM::initMemory(){
    int i;
    memory.init = "init";
    memory.ver = EEPROM_VERSION;
    for(i = 0; i < 3; i++) memory.date[i] = 0;
    for(i = 0; i < 4; i++) memory.ip[i] = 0;
    for(i = 0; i < 6; i++) memory.mac[i] = 0;
    for(i = 0; i < 11; i++) memory.switchers[i] = 0;
}


bool lionEEPROM::load(){
    int i;
    char str[16];
    //on construct do a read of structure
    EEPROM.get(0, memory);

    //check for "init" sequance. If it is already in the memory - the memory was inititalized previously
    if(initialized = (strncmp(memory.init, "init", sizeof(memory.init)) == 0)){
        //initDate
        initDate[0] = '\0';
        itoa(memory.date[0] + 2000, &str);
        strcat(initDate, str);
        for(i = 1; i < 3; i++){
            itoa(memory.date[i], &str);
            strcat(initDate, str);
            strcat(initDate, '.');
        }

        //ver
        ver = memory.ver;

        //netIP
        netIP[0] = '\0';
        for(i = 0; i < 4; i++){
            itoa(memory.ip[i], &str);
            strcat(netIP, str);
            if(i != 3)
                strcat(netIP, '.');
        }

        //netMAC
        netMAC[0] = '\0';
        for(i = 0; i < 6; i++){
            itoa(memory.mac[i], &str, 16);
            strcat(netMAC, str);
            if(i != 5)
                strcat(netMAC, ':');
        }

        //char switchers[12];//--tff--ff-- + \0
        switchers[12] = '\0';
        for(i = 0; i < 11; i++){
            switch(memory.switchers[i]){
                case 0:
                    switchers[i] = '-';
                    break;
                case 1:
                    switchers[i] = 't';
                    break;
                case 2:
                    switchers[i] = 'f';
                    break;
                default:
                    switchers[i] = 'E';
            }
        }
    }else{
        //if it wasn't init before we should prepare for saving
        initMemory();
    }

    return initialized;
}


void lionEEPROM::save(byte dateYY, byte dateMM, byte dateDD){
    memory.date[0] = dateYY;
    memory.date[1] = dateMM;
    memory.date[2] = dateDD;
    memory.ver = EEPROM_VERSION;
    EEPROM.put(0, memory);
    load();
}


void lionEEPROM::setIP(byte ip1, byte ip2, byte ip3, byte ip4){
    memory.ip[0] = ip1;
    memory.ip[1] = ip2;
    memory.ip[2] = ip3;
    memory.ip[3] = ip4;
}


void lionEEPROM::setMAC(byte mac1, byte mac2, byte mac3, byte mac4, byte mac5, byte mac6){
    memory.mac[0] = mac1;
    memory.mac[1] = mac2;
    memory.mac[2] = mac3;
    memory.mac[3] = mac4;
    memory.mac[4] = mac5;
    memory.mac[5] = mac6;
}


void lionEEPROM::setSwitch(byte indx, byte value){
    if(indx < 11)
        memory.switchers[indx] = value;
}