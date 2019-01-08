#include <string.h>
#include <stdlib.h>
#include <stdio.h>

Dynamixel Dxl(1);

int ldata[11];
int data[11];

void setup() {
  Serial3.begin(9600);

  pinMode(BOARD_BUTTON_PIN, INPUT_PULLDOWN);
  pinMode(16, INPUT_PULLUP);
  pinMode(17, INPUT_PULLUP);
  pinMode(18, INPUT_PULLUP);

  Dxl.begin(3);

  for (int c = 1; c <= 9; c++) {
    Dxl.jointMode(c);
  }

  Dxl.wheelMode(10);
  Dxl.wheelMode(11);
  Dxl.wheelMode(12);
  Dxl.wheelMode(13);

  pinMode(BOARD_LED_PIN, OUTPUT);
  
  for (int a = 0; a < 11; a++) {
      ldata[a]=512;
    }
}

int spda = 520;
int spdb = 1803;

boolean grip = false;

char input[100];


int for3;

void loop() {
  for (int a = 0; a <= 100; a++) {
    input[a] = '\0';
  }

  int c = 0;
  while (Serial3.available()) {
    input[c] = (char) Serial3.read();
    delay(2);
    c++;
  }
  while (SerialUSB.available()) {
    Serial3.write(SerialUSB.read());
  }
  if (c != 0) {
    split();
    if (data[2] < 200) {
      data[2] = 200;
    }
    for (int a = 0; a < 11; a++) {
      SerialUSB.print("data[");
      SerialUSB.print(a);
      SerialUSB.print("] : ");
      SerialUSB.print(data[a]);
      SerialUSB.println(" ");
    }
    for3 = data[0];
  }

  if (data[10] == 0) {
    Dxl.writeWord(8, 30, 300);
  }
  if (data[10] == 1) {
    Dxl.writeWord(8, 30, 550);
  }


//  if (for3 < ldata[0]) {
//    for (int a = ldata[0]; a > for3; a--) {
//      Dxl.writeWord(3, 30, a);
//    }
//  } 
//  else if (ldata[0] < for3) {
//    for (int a = ldata[0]; a < for3; a++) {
//      Dxl.writeWord(3, 30, a);
//    }
//  }
//  else if(ldata[0]==for3){
//    Dxl.writeWord(3, 30, for3);
//  }
//  
//  if (data[1] < ldata[1]) {
//    for (int a = ldata[1]; a > data[1]; a--) {
//      Dxl.writeWord(1, 30, a);
//    }
//  } 
//  else if (ldata[1] < data[1]) {
//    for (int a = ldata[1]; a < data[1]; a++) {
//      Dxl.writeWord(1, 30, a);
//    }
//  } 
//  else if(ldata[1]==data[1]){
//    Dxl.writeWord(1, 30, data[1]);
//  }
//
//  if (data[2] < ldata[2]) {
//    for (int a = ldata[2]; a > data[2]; a--) {
//      Dxl.writeWord(7, 30, a);
//    }
//  } 
//  else if (ldata[3] < data[3]) {
//    for (int a = ldata[3]; a < data[3]; a++) {
//      Dxl.writeWord(7, 30, a);
//    }
//  }
//  else if(ldata[3]==data[3]){
//    Dxl.writeWord(7, 30, data[3]);
//  }
  
//  if (data[3] < ldata[3]) {
//    for (int a = ldata[3]; a > data[3]; a--) {
//      Dxl.writeWord(5, 30, a);
//    }
//  } 
//  else if (ldata[3] < data[3]) {
//    for (int a = ldata[3]; a < data[3] ; a++) {
//      Dxl.writeWord(5, 30, a);
//    }
//  }
//  else if(ldata[3]==data[3]){
//    Dxl.writeWord(5, 30, data[4]);
//  }

//  ldata[0]=for3;
//  ldata[1]=data[1];
//  ldata[2]=data[2];
//  ldata[3]=data[3];








  Dxl.writeWord(3, 30, for3);
  Dxl.writeWord(1, 30, data[1]);
  
//  Dxl.writeWord(5,30,data[3]);
  Dxl.writeWord(7, 30, data[2]);

  if (data[4] == 0 && data[5] == 0)
  {
  }
  else if (data[4] == 0) {
    forward();
  }
  else posreset();
  if (data[5] == 0)
  {
    backward();
  }
  else posreset();
  if (data[6] == 0)
  {
    left();
  }
  else posreset();
  if (data[7] == 0)
  {
    right();
  }
  else posreset();
  if (data[8] == 0)
  {
    turnl();
  }
  else posreset();
  if (data[9] == 0)
  {
    turnr();
  }
  else posreset();
}

