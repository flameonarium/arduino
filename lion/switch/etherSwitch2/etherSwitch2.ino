#include <SPI.h>
#include <Ethernet.h>
#define SCRIPT_VERSION "2.1"
#define SCRIPT_NAME "EtherSwitch"
#define DEBUG
#define DEFAULT_SWITCH true
//the maximum length of client request for parsing
#define REQ_LEN 250
//time for one client to be processed, ms
#define DELAY_MS 500
//time between client requests ethernet
#define DELAY_LOOP 100
//URL parameters size. MAX_PARAMS - how many. URL for example for 4/10/10:
//http://x.x.x.x/urlPagexxx?urlParams1=urlValues1&urlParams2=urlValues2&urlParams3=urlValues3&urlParams4=urlValues4
#define MAX_PARAMS 4
#define MAX_PARAM_NAME_LEN 10
#define MAX_PARAM_VAL_LEN 10
//swithcers count. 0 and 1 don't touch. There are no D0 and D1 on Arduino.
#define MAX_SWITCHERS 11

//after parsing of client requested url here would be the result of parsing
char urlParams[MAX_PARAMS][MAX_PARAM_NAME_LEN];
char urlValues[MAX_PARAMS][MAX_PARAM_VAL_LEN];
char urlPage[MAX_PARAM_NAME_LEN];
//array of switchers. Every switch number (n) stands for Dn output of Arduino. So there is no switchers[0] and switchers[1]
bool switchers[MAX_SWITCHERS];


//MAX_STR stands for the length of the longest string in the PROGMEM array.
//if your strings extend from this number - you should change it
//this is for optimisation of memory usage for buffer string. Not for every array object
#define MAX_STR 45
//count of such strings. Any asked number beyond this one from PROGMEM will return ""
#define STR_CNT 23
const char string_00[] PROGMEM = "manual";
const char string_01[] PROGMEM = "auto"; //--
const char string_02[] PROGMEM = "HTTP/1.1 200 OK\r\nContent-Type: text/html";
const char string_03[] PROGMEM = "Connection: close\r\n\r\n<!DOCTYPE HTML>";
const char string_04[] PROGMEM = "<head><style>a{text-decoration:none;}";
const char string_05[] PROGMEM = "</style></head><html><font size=30>";
const char string_06[] PROGMEM = "<b>";
const char string_07[] PROGMEM = "</b>";
const char string_08[] PROGMEM = "&#9830;&nbsp;";//bold point near selected option
const char string_09[] PROGMEM = "&nbsp;&nbsp;&nbsp;";//no bold point for unselected option
const char string_10[] PROGMEM = "<a href='/manual?turn=on&num=7'>";
const char string_11[] PROGMEM = "<a href='/manual?turn=off&num=7'>";
const char string_12[] PROGMEM = "&#1042;&#1050;&#1051;";//ON in russian
const char string_13[] PROGMEM = "&#1042;&#1067;&#1050;&#1051;";//OFF in russian
const char string_14[] PROGMEM = "</a></br>";
const char string_15[] PROGMEM = "</a></font></html>";
const char string_16[] PROGMEM = "";
const char string_17[] PROGMEM = "turn";
const char string_18[] PROGMEM = "on";
const char string_19[] PROGMEM = "off";
const char string_20[] PROGMEM = "status";
const char string_21[] PROGMEM = "switch";
const char string_22[] PROGMEM = "num";

const char* const string_table[] PROGMEM = {
  string_00, string_01, string_02, string_03, string_04,
  string_05, string_06, string_07, string_08, string_09,
  string_10, string_11, string_12, string_13, string_14,
  string_15, string_16, string_17, string_18, string_19,
  string_20, string_21, string_22
  };
char buffer[MAX_STR];


byte mac[] = {0x01, 0x01, 0x01, 0x01, 0xFF, 0x01};
IPAddress ip(192, 168, 0, 60);
EthernetServer server(80);


char *memStrNum(byte strNum){
  if(strNum>=STR_CNT)
    strcpy(buffer, "");
  else
    strcpy_P(buffer, (char*)pgm_read_word(&(string_table[strNum])));
  return buffer;
}


//clean url parameters array
void clearParams(){
#ifdef DEBUG
  Serial.println("clearParams>");
#endif
  for(byte i = 0; i<MAX_PARAMS; i++){
    for(byte j = 0; j<MAX_PARAM_NAME_LEN; j++)
      urlParams[i][j] = '\x0';
    for(byte k = 0; k<MAX_PARAM_VAL_LEN; k++)
      urlValues[i][k] = '\x0';
  }
  for(byte l = 0; l<MAX_PARAM_NAME_LEN; l++)
    urlPage[l] = '\x0';
}


#ifdef DEBUG
void printParams(){
  Serial.print("page: ");
  for(byte l = 0; l<MAX_PARAM_NAME_LEN; l++)
    Serial.print(urlPage[l]);
  Serial.println();
  
  for(byte i = 0; i<MAX_PARAMS; i++){
    Serial.print("[");
    for(byte j = 0; j<MAX_PARAM_NAME_LEN; j++)
      Serial.print(urlParams[i][j]);
    Serial.print("]:[");
    for(byte k = 0; k<MAX_PARAM_VAL_LEN; k++)
      Serial.print(urlValues[i][k]);
    Serial.println("]");
  }
}
#endif


