#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <WebSocketsServer.h>
#include <EEPROM.h>
#include <Servo.h>
#include <MadgwickAHRS.h>
#include <MPU9250_WE.h>
#include <Wire.h>
#include <xyzFloat.h>
#include "ControlLoop.h"
#define MPU9250_ADDR 0x68
MPU9250_WE mpu = MPU9250_WE(MPU9250_ADDR);

// helper functions
int minMax(int value, int min_value, int max_value) {
  if (value > max_value) {
    value = max_value;
  } else if (value < min_value) {
    value = min_value;
  }
  return value;
} 

// esc declaration
Servo escFL,escFR,escBL,escBR;
const int escPinFL = 14, escPinFR = 12, escPinBL = 13, escPinBR = 15;

// operational Params
Madgwick filter;
xyzFloat accAngle , gyrAngle;
xyzFloat actualOrientation;
xyzFloat angularVelocity;
xyzFloat pid;
unsigned long timerPrint , timerMpu ,  timerEsc, timerRadio;

// radio input
float throttle,pitch,roll,yaw;
int mFL,mFR,mBL,mBR;
bool enableMotor=false;

// editable params
// const int offsetFL = 180,offsetFR = 90 , offsetBL = 30,offsetBR = 30;
const int offsetFL = 200,offsetFR = 200 , offsetBL = 200,offsetBR = 200;
unsigned int MIN_THROTTLE=1100;
unsigned int MAX_THROTTLE=1800;
const int ESC_UPDATE_INTERVAL = 20000; // update every 20000us / 50hz
const int MPU_UPDATE_INTERVAL = 20; // update every 20us / 20Kz
const int RADIO_INTERVAL = 25000; // update every 25000us / 40Hz
const int PRINT_UPDATE_INTERVAL = 20000; // update every 200ms / 50Hz
const int RATE_UPDATE_INTERVAL = 200; // 8ms
const float GYRO_RANGE_DIVIDER = 131; //for gyro range 250
const float PID_MIN = -50;
const float PID_MAX = 50;
const int OUTER_LOOP_FACTOR = 5;
xyzFloat kp = {0.5f, 0.5f, 0.0f};
xyzFloat ki = {0.02f,0.02f, 0.0f};
xyzFloat kd = {1.0f, 1.0f, 0.0f};
xyzFloat outerKp = {0.1f, 0.1f, 0.0f};
xyzFloat outerKi = {0.02f,0.02f, 0.0f};
xyzFloat outerKd = {0.05f, 0.05f, 0.0f};
xyzFloat deadBand = {2.0f,2.0f,2.0f};
xyzFloat angleSetpoint = {0.0f,0.0f,0.0f};

WebSocketsServer webSocket = WebSocketsServer(81);

// print output
bool toPrint = true;
bool toCalibrate = false;

// Function to read xyzFloat from EEPROM at a given address
xyzFloat readXYZFromEEPROM(int address) {
  xyzFloat data;
  EEPROM.get(address, data);  // Retrieve the data from EEPROM
  return data;
}

// Function to write xyzFloat to EEPROM at a given address
void writeXYZToEEPROM(int address, xyzFloat data) {
  EEPROM.put(address, data);
  EEPROM.commit();  // Make sure the data is written
}

class PitchRateDataSource : public DataSource {
public:
    virtual float get() {
        // Return the rate data from IMU (e.g., angular velocity in degrees/second)
        return angularVelocity.y;  // Replace getRateX with the actual method to get the rate
    }
};

class RollRateDataSource : public DataSource {
public:
    virtual float get() {
        // Return the rate data from IMU (e.g., angular velocity in degrees/second)
        return angularVelocity.x;  // Replace getRateX with the actual method to get the rate
    }
};

class YawRateDataSource : public DataSource {
public:
    virtual float get() {
        // Return the rate data from IMU (e.g., angular velocity in degrees/second)
        return angularVelocity.z;  // Replace getRateX with the actual method to get the rate
    }
};

class PitchAngleDataSource : public DataSource {
public:
    virtual float get() {
        // Return the rate data from IMU (e.g., angular velocity in degrees/second)
        return actualOrientation.y;  // Replace getRateX with the actual method to get the rate
    }
};

