#include <OneWire.h>
#include <DallasTemperature.h>
#include <math.h>

#define ONE_WIRE_BUS 10 // Data wire is plugged into port 2 on the Arduino
#define TERMPIN0 0
#define TERMPIN1 1
#define TERM_NUM 30

float vcc = 5.00;
float pad0 = 10000;                       // balance/pad resistor value
float pad1 = 10000;
float thermr = 10000;                   // thermistor nominal resistance
float aTerm[2][TERM_NUM];

OneWire oneWire(ONE_WIRE_BUS);// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire);// Pass our oneWire reference to Dallas Temperature. 
DeviceAddress insideThermometer;// arrays to hold device address


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

// function to print the temperature for a device
String getDallasTemperature(DeviceAddress deviceAddress)
{
  return String(sensors.getTempC(deviceAddress)) + 'C';
}

void setup(void)
{
  // start serial port
  Serial.begin(115200);
  
  for(int i=0; i<TERM_NUM; i++)
    for(int j=0; j<2; j++)
      aTerm[j][i] = 0;
  
  Serial.println("Dallas Temperature IC Control Library Demo");

  // locate devices on the bus
  Serial.print("Locating devices...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0"); 

  sensors.setResolution(insideThermometer, 9);
  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC); 
  Serial.println();
}

void loop(void)
{ 
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.print(getDallasTemperature(insideThermometer)); // Use a simple function to print out the data
  
  float term0, term1;
  term0 = getTerm(TERMPIN0, pad0);
  addTerm(0, term0);
  term1 = getTerm(TERMPIN1, pad1);
  addTerm(1, term1);
  
  Serial.print(" ");
  Serial.print(term0, 1);
  Serial.print("C (");
  Serial.print(avgTerm(0), 1);
  Serial.print("C) ");
  Serial.print(term1, 1);
  Serial.print("C (");
  Serial.print(avgTerm(1), 1);
  Serial.println("C)");
  delay(2000);
}
