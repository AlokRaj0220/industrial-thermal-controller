#include "hardware_emulator.h"
#include <cmath>
#include <algorithm>

using namespace std;

static constexpr double VCC_MV = 3300.0;
static constexpr double R_SERIES = 470.0;
static constexpr double R_NOMINAL = 100000.0;
static constexpr double BETA = 3950.0;
static constexpr double T_REF_K = 25.0 + 273.15;
static constexpr double KELVIN_OFFSET = 273.15;

VirtualHardwareBus::VirtualHardwareBus(int numZones, int numFans) {
    zones_.resize(numZones);
    fans_.resize(numFans);
}

void VirtualHardwareBus::setZoneLoad(uint8_t channel, double loadCps) {
    if (channel < zones_.size()) {
        zones_[channel].loadHeatRateCps = loadCps;
    }
}

double VirtualHardwareBus::getTrueTempC(uint8_t channel) const {
    if (channel < zones_.size()) return zones_[channel].currentTempC;
    return 0.0;
}

double VirtualHardwareBus::getTrueRpm(uint8_t channel) const {
    if (channel < fans_.size()) return fans_[channel].currentRpm;
    return 0.0;
}

int VirtualHardwareBus::readAdcMillivolts(uint8_t channel) {
    if (channel >= zones_.size()) return 0;
    
    double tempC = zones_[channel].currentTempC;
    double tempK = tempC + KELVIN_OFFSET;
    
    double rNtc = R_NOMINAL * exp(((1.0/tempK) - (1.0/T_REF_K)) * BETA);
    double v_m = VCC_MV * rNtc / (R_SERIES + rNtc);
    
    return static_cast<int>(v_m);
}

void VirtualHardwareBus::writePwmDuty(uint8_t channel, uint16_t duty) {
    if (channel < fans_.size()) {
        fans_[channel].targetPwm = min(duty, (uint16_t)255);
    }
}

uint32_t VirtualHardwareBus::readTachometerPulses(uint8_t channel) {
    if (channel >= fans_.size()) return 0;
    uint32_t pulses = fans_[channel].unreadPulses;
    fans_[channel].unreadPulses = 0;
    return pulses;
}

void VirtualHardwareBus::updatePhysics(double dtSeconds) {
    double avgRpm = 0.0;
    for (auto& fan : fans_) {
        double targetRpm = (fan.targetPwm / 255.0) * 3000.0; // Max 3000 RPM
        double rpmDiff = targetRpm - fan.currentRpm;
        fan.currentRpm += rpmDiff * (dtSeconds / 2.0); // 2 second time constant
        if (fan.currentRpm < 1.0) fan.currentRpm = 0.0;
        
        avgRpm += fan.currentRpm;
        
        double pulsesFloat = (fan.currentRpm / 60.0) * 2.0 * dtSeconds;
        fan.unreadPulses += static_cast<uint32_t>(pulsesFloat + 0.5); 
    }
    avgRpm /= (fans_.empty() ? 1 : fans_.size());
    
    for (auto& zone : zones_) {
        double ambientC = 25.0;
        double diff = zone.currentTempC - ambientC;
        
        double fanCoolingCps = (avgRpm / 3000.0) * 5.0; // Max 5 C/s cooling per zone from fans
        
        zone.currentTempC += zone.loadHeatRateCps * dtSeconds;
        zone.currentTempC -= (diff * 0.01 + fanCoolingCps * (diff/20.0)) * dtSeconds; 
        
        if (zone.currentTempC < ambientC) zone.currentTempC = ambientC;
    }
}
