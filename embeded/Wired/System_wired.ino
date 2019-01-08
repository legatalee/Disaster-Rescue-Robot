Dynamixel Dxl(1);

void setup() {
  Serial3.begin(9600);

  pinMode(BOARD_BUTTON_PIN, INPUT_PULLDOWN);
  pinMode(16, INPUT_PULLUP);
  pinMode(17, INPUT_PULLUP);
  pinMode(18, INPUT_PULLUP);
  pinMode(19, INPUT_PULLUP);
  pinMode(20, INPUT_PULLUP);
  pinMode(21, INPUT_PULLUP);
  pinMode(22, INPUT_PULLUP);

  Dxl.begin(3);

  for (int c = 1; c <= 9; c++) {
    Dxl.jointMode(c);
  }
  Dxl.wheelMode(10);
  Dxl.wheelMode(11);
  Dxl.wheelMode(12);
  Dxl.wheelMode(13);

  Dxl.jointMode(19);
  Dxl.jointMode(20);
  Dxl.jointMode(21);
  Dxl.jointMode(22);

  Dxl.writeWord(1, 30, 512);
  Dxl.writeWord(2, 30, 512);
  Dxl.writeWord(3, 30, 512);
  Dxl.writeWord(4, 30, 512);
  Dxl.writeWord(5, 30, 512);
  Dxl.writeWord(6, 30, 512);
}

int current1, current2, current3, current4;
boolean b1, b2, b3, b4, b5, b6, b7;

int spda = 523;
int spdb = 1546;

char data[100];

void loop() {
  current1 = Dxl.readWord(19, 36);
  if(current1>513)
    current1=513;
  
  current2 = Dxl.readWord(20, 36);

  current3 = Dxl.readWord(21, 36);
  current3 = map(current3, 1023, 0, 0, 2048);
  current3 -= 850;
  if(current3<200)
    current3=200;

  current4 = Dxl.readWord(22, 36);
  current4 = map(current4, 1023, 0, 0, 1023);

  Dxl.writeWord(3, 30, current1);
  Dxl.writeWord(1, 30, current2);
  Dxl.writeWord(7, 30, current3);
  Dxl.writeWord(5, 30, current4);

  SerialUSB.println(Dxl.readWord(7, 36));

  Dxl.writeWord(4, 30, 300);
  Dxl.writeWord(2, 30, 812);
  Dxl.writeWord(6, 30, 600);

  b1 = digitalRead(22);
  b2 = digitalRead(17);
  b3 = digitalRead(21);
  b4 = digitalRead(19);
  b5 = digitalRead(16);
  b6 = digitalRead(18);
  b7 = digitalRead(20);

  if (b7 == 0) {
    Dxl.writeWord(8, 30, 300);
  }
  if (b7 == 1) {
    Dxl.writeWord(8, 30, 550);
  }

  if (b1 == 0) {
    forward();
  }
  else if (b2 == 0)
  {
    backward();
  }
  else if (b3 == 0)
  {
    left();
  }
  else if (b4 == 0)
  {
    right();
  }
  else if (b5 == 0)
  {
    turnl();
  }
  else if (b6 == 0)
  {
    turnr();
  }
  else posreset();
}


void left() {
  Dxl.writeWord(10, 32, spda);
  Dxl.writeWord(11, 32, spdb);
  Dxl.writeWord(12, 32, spda);
  Dxl.writeWord(13, 32, spdb);
  delay(1);
  return;
}
void right() {
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
void backward() {
  Dxl.writeWord(10, 32, spdb);
  Dxl.writeWord(11, 32, spdb);
  Dxl.writeWord(12, 32, spda);
  Dxl.writeWord(13, 32, spda);
  delay(1);
  return;
}
void forward() {
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




