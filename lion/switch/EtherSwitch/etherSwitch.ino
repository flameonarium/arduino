#include <SPI.h>
#include <Ethernet.h>
//#define DEBUG
#define REQ_LEN 256
#define SWCH_PIN 9

byte mac[] = {0x01, 0x01, 0x01, 0x01, 0xFF, 0x01};
IPAddress ip(192, 168, 0, 60);
EthernetServer server(80);

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial);
  Serial.print("starting... ");
#endif

  Ethernet.begin(mac, ip);
  server.begin();
  pinMode(SWCH_PIN, OUTPUT);
  onOFF();

#ifdef DEBUG
  Serial.println("OK");
  Serial.print("server IP: ");
  Serial.println(Ethernet.localIP());
#endif
}


void loop() {
  EthernetClient client = server.available();
  if (client) {
#ifdef DEBUG
  Serial.println("<client>");
#endif
    String sRequest = "null";
    String htmlBody = "<html>Unknown</html>";
    sRequest = getFromClient(client);
    
#ifdef DEBUG
  Serial.println(sRequest);
#endif

    if(sRequest.startsWith("GET /?turn=on "))
      htmlBody = onON();
    else if(sRequest.startsWith("GET /?turn=off "))
      htmlBody = onOFF();
    printToClient(client, htmlBody);
    // give the web browser time to receive the data
    delay(1000);
    client.stop();
    
#ifdef DEBUG
  Serial.println("<client/>");
#endif

    Ethernet.maintain();
  }
}


String getFromClient(EthernetClient client){
  String sRequest = "";
/*#ifdef DEBUG
  int toRead = client.available();
  Serial.print("client.available():");
  Serial.println(toRead);
#endif*/
  while (client.available()>0) {
    char c = client.read();
    if(sRequest.length() < REQ_LEN)
      sRequest += c;
/*#ifdef DEBUG
  Serial.print(c);
  Serial.print(":");
  Serial.print(sRequest.length());
  Serial.print(":");
  Serial.println(client.available());
#endif*/
  }
#ifdef DEBUG
  Serial.println(sRequest);
#endif
  return sRequest;
}


void printToClient(EthernetClient client, String sBody){
  if (client.connected()) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println(sBody);
#ifdef DEBUG
  Serial.println(">>OUTPUT>>");
  Serial.println(sBody);
  Serial.println("<<OUTPUT<<");
#endif
  }
}

String onON(){
  digitalWrite(SWCH_PIN, HIGH);
  //ВКЛ = &#1042;&#1050;&#1051;
#ifdef DEBUG
  Serial.println("Turn --> ON");
#endif
  //return "<head><style>a{text-decoration:none;}</style></head><html><font size=30>&#9830;&nbsp;<a href='/?turn=on'><b>&#1042;&#1050;&#1051;</b></a></br>  &nbsp;&nbsp;&nbsp;<a href='/?turn=off'>&#1042;&#1067;&#1050;&#1051;</a></font></html>";
  return "<html><font size=30>&#9830;&nbsp;<a href='/?turn=on'><b>&#1042;&#1050;&#1051;</b></a></br>  &nbsp;&nbsp;&nbsp;<a href='/?turn=off'>&#1042;&#1067;&#1050;&#1051;</a></font></html>";
}

String onOFF(){
  digitalWrite(SWCH_PIN, LOW);
  //ВЫКЛ = &#1042;&#1067;&#1050;&#1051;
#ifdef DEBUG
  Serial.println("Turn --> OFF");
#endif
  //return "<head><style>a{text-decoration:none;}</style></head><html><font size=30>&nbsp;&nbsp;&nbsp;<a href='/?turn=on'>&#1042;&#1050;&#1051;</a></br>  &#9830;&nbsp;<a href='/?turn=off'><b>&#1042;&#1067;&#1050;&#1051;</b></a></font></html>";
  return "<html><font size=30>&nbsp;&nbsp;&nbsp;<a href='/?turn=on'>&#1042;&#1050;&#1051;</a></br>  &#9830;&nbsp;<a href='/?turn=off'><b>&#1042;&#1067;&#1050;&#1051;</b></a></font></html>";
}