class RollAngleDataSource : public DataSource {
public:
    virtual float get() {
        // Return the rate data from IMU (e.g., angular velocity in degrees/second)
        return actualOrientation.x;  // Replace getRateX with the actual method to get the rate
    }
};

class YawAngleDataSource : public DataSource {
public:
    virtual float get() {
        // Return the rate data from IMU (e.g., angular velocity in degrees/second)
        return actualOrientation.z;  // Replace getRateX with the actual method to get the rate
    }
};
// Relay class for controlling actuators
class PitchPid : public RelayUpdate {
public:
    virtual void update(float output) {
        // Convert the PID output to a suitable format for controlling the motor (e.g., PWM)
        pid.y = output;
    }
     void on() {
        // Turn on the motor if necessary
        return;
    }

    void off() {
        // Turn off the motor if necessary
        return;
    }
};
class RollPid : public RelayUpdate {
public:
    virtual void update(float output) {
        // Convert the PID output to a suitable format for controlling the motor (e.g., PWM)
        pid.x = output;
    }
     void on() {
        // Turn on the motor if necessary
        return;
    }

    void off() {
        // Turn off the motor if necessary
        return;
    }
};
class YawPid : public RelayUpdate {
public:
    virtual void update(float output) {
        // Convert the PID output to a suitable format for controlling the motor (e.g., PWM)
        pid.z = output;
    }
     void on() {
        // Turn on the motor if necessary
        return;
    }

    void off() {
        // Turn off the motor if necessary
        return;
    }
};

// Define the data sources
PitchRateDataSource yRateSource;
PitchAngleDataSource ySource;
PitchPid yPid;
// Define the data sources
RollRateDataSource xRateSource;
RollAngleDataSource xSource;
RollPid xPid;
// Define the data sources
YawRateDataSource zRateSource;
YawAngleDataSource zSource;
YawPid zPid;

ControlLoop rollControlSystem(&xRateSource, &xSource, &xPid, angleSetpoint.x);
ControlLoop pitchControlSystem(&yRateSource, &ySource, &yPid, angleSetpoint.y);
ControlLoop yawControlSystem(&zRateSource, &zSource, &zPid, angleSetpoint.z);

void setup() {
  EEPROM.begin(2 * sizeof(kp));
  Serial.begin(115200);
  Wire.begin();
  delay(1000);
  if (!mpu.init()) {
    // Serial.println("MPU connection failed.");
    delay(500);
  }
  delay(1000);
  if (toCalibrate){
    calibrateMpu();
  } else {
    // Fetch the values from EEPROM into xyzFloat type
    xyzFloat accelOffset = readXYZFromEEPROM(0);  // Read accelOffset from address 0
    xyzFloat gyroOffset = readXYZFromEEPROM(sizeof(accelOffset));  // Read gyroOffset after accelOffset
    Serial.print("gyro roll from EEPROM ");
    Serial.println(gyroOffset.x);
    Serial.print("gyro pitch from EEPROM ");
    Serial.println(gyroOffset.y);
    Serial.print("gyro yaw from EEPROM ");
    Serial.println(gyroOffset.z);
    
    // Apply the offsets to the MPU
    mpu.setAccOffsets(accelOffset);
    mpu.setGyrOffsets(gyroOffset);
  }
  
  tuneMpu();

  // Setup ESCs
  escFL.attach(escPinFL,1000,2000);
  escFR.attach(escPinFR,1000,2000);
  escBL.attach(escPinBL,1000,2000);
  escBR.attach(escPinBR,1000,2000);
  resetAllEsc();
  delay(300);

  filter.begin(MPU_UPDATE_INTERVAL*10);
  timerPrint,timerMpu,timerEsc,timerRadio= micros();
  cascadePidSetup();
  throttle = MIN_THROTTLE;
  apServer();
  // setupServer(htmlContent);
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  unsigned long looptTimer = micros();
  if (micros()-timerMpu > MPU_UPDATE_INTERVAL) {
    getSenorValue();
    sensorFusion();
    updatePID();
    
    
    timerMpu = micros();
  }
  
  if (enableMotor){
    
    if (micros()-timerEsc > ESC_UPDATE_INTERVAL) {
      updateMotorSpeed();
      timerEsc = micros();
    }

  }
  
  
  if ((toPrint) && (micros()-timerPrint > PRINT_UPDATE_INTERVAL)) {
    // processingLog();
    serialPrint();
    timerPrint = micros();
  }

  if (micros()-timerRadio > RADIO_INTERVAL) {
    // processingLog();
    telemetry();
    timerRadio = micros();
  }
  webSocket.loop();
}

