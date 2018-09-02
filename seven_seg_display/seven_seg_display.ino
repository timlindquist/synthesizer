
char A=10;
char B=9;
char C=7;
char D=5;
char E=4;
char F=2;
char G=11;
char DP=6;

char latchA=3;
char latchB=8;
char latchC=12;
char latchD=13;
char latchStrobe=A0;



void setup() {
  Serial.begin(9600);
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(E, OUTPUT);
  pinMode(F, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(DP, OUTPUT);
  pinMode(latchA, OUTPUT);
  pinMode(latchB, OUTPUT);
  pinMode(latchC, OUTPUT);
  pinMode(latchD, OUTPUT);
  pinMode(latchStrobe, OUTPUT);
}

void loop() {
  int i;
  
  for(i=0;i<16;i++){
    writeWord(i);
    writeHex(i);
    delay(100);
  }
   
//writeWord(0);
}

void writeHex(byte b){
  digitalWrite(latchA,(b&1)?HIGH:LOW);
  digitalWrite(latchB,(b&2)?HIGH:LOW);
  digitalWrite(latchC,(b&4)?HIGH:LOW);
  digitalWrite(latchD,(b&8)?HIGH:LOW);
  digitalWrite(latchStrobe, LOW);
  delay(10);
  //digitalWrite(latchStrobe, HIGH);
}


void writeWord(byte b){
  byte high=b>>4;
  byte low=b&15;

  cathode(low);
  
  /*
   * 0: A,B,C,D,E,F
   * 1: B,C
   * 2: A,B,D,E,G
   * 3: A,B,C,D,G
   * 4: B,C,D,F,G
   * 5: A,C,D,F,G
   * 6: A,C,D,E,F,G
   * 7: A,B,C
   * 8: A,B,C,D,E,F,G
   * 9: A,B,C,D,F,G
   * A: A,B,C,E,F,G
   * B: C,D,E,F,G
   * C: A,D,E,F
   * D: B,C,D,E,G
   * E: A,D,E,F,G
   * F: A,E,F,G
   * 
   */
}

void cathode(byte b){
byte x;
switch (b) {
  case 0:
    x=B11111100;
    break;
  case 1:
    x=B01100000;
    break;
  case 2:
    x=B11011010;
    break;
  case 3:
    x=B11110010;
    break;
  case 4:
    x=B01100110;
    break;
  case 5:
    x=B10110110;
    break;
  case 6:
    x=B10111110;
    break;
  case 7:
    x=B11100000;
    break;
  case 8:
    x=B11111110;
    break;
  case 9:
    x=B11110110;
    break;
  case 10:
    x=B11101110;
    break;
  case 11:
    x=B00111110;
    break;
  case 12:
    x=B10011100;
    break;
  case 13:
    x=B01111010;
    break;
  case 14:
    x=B10011110;
    break;
  case 15:
    x=B10001110;
    break;
  default:
    x=B00000000;
    break;  
  }
  //Serial.println(x,BIN);

  digitalWrite(DP,(x&1)?HIGH:LOW);
  digitalWrite(G,(x&2)?HIGH:LOW);
  digitalWrite(F,(x&4)?HIGH:LOW);
  digitalWrite(E,(x&8)?HIGH:LOW);
  digitalWrite(D,(x&16)?HIGH:LOW);
  digitalWrite(C,(x&32)?HIGH:LOW);
  digitalWrite(B,(x&64)?HIGH:LOW);
  digitalWrite(A,(x&128)?HIGH:LOW);
  
}


