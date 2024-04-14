//connect the blite dev board with b-rover: https://buildybee.github.io/beebotFullKit.html
#include <blite.h>
#include "remote.h"
#include <WebSocketsServer.h>

#define CMD_STOP 0
#define CMD_FORWARD 1
#define CMD_BACKWARD 2
#define CMD_LEFT 4
#define CMD_RIGHT 8

Blite myBot;
WebSocketsServer webSocket = WebSocketsServer(81);

void setup(){
    myBot.setup();
    myBot.reversePolarityM12();
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
        default:
          Serial.println("Unknown command");
      }

      break;
  }
}