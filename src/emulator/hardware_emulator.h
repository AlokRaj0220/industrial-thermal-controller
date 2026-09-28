#pragma once
#include "driver_hal/i_driver.h"
#include <vector>
#include <cstdint>
#include <cmath>

using namespace std;

struct ZoneEmulator {
    double currentTempC = 25.0; // Ambient
    double loadHeatRateCps = 0.0; // Degrees per second heat generated
};

struct FanEmulator {
    uint16_t targetPwm = 0;
    double currentRpm = 0.0;
    uint32_t unreadPulses = 0;
};

class VirtualHardwareBus : public IDriver {
public:
    VirtualHardwareBus(int numZones, int numFans);

    int readAdcMillivolts(uint8_t channel) override;
    void writePwmDuty(uint8_t channel, uint16_t duty) override;
    uint32_t readTachometerPulses(uint8_t channel) override;

    // Emulator physics loop
    void updatePhysics(double dtSeconds);
    
    // Manipulate load for demo
    void setZoneLoad(uint8_t channel, double loadCps);
    double getTrueTempC(uint8_t channel) const;
    double getTrueRpm(uint8_t channel) const;

private:
    vector<ZoneEmulator> zones_;
    vector<FanEmulator> fans_;
};
