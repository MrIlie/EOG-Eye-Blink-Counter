

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
#include <SoftwareSerial.h> 
#include <SerialCommand.h>              // https://github.com/scogswell/ArduinoSerialCommand
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
SimpleTimer blink_monitor;              // Afiseaza cate clipiri au fost detectate intr-un minut

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

bool is_blink_activated = 0;            // Starea programului (detecteaza / nu detecteaza clipirile)

unsigned int blink_interval;

int blink_timer;

//////////////////////////////////////

SerialCommand SCmd;

//////////////////////////////////////

void SayHello()         // Functie test
{
  char *arg;  
  arg = SCmd.next();    // Get the next argument from the SerialCommand object buffer
  if (arg != NULL)      // As long as it existed, take it
  {
    Serial.print("Hello "); 
    Serial.println(arg); 
  } 
  else {
    Serial.println("Hello, whoever you are"); 
  }
}

void start_blink_read() // Porneste citirea clipirilor
{
  if(is_blink_activated) Serial.println("Programul deja ruleaza");
  else
  {
    is_blink_activated = 1;
    Serial.println("Incepem prelucrarea datelor.");
  
    int aNumber;  
    char *arg; 
    int first_cmd, second_cmd;
  
    arg = SCmd.next();
    if (arg != NULL) 
    {
      aNumber=atoi(arg);    // Converts a char string to an integer
      Serial.print("Esantion: "); 
      Serial.print(aNumber); 
      first_cmd = atoi(arg); 
      blink_interval = aNumber * 1000;
      } 
      else {
        Serial.print("[Implicit]Esantion: 10 secunde"); 
        blink_interval = 10000;
      }

      arg = SCmd.next(); 
      if (arg != NULL) 
      {
        aNumber=atol(arg); 
        //Serial.print("Second argument was: "); 
        if(aNumber == 1) {
          if(first_cmd == 1) Serial.print(" secunda");
          else Serial.print(" secunde");
        }
        if(aNumber == 2) {
          if(first_cmd == 1) Serial.print(" minut");
          else Serial.print(" minute");
          blink_interval = blink_interval * 60;
        }
        
        //Serial.println(aNumber); 
      } 
      else {
        if(first_cmd == 1) Serial.print(" secunda");
        else Serial.print(" secunde");
      }


    Serial.println("");
    
    blink_timer = blink_monitor.setInterval(blink_interval,check_blink);
    
  }
  
}

void stop_blink_read() // Opreste citirea clipirilor
{
  if(is_blink_activated){
    is_blink_activated = 0;
    Serial.println("Ai oprit citirea clipirilor.");
    Blink_Stop();
    blink_monitor.disable(blink_timer);
    
  }
  else Serial.println("Programul este deja oprit");
}

void close_communication() // Opreste citirea clipirilor
{
  Serial.println("Se inchide...");
  Serial.end();
}
void help() // Opreste citirea clipirilor
{
  Serial.println("Cum se utilizeaza:");
  Serial.println("Comanda: start <INTERVAL> <TIMP>");
  Serial.println("Unde: |- INTERVAL reprezinta numarul de secunde / de minute");
  Serial.println("      |- TIMP reprezinta tipul de unitate de masura: |- 1 pentru secunda");
  Serial.println("                                                     |- 2 pentru minut");
  Serial.println("");
  Serial.println("Exemplu:");
  Serial.println(">start");
  Serial.println("Citeste clipirile la un interval de 10 de secunde (valoare implicita)");
  Serial.println("");
  Serial.println(">start 30");
  Serial.println("Citeste clipirile la un interval de 30 de secunde");
  Serial.println("");
  Serial.println(">start 1 2");
  Serial.println("Citeste clipirile la un interval de 1 minut");
  Serial.end();
}



//////////////////////////////////////

void setup() {

  Serial.begin(9600);

  Serial.println("Salut !");
  Serial.println("Pentru rezultate mai bune, asteapta intre 5 si 10 secunde pentru calibrare, dupa conectarea electrozilor.");
  
  
  SCmd.addDefaultHandler(unrecognized);       // Comenzi nerecunoscute 

  SCmd.addCommand("hello",SayHello);
  SCmd.addCommand("start",start_blink_read);
  SCmd.addCommand("stop",stop_blink_read);
  SCmd.addCommand("help",help);
  SCmd.addCommand("close",close_communication);  // Inchide comunicarea, doar pentru a testa
  
  for(int i = 0; i < SAMPLERATE; i++)
  {
    y_sampleBuf.push(512);
  }
      
  y_threshold_up = ydc_comp + (ydc_comp * YTH_LIMIT_PLUS);        
  y_threshold_dn = ydc_comp - (ydc_comp * YTH_LIMIT_MINUS);       

  read_data.setInterval(READING_INT,readADC_values);
  //blink_timer = blink_monitor.setInterval(1000,check_blink);
}

void Ycheck(int yValue)
{
  
   //set starea s1
   if( last_y_average < y_threshold_up && yValue > y_threshold_up && !y_stare)
   {      
      EyeBlink();
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

    if(is_blink_activated)                      // In caz ca functia de citire a clipirilor va fi activata
    {                  
      Ycheck(y_mean_filt); 
    }
    else {
        counter = 0;                            // Se reinitializeaza contorul de citiri
    }     
 

   
}

void check_blink() {
  //Serial.println("CHECK");
  Blink_Frequency(blink_interval / 1000);
  //Blink_Reset();
}

void loop() {
  
  // simulation
  read_data.run();
  blink_monitor.run();
  
  SCmd.readSerial();                              // Citeste comenzi de pe seriala
}

void unrecognized()
{
  int var = random(0, 7);
  switch (var) {
    case 1:
      Serial.println("Ce?"); 
      break;
    case 2:
      Serial.println("Nu am inteles comanda."); 
      break;
    case 3:
      Serial.println("Poftim?"); 
      break;
    case 4:
      Serial.println("S-a suit pisica pe tastatura?"); 
      break;
    case 5:
      Serial.println("Hmmmm... nu am inteles comanda."); 
      break;
    case 6:
      Serial.println("Nene, ai gresit comanda!"); 
      break;
    default:
      Serial.println("Comanda gresita."); 
  }
}

