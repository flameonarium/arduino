#include <math.h>

#define DELAY 2000
#define TERMPIN0 4
#define TERMPIN1 5
#define TERM_NUM 30

float vcc = 5.08;
float pad0 = 10010;                       // balance/pad resistor value
float pad1 = 9860;
float thermr = 10000;                   // thermistor nominal resistance
float aTerm[2][TERM_NUM];

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
    for(int j=0; j<2; j++)
      aTerm[j][i] = 0;
}


void loop()
{
  float term0, term1;
  term0 = getTerm(TERMPIN0, pad0);
  addTerm(0, term0);
  term1 = getTerm(TERMPIN1, pad1);
  addTerm(1, term1);
  
  Serial.print("A) ");
  Serial.print(term0, 1);
  Serial.print("C, avg:");
  Serial.print(avgTerm(0), 1);
  Serial.print(" C      B)");
  Serial.print(term1, 1);
  Serial.print("C, avg:");
  Serial.print(avgTerm(1), 1);
  Serial.println("");
  delay(DELAY);
}
