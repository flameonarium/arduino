#include <DHT.h>  //библиотека датчика влажности DHT11
#include <Wire.h> //Библиотека I2C. Используют: часы
#include <LedControl.h> //Библиотека для экранчика
#include <EEPROM.h> //Работа с ПЗУ
#include <MemoryFree.h> //Информация об оперативной памяти
#include "defines.h"  //сборник глобальных констант
#include "lionTermNTC.h" //lion библиотека термисторов
#include "lionTimeDS1307.h" //lion библиотека часов ds1307
#include "lionVolmeter.h" //lion библиотека вольтметров

#ifdef DEBUG_ETHERNET
#include <SPI.h>
#include <Ethernet.h>
#endif

#ifdef DEBUG_ETHERNET
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 177);
EthernetServer server(80);
#endif

//Глобальные переменные
bool wired = false; //Была ли запущена библиотека Wire.

bool alarmSwitch; //Установлено ли на охрану
bool alarmASIS; //Тревога? Опасность! Надо всем рассказать!
String alarmMsg; //Сообщение тревоги
char alarmDateSet[20], alarmDateUnset[20], alarmDateAlert[20]; //дата время возникновения событий.
unsigned long millisPerJob[JOBS_COUNT]; //массив данных по отметке времени последнего исполнения задания. Например как давно последний раз проверялась температура
char **smsSecuredPhones = 0; //массив безопасных номеров телефонов
byte smsSecuredPhonesCount = 0; //количество безопасных номеров в массиве

//Глобальные объекты
LedControl led = LedControl(PIN_LED_DIN, PIN_LED_CLK, PIN_LED_LOAD, 1); //объект сегментного экранчика

DHT humRoom = DHT(PIN_HUMIDITY_ROOM, DHT22, 0, 10); //датчик влажности DHT11 для считывания с него информации

lionTermNTC termStreet = lionTermNTC(PIN_TERM_STREET, TERM_AVG_COUNT, TERM_STREET_PAD); //датчик температуры уличный
lionTermNTC termRoom   = lionTermNTC(PIN_TERM_ROOM, TERM_AVG_COUNT, TERM_ROOM_PAD);   //датчик температуры комнатный
lionTermNTC termTubes  = lionTermNTC(PIN_TERM_TUBES, TERM_AVG_COUNT, TERM_TUBES_PAD);  //датчик температуры теплоносителя

lionTimeDS1307 clocks; //модуль часов

lionVolmeter voltMain = lionVolmeter(PIN_VOLT_MAIN, VOLT_MAIN_LOW_VOLTAGE);
lionVolmeter voltBackup = lionVolmeter(PIN_VOLT_BACKUP);

/**** Предописание нужных функций ****************************************************************************************************/
void alarmSet(bool forced = false);
void alarmUnset(bool forced = false);
/*************************************************************************************************************************************/

/**** Работа с заданиями *************************************************************************************************************/
//Функция проверяет требуется ли запускать задание - пришло ли время.
//delayNum - номер задания в списке
//delayValue - задержка в задании
bool doAJob(int delayNum, int delayValue)
{
  //для кривых заданий нет запуска.
  if((delayNum < 0) || (delayNum >= JOBS_COUNT))
    return false;
  
  unsigned long now = millis();
  //как насчёт перескакивания счётчика времени за 0?
  if(millisPerJob[delayNum] > now)
    millisPerJob[delayNum] = 0;
  
  //если не пришло время измерений - выходим с отрицательным ответом
  if(millisPerJob[delayNum] + delayValue > now)
    return false;
  else  //если время пришло - отметим время, когда измерения были сделаны
    millisPerJob[delayNum] = now;
    
  return true;
}
/*************************************************************************************************************************************/


/**** Работа с логированием **********************************************************************************************************/
//Функция записывает в лог сообщение, добавляя метку времени.
//В режиме дебага - выдаёт в монитор
void addLog(String msg)
{
  String dateTime = clocks.timeDateGetStr();
#ifdef DEBUG_SERIAL
  Serial.println(dateTime + ' ' + msg);
#else
  
#endif
}
/*************************************************************************************************************************************/


