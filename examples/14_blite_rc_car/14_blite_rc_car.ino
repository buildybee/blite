#include <blite.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include "remote.h"

#define CMD_BACKWARD 2
#define CMD_STOP 0
#define CMD_FORWARD 1
#define CMD_LEFT 4
#define CMD_RIGHT 8
#define CMD_PUSH 5
#define CMD_SRVCLCK 6
#define CMD_SRVACLCK 9


Blite mybot;
ESP8266WebServer wifiRemoteControl(80);
WebSocketsServer webSocket = WebSocketsServer(81);  // WebSocket server on port 81
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
          mybot.stopMotor();
          break;
        case CMD_FORWARD:
          Serial.println("Move Forward");
          mybot.moveForward();
          break;
        case CMD_BACKWARD:
          Serial.println("Move Backward");
          mybot.moveBackward();
          break;
        case CMD_LEFT:
          Serial.println("Turn Left");
          mybot.turnLeft();
          break;
        case CMD_RIGHT:
          Serial.println("Turn Right");
          mybot.turnRight();
          break;
        case CMD_PUSH:
          Serial.println("Push button pressed");
          //mybot.push();
          break;
        case CMD_SRVCLCK:
          Serial.println("Turn servo clockwise");
          //mybot.turnServoClockwise();
          break;
        case CMD_SRVACLCK:
          Serial.println("Turn servo anti-clockwise");
          //mybot.turnServoAntiClockwise();
          break;
        default:
          Serial.println("Unknown command");
      }

      break;
  }
}


void setup(){
    mybot.setup();
    Serial.begin(115200);
    mybot.smartConnectWiFi();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    wifiRemoteControl.on("/", HTTP_GET, []() {
      Serial.println("Web Server: received a web page request");
       String html = REMOTE_HTML_CONTENT;
       wifiRemoteControl.send(200, "text/html", html);
   });
   wifiRemoteControl.begin();

}
void loop(){
    wifiRemoteControl.handleClient();
    webSocket.loop();

}
