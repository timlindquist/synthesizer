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





#define CLK 3579545

#define numNotes 120    //enough to cover MIDI range
#define numChips 4       //number of physical chips in a track
#define numChipChannels 3  //number of channels on on a chip
#define numChannels 1   //number of channels in a single track
#define numTracks 1     //this will use seperate chip hardware
#define numSdRaw  4       //number of raw inputs in mixer

//pin defines
#define D0 0
#define D1 1
#define D2 2
#define D3 3
#define D4 4
#define D5 5
#define D6 6
#define D7 7
#define SEL0 12
#define SEL1 24
#define SEL2 25
#define SEL3 26
#define INH0 27
#define INH1  28
#define INH2  29
#define INH3  30
#define Y0 14
#define Y1 15
#define Y2 16
#define tempo_sel0 31
#define tempo_sel1 32
#define R_LED_SIG 23
#define C0  8
#define C1  9
#define C2  10
#define C3  11
#define PP_LED_SIG 22
#define R_LED_SIG 23


USBHost teensySerial;
USBHub hub1(teensySerial);
USBHub hub2(teensySerial);
MIDIDevice midiDev(teensySerial);

int counter=0;

byte chipEnablePin=0; 
byte wChannel=0;  //working channel
byte wTrack=0;    //working track
int wSdRaw=0;     //working sdRaw channel
int decayTime=0;
byte tempo=120;
byte wInstrument=0;
bool Play=false;
bool Record=false;

String library="7/";

typedef struct{
  bool on=false;
  bool keyBoard=true;
  byte track=0;
  byte chipChannel=0; //for what resourse inside chip 1,2,3
  byte chip=0;
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
  digitalWrite(INH1,LOW);
  digitalWrite(INH2,HIGH);
  digitalWrite(INH3,HIGH);
  
  pinSetup();
  midiSetup();
  sdSetup();
  seqSetup();
  controlPannelSetup();
  initialState();
  
}

void loop() {
  teensySerial.Task();  //move somewhere else
  int i;
  unsigned long time_now = 0;

  for(;;){
    
    for(i=0;i<16;i++){
      time_now=millis();
      seqReadWord();
      if(keys[wChannel][0].rec[0][i]){
        seqWrite(1);
      }
      if(i==0|i==4|i==8|i==12) tempoSpot(i/4);
      while(millis() < time_now + 256-tempo){
        midiDev.read();
        buttonPress();
      }
    }
   
  }

}







void tempoSpot(int i){
  
    if(i==0){
      digitalWrite(tempo_sel0,LOW);
      digitalWrite(tempo_sel1,LOW);
    }
    if(i==1){
      digitalWrite(tempo_sel0,HIGH);
      digitalWrite(tempo_sel1,LOW);
    }
    if(i==2){
      digitalWrite(tempo_sel0,LOW);
      digitalWrite(tempo_sel1,HIGH);
    }
    if(i==3){
      digitalWrite(tempo_sel0,HIGH);
      digitalWrite(tempo_sel1,HIGH);
    }
}


//-------------------------------------------------------------------------
//  SEQUENCER FUNCTIONS
//-------------------------------------------------------------------------
void seqRst(){
  digitalWrite(SEL0,LOW);
  digitalWrite(SEL1,LOW);
  digitalWrite(SEL2,LOW);
  digitalWrite(SEL3,LOW);
  
  digitalWrite(INH3,LOW); //inhibit 3, make reset=1
  delay(1);
  digitalWrite(INH3, HIGH);   //make reset=0
  delay(1);
  
}

