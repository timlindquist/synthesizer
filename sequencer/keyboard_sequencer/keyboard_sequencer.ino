/** Author: Tim Lindquist
 *  Rev: 1.0
 *
 *  NOTES:  General organization and structure
 */


#include <stdio.h>
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

//SD card
#define SDCARD_CS_PIN    BUILTIN_SDCARD
#define SDCARD_MOSI_PIN  11  // not actually used
#define SDCARD_SCK_PIN   13  // not actually used


#define numNotes 120    //enough to cover MIDI range
#define numChannels 16   //number of channels in a single track
#define numTracks 4     //this will use seperate chip hardware
#define numSdRaw  4       //number of raw inputs in mixer

//pin defines
#define seqSel0 0
#define seqSel1 1
#define seqSel2 2
#define seqSel3 3
#define seqINH 4
#define seqRST 5
#define seqOutMux 6


USBHost teensySerial;
USBHub hub1(teensySerial);
USBHub hub2(teensySerial);
MIDIDevice midiDev(teensySerial);

char wChannel=0;  //working channel
char wTrack=0;    //working track
int wSdRaw=0;     //working sdRaw channel
int decayTime=0;

String library="5/";

typedef struct{
  bool on=false;
  bool keyBoard=true;
  byte track=0;
  byte keyVoice=0;
  byte seqVoice=0;
  byte velocity=0;
  byte sdRaw=0;
  char rec[2][16]={}; //record[0][]=noteON  record[1][]=noteOFF
} NOTE;

NOTE keys[numChannels][numNotes];  //holds all MIDI keys (note is the index) Note 0 reserved for sequencer.
//need numnotes so rec works for each note




void setup() {
  serialSetup();
  pinSetup();
  midiSetup();
  sdSetup();
  seqSetup();
}

void loop() {
  teensySerial.Task();  //move somewhere else
  int i;
  unsigned long time_now = 0;
  int period=200;
  for(;;){
    for(i=0;i<16;i++){
      time_now=millis();
      seqReadWord();
      if(keys[wChannel][0].rec[0][i]){
        seqWrite(6);
      }
      while(millis() < time_now + period) midiDev.read();
      
    }
    
  }

}




//-------------------------------------------------------------------------
//  SEQUENCER FUNCTIONS
//-------------------------------------------------------------------------
void seqRst(){
  digitalWrite(5, HIGH);  //make reset=1
  delay(1);
  digitalWrite(5, LOW);   //make reset=0
  delay(1);
}

void seqSetBit(byte b){
  //set select lines
  digitalWrite(seqSel0,(b&1)?HIGH:LOW);
  digitalWrite(seqSel1,(b&2)?HIGH:LOW);
  digitalWrite(seqSel2,(b&4)?HIGH:LOW);
  digitalWrite(seqSel3,(b&8)?HIGH:LOW);
  delay(1);
  digitalWrite(seqINH, LOW);  //makes set=1
  delay(1);
  digitalWrite(seqINH, HIGH);  //makes set=0
  delay(1);
}

void seqSetWord(int x[16]){
  int i;
  seqRst();
  for(i=0;i<16;i++){
    if(x[i]) seqSetBit(i);
  }
}

boolean seqReadBit(byte b){
  digitalWrite(seqSel0,(b&1)?HIGH:LOW);
  digitalWrite(seqSel1,(b&2)?HIGH:LOW);
  digitalWrite(seqSel2,(b&4)?HIGH:LOW);
  digitalWrite(seqSel3,(b&8)?HIGH:LOW);
  return digitalRead(seqOutMux);
}

void seqReadWord(){
   int i;
   for(i=0;i<16;i++){
      if(seqReadBit(i)){
        keys[wChannel][0].rec[0][i]=1;
      }
      else{
        keys[wChannel][0].rec[0][i]=0;
      }
   }
}

void seqWrite(byte note){
  String f="seq/" + (String)note + ".raw";
  const char *filename= f.c_str();
  double mixGain=1;
  claimSdRaw();
  Serial.println(wSdRaw);
  if(wSdRaw==0){
     resourceOVF(note);
   return;
  }
  if(wSdRaw==1){
    mixer1.gain(0,mixGain);
    fade1.fadeIn(1);
    playSdRaw1.play(filename); 
  }
  if(wSdRaw==2){
    mixer1.gain(1,mixGain);
    fade2.fadeIn(1);
    playSdRaw2.play(filename); 
  }
  if(wSdRaw==3){
    mixer1.gain(2,mixGain);
    fade3.fadeIn(1);
    playSdRaw3.play(filename); 
  }
  if(wSdRaw==4){
    mixer1.gain(3,mixGain);
    fade4.fadeIn(1);
    playSdRaw4.play(filename); 
  }
}