/**** Работа с GSM (SMS) *************************************************************************************************************/
//Функция приведения номера телефона к общему стандарту (+7)
//возвращает приведённый номер телефона
//phNumber - сырой номер телефона
String correctNumber(String phNumber)
{
  if(phNumber.startsWith("+7"))
    return phNumber;
  if(phNumber.startsWith("8"))
    return "+7" + phNumber.substring(1);
  return "";
}

//Функция звонка на телефон
void gsmCall(String phoneNumber, String msg = "")
{
#ifdef DEBUG_SERIAL
  Serial.println("Calling to " + phoneNumber + " " + msg); //просто выведем в монитор
#else
  addLog("Calling... (" + phoneNumber + "): " + msg);
  //TODO: Добавить звонок
  addLog("End call. (" + phoneNumber + ")");
#endif
}

//Функция загрузки списка телефонов из EEPROM
void gsmLoadPhoneNumbers()
{
  ///Если массив заполнен - сначала освободим память
  if(smsSecuredPhones != 0)
  {
    for(int i=0; i<smsSecuredPhonesCount; i++)
      free(smsSecuredPhones[i]);
    free(smsSecuredPhones);
  }
  //первый байт - количество номеров
  smsSecuredPhonesCount = EEPROM.read(0);

  //разместим массив номеров в памяти
  smsSecuredPhones = (char**)malloc(smsSecuredPhonesCount * sizeof(char*));
  for(int i=0; i<smsSecuredPhonesCount; i++)
      smsSecuredPhones[i] = (char*)malloc(GSM_PHONE_LEN * sizeof(char));

  //сдвигаемся на следующий байт и читаем все номера, как нам предначертано.
  int eeAddress = sizeof(smsSecuredPhonesCount);
  char number[GSM_PHONE_LEN];
  for(int i=0; i<smsSecuredPhonesCount; i++)
  {
    EEPROM.get(eeAddress, number);
    eeAddress += sizeof(number);
    strcpy(smsSecuredPhones[i], number);
  }
}

//Функция записи списка телефонов в EEPROM
void gsmSavePhoneNumbers()
{
  int eeAddress = 0;
  char number[GSM_PHONE_LEN];
  EEPROM.put(eeAddress, smsSecuredPhonesCount);
  eeAddress += sizeof(smsSecuredPhonesCount);
  for(int i=0; i<smsSecuredPhonesCount; i++)
    {
      strncpy(number, smsSecuredPhones[i], GSM_PHONE_LEN);
      EEPROM.put(eeAddress, number);
      eeAddress += sizeof(number);
    }
}

//Функция возвращает список зарегистрированных номеров
String gsmGetPhoneNumbersList()
{
  String res = "";
  for(int i=0; i<smsSecuredPhonesCount; i++)
      res = res + String(i+1) + ") " + String(smsSecuredPhones[i]) + "\n";
  return res;
}

//Функция добавляет ещё один номер телефона в список в конец
String gsmAddPhoneToList(String command)
{
#ifdef DEBUG_SERIAL
  Serial.print("add " + String(freeMemory()));
#endif
  char number[GSM_PHONE_LEN];
  String numberS;
  byte phStartPos = 0;
  char **tempArr = 0;
  
  if(command.startsWith("p add"))
    phStartPos = 6;
  else if(command.startsWith("phones add"))
    phStartPos = 11;

  numberS = command.substring(phStartPos);
  numberS.trim();
  numberS = correctNumber(numberS);
  if(numberS == "")
    return "Plohoy nomer telefona. Nachnite s +7";
  numberS.toCharArray(number, GSM_PHONE_LEN);

  //проверим не было ли этого номера в списке уже
  if(gsmIsSecuredNumber(numberS))
    return gsmGetPhoneNumbersList();

  //создадим временный массив
  smsSecuredPhonesCount++;
  tempArr = (char**)malloc((smsSecuredPhonesCount) * sizeof(char*));
  for(int i=0; i<smsSecuredPhonesCount-1; i++)
    {
      tempArr[i] = (char*)malloc(GSM_PHONE_LEN * sizeof(char));
      strncpy(tempArr[i], smsSecuredPhones[i], GSM_PHONE_LEN);
      free(smsSecuredPhones[i]);
    }
  tempArr[smsSecuredPhonesCount-1] = (char*)malloc(GSM_PHONE_LEN * sizeof(char));
  strncpy(tempArr[smsSecuredPhonesCount-1], number, GSM_PHONE_LEN);
  free(smsSecuredPhones);
  
  //расширим массив на одну запись
  smsSecuredPhones = (char**)malloc(smsSecuredPhonesCount * sizeof(char*));
  for(int i=0; i<smsSecuredPhonesCount; i++)
    {
      smsSecuredPhones[i] = (char*)malloc(GSM_PHONE_LEN * sizeof(char));
      strncpy(smsSecuredPhones[i], tempArr[i], GSM_PHONE_LEN);
      free(tempArr[i]);
    }
  free(tempArr);
#ifdef DEBUG_SERIAL
  Serial.println(" -> " + String(freeMemory()));
#else
  gsmSavePhoneNumbers();
#endif
  return gsmGetPhoneNumbersList();
}