void readClientToNull(EthernetClient client){
#ifdef DEBUG
  Serial.print("readClientToNull> ");
#endif
  char c;
  while (client.available()>0)
    c = client.read();
#ifdef DEBUG
  Serial.println("OK");
#endif
}


//this parses url into parameters arrays
bool getFromClient(EthernetClient client)
{
  char c;
  byte paramNum = 0; //current parameter number in param array
  int urlCharNum = 0; //current url symbol number
  int paramCharNum = 0; //current parameter name character number
  int valueCharNum = 0; //current parameter value character number
  bool isPreUrlPage = true; //are we in position before even page name now
  bool isUrlPage = false; //are we in page name now
  bool isParamName = false; //are we in parameter name now
  bool isParamValue = false; //are we in parameter value now
  clearParams();
#ifdef DEBUG
  Serial.println("getFromClient>");
#endif
  while (client.available()>0)
  {
    c = client.read();
    if(urlCharNum++ < REQ_LEN) //no readings after this point
    {
      if(paramCharNum >= MAX_PARAM_NAME_LEN){//too long param name
        readClientToNull(client);
        return false;
      }
      if(valueCharNum >= MAX_PARAM_VAL_LEN){//too long param value
        readClientToNull(client);
        return false;
      }
      if(isPreUrlPage){//skip until page name
        if(c == '/'){
          isPreUrlPage = false;
          isUrlPage = true;
        }
      }else if(isUrlPage){//if there were no "?" yet - we will read page name
        if(c == '?'){//this means the end of page name and start of param name
            isUrlPage = false;
            isParamName = true;
            paramCharNum = 0;
#ifdef DEBUG
  Serial.print("[page]:[");
  Serial.print(urlPage);
  Serial.print("]");
#endif
          }
        else
          urlPage[paramCharNum++] = c;
      }else if(isParamName){
        if(c == '='){//param name finished
          isParamName = false;
          isParamValue = true;
          valueCharNum = 0;
        }else
          urlParams[paramNum][paramCharNum++] = c;
      }else if(isParamValue){
        if(c == '&'){//param value finished
          isParamValue = false;
          isParamName = true;
          paramCharNum = 0;
          if(++paramNum > MAX_PARAMS){
            readClientToNull(client);
            return false;
          }
        }else if(c == ' '){
          readClientToNull(client);
#ifdef DEBUG
  printParams();
#endif
          return true;
        }else
          urlValues[paramNum][valueCharNum++] = c;
      }
    }else{
      readClientToNull(client);
    }
  }
#ifdef DEBUG
  printParams();
#endif
  return true;
}


void printToClientStart(EthernetClient client){
#ifdef DEBUG
  Serial.println("printToClientStart>");
#endif
  if (client.connected()) {
    client.println(memStrNum(2));
    client.println(memStrNum(3));
  }
}


void onManualSwitchON(EthernetClient client){
#ifdef DEBUG
  Serial.println("onManualSwitchON>");
#endif
  client.println(memStrNum(4));
  client.println(memStrNum(5));
  client.println(memStrNum(8));
  client.println(memStrNum(10));
  client.println(memStrNum(6));
  client.println(memStrNum(12));
  client.println(memStrNum(7));
  client.println(memStrNum(14));
  client.println(memStrNum(9));
  client.println(memStrNum(11));
  client.println(memStrNum(13));
  client.println(memStrNum(15));
}


void onManualSwitchOFF(EthernetClient client){
#ifdef DEBUG
  Serial.println("onManualSwitchOFF>");
#endif
  client.println(memStrNum(4));
  client.println(memStrNum(5));
  client.println(memStrNum(9));
  client.println(memStrNum(10));
  client.println(memStrNum(12));
  client.println(memStrNum(14));
  client.println(memStrNum(8));
  client.println(memStrNum(11));
  client.println(memStrNum(6));
  client.println(memStrNum(13));
  client.println(memStrNum(7));
  client.println(memStrNum(15));
}


void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial);
  Serial.print("starting... ");
#endif

  Ethernet.begin(mac, ip);
  server.begin();

  int i;
  for(i=0; i<MAX_SWITCHERS; i++)
    switchers[i] = DEFAULT_SWITCH;
  //TODO: how to initialize switches?
  //pinMode(7, OUTPUT);
  //digitalWrite(7, LOW);

#ifdef DEBUG
  Serial.println("OK");
  Serial.print("server IP: ");
  Serial.println(Ethernet.localIP());
#endif
}

void badUrl(EthernetClient client){
#ifdef DEBUG
  Serial.println("badUrl>");
#endif
  if (client.connected()) {
    client.println(memStrNum(2));
    client.println(memStrNum(3));
    client.println("<html>Bad url, i think...</html>");
  }
}

