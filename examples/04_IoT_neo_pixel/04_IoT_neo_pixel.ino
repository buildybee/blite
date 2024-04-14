#include <blite.h>
#include "lighting.h"
#include <WebSocketsServer.h>

//install "Adafruit Neopixel" from arduino library manager
#include <Adafruit_NeoPixel.h>

//number of rgb leds present in the device
#define NUM_LEDS 16

Blite myBot;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, myBot.getIO("io1"), NEO_RGB + NEO_KHZ800);
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
    strip.setBrightness(150);
    strip.begin();
}
void loop(){
    String html = HTML_LIGHT;
    myBot.smartRenderServer(html);
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
