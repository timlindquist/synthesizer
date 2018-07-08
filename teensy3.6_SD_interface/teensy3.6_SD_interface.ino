/** Author: Tim Lindquist
 *  Rev: 1.0
 *
 *  NOTES: reads raw samples from SD card
 */

#include <USBHost_t36.h>
#include <math.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <string.h>



AudioPlaySdRaw           playRaw0;
AudioOutputAnalog        audioOutput;
AudioConnection          patchCord1(playRaw0, 0, audioOutput, 0);
AudioConnection          patchCord2(playRaw0, 1, audioOutput, 1);
AudioControlSGTL5000     sgtl5000_1;


#define rOVF 8	//resource overflow LED

//SD card
#define SDCARD_CS_PIN    BUILTIN_SDCARD
#define SDCARD_MOSI_PIN  11  // not actually used
#define SDCARD_SCK_PIN   13  // not actually used

#define numNotes 120	  //enough to cover MIDI range
#define numChips 2       //number of physical chips in a track
#define numChipChannels 3  //number of channels on on a chip
#define numChannels 1   //number of channels in a single track
#define numTracks 1     //this will use seperate chip hardware

USBHost teensySerial;
USBHub hub1(teensySerial);
USBHub hub2(teensySerial);
MIDIDevice midiDev(teensySerial);

char wChannel=0;  //working channel
char wTrack=0;    //working track

typedef struct{
  bool on=false;
  byte velocity=0;
  byte chip=0;
  byte chipChannel=0;    //resource not used
  int timeON;
  int timeOFF;
} NOTE;

NOTE keys[numTracks][numChannels][numNotes];  //holds all MIDI keys (note is the index)



void setup() {
  Serial.begin(115200);
  AudioMemory(8);
  teensySerial.begin();
  midiDev.setHandleNoteOn(keyboardNoteOn);
  midiDev.setHandleNoteOff(keyboardNoteOff);

  pinMode(rOVF, OUTPUT);
  

  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
}



void loop() {
  teensySerial.Task();
  for(;;){
  midiDev.read();
  }
  buttonsRead();
 
}




void keyboardNoteOn(byte channel, byte note, byte velocity) {
  Serial.print("note on:  ");
  Serial.print(note);
  Serial.print("\tvelocity: ");
  Serial.println(velocity);
  keys[wTrack][wChannel][note].on=true;
  keys[wTrack][wChannel][note].velocity=velocity;
  String f="bass/" + (String)note + ".raw";
  const char *filename= f.c_str();
  playRaw0.play(filename);
}

void keyboardNoteOff(byte channel, byte note, byte velocity) {
  Serial.print("note off: ");
  Serial.print(note);
  Serial.print("\tvelocity: ");
  Serial.println(velocity);
  keys[wTrack][wChannel][note].on=false;
  //playRaw0.stop();
  
}





void resourceOVF(byte note){
	digitalWrite(rOVF,(keys[wTrack][wChannel][note].on)?HIGH:LOW);
}




void buttonsRead(){
	
	
	
}


