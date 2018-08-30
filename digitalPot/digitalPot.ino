#include <Wire.h>
#define SCL A5
#define SDA A4

byte val = 0;

void setup() {
  Wire.begin();
  Serial.begin(9600);
}
 
void loop() {
  Wire.beginTransmission(0x28);
  Wire.write(0xA9);
  Wire.write(val);
  Serial.println(Wire.endTransmission());
  val++;
 if(val >254) val = 0;
 
  Serial.println(val);
  delay(1000);
}

