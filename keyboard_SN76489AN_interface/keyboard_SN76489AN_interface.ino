/** Author: Tim Lindquist
 *  Rev: 1.2
 *
 *  NOTES: writeNOTE and releaseNOTE handle edge cases
 */

#include <USBHost_t36.h>
#include <math.h>

#define CLK 3579545

//SN76489 pin assignment
#define D0 0
#define D1 1
#define D2 2
#define D3 3
#define D4 4
#define D5 5
#define D6 6
#define D7 7
#define WE 9
#define CE 10	//first CE pin
#define rOVF 8	//resource overflow LED


#define numNotes 120	  //enough to cover MIDI range
#define numChips 1       //number of physical chips in a track
#define numChipChannels 3  //number of channels on on a chip
#define numChannels 1   //number of channels in a single track
#define numTracks 1     //this will use seperate chip hardware

USBHost teensySerial;
USBHub hub1(teensySerial);
USBHub hub2(teensySerial);
MIDIDevice midiDev(teensySerial);

char chipEnablePin=CE;	  //working chip
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
  teensySerial.begin();
  midiDev.setHandleNoteOn(keyboardNoteOn);
  midiDev.setHandleNoteOff(keyboardNoteOff);
  digitalWrite(CE, HIGH);
  digitalWrite(WE, HIGH);
  pinMode(D0, OUTPUT); 
  pinMode(D1, OUTPUT); 
  pinMode(D2, OUTPUT); 
  pinMode(D3, OUTPUT); 
  pinMode(D4, OUTPUT); 
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT); 
  pinMode(D7, OUTPUT);  
  pinMode(WE, OUTPUT); 
  pinMode(CE, OUTPUT);
  initialState();  
}



void loop() {
  teensySerial.Task();
  midiDev.read();
  buttonsRead();
 
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
  double freq;
  int n;
  String bin="";
  int i;
  int weight[10]={512,256,128,64,32,16,8,4,2,1};
  byte data;
  claimNote(note);  	//reserve resource
  if(keys[wTrack][wChannel][note].chipChannel==0){
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
  if(keys[wTrack][wChannel][note].chipChannel==1) data=128;
  if(keys[wTrack][wChannel][note].chipChannel==2) data=160;
  if(keys[wTrack][wChannel][note].chipChannel==3) data=192; 
  for(i=4;i<8;i++){
    if(bin.charAt(i+2)=='1'){
      data+=weight[i+2];
    }
  }
  //update CE for correct chip select 
  chipEnablePin=CE+keys[wTrack][wChannel][note].chip-1;
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
  if(keys[wTrack][wChannel][note].chipChannel==0){
    resourceOVF(note);
	return;
  }
  //update CE for correct chip select
  chipEnablePin=CE+keys[wTrack][wChannel][note].chip-1;
  volumeAdjust(note);
  //releaseNote
  keys[wTrack][wChannel][note].chipChannel=0;
  keys[wTrack][wChannel][note].chip=0;
}

void claimNote(byte note){
  int i,j,k;
  int channel;
  for(i=1;i<=numChips;i++){
    for(j=1;j<=numChipChannels;j++){
      for(k=0;k<numNotes;k++){
        if((keys[wTrack][wChannel][k].chip==i)&&(keys[wTrack][wChannel][k].chipChannel==j)){
          channel=0;
          break;
        }
        else{
          channel=j;
        }
      }
      if(channel!=0){
        keys[wTrack][wChannel][note].chipChannel=j;
        keys[wTrack][wChannel][note].chip=i;
        return;
      }
    }
  }
  //keys[wTrack][wChannel][note].chipChannel=0;
  //keys[wTrack][wChannel][note].chip=0;
  
}


void volumeAdjust(byte note){
  byte data;
  char att;	//attenuation
  //if(keys[wTrack][wChannel][note].chipChannel==0) return;
  
  if(keys[wTrack][wChannel][note].on){  //note on: construct data based on channel
    if(keys[wTrack][wChannel][note].chipChannel==1) data=144;
    if(keys[wTrack][wChannel][note].chipChannel==2) data=176;
    if(keys[wTrack][wChannel][note].chipChannel==3) data=208;
  
    /**construct data based off velocity (attenuation: 0db=+0, 2db=+1, 4db=+2, 8db=+4, 16db=+8)
     * map 0 to 128 to 0 to 5, 0-1=16db, 1-2=8db, 2-3=4db, 3-4=2db, 4-5=0db
     * after floor 0=16db, 1=8db, 2=4db, 3=2db, 4=0db 
     * since a velocity range is 0-127 mapping to 128 will secure a value less than 5 to always occure
     */
    att=floor(map(keys[wTrack][wChannel][note].velocity,0,128,0,5));
    if(att!=4) data+=pow(2,3-att);
  }
  else{ //note off: construct data based off channel
    if(keys[wTrack][wChannel][note].chipChannel==1) data=159;
    if(keys[wTrack][wChannel][note].chipChannel==2) data=191;
    if(keys[wTrack][wChannel][note].chipChannel==3) data=223;
  }
  sendByte(data);
}



void initialState(){
  int i;
  silenceAllChannels();
  for(i=0;i<numChips;i++){
	digitalWrite(CE+i, HIGH);
  }
}

void silenceAllChannels(){
  sendByte(0x9f);
  sendByte(0xbf);
  sendByte(0xdf);
  sendByte(0xff);
}

void resourceOVF(byte note){
	digitalWrite(rOVF,(keys[wTrack][wChannel][note].on)?HIGH:LOW);
}



void sendByte(byte b) {
  digitalWrite(chipEnablePin, LOW);
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
  digitalWrite(chipEnablePin, HIGH);
}



void buttonsRead(){
	//if(digitalRead(D15)) cycleTrack();
	
	
	
}


