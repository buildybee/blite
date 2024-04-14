#include <blite.h>

Blite myBot;
int ir = myBot.getIO("io1");
bool lineFollowerMode = false;
int cs;

void setup()
{
  // Debug console
  Serial.begin(115200);
  myBot.setup();
  myBot.reversePolarityM12();
  myBot.smartConnectWiFi();

}

void loop()
{
  myBot.otaLoop();

  if(myBot.buttonPressed()) {
    lineFollowerMode = !lineFollowerMode;
  }
  if (lineFollowerMode){
      cs=digitalRead(ir);
      
    if(cs==HIGH)
    {
      myBot.moveForward();
    }
    else if(cs==LOW)
    {
      myBot.turnRight();
    }
    if (myBot.buttonPressed()){
      lineFollowerMode = false;
      myBot.stopMotor();
    }
  }

}