void split() {
  char *p;
  char* a = NULL;
  p = strtok(input, ",");
  if (p) {
    data[0] = strtol(p, &a, 10);
  }
  p = strtok(NULL, ",");
  if (p) {
    data[1] = strtol(p, &a, 10);
  }
  p = strtok(NULL, ",");
  if (p)
    data[2] = strtol(p, &a, 10);
  p = strtok(NULL, ",");
  if (p)
    data[3] = strtol(p, &a, 10);
  p = strtok(NULL, ",");
  if (p)
    data[4] = strtol(p, &a, 10);
  p = strtok(NULL, ",");
  if (p)
    data[5] = strtol(p, &a, 10);
  p = strtok(NULL, ",");
  if (p)
    data[6] = strtol(p, &a, 10);
  p = strtok(NULL, ",");
  if (p)
    data[7] = strtol(p, &a, 10);
  p = strtok(NULL, ",");
  if (p)
    data[8] = strtol(p, &a, 10);
  p = strtok(NULL, ",");
  if (p)
    data[9] = strtol(p, &a, 10);
  p = strtok(NULL, ",");
  if (p) {
    data[10] = strtol(p, &a, 10);
  }
}

void forward() {
  Dxl.writeWord(10, 32, spda);
  Dxl.writeWord(11, 32, spdb);
  Dxl.writeWord(12, 32, spda);
  Dxl.writeWord(13, 32, spdb);
  delay(1);
  return;
}
void backward() {
  Dxl.writeWord(10, 32, spdb);
  Dxl.writeWord(11, 32, spda);
  Dxl.writeWord(12, 32, spdb);
  Dxl.writeWord(13, 32, spda);
  delay(1);
  return;
}
void turnl() {
  Dxl.writeWord(10, 32, spdb);
  Dxl.writeWord(11, 32, spdb);
  Dxl.writeWord(12, 32, spdb);
  Dxl.writeWord(13, 32, spdb);
  delay(1);
  return;
}
void turnr() {
  Dxl.writeWord(10, 32, spda);
  Dxl.writeWord(11, 32, spda);
  Dxl.writeWord(12, 32, spda);
  Dxl.writeWord(13, 32, spda);
  delay(1);
  return;
}
void left() {
  Dxl.writeWord(10, 32, spdb);
  Dxl.writeWord(11, 32, spdb);
  Dxl.writeWord(12, 32, spda);
  Dxl.writeWord(13, 32, spda);
  delay(1);
  return;
}
void right() {
  Dxl.writeWord(10, 32, spda);
  Dxl.writeWord(11, 32, spda);
  Dxl.writeWord(12, 32, spdb);
  Dxl.writeWord(13, 32, spdb);
  delay(1);
  return;
}
void leftup() {
  Dxl.writeWord(10, 32, 0);
  Dxl.writeWord(11, 32, spdb);
  Dxl.writeWord(12, 32, spda);
  Dxl.writeWord(13, 32, 0);
  delay(1);
  return;
}
void leftdown() {
  Dxl.writeWord(10, 32, spdb);
  Dxl.writeWord(11, 32, 0);
  Dxl.writeWord(12, 32, 0);
  Dxl.writeWord(13, 32, spda);
  delay(1);
  return;
}
void rightup() {
  Dxl.writeWord(10, 32, spda);
  Dxl.writeWord(11, 32, 0);
  Dxl.writeWord(12, 32, 0);
  Dxl.writeWord(13, 32, spdb);
  delay(1);
  return;
}
void rightdown() {
  Dxl.writeWord(10, 32, 0);
  Dxl.writeWord(11, 32, spda);
  Dxl.writeWord(12, 32, spdb);
  Dxl.writeWord(13, 32, 0);
  delay(1);
  return;
}
void slowmode() {
  spda = 300;
  spdb = 1323;
  return;
}
void fastmode() {
  spda = 650;
  spdb = 1673;
  return;
}
void posreset() {
  Dxl.goalSpeed(10, 0);
  Dxl.goalSpeed(11, 0);
  Dxl.goalSpeed(12, 0);
  Dxl.goalSpeed(13, 0);
  return;
}




