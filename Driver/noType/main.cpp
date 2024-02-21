#include <avr/io.h>
#include <avr/interrupt.h>
#include <Arduino.h>
#include <math.h>
#define m  0.8
#define pi 3.1415926535897932384626433832795
// Look up tables with 200 entries each, normalised to have max value of 1600 which is the period of the PWM loaded into register ICR1.
int va[] = {30,35,40,44,47,49,50,50,49,47,44,40,35,30,25,20,15,10,6,3,1,0,0,1,3,6,10,15,20,25};

int vb[] = {44,40,35,30,25,20,15,10,6,3,1,0,0,1,3,6,10,15,20,25,30,35,40,44,47,49,50,50,49,47};

int vc[] = {1,0,0,1,3,6,10,15,20,25,30,35,40,44,47,49,50,50,49,47,44,40,35,30,25,20,15,10,6,3};


double a = 0.0;
double b = 0.0;
double c = 0.0;

double V_alpha = 0.0;
double V_beta = 0.0;

double theta = 0.0;

double phi = 0.0;
int sect = 0;

volatile int num = 0;

double T1 = 0.0;
double T2 = 0.0;
double T3 = 0.0;

double S1 = 0.0;
double S2 = 0.0;
double S3 = 0.0;

void setup(){
  Serial.begin(9600);
      pinMode(3,OUTPUT);
    // Register initilisation, see datasheet for more detail.
    TCCR1A = 0;
    TCCR1B = 0;
    TIMSK1 = 0;

     DDRB |= (1 << PB1);
    DDRB |= (1 << PB2);// Set PB1 and PB2 as outputs.
    TCCR1A = (1 << WGM11);
    TCCR1A |= (1 << COM1A1);
    TCCR1A |= (1 << COM1B1);
    //fre = 5k =(16M)/50*64
    TCCR1B = (1 << WGM12) | (1 << WGM13);
    TCCR1B |= (1<< CS10)|(1<< CS11);
//    TCCR1B = 0b00011001;
       /*000
         11 WGM1 3:2 for waveform 15.
         001 no prescale on the counter.
       */
       TIMSK1 = (1 << ICIE1);
//    TIMSK1 = 0b00000001;
       /*0000000
         1 TOV1 Flag interrupt enable. 
       */
    ICR1   = 49;     // Period for 16MHz crystal, for a switching frequency of 5KHz for 30 subdevisions per 50Hz sin wave cycle.
    sei();             // Enable global interrupts.
//    DDRB = 0b00000110; 
 


      TCCR2A = 0;
      TCCR2B = 0;
      DDRD |= (1 << PD3);
      DDRB |= (1 << PB3);
      TCCR2A |= (0 << WGM21) | (1 << WGM20) | (0 << WGM22);//TOP_value =256
      TCCR2B |= (1 << CS20);//No Prescaling = F_Clock or F_clock/1=16mhz
      TCCR2A |= (1 << COM2B1) | (0 << COM2B0);// None-inverted mode
    // TCCR2A |= (1 << COM2A1) | (0 << COM2A0);
////      OCR2B = 256*0.8;
//////      
////    pinMode(13,OUTPUT);

      TCCR0A = 0;
      TCCR0B = 0;
      TIMSK0 = 0;
      DDRD |= (1 << PD5);
      DDRD |= (1 << PD6);
      TCCR0A |= (0 << WGM02) | (0 << WGM01) | (1 << WGM00);  //TOP_value =256
      TCCR0B |= (0 << CS12) | (0 << CS11) | (1 << CS10); //No Prescaling = F_Clock or F_clock/1=16mhz
      TCCR0A |= (1 << COM0A1) | (0 << COM0A0);// None-inverted mode
      TCCR0A |= (1 << COM0B1) | (1 << COM0B0);
//      OCR0A = 128;
          
}

void loop(){ 
  
       




       
  }



ISR(TIMER1_CAPT_vect){
       
        
        V_alpha = (sqrt(2.0/3.0)*(va[num]-0.5*vb[num]-0.5*vc[num]));
        V_beta = sqrt(2.0/3.0)*sqrt(3)/2*(vb[num] - vc[num]);
        theta = atan2(V_beta,V_alpha)*180/pi;
//        Serial.println(V_alpha);
         if(theta > 0 && theta <= 60)
      {
        phi = theta;
        sect = 1;
      }
      else if(theta > 60 && theta <= 120)
      {
        phi = theta - 60;
        sect = 2;
      }
      else if (theta > 120 && theta <= 180)
      {
        phi = theta - 120;
        sect = 3;
      }
      else if (theta > -180 && theta <= -120)
      {
        /* code */
        phi = theta + 180;
        sect = 4;
      }
      else if ( theta > -120 && theta <= -60)
      {
        /* code */
        phi = theta + 120;
        sect = 5;
      }
      else 
      {
        /* code */
        phi = theta + 60;
        sect = 6;
      }
//       Serial.println(phi);
//Serial.println(OCR1A);
       T1  = m*sin((60-phi)*pi/180)*40;
      T2  = m*sin(phi * pi/180)*40;
      T3  = (1 - T1 - T2);
  switch (sect){
  case 1:
    S1  = T1 + T2;
    S2  = T2;
    S3  = 0;
    break;

  case 2:
    S1  = T1;
    S2  = T1 + T2;
    S3  = 0;
    break;

  case 3:
    S1  = 0;
    S2  = T1 + T2;
    S3  = T2;
    break;

  case 4:
    S1  = 0;
    S2  = T1;
    S3  = T1 + T2;
    break;

  case 5:
    S1  = T2;
    S2  = 0;
    S3  = T1 + T2;
    break;

  case 6:
    S1  = T1 + T2;
    S2  = 0;
    S3  = T1;
    break;
}
      OCR1A = S1*1.5;
      OCR2B = S2*6.6;
      OCR0A = S3*7;
//      Serial.println(S1);
 
//     Serial.println(OCR1A);
     
      
    if(++num >= 29) // Pre-increment num then check it's below 200.
       num = 0;       // Reset num.
     
}
