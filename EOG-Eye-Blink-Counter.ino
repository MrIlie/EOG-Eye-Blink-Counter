
// 14-05-2018
//     |
//     |
//     V
// 23-06-2018
//     |
//     |
//     V
//    ...


#include <CircularBuffer.h>
#include <SimpleTimer.h>
#include "main.h"



#define SAMPLERATE   200                // Frecventa de esantionare semnal / canal
#define READINGS     20                 // Interval pt calculul mediei semnalului   
#define INACTIVE_CMD 20                 // Interval inactivare comanda noua t = 0.1sec
#define READING_INT  5                  // Durata in ms intre esantionari succesive 

#define YTH_LIMIT_PLUS 60
#define YTH_LIMIT_MINUS 60
 

unsigned int y_threshold_up = 0; 
unsigned int y_threshold_dn = 0; 

SimpleTimer read_data;

CircularBuffer<int,SAMPLERATE> x_sampleBuf;
CircularBuffer<int,SAMPLERATE> y_sampleBuf;

int analog_yValue;                      // Valori analogice de semnal de la convertor 

int read_time = 0;

unsigned long y_mean_filt = 512;
unsigned long y_last_mean_filt = 512;
  
unsigned long ydc_comp = 512;


bool debug_values = false;
bool debug_plotter = true;


bool active_cmd_yAxis = true;

unsigned char y_stare = 0;

float bpm = 0;                          // clipiri pe minut
unsigned int counter = 0;               // contor timp

int last_y_average = 0;                 // Ultima valoare medie de pe axa y

//////////////////////////////////////



//////////////////////////////////////

void setup() {

  Serial.begin(9600);

  for(int i = 0; i < SAMPLERATE; i++)
  {
    y_sampleBuf.push(512);
  }
      
  y_threshold_up = ydc_comp + (ydc_comp * YTH_LIMIT_PLUS);        
  y_threshold_dn = ydc_comp - (ydc_comp * YTH_LIMIT_MINUS);       

  read_data.setInterval(READING_INT,readADC_values);
}

void Ycheck(int yValue)
{
  if(counter == 7200) 
  {
    Blink_Frequency();
    EyeBlink(1);
    counter = 0; 
  }
    counter++;
   
   //set starea s1
   if( last_y_average < y_threshold_up && yValue > y_threshold_up && !y_stare)
   {      
      EyeBlink(0);
   }


   //reset in starea s0
   if(( yValue <= y_threshold_dn && y_stare == 1) || 
   ( yValue >= y_threshold_up && y_stare == 2))
   {
      y_stare = 0;
   }

   last_y_average = yValue;
    
}
 

void readADC_values() {
  
    analog_yValue = analogRead(A4);
    
    y_sampleBuf.shift();                          // Eliminam din buffer prima valoare
    y_sampleBuf.push(analog_yValue);              // Adaugam in buffer pe ultima pozitie


    for(int i = 0; i < SAMPLERATE; i++)
      ydc_comp += y_sampleBuf[i];
    ydc_comp = ydc_comp / SAMPLERATE;
     
    y_threshold_up = ydc_comp + YTH_LIMIT_PLUS;        
    y_threshold_dn = ydc_comp - YTH_LIMIT_MINUS;      

    y_last_mean_filt = y_mean_filt;               // Salvare ultima valoare filtru medie 
    y_mean_filt = 0;            
    for(int i = SAMPLERATE-1; i > SAMPLERATE-READINGS-1; i--)
      y_mean_filt += y_sampleBuf[i];
    y_mean_filt = y_mean_filt / READINGS;

    Ycheck(y_mean_filt); 
     
 

   
}



void loop() {
  
  // simulation
  read_data.run();
  
  
}
