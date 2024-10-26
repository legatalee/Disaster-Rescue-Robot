#include <WiFi.h>
#include <Dynamixel2Arduino.h>
#include <NetworkUdp.h>

const uint16_t port = 3000;
const char* host = "192.168.4.1";

const char* ssid = "RescueRobot";
const char* password = "newssusw";

// NetworkClient client;
NetworkUDP udp;

#define DXL_SERIAL Serial2
#define DXL_BAUDRATE 1000000
#define NUM_OF_DXL 8

const uint8_t DXL_DIR_PIN = 33;
const float DXL_PROTOCOL_VERSION = 1.0;

using namespace ControlTableItem;

class NewSerialPortHandler : public DYNAMIXEL::SerialPortHandler {
public:
  NewSerialPortHandler(HardwareSerial& port, const int dir_pin = -1)
    : SerialPortHandler(port, dir_pin), port_(port), dir_pin_(dir_pin) {}
  virtual size_t write(uint8_t c) override {
    size_t ret = 0;
    digitalWrite(dir_pin_, LOW);
    while (digitalRead(dir_pin_) != LOW)
      ;
    ret = port_.write(c);
    port_.flush();
    digitalWrite(dir_pin_, HIGH);
    while (digitalRead(dir_pin_) != HIGH)
      ;
    return ret;
  }
  virtual size_t write(uint8_t* buf, size_t len) override {
    size_t ret;
    digitalWrite(dir_pin_, LOW);
    while (digitalRead(dir_pin_) != LOW)
      ;
    ret = port_.write(buf, len);
    port_.flush();
    digitalWrite(dir_pin_, HIGH);
    while (digitalRead(dir_pin_) != HIGH)
      ;
    return ret;
  }
private:
  HardwareSerial& port_;
  const int dir_pin_;
};

Dynamixel2Arduino dxl;
NewSerialPortHandler dxl_port(DXL_SERIAL, DXL_DIR_PIN);

void setTorqueOff(uint8_t dxl_id) {
  if (!dxl.ping(dxl_id)) {
    Serial.print(dxl_id);
    Serial.println(" not connected");
    return;
  }
  dxl.setOperatingMode(dxl_id, OP_POSITION);
  dxl.writeControlTableItem(PROFILE_VELOCITY, dxl_id, 30);
  dxl.torqueOff(dxl_id);

  dxl.writeControlTableItem(GOAL_TORQUE, dxl_id, 0);
}


void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  dxl.setPort(dxl_port);
  dxl.begin(DXL_BAUDRATE);

  DXL_SERIAL.end();
  DXL_SERIAL.begin(DXL_BAUDRATE, SERIAL_8N1, 25, 26);

  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

  for (int i = 0; i < 10; i++) {
    setTorqueOff(i);
  }

  Serial.println();
  Serial.print("Waiting for Robot... ");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected to Robot AP.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  delay(500);
}

int id[NUM_OF_DXL] = { 1, 2, 3, 4, 5, 6, 7, 8 };
int prevPosition[NUM_OF_DXL] = { 512, 512, 512, 512, 512, 512, 512, 512 };
int newPosition[NUM_OF_DXL] = { 512, 512, 512, 512, 512, 512, 512, 512 };

unsigned long previousMillis = 0;

void loop() {
  // while (!client.connected()) {
  //   Serial.println("Connecting to Robot...");
  //   client.connect(host, port);
  //   if (client.connected()) {
  //     Serial.println("Robot Connected.");
  //   }
  //   delay(1000);
  // }
  // while (client.connected()) {
  newPosition[0] = dxl.getPresentPosition(1);
  // newPosition[1] = dxl.getPresentPosition(2);
  newPosition[2] = dxl.getPresentPosition(3);
  if (newPosition[2] > 513)
    newPosition[2] = 513;
  // newPosition[3] = dxl.getPresentPosition(4);
  newPosition[4] = dxl.getPresentPosition(5);
  newPosition[4] = map(newPosition[4], 1023, 0, 0, 1023);
  // newPosition[5] = dxl.getPresentPosition(6);
  newPosition[6] = dxl.getPresentPosition(7);
  newPosition[6] = map(newPosition[6], 1023, 0, 0, 2048);
  newPosition[6] -= 850;
  if (newPosition[6] < 200)
    newPosition[6] = 200;
  // newPosition[7] = dxl.getPresentPosition(8);

  String data = "";
  int i = 0;
  while (1) {
    if (((prevPosition[i] + 100) > newPosition[i]) && ((prevPosition[i] - 100) < newPosition[i])) {
      data += id[i];
      data += ':';
      data += newPosition[i];
      prevPosition[i] = newPosition[i];
    } else {
      data += id[i];
      data += ':';
      data += prevPosition[i];
    }
    // int delta = abs(newPosition[i] - prevPosition[i]);
    // if (delta <= 20) {
    //   data += newPosition[i];
    //   prevPosition[i] = newPosition[i];
    // } else {
    //   delay(10);
    //   newPosition[i] = dxl.getPresentPosition(7);
    //   if (abs(newPosition[i] - prevPosition[i]) <= 20) {
    //     data += newPosition[i];
    //     prevPosition[i] = newPosition[i];
    //   } else data += prevPosition[i];
    // }
    i++;
    if (i == NUM_OF_DXL) {
      data += '$';
      break;
    }
    data += ',';
  }
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 20) {  //20에서 낫뱃(뚝뚝거리긴함), 50에서 무난, 10이하는 통신문제, 5로 해보기, 17무난
    Serial.println(data);
    udp.beginPacket(host, port);
    udp.print(data);
    udp.endPacket();
    previousMillis = currentMillis;
  }
}