void apServer(){
    WiFi.disconnect();
    const char* ssid = "buidybee_drone";
    IPAddress local_IP(192, 168, 4, 1);
    // We set a Gateway IP address
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    // Connecting WiFi
    WiFi.softAPConfig(local_IP,gateway,subnet);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid);
}

void tuneMpu(){
  mpu.enableGyrDLPF();
  mpu.setGyrDLPF(MPU9250_DLPF_6);
  mpu.setSampleRateDivider(5);
  mpu.setGyrRange(MPU9250_GYRO_RANGE_250);
  mpu.setAccRange(MPU9250_ACC_RANGE_2G);
  mpu.enableAccDLPF(true);
  mpu.setAccDLPF(MPU9250_DLPF_6);
}

void calibrateMpu(){
  Serial.println("Position your MPU9250 flat and don't move it - calibrating...");
  delay(1000);
  mpu.autoOffsets();
  xyzFloat gyroOffset = mpu.getGyrOffsets();
  xyzFloat accelOffset = mpu.getAccOffsets();

  Serial.print("gyro roll to EEPROM ");
  Serial.println(gyroOffset.x);
  Serial.print("gyro pitch to EEPROM ");
  Serial.println(gyroOffset.y);
  Serial.print("gyro yaw to EEPROM ");
  Serial.println(gyroOffset.z);
    
  // Save the values in EEPROM
  writeXYZToEEPROM(0, accelOffset);  // Write accelOffset at EEPROM address 0
  writeXYZToEEPROM(sizeof(accelOffset), gyroOffset);  // Write gyroOffset right after accelOffset
  
  Serial.println("Offsets written to EEPROM!");
}

void getSenorValue(){
  accAngle = mpu.getGValues();
  gyrAngle = mpu.getGyrValues();
  angularVelocity =  (angularVelocity *0.7) + (mpu.getCorrectedGyrRawValues()/GYRO_RANGE_DIVIDER)*0.3; // filter with 0.7 - 0.3 split.
}

void sensorFusion(){
  filter.updateIMU(gyrAngle.x, gyrAngle.y, gyrAngle.z, accAngle.x, accAngle.y, accAngle.z);
  actualOrientation.x = (abs(actualOrientation.x-filter.getRoll()) > deadBand.x) ? filter.getRoll(): actualOrientation.x;
  actualOrientation.y = (abs(actualOrientation.y-filter.getPitch()) > deadBand.y) ? filter.getPitch(): actualOrientation.y;
  actualOrientation.z = (abs(actualOrientation.z-filter.getYaw()) > deadBand.z) ? filter.getYaw(): actualOrientation.z;
 }

void updatePID(){
    rollControlSystem.Compute();
    pitchControlSystem.Compute();
    yawControlSystem.Compute();
 }

void updateMotorSpeed(){
  mFL = throttle + int(trunc(pid.x)) - int(trunc(pid.y)) + int(trunc(pid.z));
  mFR = throttle - int(trunc(pid.x)) - int(trunc(pid.y)) - int(trunc(pid.z));
  mBL = throttle + int(trunc(pid.x)) + int(trunc(pid.y)) - int(trunc(pid.z));
  mBR = throttle - int(trunc(pid.x)) + int(trunc(pid.y)) + int(trunc(pid.z));


  mFL = minMax(mFL,MIN_THROTTLE,MAX_THROTTLE) + offsetFL;
  mFR = minMax(mFR,MIN_THROTTLE,MAX_THROTTLE) + offsetFR;
  mBL = minMax(mBL,MIN_THROTTLE,MAX_THROTTLE) + offsetBL;
  mBR = minMax(mBR,MIN_THROTTLE,MAX_THROTTLE) + offsetBR;

  escFL.write(mFL);
  escFR.write(mFR);
  escBL.write(mBL);
  escBR.write(mBR);
 }

