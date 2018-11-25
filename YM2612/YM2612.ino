/*
 * Test code for working with YM2612
 */

 
#include <stdio.h>



#define D0 0
#define D1 1
#define D2 2
#define D3 3
#define D4 4
#define D5 5
#define D6 6
#define D7 7
#define a1 9
#define a0 8
#define nRD 10
#define nWR 11
#define SEL0 12
#define SEL1 24
#define SEL2 25
#define SEL3 26
#define INH0 27
#define INH1  28
#define INH2  29
#define INH3  30


void setup() {
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(a1, OUTPUT);
  pinMode(a0, OUTPUT);
  pinMode(nRD, OUTPUT);
  pinMode(nWR, OUTPUT);
  pinMode(SEL0, OUTPUT); 
  pinMode(SEL1, OUTPUT); 
  pinMode(SEL2, OUTPUT); 
  pinMode(SEL3, OUTPUT); 
   
   pinMode(INH0, OUTPUT);
   pinMode(INH1, OUTPUT);
  pinMode(INH2, OUTPUT);
  pinMode(INH3, OUTPUT);


digitalWrite(INH0, HIGH);
 digitalWrite(INH1, HIGH);
  digitalWrite(INH3, HIGH);
  digitalWrite(INH2, HIGH);
  
  digitalWrite(a1, LOW);
  digitalWrite(a0, LOW);
  digitalWrite(nRD, HIGH);
  digitalWrite(nWR, HIGH);
  digitalWrite(INH2, HIGH);

  digitalWrite(SEL0, LOW);
  digitalWrite(SEL1, LOW);
  digitalWrite(SEL2, LOW);
  digitalWrite(SEL3, LOW);
  

}
int i=0;
void loop() {
  
  /* YM2612 Test code */ 
  setreg(0x22, 0x00); // LFO off
  setreg(0x27, 0x00); // Note off (channel 0)
  setreg(0x28, 0x01); // Note off (channel 1)
  setreg(0x28, 0x02); // Note off (channel 2)
  setreg(0x28, 0x04); // Note off (channel 3)
  setreg(0x28, 0x05); // Note off (channel 4)
  setreg(0x28, 0x06); // Note off (channel 5)
  setreg(0x2B, 0x00); // DAC off
  setreg(0x30, 0x71); //
  setreg(0x34, 0x0D); //
  setreg(0x38, 0x33); //
  setreg(0x3C, 0x01); // DT1/MUL
  setreg(0x40, 0x23); //
  setreg(0x44, 0x2D); //
  setreg(0x48, 0x26); //
  setreg(0x4C, 0x00); // Total level
  setreg(0x50, 0x5F); //
  setreg(0x54, 0x99); //
  setreg(0x58, 0x5F); //
  setreg(0x5C, 0x94); // RS/AR 
  setreg(0x60, 0x05); //
  setreg(0x64, 0x05); //
  setreg(0x68, 0x05); //
  setreg(0x6C, 0x07); // AM/D1R
  setreg(0x70, 0x02); //
  setreg(0x74, 0x02); //
  setreg(0x78, 0x02); //
  setreg(0x7C, 0x02); // D2R
  setreg(0x80, 0x11); //
  setreg(0x84, 0x11); //
  setreg(0x88, 0x11); //
  setreg(0x8C, 0xA6); // D1L/RR
  setreg(0x90, 0x00); //
  setreg(0x94, 0x00); //
  setreg(0x98, 0x00); //
  setreg(0x9C, 0x00); // Proprietary
  setreg(0xB0, 0x30); // Feedback/algorithm
  setreg(0xB4, 0xC0); // Both speakers on
  setreg(0x28, 0x00); // Key off
  setreg(0xA4, 0x22); // 
  //setreg(0xA0, 0x69); // Set frequency
  setreg(0xA0, 0x69); // Set frequency

  //for(;;) {
    //delay(1000);
    setreg(0x28, 0xF0); // Key on
    delay(1000);
    setreg(0x28, 0x00); // Key off

  //}
  //i=rand()*255;
}


static void setreg(uint8_t reg, uint8_t data) {
  digitalWrite(a0, LOW);
  write_ym(reg);
  digitalWrite(a0, HIGH);
  write_ym(data);
}



static void write_ym(uint8_t data) {
  digitalWrite(INH2, LOW);
  digitalWrite(D0,(data&1)?HIGH:LOW);
  digitalWrite(D1,(data&2)?HIGH:LOW);
  digitalWrite(D2,(data&4)?HIGH:LOW);
  digitalWrite(D3,(data&8)?HIGH:LOW);
  digitalWrite(D4,(data&16)?HIGH:LOW);
  digitalWrite(D5,(data&32)?HIGH:LOW);
  digitalWrite(D6,(data&64)?HIGH:LOW);
  digitalWrite(D7,(data&128)?HIGH:LOW);
  delayMicroseconds(1);
  digitalWrite(nWR, LOW);
  delayMicroseconds(1);
  digitalWrite(nWR, HIGH);
  delayMicroseconds(1);
  digitalWrite(INH2, HIGH);
}
