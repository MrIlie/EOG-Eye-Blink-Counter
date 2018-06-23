#include "Arduino.h"

#define SAMPLES 200000

int blinks = 0;          //contorizeaza numarul de clipiri


void EyeBlink()
{
  blinks++;
  //Serial.println(blinks);
}

void Blink_Frequency(int interval)
{
  Serial.println("--------------------");
  Serial.print(blinks);
  Serial.print(" clipiri per ");
  Serial.print(interval);
  Serial.println(" secunde.");
  blinks = 0;
}


void Blink_Stop()
{
  Serial.println("--------------------");
  Serial.print("Pana acum s-au citit ");
  Serial.print(blinks);
  Serial.print(" clipiri");
  Serial.println("");
  blinks = 0;
}

