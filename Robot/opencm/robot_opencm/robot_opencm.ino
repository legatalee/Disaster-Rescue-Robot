#include <string.h>
Dynamixel Dxl(1);

void setup() {
  Serial3.begin(115200);

  pinMode(BOARD_BUTTON_PIN, INPUT_PULLDOWN);
  //  pinMode(16, INPUT_PULLUP);
  //  pinMode(17, INPUT_PULLUP);
  //  pinMode(18, INPUT_PULLUP);
  //  pinMode(19, INPUT_PULLUP);
  //  pinMode(20, INPUT_PULLUP);
  //  pinMode(21, INPUT_PULLUP);
  //  pinMode(22, INPUT_PULLUP);

  Dxl.begin(3);

  for (int c = 1; c <= 9; c++) {
    Dxl.jointMode(c);
  }
  Dxl.wheelMode(11);
  Dxl.wheelMode(12);
  Dxl.wheelMode(13);
  Dxl.wheelMode(14);
  
  //  Dxl.jointMode(19);
  //  Dxl.jointMode(20);
  //  Dxl.jointMode(21);
  //  Dxl.jointMode(22);

  Dxl.writeWord(1, 30, 512);
  Dxl.writeWord(2, 30, 512);
  Dxl.writeWord(3, 30, 512);
  Dxl.writeWord(4, 30, 512);
  Dxl.writeWord(5, 30, 512);
  Dxl.writeWord(6, 30, 512);
  Dxl.writeWord(7, 30, 512);
  Dxl.writeWord(8, 30, 512);
  Dxl.writeWord(9, 30, 512);

  delay(100);

  Dxl.writeWord(1, 30, 512);
  Dxl.writeWord(2, 30, 512);
  Dxl.writeWord(3, 30, 512);
  Dxl.writeWord(4, 30, 512);
  Dxl.writeWord(5, 30, 512);
  Dxl.writeWord(6, 30, 512);
  Dxl.writeWord(7, 30, 512);
  Dxl.writeWord(8, 30, 512);
  Dxl.writeWord(9, 30, 512);
}


boolean b1, b2, b3, b4, b5, b6, b7;

int spda = 523;
int spdb = 1546;

char data[100];
int prevPosition[20] = {
  512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512
};

int iterator_input = 0;
char input[128] = {
    0,
};
int check = 0;

void loop() {
  while (Serial3.available()) {
    char ch = Serial3.read();
    if (ch == '$') {
        SerialUSB.println(input);
        check = 1;
        input[iterator_input] = 0;
        break;
    }
    input[iterator_input] = ch;
    iterator_input++;
  }

  if (check == 1) {
    char* command = strtok(input, ",");
    int iterator = 0;
    while (command != 0) {
      char* separator = strchr(command, ':');
      if (separator != 0) {
        *separator = 0;
        int servoId = atoi(command);
        ++separator;
        int position = atoi(separator);

        if (servoId==100) {
          if(position==0) stop();
          else if(position==1) forward();
          else if(position==2) backward();
          else if(position==3) left();
          else if(position==4) right();
          else if(position==5) turnl();
          else if(position==6) turnr();
          else if(position==7) SerialUSB.println("gripper!");
        }
        if (servoId==200) {
          if(position==0) Dxl.writeWord(9, 30, 550);
          else if(position==1) Dxl.writeWord(9, 30, 300);
        }
        else if (position > 0 && position < 1023) {
          Dxl.writeWord(servoId, 30, position);
          prevPosition[servoId] = position;
        }

        iterator++;
      }
      command = strtok(0, ",");
    }

    SerialUSB.println("");
    check = 0;
    iterator_input = 0;
  }
}

void stop() { //0
  Dxl.writeWord(11, 32, 0);
  Dxl.writeWord(12, 32, 0);
  Dxl.writeWord(13, 32, 0);
  Dxl.writeWord(14, 32, 0);
  return;
}
void forward() { //1
  Dxl.writeWord(11, 32, spda);
  Dxl.writeWord(12, 32, spda);
  Dxl.writeWord(13, 32, spdb);
  Dxl.writeWord(14, 32, spdb);
  delay(1);
  return;
}
void backward() { //2
  Dxl.writeWord(11, 32, spdb);
  Dxl.writeWord(12, 32, spdb);
  Dxl.writeWord(13, 32, spda);
  Dxl.writeWord(14, 32, spda);
  delay(1);
  return;
}
void left() { //3
  Dxl.writeWord(11, 32, spda);
  Dxl.writeWord(12, 32, spdb);
  Dxl.writeWord(13, 32, spda);
  Dxl.writeWord(14, 32, spdb);
  delay(1);
  return;
}
void right() { //4
  Dxl.writeWord(11, 32, spdb);
  Dxl.writeWord(12, 32, spda);
  Dxl.writeWord(13, 32, spdb);
  Dxl.writeWord(14, 32, spda);
  delay(1);
  return;
}
void turnl() { //5
  Dxl.writeWord(11, 32, spdb);
  Dxl.writeWord(12, 32, spdb);
  Dxl.writeWord(13, 32, spdb);
  Dxl.writeWord(14, 32, spdb);
  delay(1);
  return;
}
void turnr() { //6
  Dxl.writeWord(11, 32, spda);
  Dxl.writeWord(12, 32, spda);
  Dxl.writeWord(13, 32, spda);
  Dxl.writeWord(14, 32, spda);
  delay(1);
  return;
}
void leftup() { //7
  Dxl.writeWord(11, 32, 0);
  Dxl.writeWord(12, 32, spdb);
  Dxl.writeWord(13, 32, spda);
  Dxl.writeWord(14, 32, 0);
  delay(1);
  return;
}
void rightup() { //8
  Dxl.writeWord(11, 32, spda);
  Dxl.writeWord(12, 32, 0);
  Dxl.writeWord(13, 32, 0);
  Dxl.writeWord(14, 32, spdb);
  delay(1);
  return;
}
void leftdown() { //9
  Dxl.writeWord(11, 32, spdb);
  Dxl.writeWord(12, 32, 0);
  Dxl.writeWord(13, 32, 0);
  Dxl.writeWord(14, 32, spda);
  delay(1);
  return;
}
void rightdown() { //10
  Dxl.writeWord(11, 32, 0);
  Dxl.writeWord(12, 32, spda);
  Dxl.writeWord(13, 32, spdb);
  Dxl.writeWord(14, 32, 0);
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