void resetOrientation(){
  angleSetpoint  = angleSetpoint * 0;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            enableMotor = false;
            resetAllEsc();  // Stop motors when disconnected
            break;

        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num); 
        }
        break;

        case WStype_TEXT: {
          String payloadStr = String((char*)payload);
          timerRadio = micros();
            // Start ESC when the command is received

          // Serial.println(payloadStr);

          if (payloadStr == "startESC") {
            enableMotor = true;   // Enable motors
            softStartEsc();
            break;
          }

          // Stop ESC when the command is received
          if (payloadStr == "stopESC") {
            resetAllEsc();
            enableMotor = false;
            break;

          }

          if (payloadStr == "REQUEST_PID") {
                String response = "kpX:" + String(kp.x) + '|' + \
                "kpY:" + String(kp.y) + '|' + \
                "kpZ:" + String(kp.z) + '|' + \
                "kiX:" + String(ki.x) + '|' + \
                "kiY:" + String(ki.y) + '|' + \
                "kiZ:" + String(ki.z) + '|' + \
                "kdX:" + String(kd.x) + '|' + \
                "kdY:" + String(kd.y) + '|' + \
                "kdZ:" + String(kd.z);
                // Serial.println(response);
                // Send the current PID values to the client
                webSocket.broadcastTXT(response);
                break; // Exit early as we've handled the action
            }

            StaticJsonDocument<200> doc;
            DeserializationError error = deserializeJson(doc, payloadStr);
            

            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
                return;
            }

           

            // Extract joystick data (throttle, yaw, pitch, roll) if present
            if (doc.containsKey("throttle")) {
                throttle = MIN_THROTTLE + int(doc["throttle"]);
                // angleSetpoint.z = float(doc["yaw"]);
                break;;
            }
            if (doc.containsKey("roll")) {
                angleSetpoint.x = float(doc["roll"]);
                angleSetpoint.y = float(doc["pitch"]);
                break;
            }

            if (doc.containsKey("kpX")) {
                kp.x = float(doc["kpX"]);
                kp.y = float(doc["kpY"]);
                kp.z = float(doc["kpZ"]);
                ki.x = float(doc["kiX"]);
                ki.y = float(doc["kiY"]);
                ki.z = float(doc["kiZ"]);
                kd.x = float(doc["kdX"]);
                kd.y = float(doc["kdY"]);
                kd.z = float(doc["kdZ"]);
                retunePid();
                break;
                // Serial.println("Pids updated");
            }

            // Update radio timer
            
            break;
        }

        default:
            if ((timerRadio > 0) && (micros() - timerRadio > RADIO_INTERVAL * 1000)) {
                resetOrientation();
                timerRadio = 0;
            }
            break;
    }
}


void resetAllEsc(){
  escFL.write(1000);
  escFR.write(1000);
  escBL.write(1000);
  escBR.write(1000);
}

void softStartEsc(){
  throttle = MIN_THROTTLE;
  escFL.write(MIN_THROTTLE  + offsetFL);
  escFR.write(MIN_THROTTLE  + offsetFR);
  escBL.write(MIN_THROTTLE  + offsetBL);
  escBR.write(MIN_THROTTLE  + offsetBR);
}

