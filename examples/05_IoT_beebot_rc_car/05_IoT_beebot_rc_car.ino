//connect the blite dev board with b-rover: https://buildybee.github.io/beebotFullKit.html
#include <blite.h>
#include "remote.h"
#include <WebSocketsServer.h>

#define CMD_STOP 0
#define CMD_FORWARD 1
#define CMD_BACKWARD 2
#define CMD_LEFT 4
#define CMD_RIGHT 8
#define CMD_LIGHT 5
#define CMD_REVM12 6
#define CMD_REVM34 9
#define CMD_SPD_1 10
#define CMD_SPD_2 11
#define CMD_SPD_3 12

Blite myBot;
WebSocketsServer webSocket = WebSocketsServer(81);
int led = myBot.getIO("io1");

void setup(){
    myBot.setup();
    pinMode(led,OUTPUT);
    digitalWrite(led, HIGH);
    Serial.begin(115200);
    delay(1000);
    if (myBot.buttonPressed()){
      myBot.APServer();
    } else {
      myBot.smartConnectWiFi();
    }
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

}
void loop(){
    String html = REMOTE_HTML_CONTENT;
    myBot.smartRenderServer(html);
    webSocket.loop();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
      }
      break;
    case WStype_TEXT:
      //Serial.printf("[%u] Received text: %s\n", num, payload);
      String angle = String((char*)payload);
      int command = angle.toInt();
      Serial.print("command: ");
      Serial.println(command);

      switch (command) {
        case CMD_STOP:
          Serial.println("Stop");
          myBot.stopMotor();
          break;
        case CMD_FORWARD:
          Serial.println("Move Forward");
           myBot.moveForward();
          break;
        case CMD_BACKWARD:
          Serial.println("Move Backward");
           myBot.moveBackward();
          break;
        case CMD_LEFT:
          Serial.println("Turn Left");
           myBot.turnLeft();
          break;
        case CMD_RIGHT:
          Serial.println("Turn Right");
           myBot.turnRight();
          break;
        case CMD_LIGHT:
          Serial.println("Changing LIGHT status");
          digitalWrite(led, !digitalRead(led));
          break;
        case CMD_REVM12:
          Serial.println("Reverse polarity for M12");
          myBot.reversePolarityM12();
          break;
        case CMD_REVM34:
          Serial.println("Reverse polarity for M34");
          myBot.reversePolarityM34();
          break;
        case CMD_SPD_1:
          Serial.println("Speed set to 80");
           myBot.setSpeed(80);
          break;
        case CMD_SPD_2:
          Serial.println("Speed set to 160");
           myBot.setSpeed(160);
          break;
        case CMD_SPD_3:
          Serial.println("Speed set to 255");
           myBot.setSpeed(255);
          break;
        default:
          Serial.println("Unknown command");
      }

      break;
  }
}