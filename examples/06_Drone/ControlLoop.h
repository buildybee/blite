
#ifndef _CONTROLLOOP_H
#define _CONTROLLOOP_H

#include <Arduino.h>
#include "PID_v1.h"

class RelayUpdate {
  public:
    virtual void on() = 0;
    virtual void off() = 0;
    virtual void update(float) = 0;
};

class DataSource {
  public:
    virtual float get() = 0;
};



class ControlLoop {

  public:
    ControlLoop(DataSource*, DataSource*, RelayUpdate*, float);
    ControlLoop(DataSource* data, RelayUpdate* update, float setpoint) : ControlLoop(NULL, data, update, setpoint) {;};


    static const bool PID_DEBUG = true;

    static const int INNER  = 0;
    static const int OUTER = 1;

    static const int ONOFF = 10;
    static const int STD = 11;
    static const int CASCADE = 12;

    bool Compute();

    void setPoint(float);
    float getSetPoint() { return _setpoint; }
    float getInnerSetPoint();



    void enableBangBang() { _bangBangOn = true; }
    void disableBangBang() { _bangBangOn = false; }
    bool isBangBangOn() { return _bangBangOn;  }
    void setBangBangRange(float x) { setBangBangRange(-x, x); }
    void setBangBangRange(float, float);
    float getBangBangLower() { return _bangBangLower; }
    float getBangBangUpper() { return _bangBangUpper;}

    void setControlType(int);
    bool isControlOnOff() { return this->_ControlState == ControlLoop::ONOFF; }
    bool isControlStandardPID() { return this->_ControlState == ControlLoop::STD; }
    bool isControlCascadePID() { return this->_ControlState == ControlLoop::CASCADE; }
    int getControlType(){ return this->_ControlState; }


    void setSampleTime(int);
    void setOuterSampleFactor(int);

    bool isOn() { return _isOn; }
    void setOn() { setOnOff(true); }
    void setOff() { setOnOff(false); }

    void setOutputLimits(int, float, float);
    void setDirectionIncrease(int, bool);
    bool getDirectionIncrease(int);

    void setTunings(float p, float i, float d) { setTunings(ControlLoop::INNER, p, i, d);}
    void setTunings(int, float, float, float);
    float getKp(int);
    float getKi(int);
    float getKd(int);


  protected:

    void updateInputs();

    void setOnOff(bool);

  private:

    float outerIn = 0.0;
    float outerOutInnerSet = 0.0;
    float outerSet = 0.0;
    // Setpoint is shared with inner Out

    float innerIn = 0.0;
    float innerOut = 0.0;

    float _outMin = 0.0;
    float _outMax = 1.0;


    PID outer;
    PID inner;

    PID *getController(int c);

    long _sampleTimeMS = 2500;
    long _sampleFactor = 4; // Recommended to be 3-5 times

    int _ControlState = STD;

    float _setpoint;
    float _setpointLower;
    float _setpointUpper;

    bool _isOn = false;
    bool _bangBangOn = false;
    float _bangBangLower = 0.0;
    float _bangBangUpper = 0.0;


    DataSource *_innerDataSource = NULL;
    DataSource *_outerDataSource = NULL;
    RelayUpdate *_relay = NULL;

};

#endif