//Функция удаляет номер телефона из списка по номеру в списке
String gsmDelPhoneFromList(String command, String senderPhoneNumber)
{
#ifdef DEBUG_SERIAL
  Serial.print("del " + String(freeMemory()));
#endif
  byte phStartPos = 0;
  byte number = 0;
  String numberS;
  if(command.startsWith("p del"))
    phStartPos = 6;
  else if(command.startsWith("phones del"))
    phStartPos = 11;

  numberS = command.substring(phStartPos);
  numberS.trim();
  number = numberS.toInt() - 1;
  
  if((number < 0) || (number > smsSecuredPhonesCount-1))
    return "Vyberite nomer iz spiska.";

  if(senderPhoneNumber == smsSecuredPhones[number])
    return "Nel'zya udalyat' sebya iz spiska";

  //сдвинем все номера влево, затирая удаляемый
  for(int i=number; i<smsSecuredPhonesCount-1; i++)
    strcpy(smsSecuredPhones[i], smsSecuredPhones[i+1]);

  //smsSecuredPhonesCount--;
  //smsSecuredPhones = (char**)realloc(smsSecuredPhones, smsSecuredPhonesCount * sizeof(char*));

  char **tempArr = 0;
  //создадим временный массив
  smsSecuredPhonesCount--;
  tempArr = (char**)malloc((smsSecuredPhonesCount) * sizeof(char*));
  for(int i=0; i<smsSecuredPhonesCount; i++)
    {
      tempArr[i] = (char*)malloc(GSM_PHONE_LEN * sizeof(char));
      strncpy(tempArr[i], smsSecuredPhones[i], GSM_PHONE_LEN);
      free(smsSecuredPhones[i]);
    }
  free(smsSecuredPhones[smsSecuredPhonesCount]);
  free(smsSecuredPhones);
  
  //сузим массив на одну запись
  smsSecuredPhones = (char**)malloc(smsSecuredPhonesCount * sizeof(char*));
  for(int i=0; i<smsSecuredPhonesCount; i++)
    {
      smsSecuredPhones[i] = (char*)malloc(GSM_PHONE_LEN * sizeof(char));
      strncpy(smsSecuredPhones[i], tempArr[i], GSM_PHONE_LEN);
      free(tempArr[i]);
    }
  free(tempArr);
  
#ifdef DEBUG_SERIAL
  Serial.println(" -> " + String(freeMemory()));
#else
  gsmSavePhoneNumbers();
#endif
  return gsmGetPhoneNumbersList();
}

//Функция получения СМС
//возвращает есть ли новые смс
//phoneNumber - исходящий параметр, номер телефона, с которой получена смс
//sms - исходящий параметр, полный текст смс
bool smsGet(String &phoneNumber, String &sms) //TODO: что-то странное происходит, если приходят пробелы
{
  String msg = "";
#ifdef DEBUG_SERIAL
  while(Serial.available() > 0)
    msg += char(Serial.read());
  phoneNumber = String("+79232471964");
#else
  //TODO: добавить обработчик получения СМС
  //  phoneNumber = correctNumber(numB?);
#endif
  if(msg != "")
    addLog("Received SMS (" + phoneNumber + "): " + msg);
  sms = msg;
  return (msg != "");
}

