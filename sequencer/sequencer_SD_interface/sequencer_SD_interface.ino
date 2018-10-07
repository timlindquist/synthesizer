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
#define numRaw    4     //number of raw inputs in mixer
#define numChannels 16   //number of channels in a single track
#define numTracks 1     //this will use seperate chip hardware

USBHost teensySerial;
USBHub hub1(teensySerial);
USBHub hub2(teensySerial);

char wChannel=0;  //working channel
char wTrack=0;    //working track
int seq[16];      //sequencer values

int period = 125;
unsigned long time_now = 0;


String library="bass";

typedef struct{
  bool on=false;
  bool seq=true;
  char rec[2][16]={}; //record[0][]=noteON  record[1][]=noteOFF
  char instrument=0;
  byte velocity=0;
  byte sdRaw=0;
} NOTE;

NOTE keys[numTracks][numChannels][120];  //holds all MIDI keys (note is the index)

void setup() {                
  Serial.begin(9600);
  pinMode(0, OUTPUT); 
  pinMode(1, OUTPUT); 
  pinMode(2, OUTPUT); 
  pinMode(3, OUTPUT); 
  pinMode(4, OUTPUT); //inhibit
  pinMode(5, OUTPUT); //reset 
  pinMode(6, INPUT);   //mux output
  pinMode(32, INPUT); //pot
  //initial state
  digitalWrite(4, HIGH);  //makes set=0
  digitalWrite(5, LOW);   //make reset=0  
  rst();
  
  AudioMemory(8);
  teensySerial.begin();
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

// the loop routine runs over and over again forever:
void loop() {
  int i;
  int sixteenth=60*1000/120.0;
  teensySerial.Task();
  for(;;){

    for(i=0;i<16;i++){
      time_now=millis();
      period=50;
      readWord();
      if(seq[i]){
        writeNote(60);
      }
      while(millis() < time_now + period);
      
    }
  }
 

}



void rst(){
  digitalWrite(5, HIGH);  //make reset=1
  delay(1);
  digitalWrite(5, LOW);   //make reset=0
  delay(1);
}

void set_bit(byte b){
  //set select lines
  digitalWrite(0,(b&1)?HIGH:LOW);
  digitalWrite(1,(b&2)?HIGH:LOW);
  digitalWrite(2,(b&4)?HIGH:LOW);
  digitalWrite(3,(b&8)?HIGH:LOW);
  delay(1);
  digitalWrite(4, LOW);  //makes set=1  INH=LOW=false
  delay(1);
  digitalWrite(4, HIGH);  //makes set=0 INH=HIGH=true
  delay(1);
}

void set_word(int x[16]){
  int i;
  rst();
  for(i=0;i<16;i++){
    if(x[i]){
      set_bit(i);
    }
  }
  
}

boolean readBit(byte b){
  digitalWrite(0,(b&1)?HIGH:LOW);
  digitalWrite(1,(b&2)?HIGH:LOW);
  digitalWrite(2,(b&4)?HIGH:LOW);
  digitalWrite(3,(b&8)?HIGH:LOW);
  return digitalRead(6);
}

void readWord(){
   int i;
   for(i=0;i<16;i++){
      if(readBit(i)){
        seq[i]=1;
        //keys[wTrack][wChannel][0].rec[0][i]=1;
      }
      else{
        seq[i]=0;
        //keys[wTrack][wChannel][0].rec[0][i]=0;
      }
      //Serial.print(seq[i]);
   }
   //Serial.print("\n");
}


void writeNote(byte note){
  String f=library+"/" + (String)note + ".raw";
  const char *filename= f.c_str();
  double mixGain=1;
  
  claimNote(note);    //reserve resource
  Serial.println(keys[wTrack][wChannel][note].sdRaw);
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

