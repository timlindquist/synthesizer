/** Author: Tim Lindquist
 *  Rev: 2.0
 *
 *  NOTES: reads raw samples from SD card. Adjusts gain of notes from velocity, multiple notes can be played at once, sustain knob implemented
 */

#include <USBHost_t36.h>
#include <math.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <string.h>



// GUItool: begin automatically generated code
AudioPlaySdRaw           playSdRaw4;     //xy=155,373
AudioPlaySdRaw           playSdRaw3;     //xy=158,310
AudioPlaySdRaw           playSdRaw1;     //xy=163,207
AudioPlaySdRaw           playSdRaw2;     //xy=163,254
AudioEffectFade          fade4;          //xy=321,367
AudioEffectFade          fade2;          //xy=323,252
AudioEffectFade          fade3;          //xy=324,309
AudioEffectFade          fade1;          //xy=325,208
AudioMixer4              mixer1;         //xy=502,230
AudioOutputAnalog        dac1;           //xy=658,230
AudioConnection          patchCord1(playSdRaw4, fade4);
AudioConnection          patchCord2(playSdRaw3, fade3);
AudioConnection          patchCord3(playSdRaw1, fade1);
AudioConnection          patchCord4(playSdRaw2, fade2);
AudioConnection          patchCord5(fade4, 0, mixer1, 3);
AudioConnection          patchCord6(fade2, 0, mixer1, 1);
AudioConnection          patchCord7(fade3, 0, mixer1, 2);
AudioConnection          patchCord8(fade1, 0, mixer1, 0);
AudioConnection          patchCord9(mixer1, dac1);

// GUItool: end automatically generated code



//Button and output defines
#define rOVF 8  //resource overflow LED
#define SUSTAIN_KNOB 32

//SD card
#define SDCARD_CS_PIN    BUILTIN_SDCARD
#define SDCARD_MOSI_PIN  11  // not actually used
#define SDCARD_SCK_PIN   13  // not actually used

#define numNotes 120	  //enough to cover MIDI range
#define numRaw    4     //number of raw inputs in mixer
#define numChannels 1   //number of channels in a single track
#define numTracks 1     //this will use seperate chip hardware


USBHost teensySerial;
USBHub hub1(teensySerial);
USBHub hub2(teensySerial);
MIDIDevice midiDev(teensySerial);

char wChannel=0;  //working channel
char wTrack=0;    //working track

String library="kygo";
int decayTime=0;

typedef struct{
  bool on=false;
  byte velocity=0;
  byte sdRaw=0;
} NOTE;

NOTE keys[numTracks][numChannels][numNotes];  //holds all MIDI keys (note is the index)



void setup() {
  Serial.begin(115200);
  AudioMemory(8);
  teensySerial.begin();
  midiDev.setHandleNoteOn(keyboardNoteOn);
  midiDev.setHandleNoteOff(keyboardNoteOff);

  pinMode(rOVF, OUTPUT);
  pinMode(SUSTAIN_KNOB,INPUT);  //decay on note after release
  
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
    buttonsRead();
  }
 
}




void keyboardNoteOn(byte channel, byte note, byte velocity) {
  Serial.print("note on:  ");
  Serial.print(note);
  Serial.print("\tvelocity: ");
  Serial.println(velocity);
  keys[wTrack][wChannel][note].on=true;
  keys[wTrack][wChannel][note].velocity=velocity;
  writeNote(note);
}

void keyboardNoteOff(byte channel, byte note, byte velocity) {
  Serial.print("note off: ");
  Serial.print(note);
  Serial.print("\tvelocity: ");
  Serial.println(velocity);
  keys[wTrack][wChannel][note].on=false;
  releaseNote(note);
}

void writeNote(byte note){
  String f=library+"/" + (String)note + ".raw";
  const char *filename= f.c_str();
  double mixGain=keys[wTrack][wChannel][note].velocity/255.0;
  
  claimNote(note);    //reserve resource
  if(keys[wTrack][wChannel][note].sdRaw==0){
     resourceOVF(note);
   return;
  }
  
  if(keys[wTrack][wChannel][note].sdRaw==1){
    mixer1.gain(0,mixGain);
    fade1.fadeIn(1);
    playSdRaw1.play(filename); 
  }
  if(keys[wTrack][wChannel][note].sdRaw==2){
    mixer1.gain(1,mixGain);
    fade2.fadeIn(1);
    playSdRaw2.play(filename); 
  }
  if(keys[wTrack][wChannel][note].sdRaw==3){
    mixer1.gain(2,mixGain);
    fade3.fadeIn(1);
    playSdRaw3.play(filename); 
  }
  if(keys[wTrack][wChannel][note].sdRaw==4){
    mixer1.gain(3,mixGain);
    fade4.fadeIn(1);
    playSdRaw4.play(filename); 
  }
  
}


void releaseNote(byte note){
  if(keys[wTrack][wChannel][note].sdRaw==0){      //need to turn LED off
    resourceOVF(note);
  return;
  }
  
  if(keys[wTrack][wChannel][note].sdRaw==1) fade1.fadeOut(decayTime);
  if(keys[wTrack][wChannel][note].sdRaw==2) fade2.fadeOut(decayTime);
  if(keys[wTrack][wChannel][note].sdRaw==3) fade3.fadeOut(decayTime);
  if(keys[wTrack][wChannel][note].sdRaw==4) fade4.fadeOut(decayTime);

  //releaseNote 
  keys[wTrack][wChannel][note].sdRaw=0;
}


void claimNote(byte note){
  int i,j;
  int channel;
  for(i=1;i<=numRaw;i++){
      for(j=0;j<numNotes;j++){
        if((keys[wTrack][wChannel][j].sdRaw==i)){
          channel=0;
          break;
        }
        else{
          channel=i;
        }
      }
      if(channel!=0){
        keys[wTrack][wChannel][note].sdRaw=i;
        return;
      }
  }
}



void resourceOVF(byte note){
	digitalWrite(rOVF,(keys[wTrack][wChannel][note].on)?HIGH:LOW);
}




void buttonsRead(){
	decayTime=analogRead(SUSTAIN_KNOB)*3+1; //this should be on a button and decaytime should be a function
	
}