//Функция просто отправляет СМС
//phoneNumber - номер телефона
//msg - текст сообщения
void smsSend(String phoneNumber, String msg)
{
  if(msg == "")
    return;
#ifdef DEBUG_SERIAL
  Serial.println(msg); //просто выведем в монитор
#else
  //TODO: Добавить отправку СМС
  addLog("Sent SMS (" + phoneNumber + "): " + msg);
#endif
}

//Функция проверяет наличие телефона с писке разрешённых
bool gsmIsSecuredNumber(String phoneNumber)
{
  bool res = false;
  for(int i=0; i<smsSecuredPhonesCount; i++)
    if(phoneNumber == smsSecuredPhones[i])
    {
      res = true;
      break;
    }
  return res;
}


//Функция проверки безопасности СМС. Проверяется допустимость номера телефона ("и код безопасности" - если захотим прикрутить это сюда)
//возвращает является ли это сообщение безопасным
//phoneNumber - номер телефона, с которого пришло сообщение
//sms - само сообщение
bool smsSecure(String phoneNumber, String sms)
{
  //проверим, что номер в списке разрешённых
  bool numberOk = gsmIsSecuredNumber(phoneNumber);
  //если номер в списке разрешённых, то код читать не будем, может быть прикрутим код безопасности, если понадобится
  return numberOk;
}

//Функция возвращает команду из смс. Поскольку кода безопасности нет, то просто уберём всё лишнее
//sms - само текстовое сообщение.
String smsGetCommand(String sms)
{
  sms.trim();
  return sms;
}

//Функция возвращает текстовое сообщение с помощью по командам
String smsHelp()
{
  return SMS_MSG_HELP;
}

//Функция возвращает общую информацию по текущему статусу
String smsGetMainInfo()
{
  String res = SMS_MSG_MAIN_INFO;
  res.replace("%a%", alarmStatusStr());
  res.replace("%Vmain%", voltMain.getStr());
  res.replace("%troom%", String(termRoom.getAvgTerm()));
  res.replace("%Vacc%", String(voltBackup.getFloat()));
  return res;
}

//Функция возвращает шаблон непонятности.
String smsUnknown()
{
  return SMS_MSG_UNKNOWN;
}

//Функция возвращает данные погоды
String smsWeather()
{
  String res = SMS_MSG_WEATHER;
  res.replace("%tBat%", String(termTubes.getAvgTerm()));
  res.replace("%tOut%", String(termStreet.getAvgTerm()));
  res.replace("%tRoom%", String(termRoom.getAvgTerm()));
  res.replace("%hRoom%", String(humRoom.readAvgHumidity()));
//p ком: %pRoom%mmHg - влажность в комнате
  return res;
}

//Функция возвращает сообщение со статусом охраны и датами постановки/снятия/тревоги
String smsAlarmState()
{
  String res = SMS_MSG_ALARM_INFO;
  res.replace("%a%", alarmStatusStr());
  res.replace("%aSetDate%", alarmDateSet);
  res.replace("%aUnsDate%", alarmDateUnset);
  res.replace("%alertDate%", alarmDateAlert);
  return res;
}

//Функция реакция на смс постановки на охрану
String smsAlarmSet()
{
  String res;
  if(alarmSwitch)
  {
    res = SMS_MSG_ALARM_ALREADY_SET;
    res.replace("%aUnsDate%", alarmDateUnset);
    return res;
  }
  alarmSet();
  return SMS_MSG_ALARM_SET;
}

//Функция реакция на смс снятия с охраны
String smsAlarmUnset()
{
  String res;
  if(alarmSwitch)
  {
    res = SMS_MSG_ALARM_ALREADY_UNSET;
    res.replace("%aSetDate%", alarmDateSet);
    return res;
  }
  alarmUnset();
  return SMS_MSG_ALARM_UNSET;
}

