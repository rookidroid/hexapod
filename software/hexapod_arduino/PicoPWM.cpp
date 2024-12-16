#include "PicoPWM.h"

PicoPWM::PicoPWM() : 
    _pin(0), 
    _slice(0), 
    _channel(0), 
    _dutyCycle(0.0f), 
    _frequency(1000), 
    _wrap(0),
    _isRunning(false) {
}

bool PicoPWM::begin(uint8_t pin, uint32_t frequency) {
    _pin = pin;
    _frequency = frequency;
    
    // Get PWM slice and channel for the pin
    _slice = pwm_gpio_to_slice_num(_pin);
    _channel = pwm_gpio_to_channel(_pin);
    
    // Set pin to PWM mode
    gpio_set_function(_pin, GPIO_FUNC_PWM);
    
    calculateWrap();
    
    // Configure PWM
    pwm_set_wrap(_slice, _wrap);
    pwm_set_chan_level(_slice, _channel, 0);
    pwm_set_enabled(_slice, true);
    
    _isRunning = true;
    return true;
}

void PicoPWM::calculateWrap() {
    // System clock is 125MHz
    uint32_t clockDiv = 1;
    _wrap = (125000000 / (_frequency * clockDiv)) - 1;
    
    // Set clock divider
    pwm_set_clkdiv(_slice, clockDiv);
}

void PicoPWM::setDutyCycle(float dutyCycle) {
    _dutyCycle = constrain(dutyCycle, 0.0f, 100.0f);
    
    if (_isRunning) {
        uint16_t compareValue = (uint16_t)(_wrap * (_dutyCycle / 100.0f));
        pwm_set_chan_level(_slice, _channel, compareValue);
    }
}

void PicoPWM::setPWM(int dutyCycle) {
    _dutyCycle = constrain(dutyCycle, 0, 1024);
    
    if (_isRunning) {
        uint16_t compareValue = (uint16_t)(_wrap * (_dutyCycle / 1024.0f));
        pwm_set_chan_level(_slice, _channel, compareValue);
    }
}

bool PicoPWM::setFrequency(uint32_t frequency) {
    if (frequency < 1 || frequency > 62500000) {
        return false;
    }
    
    _frequency = frequency;
    calculateWrap();
    
    if (_isRunning) {
        pwm_set_wrap(_slice, _wrap);
        setDutyCycle(_dutyCycle);
    }
    
    return true;
}

uint32_t PicoPWM::getFrequency() const {
    return _frequency;
}

float PicoPWM::getDutyCycle() const {
    return _dutyCycle;
}

void PicoPWM::stop() {
    if (_isRunning) {
        pwm_set_enabled(_slice, false);
        _isRunning = false;
    }
}