void claimSdRaw(){
  int i;
  for(i=0;i<numSdRaw;i++){
    wSdRaw++;
    if(wSdRaw>numSdRaw) wSdRaw=1;
    if(sdRawAvailable(wSdRaw)) return; 
  }
  wSdRaw=0;
  
}

boolean sdRawAvailable(char x){
  int i,j;
  for(i=0;i<numChannels;i++){
    for(j=0;j<numNotes;j++){
      if(keys[i][j].sdRaw==x) return false;
    }
  }
  return true;
}



//-------------------------------------------------------------------------



void resourceOVF(byte note){
  digitalWrite(rOVF,(keys[wChannel][note].on)?HIGH:LOW);
}




//-------------------------------------------------------------------------
//  KEYBOARD FUNCTIONS
//-------------------------------------------------------------------------
void keyboardNoteOn(byte channel, byte note, byte velocity) {
  keys[wChannel][note].on=true;
  keys[wChannel][note].velocity=velocity;
  writeNote(note);
}

void keyboardNoteOff(byte channel, byte note, byte velocity) {
  keys[wChannel][note].on=false;
  releaseNote(note);
}


void writeNote(byte note){
  String f="key/" + library +(String)note + ".raw";
  const char *filename= f.c_str();
  double mixGain=keys[wChannel][note].velocity/255.0;
  
  claimNote(note);    //reserve resource
  if(keys[wChannel][note].sdRaw==0){
     resourceOVF(note);
   return;
  }
  
  if(keys[wChannel][note].sdRaw==1){
    mixer1.gain(0,mixGain);
    fade1.fadeIn(1);
    playSdRaw1.play(filename); 
  }
  if(keys[wChannel][note].sdRaw==2){
    mixer1.gain(1,mixGain);
    fade2.fadeIn(1);
    playSdRaw2.play(filename); 
  }
  if(keys[wChannel][note].sdRaw==3){
    mixer1.gain(2,mixGain);
    fade3.fadeIn(1);
    playSdRaw3.play(filename); 
  }
  if(keys[wChannel][note].sdRaw==4){
    mixer1.gain(3,mixGain);
    fade4.fadeIn(1);
    playSdRaw4.play(filename); 
  }
  
}


void claimNote(byte note){
  int i,j;
  int channel;
  for(i=1;i<=numSdRaw;i++){
    if(i!=wSdRaw){ //needed to not overwite the sequencer
      for(j=0;j<numNotes;j++){
        if((keys[wChannel][j].sdRaw==i)){
          channel=0;
          break;
        }
        else{
          channel=i;
        }
      }
      if(channel!=0){
        keys[wChannel][note].sdRaw=i;
        return;
      }
    }
  }
}


void releaseNote(byte note){
  if(keys[wChannel][note].sdRaw==0){      //need to turn LED off
    resourceOVF(note);
  return;
  }
  
  if(keys[wChannel][note].sdRaw==1) fade1.fadeOut(decayTime);
  if(keys[wChannel][note].sdRaw==2) fade2.fadeOut(decayTime);
  if(keys[wChannel][note].sdRaw==3) fade3.fadeOut(decayTime);
  if(keys[wChannel][note].sdRaw==4) fade4.fadeOut(decayTime);

  //releaseNote 
  keys[wChannel][note].sdRaw=0;
}



//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
//  SETUP FUNCTIONS
//-------------------------------------------------------------------------
void serialSetup(){
  Serial.begin(9600);
}


void midiSetup(){
  teensySerial.begin();
  midiDev.setHandleNoteOn(keyboardNoteOn);
  midiDev.setHandleNoteOff(keyboardNoteOff);
}


void sdSetup(){
  AudioMemory(8);
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
}

void seqSetup(){
  digitalWrite(seqINH, HIGH);  //makes set=0
  digitalWrite(seqRST, LOW);   //make reset=0  
  seqRst();
}

void pinSetup(){
  pinMode(rOVF, OUTPUT);
  pinMode(seqSel0, OUTPUT); 
  pinMode(seqSel1, OUTPUT); 
  pinMode(seqSel2, OUTPUT); 
  pinMode(seqSel3, OUTPUT); 
  pinMode(seqINH, OUTPUT);
  pinMode(seqRST, OUTPUT); 
  pinMode(seqOutMux, INPUT);   //mux output
}

//-------------------------------------------------------------------------

