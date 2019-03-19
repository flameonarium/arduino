#include <EtherCard.h>
 
static byte mymac[] = { 0x0,0x0,0x0,0x0,0x1,0x1 };
 
byte Ethernet::buffer[700];
 
void setup ()
{
  Serial.begin(115200);
   
  Serial.print("MAC: ");
  for (byte i = 0; i < 6; ++i)
    {
      Serial.print(mymac[i], HEX);
      if (i < 5)
        Serial.print(':');
    }
  Serial.println();
   
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
    Serial.println(F("Failed to access Ethernet controller"));
   
  //Serial.println(F("Setting up DHCP"));
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failed"));
   
  ether.printIp("My IP: ", ether.myip);
  ether.printIp("Netmask: ", ether.netmask);
  ether.printIp("GW IP: ", ether.gwip);
  ether.printIp("DNS IP: ", ether.dnsip);
}
 
void loop () {}
