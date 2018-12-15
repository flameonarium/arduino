#include <SPI.h>
#include <Ethernet.h>
//#define DEBUG
#define REQ_LEN 256

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 0, 52);
EthernetServer server(80);

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial);
  Serial.print("starting... ");
#endif

  Ethernet.begin(mac, ip);
  server.begin();
  pinMode(7, OUTPUT);
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
    String htmlBody = "null";
    sRequest = getFromClient(client);
    
#ifdef DEBUG
  Serial.println(sRequest);
#endif

    if(sRequest.startsWith("GET /?turn=on "))
      htmlBody = onON();
    else if(sRequest.startsWith("GET /?turn=off "))
      htmlBody = onOFF();
    /*else
      htmlBody = "<html><h1>Unknown</h1></html>";*/
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
  }
}

String onON(){
  digitalWrite(7, HIGH);
  //ВКЛ = &#1042;&#1050;&#1051;
  return "<head><style>a{text-decoration:none;}</style></head><html><font size=30>&#9830;&nbsp;<a href='/?turn=on'><b>&#1042;&#1050;&#1051;</b></a></br>  &nbsp;&nbsp;&nbsp;<a href='/?turn=off'>&#1042;&#1067;&#1050;&#1051;</a></font></html>";
}

String onOFF(){
  digitalWrite(7, LOW);
  //ВЫКЛ = &#1042;&#1067;&#1050;&#1051;
  return "<head><style>a{text-decoration:none;}</style></head><html><font size=30>&nbsp;&nbsp;&nbsp;<a href='/?turn=on'>&#1042;&#1050;&#1051;</a></br>  &#9830;&nbsp;<a href='/?turn=off'><b>&#1042;&#1067;&#1050;&#1051;</b></a></font></html>";
}

