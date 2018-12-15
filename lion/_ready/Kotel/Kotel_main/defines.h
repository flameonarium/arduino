#include "Arduino.h"

#ifndef lionDefines_h
#define lionDefines_h

//Глобальные константы
//#define DEBUG_SERIAL            //использование серийного монитора для отладки
#define DEBUG_ETHERNET          //использование Ethernet для отладки
#define JOBS_COUNT 5            //количество заданий для массива этих заданий

//Значения задержек между выполнениями заданий
#define DELAY_MAIN_LOOP 50      //задержка в выполнении главного цикла
#define DELAY_TERM_NUM 0        //номер задания проб температуры в массиве заданий
#define DELAY_TERM_PROBES 2000  //задержка между измерениями температуры
#define DELAY_GSM_NUM 1         //номер задания для СМС
#define DELAY_GSM_PROBES 500    //задержка между проверками СМС
#define DELAY_HUM_NUM 2         //номер задания для влажности
#define DELAY_HUM_PROBES 6000   //задержка между проверками влажности
#define DELAY_ALARM_NUM 3       //номер задания проверок тревоги
#define DELAY_ALARM_CHECK 1000  //задержка между проверками условий тревоги
#define DELAY_ALERT_NUM 4       //номер задания звонков тревоги на телефоны
#define DELAY_ALERT_FREQ 30000  //задержка между звонками тревоги

//Порты подключения на плате
#define PIN_TERM_STREET 0    //A0 термометр на улице
#define PIN_TERM_ROOM 1      //A1 термометр в комнате - временно вставлен в 0 для отладки
#define PIN_TERM_TUBES 0     //A2 термометр на трубе  - временно вставлен в 0 для отладки
#define PIN_VOLT_BACKUP 3    //A3 вольтметр на аккумуляторе бесперебойника
#define PIN_VOLT_MAIN 6      //A6 вольтметр на адаптере из сети
#define PIN_GSM_TX 2         //D2 TX провод на GSM модуль
#define PIN_GSM_RX 3         //D3 RX провод на GSM модуль
#define PIN_HUMIDITY_ROOM 4  //D4 датчик влажности в помещении
#define PIN_MOTION 5         //D5 датчик движения в комнате
#define PIN_ALARM_LED 13     //D13 - лампочка постановки на охрану
#define PIN_LED_DIN 6        //D6 подключение 7-сегментного экрана
#define PIN_LED_CLK 7        //D7 ...
#define PIN_LED_LOAD 8       //D8 ...

//Константы для температуры
#define TERM_AVG_COUNT 20      //количество измерений для среднего показания датчиков
#define TERM_STREET_PAD 10000  //сопротивление балансного резистора для уличного термометра
#define TERM_ROOM_PAD 10000    //...................................... комнатного термометра
#define TERM_TUBES_PAD 10000   //...................................... трубного термометра

//константы для смс
#define GSM_PHONE_LEN 14
//шаблоны сообщений
//#define SMS_MSG_HELP "Команды (лат.буквы)\ni:общая информация\na (set,unset):охрана\nt:погода в котельной"
//#define SMS_MSG_MAIN_INFO "охрана: %o%\nсеть: %Vmain%\nt пом: %troom%C\nV акк: %Vacc%"
//#define SMS_MSG_UNKNOWN "Неизвестная команда. Пришлите символ ? чтобы увидеть список допустимых команд."
//#define SMS_MSG_WEATHER "t бат: %tBat%C\nt ули: %tOut%C\nt ком: %tRoom%C\nh ком: %hRoom%%\np ком: %pRoom%mmHg"
#define SMS_MSG_WEATHER "t bat: %tBat%C\nt uli: %tOut%C\nt kom: %tRoom%C\nh kom: %hRoom%%\np kom: %pRoom%mmHg"
#define SMS_MSG_HELP "Komandy (lat.bukvy)\ni:obschaya informaciya\na (set,unset):ohrana\nt:pogoda v kotel'noy"
#define SMS_MSG_MAIN_INFO "Ohrana: %a%\nSet': %Vmain%\nt kotel'noy: %troom%C\nV akk: %Vacc%V"
#define SMS_MSG_UNKNOWN "Neizvestnaya komanda. Prishlite '?' chtoby uvidet' spisok dopustimyh komand."
#define SMS_MSG_ALARM_INFO "Status ohrany: %a%\nData postanovki: %aSetDate%\nData snyatiya: %aUnsDate%\nData trevogi: %alertDate%"
#define SMS_MSG_ALARM_SET "Postavleno na ohranu."
#define SMS_MSG_ALARM_UNSET "Snyato s ohrany."
#define SMS_MSG_ALARM_ALREADY_SET "Uje stoit na ohrane. Data postanovki: %aSetDate%"
#define SMS_MSG_ALARM_ALREADY_UNSET "Uje snyato s ohrany. Data snyatiya: %aUnsDate%"

//Константы для измерения Вольтажа
#define VOLT_MAIN_LOW_VOLTAGE 4.60 //меньше этого значения на датчике будет считаться отсутствием сети

//Константы для охраны
#define ALARM_MSG_MAIN "Trevoga! Prichina trevogi: "
#define ALARM_MSG_MOTION "Dvijenie"
#define ALARM_MSG_TERM_ROOM "t Komnaty: %tRoom%C"
#define ALARM_MSG_TERM_TUBES "t Batarei: %tBat%C"
#define ALARM_MSG_VOLT_ACC "v Accumulyatora: %Vacc%V\nSet': %Vmain%"
#define ALARM_TERM_ROOM_MIN 9.00    //минимальная  температура комнаты
#define ALARM_TERM_ROOM_MAX 35.00   //максимальная температура комнаты
#define ALARM_TERM_TUBES_MIN 10.00  //минимальная  температура теплоносителя
#define ALARM_TERM_TUBES_MAX 100.00 //максимальная температура теплоносителя
#define ALARM_VOLT_ACC_MIN 2.00     //минимальное  напряжение аккумулятора //TODO: изменить на реальную цифру
#define ALARM_VOLT_ACC_MAX 4.00     //максимальное напряжение аккумулятора //TODO: изменить на реальную цифру

#endif
