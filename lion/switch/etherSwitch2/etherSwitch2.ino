#include <SPI.h>
#include <Ethernet.h>
#define DEBUG
#define DEFAULT_SWITCH true
#define REQ_LEN 250
//время на обработку клиенту, ms
#define DELAY_MS 500
//время между опросами ethernet
#define DELAY_LOOP 100
//размерности параметров
#define MAX_PARAMS 4
#define MAX_PARAM_NAME_LEN 10
#define MAX_PARAM_VAL_LEN 10
//количество переключателей. Первые два - не касаемся
#define MAX_SWITCHERS 11


char urlParams[MAX_PARAMS][MAX_PARAM_NAME_LEN];
char urlValues[MAX_PARAMS][MAX_PARAM_VAL_LEN];
char urlPage[MAX_PARAM_NAME_LEN];
bool switchers[MAX_SWITCHERS]; //Первые два - не касаемся


//самая длинная строка из памяти (PROGMEM) если строка длиннее, то изменить!!!
#define MAX_STR 45
const char string_00[] PROGMEM = "manual";
const char string_01[] PROGMEM = "auto"; //--
const char string_02[] PROGMEM = "HTTP/1.1 200 OK\r\nContent-Type: text/html";
const char string_03[] PROGMEM = "Connection: close\r\n\r\n<!DOCTYPE HTML>";
const char string_04[] PROGMEM = "<head><style>a{text-decoration:none;}";
const char string_05[] PROGMEM = "</style></head><html><font size=30>";
const char string_06[] PROGMEM = "<b>";
const char string_07[] PROGMEM = "</b>";
const char string_08[] PROGMEM = "&#9830;&nbsp;";//жирная точка рядом с нужным пунктом
const char string_09[] PROGMEM = "&nbsp;&nbsp;&nbsp;"; //без точки
const char string_10[] PROGMEM = "<a href='/manual?turn=on&num=7'>";
const char string_11[] PROGMEM = "<a href='/manual?turn=off&num=7'>";
const char string_12[] PROGMEM = "&#1042;&#1050;&#1051;";//ВКЛ
const char string_13[] PROGMEM = "&#1042;&#1067;&#1050;&#1051;";//ВЫКЛ  
const char string_14[] PROGMEM = "</a></br>";
const char string_15[] PROGMEM = "</a></font></html>";
const char string_16[] PROGMEM = ""; //-- //хотел сделать, да ну его нафиг: <head><meta http-equiv=\"refresh\" content=\"0; URL=/manual?turn=status&num=7\" /></head>
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
  if(strNum>=23)
    strcpy(buffer, "");
  else
    strcpy_P(buffer, (char*)pgm_read_word(&(string_table[strNum])));
  return buffer;
}


/*void getFromClient(EthernetClient client){
char sRequest[REQ_LEN];

  while (client.available()>0) {
    char c = client.read();
    if(strlen(sRequest) < REQ_LEN)
    {
      int len = strlen(sRequest); 
      sRequest[len] = c;
      sRequest[len+1] = '\x0';
    }
  }
#ifdef DEBUG
  Serial.println(sRequest);
#endif
  return sRequest;
}*/


//очистить массив параметров = всё заранее заполнить нолями
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
  Serial.print(" readClientToNull:");
#endif
  char c;
  while (client.available()>0)
    c = client.read();
#ifdef DEBUG
  Serial.println("OK");
#endif
}


