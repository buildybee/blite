#ifndef BLITE_H_
#define BLITE_H_

#if defined(ESP8266)

#define IO1 12
#define IO2 13
#define SW1 16

#define M1 0
#define M2 2
#define M3 14
#define M4 15

#define I2C_SCL 5
#define I2C_SDA 4

#define ADC1 A0

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <WebSocketsServer.h>
#include <SSD1306Wire.h>

class Blite {
public:
void setup();
bool APServer();
bool connectWiFi(const char * username, const char * password);
bool smartConnectWiFi();
void moveForward();
void moveBackward();
void turnRight();
void turnLeft();
void stopMotor();
void setSpeed(int speed);

void reversePolarityM12();
void turnM12(bool direction);
void stopM12();
void reversePolarityM34();
void turnM34(bool direction);
void stopM34();

int getIO(const char * io);
bool buttonPressed();
void blinkLed(int c);
int readADC();

void setupServer(String &html_content);
void renderServer();
void smartRenderServer(String &html_content);

void otaSetup();
void otaLoop();

void printTxt(const char *dispalytxt);

private:
int m1,m2,m3,m4,speed,lineNo;
ESP8266WebServer webServer = ESP8266WebServer(80);
SSD1306Wire display = SSD1306Wire(0x3c, I2C_SDA, I2C_SCL);
bool serverSetupDone = false;

};
#endif
#endif