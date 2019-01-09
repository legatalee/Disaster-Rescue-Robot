#include <stdio.h>

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

  Dxl.jointMode(19);
  Dxl.jointMode(20);
  Dxl.jointMode(21);
  Dxl.jointMode(22);
}

int current1, current2, current3, current4;
boolean b1, b2, b3, b4, b5, b6, b7;

char data[100];

void loop() {
  for(int a=0;a<=100;a++){
    data[a]='\0';
  }

  current1 = Dxl.readWord(19, 36);
  current1=map(current1, 1023, 0, 0, 1023);

  current2 = Dxl.readWord(20, 36);

  current3 = Dxl.readWord(21, 36);
  current3 = map(current3, 1023, 0, 0, 2048);
  current3 -= 850;
  
  current4 = Dxl.readWord(20, 36);

  b1 = digitalRead(22);
  b2 = digitalRead(17);
  b3 = digitalRead(21);
  b4 = digitalRead(19);
  b5 = digitalRead(16);
  b6 = digitalRead(18);
  b7 = digitalRead(20);

  sprintf(data,"%ld,%ld,%ld,%ld,%d,%d,%d,%d,%d,%d,%d", current1, current2, current3, current4, b1, b2, b3, b4, b5, b6, b7);

  for(int c=0;c<=100;c++) {
    SerialUSB.print(data[c]);
    Serial3.write(data[c]);
  }
  SerialUSB.println();

  while (Serial3.available()) {
    SerialUSB.write(Serial3.read());
  }
  delay(300);//250 yes 220no
}