//find the "num" param and give it's value. 0 means no "num"
byte findNum(){
#ifdef DEBUG
  Serial.print("findNum> ");
#endif
  byte num = 0;
  byte paramNum = 0;
  while(strlen(urlParams[paramNum])>0){
    if(strncmp(urlParams[paramNum], memStrNum(22), strlen(memStrNum(22))) == 0){//&num=
#ifdef DEBUG
  Serial.print("(");
  Serial.print(urlValues[paramNum]);
  Serial.print(") ");
#endif
      num = atoi(urlValues[paramNum]);
      break;
    }
    paramNum++;
  }
#ifdef DEBUG
  Serial.println(num);
#endif
  return num;
}


void printSwitchStatus(EthernetClient client, bool _status, bool manual){
#ifdef DEBUG
  Serial.print("printSwitchStatus> _status=");
  Serial.print(_status?"true":"false");
  Serial.print(" manual=");
  Serial.println(manual?"true":"false");
#endif
  if(manual){//print for human
    if(_status)
      onManualSwitchON(client);
    else
      onManualSwitchOFF(client);
  }else{//print for automation
    if(_status)
      client.println(memStrNum(18));//on
    else
      client.println(memStrNum(19));//off
  }
}


//command: 0-off, 1-on, 2-switch, 3-status
bool onTurn(EthernetClient client, bool manual, byte command){
#ifdef DEBUG
  Serial.print("onTurn> manual=");
  Serial.print(manual?"true":"false");
  Serial.print(" command=");
  Serial.println(command);
#endif
  byte num = findNum();
  bool isOk = false;
  if(num < 2)
    return false;
  pinMode(num, OUTPUT);
  
  switch(command){
    case 0: //off
      digitalWrite(num, LOW);
      switchers[num] = false;
      isOk = true;
      break;
    case 1: //on
      digitalWrite(num, HIGH);
      switchers[num] = true;
      isOk = true;
      break;
    case 2: //switch
      if(switchers[num])
        digitalWrite(num, LOW);
      else
        digitalWrite(num, HIGH);
      switchers[num] = !switchers[num];
      isOk = true;
      break;
    case 3: //status
      isOk = true;
      break;
  }
  printSwitchStatus(client, switchers[num], manual);
  return isOk;
}


byte mac[] = {0x01, 0x01, 0x01, 0x01, 0xFF, 0x01};


bool onStatus(EthernetClient client){
#ifdef DEBUG
    Serial.print("onStatus> ");
    client.println("DEBUG");
#endif
    client.print("scr_name: ");
    client.println(SCRIPT_NAME);
    client.print("scr_ver: ");
    client.println(SCRIPT_VERSION);
    client.print("server IP: ");
    client.println(Ethernet.localIP());
    client.print("server MAC: ");
    for(i=0; i<6; i++){
        client.print(String(mac[i], HEX));
        client.print(":");
        }
    client.println("");
#ifdef DEBUG
    Serial.println("ok");
#endif
    return true;
}


void loop() {
  delay(DELAY_LOOP);
  EthernetClient client = server.available();
  if (client) {
#ifdef DEBUG
  Serial.println("<client>");
#endif
    bool done = false;
    bool urlOk = getFromClient(client);
    
    if(!urlOk)
      badUrl(client);
    else{
      printToClientStart(client);//wha? rly nid it? dn't thnk so. (after thinking i think it may be a necessity

      bool manual = (strncmp(urlPage, memStrNum(0), strlen(memStrNum(0))) == 0); //"manual"
      byte paramNum = 0;
      while(strlen(urlParams[paramNum])>0){
#ifdef DEBUG
  Serial.print(urlParams[paramNum]);
  Serial.print(":");
  Serial.println(urlValues[paramNum]);
#endif
        if(strncmp(urlParams[paramNum], memStrNum(17), strlen(memStrNum(17))) == 0){            //&turn=
          if(strncmp(urlValues[paramNum], memStrNum(18), strlen(memStrNum(18))) == 0){       //on
            done = onTurn(client, manual, 1);
            break;
          }else if(strncmp(urlValues[paramNum], memStrNum(19), strlen(memStrNum(19))) == 0){ //off
            done = onTurn(client, manual, 0);
            break;
          }else if(strncmp(urlValues[paramNum], memStrNum(21), strlen(memStrNum(21))) == 0){ //switch
            done = onTurn(client, manual, 2);
            break;
          }else if(strncmp(urlValues[paramNum], memStrNum(20), strlen(memStrNum(20))) == 0){ //status
            done = onTurn(client, manual, 3);
            break;
          }
        }
        else if(strncmp(urlValues[paramNum], "status", strlen("status")) == 0){
            done = onStatus(client);
        }
        paramNum++;
      }


      if(!done)
        badUrl(client);
      // give the client web browser time to receive data. Is there a better way for this? Should be something
      delay(DELAY_MS);
      client.stop();
    
#ifdef DEBUG
Serial.println("<client/>");
#endif
    }
    Ethernet.maintain();
  }
}
