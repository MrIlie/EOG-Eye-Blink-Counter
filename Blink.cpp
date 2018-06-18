#include "Arduino.h"

#define SAMPLES 200000

int blinks = 0;          //contorizeaza numarul de clipiri


void EyeBlink( int reset )
{
  
  if( reset == 1 )
  {
    blinks = 0; 
    Serial.println("reset");
  }
  else blinks++;
  Serial.println(blinks);
}

void Blink_Frequency()
{
  Serial.print("--------------------");
  Serial.println(blinks);
}

