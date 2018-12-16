#include <math.h>

#define REDPIN 11
#define GREENPIN 10
#define BLUEPIN 9
#define DELAY 100
#define NUM_NTC_ITERS 10
#define MAX_BRIGHT 200
#define MIN_BRIGHT 10
#define MIN_TERM 25.0
#define MAX_TERM 36.0
#define TERMPIN0 0
#define TERM_NUM 30

float vcc = 4.91;
float pad = 10000;                       // balance/pad resistor value
float thermr = 10000;                   // thermistor nominal resistance
float aTerm[TERM_NUM];

void setColor(int red, int green, int blue)
{
  analogWrite(REDPIN, 255 - red);
  analogWrite(GREENPIN, 255 - green);
  analogWrite(BLUEPIN, 255 - blue);
}

void setTermColor(float term)
{
  int bRed, bBlue, bGreen = 0;
  term = min(max(term, MIN_TERM), MAX_TERM);
  float temp = (((term - MIN_TERM) / (MAX_TERM - MIN_TERM)) * (MAX_BRIGHT - MIN_BRIGHT)) + MIN_BRIGHT;
  bRed = int(temp);
  bBlue = MAX_BRIGHT - bRed + MIN_BRIGHT;
  setColor(bRed, bGreen, bBlue);
}

float getTerm(int termPIN)
{
  long Resistance;  
  float Temp;
  Resistance=pad*((1024.0 / analogRead(termPIN)) - 1); 
  Temp = log(Resistance);
  Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp));
  Temp = Temp - 273.15;
  return Temp;
}

void addTerm(float term)
{
  for(int i=0; i<TERM_NUM-1; i++)
    aTerm[i] = aTerm[i+1];
  aTerm[TERM_NUM-1] = term;
}

float avgTerm()
{
  float res = 0;
  for(int i=0; i<TERM_NUM; i++)
    res = res + aTerm[i];
  return res / TERM_NUM;
}

void setup()
{
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(13, OUTPUT);
  Serial.begin(115200);
  for(int i=0; i<TERM_NUM; i++)
    aTerm[i] = 0;
}


void loop()
{
  float term;
  for(int i=0; i<NUM_NTC_ITERS; i++)
    {
      term = getTerm(TERMPIN0);
      addTerm(term);
      setTermColor(term);
      delay(DELAY);
    }
  Serial.print(term, 2);
  Serial.print("C, avg:");
  Serial.print(avgTerm(), 2);
  Serial.print(" C");
  Serial.println("");
  delay(DELAY);
}
