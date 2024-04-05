#include <blite.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include "lighting.h"
#include <Adafruit_NeoPixel.h>
// Neopixel Config
#define NeoPIN 2
#define NUM_LEDS 16

int brightness = 150;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, NeoPIN, NEO_RGB + NEO_KHZ800);
Blite mybot;
ESP8266WebServer wifiRemoteControl(80);
WebSocketsServer webSocket = WebSocketsServer(81);  // WebSocket server on port 81
const int led = 13;
 
void setup(){
    mybot.setup();
    Serial.begin(115200);
    mybot.smartConnectWiFi();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    wifiRemoteControl.on("/", HTTP_GET, []() {
      Serial.println("Web Server: received a web page request");
       String html = HTML_LIGHT;
       wifiRemoteControl.send(200, "text/html", html);
   });
   wifiRemoteControl.begin();

}
 
void loop(){
    wifiRemoteControl.handleClient();
    webSocket.loop();

}
 
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    String command = String((char*)payload);
    Serial.print("command: ");
    Serial.println(command);
    // setting the color to the strip 
    setNeoColor(command);
}
 
void setNeoColor(String value){
  Serial.print("Setting Neopixel...");
  // converting Hex to Int
  int number = (int) strtol( &value[1], NULL, 16); 
  // splitting into three parts
  int r = number >> 16;
  int g = number >> 8 & 0xFF;
  int b = number & 0xFF;
  
  // DEBUG
  Serial.print("RGB: ");
  Serial.print(r, DEC);
  Serial.print(" ");
  Serial.print(g, DEC);
  Serial.print(" ");
  Serial.print(b, DEC);
  Serial.println(" ");
  
  // setting whole strip to the given color
  for(int i=0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color( g, r, b ) );
  }
  // init
  strip.show();
  
  Serial.println("on.");
}