//Функция разбора команд из смс
String smsGetAnswer(String command, String phoneNumber)
{
  if(command == "")
    return "";
  String answer = "";
  if((command == "?") || (command == "help"))
    answer = smsHelp();
  else if((command == "info") || (command == "i"))
    answer = smsGetMainInfo();
  else if(command == "a")
    answer = smsAlarmState();
  else if(command == "a set")
    answer = smsAlarmSet();
  else if(command == "a unset")
    answer = smsAlarmUnset();
  else if((command == "t") || (command == "w") || (command == "weather"))
    answer = smsWeather();
  else if((command == "phones") || (command == "p"))
    answer = gsmGetPhoneNumbersList();
  else if(command.startsWith("p add") || command.startsWith("phones add"))
    answer = gsmAddPhoneToList(command);
  else if(command.startsWith("p del") || command.startsWith("phones del"))
    answer = gsmDelPhoneFromList(command, phoneNumber);
  else answer = smsUnknown();
  
  return answer;
}
/*************************************************************************************************************************************/



/**** Работа модуля охраны ***********************************************************************************************************/
//Постановка на охрану
void alarmSet(bool forced)
{
  if(!alarmSwitch || forced) //если не было поставлено на охрану
  {
    //TODO: проверить, что датчики уже заполнились, то есть процесс инициализации прошёл.
    alarmSwitch = true;
    alarmASIS = false;
    digitalWrite(PIN_ALARM_LED, HIGH);
    alarmMsg = "";
    //установим в ноль последнюю проверку тревоги на всякий случай, чтобы точно запустилась.
    millisPerJob[DELAY_ALARM_NUM] = 0;
    clocks.timeDateGetStr().toCharArray(alarmDateSet, 20);
  }
}

//Снятие с охраны
void alarmUnset(bool forced)
{
  if(alarmSwitch || forced)//если на охране не стоит, то не надо нас тут путать.
  {
    alarmSwitch = false;
    alarmASIS = false;
    digitalWrite(PIN_ALARM_LED, LOW);
    clocks.timeDateGetStr().toCharArray(alarmDateUnset, 20);
  }
}

//Функция рассылки тревожных смс по зарегистрированным номерам
void alarmAlertSMS()
{
  String msg = ALARM_MSG_MAIN + alarmMsg;
  for(int i=0; i<smsSecuredPhonesCount; i++)
    smsSend(smsSecuredPhones[i], msg);
}

//Функция реакции на тревогу.
void alarm()
{
  if(!alarmASIS)//если уже тревога, то не надо так волноваться - уже и так всем страшно.
  {
    //выставим глобальную константу в тревогу
    alarmASIS = true;
    clocks.timeDateGetStr().toCharArray(alarmDateAlert, 20);
    
    //напишем всем СМС
    alarmAlertSMS();
    
    //с установкой признака alarmASIS начинается цикл звонков
    millisPerJob[DELAY_ALERT_NUM] = 0;
  }
}

//Функция проверки на условия тревоги
bool alarmCheck()
{
  //Сначала проверим датчик движения
  bool motionNow = (digitalRead(PIN_MOTION) == HIGH);
  //Если датчик движения что-то зарегистрировал, то выдаём тревогу
  if(motionNow)
  {
    alarmMsg = ALARM_MSG_MOTION;
    return true;
  }
  
  //Проверим температуру комнаты
  float term = termRoom.getAvgTerm();
  if((term < ALARM_TERM_ROOM_MIN) || (term > ALARM_TERM_ROOM_MAX))
  {
    alarmMsg = ALARM_MSG_TERM_ROOM;
    alarmMsg.replace("%tRoom%", String(term));
    return true;
  }
  
  //Проверим температуру теплоносителя
  term = termTubes.getAvgTerm();
  if((term < ALARM_TERM_TUBES_MIN) || (term > ALARM_TERM_TUBES_MAX))
  {
    alarmMsg = ALARM_MSG_TERM_TUBES;
    alarmMsg.replace("%tBat%", String(term));
    return true;
  }
  
  //Проверим напряжение аккумулятора
  term = voltBackup.getFloat();
  if((term < ALARM_VOLT_ACC_MIN) || (term > ALARM_VOLT_ACC_MAX))
  {
    alarmMsg = ALARM_MSG_VOLT_ACC;
    alarmMsg.replace("%Vmain%", voltMain.getStr());
    alarmMsg.replace("%Vacc%", String(term));
    return true;
  }
  
  //если проверки прошли успешно, то всё ок.
  return false;
}

