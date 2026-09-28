#pragma once
#include "i_driver.h"
#include <string>

class LinuxThermalDriver : public IDriver {
public:
    explicit LinuxThermalDriver(const std::string& dev_path = "/dev/thermal_controller");
    ~LinuxThermalDriver() override;

    int readAdcMillivolts(uint8_t channel) override;
    void writePwmDuty(uint8_t channel, uint16_t duty) override;
    uint32_t readTachometerPulses(uint8_t channel) override;

    void setSimulatedLoad(uint8_t channel, double loadCps);

private:
    int fd_;
};
