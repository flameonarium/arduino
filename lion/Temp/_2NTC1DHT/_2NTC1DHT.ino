#include <math.h>
#include "DHT.h"

#define DHTPIN 2
#define DELAY 2000
#define TERMPIN0 5
#define TERMPIN1 4
#define TERM_NUM 30
#define RES_COUNT 4

DHT dht11(DHTPIN, DHT11);
float vcc = 5.08;
float pad0 = 10010;                       // balance/pad resistor value
float pad1 = 9860;
float thermr = 10000;                   // thermistor nominal resistance
float aTerm[RES_COUNT][TERM_NUM];

float getTerm(int termPIN, float pad = 10000)
{
  long Resistance;  
  float Temp;
  Resistance=pad*((1024.0 / analogRead(termPIN)) - 1); 
  Temp = log(Resistance);
  Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp));
  Temp = Temp - 273.15;
  return Temp;
}

void addTerm(int num, float term)
{
  for(int i=0; i<TERM_NUM-1; i++)
    aTerm[num][i] = aTerm[num][i+1];
  aTerm[num][TERM_NUM-1] = term;
}

float avgTerm(int num)
{
  float res = 0;
  for(int i=0; i<TERM_NUM; i++)
    res = res + aTerm[num][i];
  return res / TERM_NUM;
}

void setup()
{
  pinMode(13, OUTPUT);
  Serial.begin(115200);
  for(int i=0; i<TERM_NUM; i++)
    for(int j=0; j<RES_COUNT; j++)
      aTerm[j][i] = 0;
  dht11.begin();
}


void loop()
{
  float hum;
  float term0, term1, term2;
  //String msg;
  
  term0 = getTerm(TERMPIN0, pad0);
  addTerm(0, term0);
  term1 = getTerm(TERMPIN1, pad1);
  addTerm(1, term1);
  term2 = dht11.readTemperature();
  addTerm(2, term2);
  hum = dht11.readHumidity();
  addTerm(3, hum);
  //msg = "A) " + String(term0) + "C (" + String(avgTerm(0)) + "C)  B) " + String(term1) + "C (" + String(avgTerm(1)) + "C)";
  
  Serial.print("A) ");    Serial.print(term0, 1); Serial.print("C ("); Serial.print(avgTerm(0), 1);
  Serial.print("C)  B)"); Serial.print(term1, 1); Serial.print("C ("); Serial.print(avgTerm(1), 1);
  Serial.print("C)  C)"); Serial.print(term2, 1); Serial.print("C ("); Serial.print(avgTerm(2), 1);
  Serial.print("C)  H)"); Serial.print(hum, 1);   Serial.print("% ("); Serial.print(avgTerm(3), 1);
  Serial.println("%)");
  delay(DELAY);
}
