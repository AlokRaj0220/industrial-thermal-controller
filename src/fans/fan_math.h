#pragma once
#include <cstdint>
#include <algorithm>

namespace fans {
    inline uint16_t calculateDuty(int speedPercent, uint16_t resolution = 255) {
        if (speedPercent < 0) speedPercent = 0;
        if (speedPercent > 100) speedPercent = 100;
        return static_cast<uint16_t>(speedPercent * resolution / 100);
    }
    
    inline uint16_t calculateRpm(uint32_t pulses, double dtSeconds, uint8_t pulsesPerRev = 2) {
        if (dtSeconds <= 0.0) return 0;
        double revs = static_cast<double>(pulses) / pulsesPerRev;
        double rpm = (revs / dtSeconds) * 60.0;
        return static_cast<uint16_t>(rpm);
    }
}
