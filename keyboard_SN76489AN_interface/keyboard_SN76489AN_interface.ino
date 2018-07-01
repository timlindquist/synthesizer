/** Author: Tim Lindquist
 *  Rev: 0.0
 */

#include <USBHost_t36.h>
#include <math.h>

#define D0 0
#define D1 1
#define D2 2
#define D3 3
#define D4 4
#define D5 5
#define D6 6
#define D7 7
#define WE 9

USBHost teensySerial;
USBHub hub1(teensySerial);
USBHub hub2(teensySerial);
MIDIDevice midiDev(teensySerial);

void setup() {
  Serial.begin(115200);
  teensySerial.begin();
  midiDev.setHandleNoteOn(keyboardNoteOn);
  midiDev.setHandleNoteOff(keyboardNoteOff);
}

void loop() {
  silenceAllChannels();
  
  teensySerial.Task();
  midiDev.read();
}

void keyboardNoteOn(byte channel, byte note, byte velocity) {
  Serial.print("note on:  ");
  Serial.print(note);
  Serial.print("\tvelocity: ");
  Serial.println(velocity);
}

void keyboardNoteOff(byte channel, byte note, byte velocity) {
  Serial.print("note off: ");
  Serial.print(note);
  Serial.print("\tvelocity: ");
  Serial.println(velocity);
}

void silenceAllChannels(){
  sendByte(0x9f);
  sendByte(0xbf);
  sendByte(0xdf);
  sendByte(0xff);
}

void sendByte(byte b) {
  digitalWrite(WE, HIGH);
  digitalWrite(D0,(b&1)?HIGH:LOW);
  digitalWrite(D1,(b&2)?HIGH:LOW);
  digitalWrite(D2,(b&4)?HIGH:LOW);
  digitalWrite(D3,(b&8)?HIGH:LOW);
  digitalWrite(D4,(b&16)?HIGH:LOW);
  digitalWrite(D5,(b&32)?HIGH:LOW);
  digitalWrite(D6,(b&64)?HIGH:LOW);
  digitalWrite(D7,(b&128)?HIGH:LOW);
  digitalWrite(WE, LOW);
  delay(5);
  digitalWrite(WE, HIGH);
}
