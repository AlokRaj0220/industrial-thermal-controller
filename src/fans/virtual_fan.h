#pragma once
#include "driver_hal/i_driver.h"
#include "fan_math.h"

class VirtualFan {
public:
    VirtualFan(IDriver* driver, uint8_t channel) : driver_(driver), channel_(channel) {}
    
    void setSpeedPercent(int percent) {
        uint16_t duty = fans::calculateDuty(percent);
        driver_->writePwmDuty(channel_, duty);
    }
    
    uint16_t readRpm(double dtSeconds) {
        uint32_t pulses = driver_->readTachometerPulses(channel_);
        return fans::calculateRpm(pulses, dtSeconds);
    }
private:
    IDriver* driver_;
    uint8_t channel_;
};