//Функция возвращает текущий статус охраны (ON, OFF, ALARM)
String alarmStatusStr()
{
  if(alarmASIS)
    return "ALARM";
  if(alarmSwitch)
    return "ON";
  else
    return "OFF";
}
/*************************************************************************************************************************************/



/**** Установка начальных значений в setup() *****************************************************************************************/
//Установка монитора для отладки, если отладка включена
void setupLog()
{
#ifdef DEBUG_SERIAL
  Serial.begin(115200);
#endif
#ifdef DEBUG_ETHERNET
  Ethernet.begin(mac, ip);
  server.begin();
#endif
}

//Установка начальных значений времени запуска для заданий
void setupJobs()
{
  for(int i=0; i<JOBS_COUNT; i++)
    millisPerJob[i] = 0;
}

//Установка начальных значений для GSM.
void setupGSM()
{
  gsmLoadPhoneNumbers();
  //заполним вручную номера телефонов
  //smsSecuredPhones[0] = "+79999999999";  //отладочный номер
  //smsSecuredPhones[1] = "+79232471964"; 
  //если включена отладка - не надо запускать модуль GSM

//#ifndef DEBUG_SERIAL
//сюда код настоящего GSM
//#endif
}

//Установки для работы часов
void setupTime()
{
  clocks.begin(wired);
}

//Запуск датчиков DHT - влажность и температура
void setupDHT()
{
  humRoom.begin();
}

//Запуск модуля охраны
void setupAlarm()
{
  //лампочка будет говорить на охране или нет
  pinMode(PIN_ALARM_LED, OUTPUT);
  
  //для начала на охрану не установлено
  alarmUnset(true);
}

//Установка значений для детектора движений
void setupMotionD()
{
  //установим датчик на плате
  pinMode(PIN_MOTION, INPUT);
}

//Инициализация экранчика MAX7219
void setupLED()
{
  led.shutdown(0, false);  //Включение
  led.setIntensity(0, 10); //Установка яркости (0 is min, 15 is max)
  led.clearDisplay(0);     //Очистка экрана
}
/*************************************************************************************************************************************/


/**** Функциональные итерации ********************************************************************************************************/
//Итерация пробы температуры
void loopTerm()
{
  //Проверим, что пришло время измерений
  if(!doAJob(DELAY_TERM_NUM, DELAY_TERM_PROBES))
    return;
  
  //соберём данные с датчиков температуры
  termStreet.getTerm();
  termRoom.getTerm();
  termTubes.getTerm();
}

//Итерация проб влажности
void loopHumidity()
{
  //Проверим, что пришло время измерений
  if(!doAJob(DELAY_HUM_NUM, DELAY_HUM_PROBES))
    return;

  //соберём данные с датчиков температуры в массивы
  humRoom.readHumidity();
}

//Итерация для GSM (или отладки)
void loopGSM()
{
  if(!doAJob(DELAY_GSM_NUM, DELAY_GSM_PROBES))
    return;
  
  //TODO: Проверим что модуль работает
  //TODO: если модуль не работает - запишем в лог
  //Проверим входящие СМС
  String sms, phNumber;
  
  if(!smsGet(phNumber, sms))  //не получили СМС - выходим
    return;
  //else
  //  addLog("got SMS (" + phNumber + "): " + sms);
  
  //Проверим секьюрность СМС
  if(!smsSecure(phNumber, sms))
  {
    //Запишем в лог несекьюрную СМС
    addLog("Unsecure SMS!!");
    return;
  }
    
  //если есть что ответить - отправим
  smsSend(phNumber, smsGetAnswer(smsGetCommand(sms), phNumber));
}

