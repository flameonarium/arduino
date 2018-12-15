#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0 
#define WHITE 0xFFFF
#define TERM_COUNT 6
#include <TFTLCD.h>
#include "lionTermNTC.h"

lionTermNTC *term;
int termF[TERM_COUNT];

TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);


void printTerm(int num, int x, int y, int _size = 5)
{
  int _termF = term[num].getTerm();
  String str;
  int color = GREEN;
  if(_termF != termF[num])
  {
    termF[num] = _termF;
    if(_termF == -273)
      str = "X";
    else if(_termF > 100)
    {
      str = "+99";
      color = RED;
    }
    else if(_termF < -100)
    {
      str = "-99";
      color = RED;
    }
    else if(_termF > 0)
    {
      str = "+" + String(_termF);
      color = GREEN;
    }
    else
    {
      str = String(_termF);
      color = BLUE;
    }
    tft.fillRect(x+(_size*2), y, _size*20, _size*10, BLACK);
    tft.setCursor(x+(_size*2)+2, y+(_size*2));
    tft.setTextColor(color);
    tft.setTextSize(_size);
    tft.println(str);
  }
}


void setTerm(int num, int pin)
{
  if(num >= TERM_COUNT)
    return;
  term[num] = lionTermNTC(pin, 20);
  termF[num] = +273;
}


void test()
{
  int x = 00;
  int y = 00;
  int _size = 8;
  tft.fillRect(x+(_size*2), y, _size*20, _size*10, BLACK);
  tft.setCursor(x+(_size*2)+2, y+(_size*2));
    tft.setTextColor(GREEN);
    tft.setTextSize(_size);
    tft.println("-28");
}


void setup()
{
  Serial.begin(115200);
  tft.reset();
  tft.initDisplay();
  tft.setRotation(3);
 //test();
  tft.fillScreen(BLACK);
  term = (lionTermNTC*)malloc(TERM_COUNT * sizeof(lionTermNTC));
  setTerm(0, A8);
  setTerm(1, A9);
  setTerm(2, A10);
  setTerm(3, A11);
  setTerm(4, A12);
  setTerm(5, A13);
}


void loop()
{
  printTerm(0, 0, 0, 8);
  printTerm(1, 0, 80, 8);
  printTerm(2, 0, 160, 8);
  printTerm(3, 160, 0, 8);
  printTerm(4, 160, 80, 8);
  printTerm(5, 160, 160, 8);
  delay(500);
}

