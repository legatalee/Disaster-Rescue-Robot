#include <WiFi.h>
#include <NetworkClient.h>
#include <WiFiAP.h>
// #include <AsyncUDP.h>
#include <WiFiUdp.h>
#include <HardwareSerial.h>
HardwareSerial robot(2);

const char* ssid = "RescueRobot";
const char* password = "newssusw";

WiFiUDP udp;
const int udpPort = 3000;

char incomingPacket[255];

void setup() {
  Serial.begin(115200);
  robot.begin(115200, SERIAL_8N1, 5, 4);

  Serial.println("Configuring access point...");

  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  udp.begin(udpPort);

  Serial.println("Server started");
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;
    }
    Serial.println(incomingPacket);
    robot.print(incomingPacket);
  }
}