void seqSetBit(byte b){
  //set select lines
  digitalWrite(SEL0,(b&1)?HIGH:LOW);
  digitalWrite(SEL1,(b&2)?HIGH:LOW);
  digitalWrite(SEL2,(b&4)?HIGH:LOW);
  digitalWrite(SEL3,(b&8)?HIGH:LOW);
  delay(1);
  digitalWrite(INH0, LOW);  //makes set=1
  delay(1);
  digitalWrite(INH0, HIGH);  //makes set=0
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
  digitalWrite(SEL0,(b&1)?HIGH:LOW);
  digitalWrite(SEL1,(b&2)?HIGH:LOW);
  digitalWrite(SEL2,(b&4)?HIGH:LOW);
  digitalWrite(SEL3,(b&8)?HIGH:LOW);
  return digitalRead(Y0);
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
//  CONTROL PANNEL FUNCTIONS
//-------------------------------------------------------------------------
void decoder3(byte b){
  digitalWrite(SEL0,(b&1)?HIGH:LOW);
  digitalWrite(SEL1,(b&2)?HIGH:LOW);
  digitalWrite(SEL2,(b&4)?HIGH:LOW);
  digitalWrite(SEL3,(b&8)?HIGH:LOW);
  delay(1);
  digitalWrite(INH3,LOW); //inhibit 3, make reset=1
  delay(1);
  digitalWrite(INH3, HIGH);   //make reset=0
  delay(1);
}

void buttonPress(){
  if(counter%100000!=0){
    counter++;
    return;
  }
  counter++;
  //check all buttons
  
  //tempo
  accessBtn(8);
  if(tempo<255 & digitalRead(Y1)) tempo++;
  accessBtn(7);
  if(tempo>0 && digitalRead(Y1)) tempo--;
  writeHex(4,tempo & 0x0F); //LOW 4 bits
  writeHex(3,tempo >> 4); //HIGH 4 bits

  //Instrument
  accessBtn(3);
  if(wInstrument<255 && digitalRead(Y1)) wInstrument++;
  accessBtn(2);
  if(wInstrument>0 && digitalRead(Y1)) wInstrument--;
  writeHex(2,wInstrument & 0x0F); //LOW 4 bits
  writeHex(1,wInstrument >> 4); //HIGH 4 bits

  //channel
  
  accessBtn(1);
  if(wChannel<15 && digitalRead(Y1)) wChannel++;
  accessBtn(0);
  if(wChannel>0 && digitalRead(Y1)) wChannel--;
  writeHex(0,wChannel & 0x0F); //LOW 4 bits


  //play pause stop
  accessBtn(4);
  if(digitalRead(Y1)) Play=!Play;
  accessBtn(5);
  if(digitalRead(Y1)) Play=false;
    
  //record
  accessBtn(6);
  if(digitalRead(Y1)) Record=!Record;

  digitalWrite(PP_LED_SIG,(Play)?HIGH:LOW);
  digitalWrite(R_LED_SIG,(Record)?HIGH:LOW);
 
  
}

void accessBtn(byte b){
  digitalWrite(SEL0,(b&1)?HIGH:LOW);
  digitalWrite(SEL1,(b&2)?HIGH:LOW);
  digitalWrite(SEL2,(b&4)?HIGH:LOW);
  digitalWrite(SEL3,(b&8)?HIGH:LOW);
  delay(1);
}


//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
//  DISPLAY FUNCTIONS
//-------------------------------------------------------------------------

void writeHex(byte num, byte b){
  num+=4; //maps to correct display
  //select display
  digitalWrite(SEL0,(num&1)?HIGH:LOW);
  digitalWrite(SEL1,(num&2)?HIGH:LOW);
  digitalWrite(SEL2,(num&4)?HIGH:LOW);
  digitalWrite(SEL3,(num&8)?HIGH:LOW);
  //set number
  digitalWrite(C0,(b&1)?HIGH:LOW);
  digitalWrite(C1,(b&2)?HIGH:LOW);
  digitalWrite(C2,(b&4)?HIGH:LOW);
  digitalWrite(C3,(b&8)?HIGH:LOW);
  delay(1);
  digitalWrite(INH2,LOW);
  delay(10);
  digitalWrite(INH2,HIGH);
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
  if(wInstrument==1){
    writeNote(note);
  }
  else{
    writeNoteRaw(note);
  }
  
}

void keyboardNoteOff(byte channel, byte note, byte velocity) {
  keys[wChannel][note].on=false;
  if(wInstrument==1){
     releaseNote(note);
  }
  else{
    releaseNoteRaw(note);
  }
}
//-------------------------------------------------------------------------



//-------------------------------------------------------------------------
//  TI FUNCTIONS
//-------------------------------------------------------------------------
void writeNote(byte note){
  double freq;
  int n;
  String bin="";
  int i;
  int weight[10]={512,256,128,64,32,16,8,4,2,1};
  byte data;
  claimNote(note);    //reserve resource
  if(keys[wChannel][note].chipChannel==0){
     resourceOVF(note);
   return;
  }
  freq=pow(2,(note-69)/12.0)*440;
  n=round(CLK/(32.0*freq));  
  //10 bit binary conversion of n
  for(i=0;i<10;i++){
    if(n-weight[i]>=0){
      bin=bin+'1';
      n=n-weight[i];
    }
    else{
      bin=bin+'0';
    }
  }
  //construct data for sending 1st byte
  if(keys[wChannel][note].chipChannel==1) data=128;
  if(keys[wChannel][note].chipChannel==2) data=160;
  if(keys[wChannel][note].chipChannel==3) data=192; 
  for(i=4;i<8;i++){
    if(bin.charAt(i+2)=='1'){
      data+=weight[i+2];
    }
  }
  //update CE for correct chip select 
  chipEnablePin=keys[wChannel][note].chip-1;
  sendByte(data);
  
  //construct data for sending 2nd byte
  data=0;
  for(i=2;i<8;i++){
    if(bin.charAt(i-2)=='1'){
      data+=weight[i+2];
    }
  }
  sendByte(data);
  volumeAdjust(note); 
}


void releaseNote(byte note){
  if(keys[wChannel][note].chipChannel==0){
    resourceOVF(note);
  return;
  }
  //update CE for correct chip select
  chipEnablePin=keys[wChannel][note].chip-1;
  volumeAdjust(note);
  //releaseNote
  keys[wChannel][note].chipChannel=0;
  keys[wChannel][note].chip=0;
}

void claimNote(byte note){
  int i,j,k;
  int channel;
  for(i=1;i<=numChips;i++){
    for(j=1;j<=numChipChannels;j++){
      for(k=0;k<numNotes;k++){
        if((keys[wChannel][k].chip==i)&&(keys[wChannel][k].chipChannel==j)){
          channel=0;
          break;
        }
        else{
          channel=j;
        }
      }
      if(channel!=0){
        keys[wChannel][note].chipChannel=j;
        keys[wChannel][note].chip=i;
        return;
      }
    }
  }
  
}


void volumeAdjust(byte note){
  byte data;
  char att; //attenuation
  //if(keys[wTrack][wChannel][note].chipChannel==0) return;
  
  if(keys[wChannel][note].on){  //note on: construct data based on channel
    if(keys[wChannel][note].chipChannel==1) data=144;
    if(keys[wChannel][note].chipChannel==2) data=176;
    if(keys[wChannel][note].chipChannel==3) data=208;
  
    /**construct data based off velocity (attenuation: 0db=+0, 2db=+1, 4db=+2, 8db=+4, 16db=+8)
     * map 0 to 128 to 0 to 5, 0-1=16db, 1-2=8db, 2-3=4db, 3-4=2db, 4-5=0db
     * after floor 0=16db, 1=8db, 2=4db, 3=2db, 4=0db 
     * since a velocity range is 0-127 mapping to 128 will secure a value less than 5 to always occure
     */
    att=floor(map(keys[wChannel][note].velocity,0,128,0,5));
    if(att!=4) data+=pow(2,3-att);
  }
  else{ //note off: construct data based off channel
    if(keys[wChannel][note].chipChannel==1) data=159;
    if(keys[wChannel][note].chipChannel==2) data=191;
    if(keys[wChannel][note].chipChannel==3) data=223;
  }
  sendByte(data);
}

void silenceAllChannels(){
  sendByte(0x9f);
  sendByte(0xbf);
  sendByte(0xdf);
  sendByte(0xff);
}

void sendByte(byte b) {
  digitalWrite(INH1, LOW);
  digitalWrite(SEL0,(chipEnablePin&1)?HIGH:LOW);
  digitalWrite(SEL1,(chipEnablePin&2)?HIGH:LOW);
  digitalWrite(SEL2,(chipEnablePin&4)?HIGH:LOW);
  digitalWrite(SEL3,(chipEnablePin&8)?HIGH:LOW);

  digitalWrite(D0,(b&1)?HIGH:LOW);
  digitalWrite(D1,(b&2)?HIGH:LOW);
  digitalWrite(D2,(b&4)?HIGH:LOW);
  digitalWrite(D3,(b&8)?HIGH:LOW);
  digitalWrite(D4,(b&16)?HIGH:LOW);
  digitalWrite(D5,(b&32)?HIGH:LOW);
  digitalWrite(D6,(b&64)?HIGH:LOW);
  digitalWrite(D7,(b&128)?HIGH:LOW);
  delay(5);
  digitalWrite(C3, LOW);
  delay(5);
  digitalWrite(C3, HIGH);
  digitalWrite(INH1, HIGH);
}


//-------------------------------------------------------------------------






//-------------------------------------------------------------------------
//  RAW FUNCTIONS
//-------------------------------------------------------------------------
void writeNoteRaw(byte note){
  String f="key/" + (String)wInstrument + "/" + (String)note + ".raw";
  //String f="key/" + library + (String)note + ".raw";
  const char *filename= f.c_str();
  double mixGain=keys[wChannel][note].velocity/255.0;
  
  claimNoteRaw(note);    //reserve resource
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


void claimNoteRaw(byte note){
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


void releaseNoteRaw(byte note){
  if(keys[wChannel][note].sdRaw==0){      //need to turn LED off
    resourceOVF(note);
  return;
  }
  
  if(keys[wChannel][note].sdRaw==1) fade1.fadeOut(decayTime);
  if(keys[wChannel][note].sdRaw==2) fade2.fadeOut(decayTime);
  if(keys[wChannel][note].sdRaw==3) fade3.fadeOut(decayTime);
  if(keys[wChannel][note].sdRaw==4) fade4.fadeOut(decayTime);

  //releaseNoteRaw 
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
  digitalWrite(INH0, HIGH);  //makes set=0
  seqRst();
}


void initialState(){
  //TI board
  int i;
  digitalWrite(C3, HIGH);
  digitalWrite(INH0, HIGH);
  digitalWrite(INH1, HIGH);
  digitalWrite(INH2, HIGH);
  digitalWrite(INH3, HIGH);
  for(i=0;i<numChips;i++){
  digitalWrite(i, HIGH);
  chipEnablePin=i;
  silenceAllChannels();
  }
}

void controlPannelSetup(){
  //playback
  decoder3(2); //clear playBack LED

 decoder3(8); //set playBack to middle

  //track
  decoder3(1); //clear track
  decoder3(4); //set track to 1  
}

void pinSetup(){
  pinMode(D0, OUTPUT); 
  pinMode(D1, OUTPUT); 
  pinMode(D2, OUTPUT); 
  pinMode(D3, OUTPUT); 
  pinMode(D4, OUTPUT); 
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT); 
  pinMode(D7, OUTPUT); 
  pinMode(rOVF, OUTPUT);
  pinMode(SEL0, OUTPUT); 
  pinMode(SEL1, OUTPUT); 
  pinMode(SEL2, OUTPUT); 
  pinMode(SEL3, OUTPUT); 
  pinMode(INH0, OUTPUT);
  pinMode(Y0, INPUT);   //mux output
  pinMode(Y1, INPUT);
  pinMode(Y2, INPUT);
  pinMode(tempo_sel0, OUTPUT);
  pinMode(tempo_sel1, OUTPUT);
  pinMode(INH1, OUTPUT);
  pinMode(INH2, OUTPUT);
  pinMode(INH3, OUTPUT);
  pinMode(R_LED_SIG, OUTPUT);
  pinMode(C0, OUTPUT);
  pinMode(C1, OUTPUT);
  pinMode(C2, OUTPUT);
  pinMode(C3, OUTPUT);
  pinMode(PP_LED_SIG, OUTPUT);
  pinMode(R_LED_SIG, OUTPUT);
  digitalWrite(INH1, HIGH);
  digitalWrite(INH3, HIGH);
  digitalWrite(INH2, HIGH);
 
}

//-------------------------------------------------------------------------

