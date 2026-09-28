#pragma once
#include "driver_hal/i_driver.h"
#include "ntc_math.h"

class VirtualTempSensor {
public:
    VirtualTempSensor(IDriver* driver, uint8_t channel, const sensors::NtcConfig& config)
        : driver_(driver), channel_(channel), config_(config) {}
        
    double readTempC() {
        int mv = driver_->readAdcMillivolts(channel_);
        return sensors::ntcMillivoltsToTempC(mv, config_);
    }
private:
    IDriver* driver_;
    uint8_t channel_;
    sensors::NtcConfig config_;
};
