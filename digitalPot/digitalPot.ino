#include <Wire.h>
#define SCL A5
#define SDA A4
#define LP_S0 2
#define LP_S1 3
#define HP_S0 4
#define HP_S1 5
#define S0 6
#define S1 7
#define S2 8


void setup() {
  Wire.begin();
  Serial.begin(9600);
  pinMode(A3,INPUT);
  pinMode(A2,INPUT);
  pinMode(LP_S0, OUTPUT);
  pinMode(LP_S1, OUTPUT);
  pinMode(HP_S0, OUTPUT);
  pinMode(HP_S1, OUTPUT);
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  digitalWrite(LP_S0, HIGH);
  digitalWrite(LP_S1, HIGH);

  digitalWrite(HP_S0, HIGH);
  digitalWrite(HP_S1, HIGH);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, HIGH);
  digitalWrite(S2, LOW);
}
 
void loop() {

  byte val1=map(analogRead(A3),0,1023,0,253);
  byte val2=map(analogRead(A2),0,1023,0,253);
  Wire.beginTransmission(0x28);
  Wire.write(0xAF);
  Wire.write(val1);
  Wire.endTransmission();
  Wire.beginTransmission(0x29);
  Wire.write(0xAF);
  Wire.write(val2);
  Wire.endTransmission();
  

  
  Serial.println(val1);
  delay(100);
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

