#define REDPIN 11
#define GREENPIN 10
#define BLUEPIN 9
#define DELAY 50
#define MAX_BRIGHT 200
#define MIN_BRIGHT 10

int bRed, bGreen, bBlue;
int dRed, dGreen, dBlue;

void setColor(int red, int green, int blue)
{
  analogWrite(REDPIN, 255 - red);
  analogWrite(GREENPIN, 255 - green);
  analogWrite(BLUEPIN, 255 - blue);
}

void setup()
{
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(13, OUTPUT);
  bRed = MIN_BRIGHT;
  bGreen = MAX_BRIGHT / 3;
  bBlue = 2 * MAX_BRIGHT / 3;
  dRed = 1;
  dGreen = 2;
  dBlue = 3;
}

void loopWhiteFade()
{
  digitalWrite(13, HIGH);
  for(int i=0; i<MAX_BRIGHT; i++)
  {
    setColor(i, i, i);
    delay(DELAY);
  }
  digitalWrite(13, LOW);
  for(int i=MAX_BRIGHT-1; i>=0; i--)
  {
    setColor(i, i, i);
    delay(DELAY);
  }
}

void loopRound()
{
  bRed = bRed + dRed;
  bGreen = bGreen + dGreen;
  bBlue = bBlue + dBlue;
  if((bRed >= MAX_BRIGHT) || (bRed < MIN_BRIGHT)) dRed = -1 * dRed;
  if((bGreen >= MAX_BRIGHT) || (bGreen < MIN_BRIGHT)) dGreen = -1 * dGreen;
  if((bBlue >= MAX_BRIGHT) || (bBlue < MIN_BRIGHT)) dBlue = -1 * dBlue;
  setColor(bRed, bGreen, bBlue);
  delay(DELAY);
}

void loop()
{
//  loopWhiteFade();
  loopRound();
}

/*setColor(255, 0, 0);  // red
  delay(DELAY);
  setColor(0, 255, 0);  // green
  delay(DELAY);
  setColor(0, 0, 255);  // blue
  delay(DELAY);
  setColor(255, 255, 0);  // yellow
  delay(DELAY);  
  setColor(80, 0, 80);  // purple
  delay(DELAY);
  setColor(0, 255, 255);  // aqua
  delay(DELAY);*/
//  digitalWrite(13, LOW);
//  delay(DELAY);
