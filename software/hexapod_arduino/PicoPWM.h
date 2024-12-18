#ifndef PICO_PWM_H
#define PICO_PWM_H

#include "Arduino.h"
#include <hardware/pwm.h>
#include <hardware/clocks.h>

class PicoPWM {
public:
    PicoPWM();
    
    bool begin(uint8_t pin, uint32_t frequency = 1000);
    void setDutyCycle(float dutyCycle);
    void setPWM(int dutyCycle);
    bool setFrequency(uint32_t frequency);
    uint32_t getFrequency() const;
    float getDutyCycle() const;
    void stop();
    void setServoAngle(float angle);

private:
    uint8_t _pin;
    uint8_t _slice;
    uint8_t _channel;
    float _dutyCycle;
    uint32_t _frequency;
    uint16_t _wrap;
    bool _isRunning;
    
    void calculateWrap();
};

#endif