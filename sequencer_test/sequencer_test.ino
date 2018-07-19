#include <stdio.h>



void setup() {                
  Serial.begin(9600);
  pinMode(0, OUTPUT); 
  pinMode(1, OUTPUT); 
  pinMode(2, OUTPUT); 
  pinMode(3, OUTPUT); 
  pinMode(4, OUTPUT); //inhibit
  pinMode(5, OUTPUT); //reset 
  pinMode(10, INPUT);    
  //initial state
  digitalWrite(4, HIGH);  //makes set=0
  digitalWrite(5, LOW);   //make reset=0  
  rst();
}

// the loop routine runs over and over again forever:
void loop() {
 int x[16]={0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1};
 int y[16]={1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0};
 set_word(x);
 read_word();

 set_word(y);
 read_word();

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
  digitalWrite(4, LOW);  //makes set=1
  delay(1);
  digitalWrite(4, HIGH);  //makes set=0
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

boolean read_bit(byte b){
  int x;
  digitalWrite(0,(b&1)?HIGH:LOW);
  digitalWrite(1,(b&2)?HIGH:LOW);
  digitalWrite(2,(b&4)?HIGH:LOW);
  digitalWrite(3,(b&8)?HIGH:LOW);
  delay(4);
  return digitalRead(10);
}

void read_word(){
   int i;
   int x[16];

   for(i=0;i<16;i++){
      if(read_bit(i)){
        x[i]=1;
      }
      else{
        x[i]=0;
      }
      Serial.print(x[i]);
   }
  Serial.print("\n");
  
}