bool getFromClient(EthernetClient client)
{
  char c;
  byte paramNum = 0; //номер параметра в массиве
  int urlCharNum = 0; //номер символа во всём урле
  int paramCharNum = 0; //номер символа в имени параметра
  int valueCharNum = 0; //номер символа в значении параметра
  bool isPreUrlPage = true; //ещё не дошли до урла
  bool isUrlPage = false; //сейчас читаем имя страницы
  bool isParamName = false; //сейчас читаем имя параметра
  bool isParamValue = false; //сейчас читаем значения параметра
  clearParams();
#ifdef DEBUG
  Serial.println(" getFromClient:");
#endif
  while (client.available()>0)
  {
    c = client.read();
    if(urlCharNum++ < REQ_LEN)//больше максимальной длины не читаем
    {
#ifdef DEBUG
//  Serial.print(c);
#endif
      if(paramCharNum >= MAX_PARAM_NAME_LEN){//если слишком длинное имя параметра
        readClientToNull(client);
        return false;
      }
      if(valueCharNum >= MAX_PARAM_VAL_LEN){//если слишком длинное значение параметра
        readClientToNull(client);
        return false;
      }
      if(isPreUrlPage){//ещё не дошли до имени страницы
        if(c == '/'){
          isPreUrlPage = false;
          isUrlPage = true;
        }
      }else if(isUrlPage){//если ещё не натолкнулись на "?" значит читаем имя страницы
        if(c == '?'){//значит имя страницы закончилось
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
        if(c == '='){//имя параметра закончилось
          isParamName = false;
          isParamValue = true;
          valueCharNum = 0;
        }else
          urlParams[paramNum][paramCharNum++] = c;
      }else if(isParamValue){
        if(c == '&'){//значение параметра закончилось
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


void onSwitchON(EthernetClient client){
#ifdef DEBUG
  Serial.println("onSwitchON>");
#endif
  //digitalWrite(7, HIGH);
  //return "<head><style>a{text-decoration:none;}</style></head><html><font size=30>&#9830;&nbsp;<a href='/?turn=on'><b>&#1042;&#1050;&#1051;</b></a></br>  &nbsp;&nbsp;&nbsp;<a href='/?turn=off'>&#1042;&#1067;&#1050;&#1051;</a></font></html>";
  //memStrNum(04) memStrNum(05) memStrNum(08) memStrNum(10) memStrNum(06) memStrNum(12) memStrNum(07) memStrNum(14) memStrNum(09) memStrNum(11) memStrNum(13) memStrNum(15;
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
  //return true;
}


void onSwitchOFF(EthernetClient client){
#ifdef DEBUG
  Serial.println("onSwitchOFF>");
#endif
  //digitalWrite(7, LOW);
  //return "<head><style>a{text-decoration:none;}</style></head><html><font size=30>&nbsp;&nbsp;&nbsp;<a href='/?turn=on'>&#1042;&#1050;&#1051;</a></br>  &#9830;&nbsp;<a href='/?turn=off'><b>&#1042;&#1067;&#1050;&#1051;</b></a></font></html>";
  //memStrNum(04) memStrNum(05) memStrNum(09) memStrNum(10) memStrNum(12) memStrNum(14) memStrNum(08) memStrNum(11) memStrNum(06) memStrNum(13) memStrNum(07) memStrNum(15;
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
  //return true;
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

//найти параметр num. 0=нет параметра
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
  if(manual){//нарисовать для человека
    if(_status)
      onSwitchON(client);
    else
      onSwitchOFF(client);
  }else{//нарисовать для машины
    if(_status)
      client.println(memStrNum(18));//on
    else
      client.println(memStrNum(19));//off
  }
}


//0-off, 1-on, 2-switch, 3-status
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
      //printSwitchStatus(client, false, manual);
      isOk = true;
      break;
    case 1: //on
      digitalWrite(num, HIGH);
      switchers[num] = true;
      //printSwitchStatus(client, true, manual);
      isOk = true;
      break;
    case 2: //switch
      if(switchers[num])
        digitalWrite(num, LOW);
      else
        digitalWrite(num, HIGH);
      switchers[num] = !switchers[num];
      //printSwitchStatus(client, switchers[num], manual);
      isOk = true;
      break;
    case 3: //status
      //printSwitchStatus(client, switchers[num], manual);
      isOk = true;
      break;
  }
  printSwitchStatus(client, switchers[num], manual);
  return isOk;
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
      printToClientStart(client);//wha? rly nid it? dn't thnk so.

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
        paramNum++;
      }
      
      /*if(strncmp(sRequest, memStrNum(0), strlen(memStrNum(0))) == 0)  //"GET /?turn=on "
        done = onON(client);
      else if(strncmp(sRequest, memStrNum(1), strlen(memStrNum(1))) == 0)  //"GET /?turn=off "
        done = onOFF(client);*/

      if(!done)
        badUrl(client);
      // give the web browser time to receive the data
      delay(DELAY_MS);
      client.stop();
    
#ifdef DEBUG
Serial.println("<client/>");
#endif
      //Ethernet.maintain(); - было тут. Что будет, если вынести за цикл?
    }
    Ethernet.maintain();
  }
}