void serialPrint(){
  Serial.print("mFL:"); Serial.print(mFL); Serial.print(",");
  Serial.print("mFR:"); Serial.print(mFR); Serial.print(",");
  Serial.print("mBL:"); Serial.print(mBL); Serial.print(",");
  Serial.print("mBR:"); Serial.print(mBR); Serial.print(",");
  Serial.print("Roll:"); Serial.print(actualOrientation.x,2); Serial.print(",");
  Serial.print("Pitch:"); Serial.print(actualOrientation.y,2); Serial.print(",");
  Serial.print("Yaw:"); Serial.print(actualOrientation.z,2); Serial.print(",");
  Serial.print("Roll:"); Serial.print(actualOrientation.x,2); Serial.print(",");
  Serial.print("Pitch:"); Serial.print(actualOrientation.y,2); Serial.print(",");
  Serial.print("Yaw:"); Serial.print(actualOrientation.z,2); Serial.print(",");
  Serial.print("pidX:"); Serial.print(pid.x); Serial.print(",");
  Serial.print("pidY:"); Serial.print(pid.y); Serial.print(",");
  Serial.print("pidZ:"); Serial.print(pid.z); Serial.print(",");
  Serial.print("rateRollX:"); Serial.print(angularVelocity.x); Serial.print(",");
  Serial.print("ratePitchY:"); Serial.print(angularVelocity.y); Serial.print(",");
  Serial.print("rateYawZ:"); Serial.println(angularVelocity.y);
 }

void processingLog(){
  Serial.print(actualOrientation.x,3);
  Serial.print(" | ");
  Serial.println(actualOrientation.y,3);
 }

void telemetry(){
  String telemetryTx = String(int(actualOrientation.y)) + "|" + String(int(actualOrientation.x)) + "|" + String(int(actualOrientation.z)); 
  webSocket.broadcastTXT(telemetryTx);
}

void retunePid(){
  rollControlSystem.setTunings(ControlLoop::INNER,kp.x,ki.x,kd.x);
  pitchControlSystem.setTunings(ControlLoop::INNER,kp.y,ki.y,kd.y);
  yawControlSystem.setTunings(ControlLoop::INNER,kp.z,ki.z,kd.z);
  rollControlSystem.setTunings(ControlLoop::OUTER,outerKp.x,outerKi.x,outerKd.x);
  pitchControlSystem.setTunings(ControlLoop::OUTER,outerKp.y,outerKi.y,outerKd.y);
  yawControlSystem.setTunings(ControlLoop::OUTER,outerKp.z,outerKi.z,outerKd.z);
}

void cascadePidSetup(){
  rollControlSystem.setTunings(ControlLoop::INNER,kp.x,ki.x,kd.x);
  rollControlSystem.setTunings(ControlLoop::OUTER,outerKp.x,outerKi.x,outerKd.x);
  rollControlSystem.setOutputLimits(ControlLoop::INNER, PID_MIN, PID_MAX);
  rollControlSystem.setOutputLimits(ControlLoop::OUTER, PID_MIN, PID_MAX);
  rollControlSystem.setControlType(ControlLoop::CASCADE);
  rollControlSystem.setSampleTime(RATE_UPDATE_INTERVAL);
  rollControlSystem.setOuterSampleFactor(OUTER_LOOP_FACTOR);
  rollControlSystem.setOn();
  pitchControlSystem.setTunings(ControlLoop::INNER,kp.y,ki.y,kd.y);
  pitchControlSystem.setTunings(ControlLoop::OUTER,outerKp.y,outerKi.y,outerKd.y);
  pitchControlSystem.setOutputLimits(ControlLoop::INNER, PID_MIN, PID_MAX);
  pitchControlSystem.setOutputLimits(ControlLoop::OUTER, PID_MIN, PID_MAX);
  pitchControlSystem.setControlType(ControlLoop::CASCADE);
  pitchControlSystem.setSampleTime(RATE_UPDATE_INTERVAL);
  pitchControlSystem.setOuterSampleFactor(OUTER_LOOP_FACTOR);
  pitchControlSystem.setOn();
  yawControlSystem.setTunings(ControlLoop::INNER,kp.z,ki.z,kd.z);
  yawControlSystem.setTunings(ControlLoop::OUTER,outerKp.z,outerKi.z,outerKd.z);
  yawControlSystem.setOutputLimits(ControlLoop::INNER, PID_MIN, PID_MAX);
  yawControlSystem.setOutputLimits(ControlLoop::OUTER, PID_MIN, PID_MAX);
  yawControlSystem.setControlType(ControlLoop::CASCADE);
  yawControlSystem.setSampleTime(RATE_UPDATE_INTERVAL);
  yawControlSystem.setOuterSampleFactor(OUTER_LOOP_FACTOR);
  yawControlSystem.setOn();
}