#pragma once
#include <cstdint>

class IDriver {
public:
    virtual ~IDriver() = default;
    
    // ADC returns millivolts
    virtual int readAdcMillivolts(uint8_t channel) = 0;
    
    // PWM duty is 0-255
    virtual void writePwmDuty(uint8_t channel, uint16_t duty) = 0;
    
    // Returns pulse count since last read
    virtual uint32_t readTachometerPulses(uint8_t channel) = 0;
};
