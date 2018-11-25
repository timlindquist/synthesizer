/* Author: Tim Lindquist
 *  Rev 1.0
 *  Code to control filter module
 */

#include <Wire.h>
#define LP_POT A0
#define HP_POT A1
#define LP_ORDER A2
#define HP_ORDER A3
#define SCL A5
#define SDA A4
#define LP_S0 0
#define LP_S1 1
#define HP_S0 2
#define HP_S1 3
#define FILT_S0 4
#define FILT_S1 5
#define FILT_S2 6

byte lpOrder=0;
byte hpOrder=0;
bool LP_ON=false;
bool HP_ON=false;
bool series=true;

unsigned long time_now = 0;
unsigned long hold=2000;
unsigned long miss=50;
  
void setup() {
  Wire.begin();
  pinMode(LP_POT,INPUT);
  pinMode(HP_POT,INPUT);
  pinMode(LP_ORDER,INPUT);
  pinMode(HP_ORDER,INPUT);
  pinMode(LP_S0, OUTPUT);
  pinMode(LP_S1, OUTPUT);
  pinMode(HP_S0, OUTPUT);
  pinMode(HP_S1, OUTPUT);
  pinMode(FILT_S0, OUTPUT);
  pinMode(FILT_S1, OUTPUT);
  pinMode(FILT_S2, OUTPUT);
  
  initialState();

}

void loop() {
  byte HP_val=0;
  byte HP_val_prev=0;
  byte LP_val;
  byte LP_val_prev=0;
  
  byte lpOrder_prev=0;
  byte hpOrder_prev=0;
  bool LP_ON_prev=false;
  bool HP_ON_prev=false;
  bool series_prev=true;
  
  for(;;){
    //resistors
    LP_val=1023-map(analogRead(LP_POT),0,1023,0,253);
    HP_val=1023-map(analogRead(HP_POT),0,1023,0,253);
    if(LP_val!=LP_val_prev || HP_val!=HP_val_prev) transmit(LP_val, HP_val);
    LP_val_prev=LP_val;
    HP_val_prev=HP_val;

    //switching
    buttons();
    if(lpOrder_prev!=lpOrder || hpOrder_prev!=hpOrder || LP_ON_prev!=LP_ON || HP_ON_prev!=HP_ON ||series_prev!=series) signalPath();
    lpOrder_prev=lpOrder;
    hpOrder_prev=hpOrder;
    LP_ON_prev=LP_ON;
    HP_ON_prev=HP_ON;
    series_prev=series;
    delay(100);
  }
  
}


void initialState(){
  digitalWrite(LP_S0, LOW);
  digitalWrite(LP_S1, LOW);
  digitalWrite(HP_S0, LOW);
  digitalWrite(HP_S1, LOW);
  digitalWrite(FILT_S0, LOW);
  digitalWrite(FILT_S1, LOW);
  digitalWrite(FILT_S2, LOW);
}

void buttons(){  
  //LP BUTTON
  time_now=millis();
  if(analogRead(LP_ORDER)&&!analogRead(HP_ORDER)){  //if only LP pressed
    while(analogRead(LP_ORDER)&&!analogRead(HP_ORDER));
    if(millis() > time_now + hold){  //if a hold
      LP_ON=false;
      lpOrder=0;
    }
    else if(millis() > time_now + miss){ //incrememnt order
      LP_ON=true;
      lpOrder++;
      if(lpOrder>4) lpOrder=4;
    }
  }
  //HP BUTTON
  time_now=millis();
  if(!analogRead(LP_ORDER)&&analogRead(HP_ORDER)){  //if only HP pressed
    while(analogRead(HP_ORDER)&&!analogRead(LP_ORDER));
    if(millis() > time_now + hold){  //if a hold
      HP_ON=false;
      hpOrder=0;
    }
    else if(millis() > time_now + miss){//incrememnt order
      HP_ON=true;
      hpOrder++;
      if(hpOrder>4) hpOrder=4;
    }
  }

  //BOTH BUTTON
  time_now=millis();
  if(analogRead(LP_ORDER)&&analogRead(HP_ORDER)){  //if only both pressed
    while(analogRead(LP_ORDER)&&analogRead(HP_ORDER));
    if(millis() > time_now + hold) series=!series;  //series or parallel
    if(millis() > time_now + 3*hold){   //reset to normal mode
      series=true;
      LP_ON=false;
      HP_ON=false;
      lpOrder=0;
      hpOrder=0;
    }
  }
    
 
}


void transmit(byte LP_val, byte HP_val){
  Wire.beginTransmission(0x28);
  Wire.write(0xAF);
  Wire.write(LP_val);
  Wire.endTransmission();
  Wire.beginTransmission(0x29);
  Wire.write(0xAF);
  Wire.write(HP_val);
  Wire.endTransmission();
}


void signalPath(){
  switch(lpOrder){
    case 1:
      digitalWrite(LP_S0, LOW);
      digitalWrite(LP_S1, LOW);
      break;
    case 2:
      digitalWrite(LP_S0, HIGH);
      digitalWrite(LP_S1, LOW);
      break;
    case 3:
      digitalWrite(LP_S0, LOW);
      digitalWrite(LP_S1, HIGH);
      break;
    case 4:
      digitalWrite(LP_S0, HIGH);
      digitalWrite(LP_S1, HIGH);
      break;
  }

  switch(hpOrder){
    case 1:
      digitalWrite(HP_S0, LOW);
      digitalWrite(HP_S1, LOW);
      break;
    case 2:
      digitalWrite(HP_S0, HIGH);
      digitalWrite(HP_S1, LOW);
      break;
    case 3:
      digitalWrite(HP_S0, LOW);
      digitalWrite(HP_S1, HIGH);
      break;
    case 4:
      digitalWrite(HP_S0, HIGH);
      digitalWrite(HP_S1, HIGH);
      break;
  }

  if(!LP_ON&&!HP_ON){ //passthrough
      digitalWrite(FILT_S0, LOW);
      digitalWrite(FILT_S1, LOW);
      digitalWrite(FILT_S2, LOW);
  }

  if(LP_ON&&!HP_ON){  //lowpass
      digitalWrite(FILT_S0, HIGH);
      digitalWrite(FILT_S1, LOW);
      digitalWrite(FILT_S2, LOW);    
  }

  if(!LP_ON&&HP_ON){  //highpass
      digitalWrite(FILT_S0, LOW);
      digitalWrite(FILT_S1, HIGH);
      digitalWrite(FILT_S2, LOW);    
  }

  if(LP_ON&&HP_ON&&series){  //bandpass
      digitalWrite(FILT_S0, LOW);
      digitalWrite(FILT_S1, HIGH);
      digitalWrite(FILT_S2, HIGH);    
  }
  
  if(LP_ON&&HP_ON&&!series){  //bandstop
      digitalWrite(FILT_S0, HIGH);
      digitalWrite(FILT_S1, HIGH);
      digitalWrite(FILT_S2, LOW);    
  }
  
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

