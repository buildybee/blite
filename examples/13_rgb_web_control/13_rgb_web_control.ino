#include <blite.h>
#include "lighting.h"
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
    String html = HTML_LIGHT;
    myBot.smartRenderServer(html);
    webSocket.loop();
}