//Итерация проверки тревоги
void loopAlarm()
{
  //если постановки на охрану не было, то не надо ничего делать
  if(!alarmSwitch)
    return;
  
  //если уже тревога, то проверять ничего не нужно
  if(alarmASIS)
    return;
  
  //если не пришло время проверять, то тоже нечего делать
  if(!doAJob(DELAY_ALARM_NUM, DELAY_ALARM_CHECK))
    return;
  
  //проверим на предмет тревоги и если что - запускаем режим тревоги!
  if(alarmCheck())
    alarm();
}

//Итерация звонков по тревоге
void loopAlertCalls()
{
  //если тревоги нет - выходим
  if(!alarmASIS)
    return;
  
  //если не пришло время звонить, то тоже нечего делать
  if(!doAJob(DELAY_ALERT_NUM, DELAY_ALERT_FREQ))
    return;
  
  //Звоним
  for(int i=0; i<smsSecuredPhonesCount; i++)
    gsmCall(smsSecuredPhones[i]);
}


#ifdef DEBUG_ETHERNET
void loopEthernet()
{
  EthernetClient client = server.available();
  boolean currentLineIsBlank = true;
  while (client.connected())
  {
    if (client.available())
    {
      char c = client.read();
      if (c == '\n' && currentLineIsBlank)
      {
        // send a standard http response header
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connection: close");  // the connection will be closed after completion of the response
        client.println("Refresh: 20");  // refresh the page automatically every 5 sec
        client.println();
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");
        client.println("Current time     " + clocks.timeDateGetStr() + "<br />");
        client.println("Free memory      " + String(freeMemory()) + " bytes<br />");
        client.println("Alarm status     " + alarmStatusStr() + "<br />");
        client.println("Alarm   set date " + String(alarmDateSet) + "<br />");
        client.println("Alarm unset date " + String(alarmDateUnset) + "<br />");
        client.println("Alarm alert date " + String(alarmDateAlert) + "<br />");
        client.println("Term batterie    " + String(termTubes.getAvgTerm()) + "C<br />");
        client.println("Term outside     " + String(termStreet.getAvgTerm()) + "C<br />");
        client.println("Term room        " + String(termRoom.getAvgTerm()) + "C<br />");
        client.println("Humidity room    " + String(humRoom.readAvgHumidity()) + "%<br />");
        client.println("Voltage main is  " + voltMain.getStr() + "<br />");
        client.println("Voltage main     " + String(voltMain.getFloat()) + "V<br />");
        client.println("Voltage backup   " + String(voltBackup.getFloat()) + "V<br />");
        client.println("</html>");
        break;
      }
      if (c == '\n')
        // you're starting a new line
        currentLineIsBlank = true;
      else if (c != '\r')
        // you've gotten a character on the current line
        currentLineIsBlank = false;
    }
  }
  // give the web browser time to receive the data
  delay(50); //lion. Was: delay(1);
  client.stop();
  Ethernet.maintain();
}
#endif
/*************************************************************************************************************************************/

void setup()
{
  setupLog();     //Устанавливаем монитор отладки, или Логирование на карту памяти
  setupGSM();     //Запускаем плату GSM
  //setupTerm();  //Устанавливаем начальные данные для датчиков температуры
  setupTime();    //Запуск библиотеки для работы с часами
  setupDHT();     //Запуск датчиков DHT - влажность и температура
  setupMotionD(); //Запуск датчика движения
  setupAlarm();   //Запуск модуля охраны
  //setupLED();     //Запуск экранчика
}

void loop()
{
  unsigned long startTime = millis(); //запоминаем момент, когда началась главная итерация
  
  //функциональные итерации
  loopGSM();        //итерация ответов на СМС
  loopTerm();       //итерация проб температур
  loopHumidity();   //итерация проб влажности
  loopAlarm();      //итерация охраны
  loopAlertCalls(); //итерация звонков в случае трефоги
#ifdef DEBUG_ETHERNET
  loopEthernet();   //итерация эзернета для отладки
#endif
  
  //вычислм сколько осталось времени на отдых (задержку). Общее время главной итерации не должно быть меньше, чем требуется переменной задержки
  if(DELAY_MAIN_LOOP > (millis() - startTime))
    delay(DELAY_MAIN_LOOP - (millis() - startTime)); //задержка ровно на столько, сколько осталось
}
