#include "blite.h"

int Blite::getIO(const char * io){
    if (io == "io1") {
        return IO1;
    } else if (io =="io2") {
        return IO2;
    } else if (io == "scl") {
        return I2C_SCL;
    } else if (io == "sda"){
        return I2C_SDA;
    } else if (io == "io3") {
        return I2C_SCL;
    } else if (io == "io4"){
        return I2C_SDA;
    }
    return -1;
    
}

void Blite::reversePolarityM12(){
    if (this->m2 == M2){
        this->m2 = M1;
        this->m1 = M2;
    } else {
        this->m1 = M1;
        this->m2 = M2;
    }
}
void Blite::reversePolarityM34(){
    if (this->m3 == M3){
        this->m4 = M3;
        this->m3 = M4;
    } else {
        this->m3 = M3;
        this->m4 = M4;
    }
}

void Blite::turnM12(bool direction){
  if (direction) {
    analogWrite(this->m1,this->speed);
    digitalWrite(this->m2,LOW);
  } else {
    analogWrite(this->m2,this->speed);
    digitalWrite(this->m1,LOW);
  }
}
void Blite::stopM12(){
  digitalWrite(this->m1,LOW);
  digitalWrite(this->m2,LOW);
}
void Blite::turnM34(bool direction){
  if (direction) {
    analogWrite(this->m3,this->speed);
    digitalWrite(this->m4,LOW);
  } else {
    analogWrite(this->m4,this->speed);
    digitalWrite(this->m3,LOW);
  }
}
void Blite::stopM34(){
  digitalWrite(this->m3,LOW);
  digitalWrite(this->m4,LOW);
}

void Blite::moveForward(){
    this->turnM12(true);
    this->turnM34(false);
}
void Blite::moveBackward(){
    this->turnM12(false);
    this->turnM34(true);
}
void Blite::turnRight(){
    this->turnM12(false);
    this->turnM34(false);
}
void Blite::turnLeft(){
    this->turnM12(true);
    this->turnM34(true);
}
void Blite::stopMotor(){
  this->stopM12();
  this->stopM34();
}
void Blite::setSpeed(int s){
    this->speed = s ;
}

bool Blite::connectWiFi(const char *username, const char *password){
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    int retry = 0;
    while (retry <= 20) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Trying to connect to wifi");
        WiFi.begin(username,password);
        WiFi.waitForConnectResult();
        delay(1000);
    } else {
        Serial.println("connected to wifi");
        Serial.println(WiFi.localIP());
        this->printTxt("connected to wifi");
        this->printTxt(WiFi.localIP().toString().c_str());
        return 1;
    }
    retry++;
    }
    return 0;
}
bool Blite::smartConnectWiFi(){
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFiManager wm;
    bool res;
    res = wm.autoConnect("Buildybee-smart-config","buildybee"); // password protected ap
    if(res){
      this->printTxt("Connected to wifi..!!");
      this->printTxt("local IP:");
      this->printTxt(WiFi.localIP().toString().c_str());
    }
    return res;
}
bool Blite::APServer() {
    if (WiFi.isConnected()){
        if (WiFi.disconnect()){
            return 0;
        }
    }
    const char* ssid = "buidybee_rc_car";
    IPAddress local_IP(192, 168, 4, 1);
    // We set a Gateway IP address
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    // Connecting WiFi
    WiFi.softAPConfig(local_IP,gateway,subnet);
    WiFi.mode(WIFI_AP);
    this->printTxt("Running local Wifi");
    this->printTxt("SSID: buidybee_rc_car");
    this->printTxt("IP: 192.168.4.1");
    return WiFi.softAP(ssid);
}
bool Blite::buttonPressed() {
    return !digitalRead(SW1);
}

void Blite::setup(){
  pinMode(SW1,INPUT_PULLUP);
  pinMode(M1,OUTPUT);
  pinMode(M2,OUTPUT);
  pinMode(M3,OUTPUT);
  pinMode(M4,OUTPUT);
  this->m1 = M1;
  this->m2 = M2;
  this->m3 = M3;
  this->m4 = M4;
  this->stopMotor();
  this->speed = 255;
  this->display.init();
  this->display.flipScreenVertically();
  this->display.clear();
  this->display.setFont(ArialMT_Plain_10);
  this->display.setTextAlignment(TEXT_ALIGN_LEFT);
  this->lineNo = 0;
  this->printTxt("Welcome Buildybees..!!");
  String newHostname = "buildybee";
  WiFi.hostname(newHostname.c_str());
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  this->otaSetup();

}

void Blite::blinkLed(int c){
  for (int i=0;i<c;i++){
     digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
     delay(500);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(500);
  }
}


int Blite::readADC(){
    return analogRead(ADC1);
}

void Blite::setupServer(String &html_content) {
    this->webServer.on("/", HTTP_GET, [=]() {
        this->webServer.send(200, "text/html", html_content);
    });
    this->webServer.begin();
    this->serverSetupDone = true;
}
void Blite::renderServer() {
    this->webServer.handleClient();
    this->otaLoop();
}
void Blite::smartRenderServer(String &html_content){
    if (!this->serverSetupDone) {
        this->setupServer(html_content);
    }
    this->renderServer();
}
void Blite::otaSetup(){

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}
void Blite::otaLoop(){
    ArduinoOTA.handle();
}

void Blite::printTxt(const char *dispalytxt){
this->display.drawString(0,this->lineNo*10+2,dispalytxt);
this->display.display();
this->lineNo++;
}