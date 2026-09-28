#include "linux_thermal_driver.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdexcept>
#include <iostream>

// Include the IOCTL header relative to this file's location
#include "../../driver/thermal_controller_ioctl.h"

using namespace std;

LinuxThermalDriver::LinuxThermalDriver(const string& dev_path) {
    fd_ = open(dev_path.c_str(), O_RDWR);
    if (fd_ < 0) {
        throw runtime_error("Failed to open " + dev_path);
    }
}

LinuxThermalDriver::~LinuxThermalDriver() {
    if (fd_ >= 0) {
        close(fd_);
    }
}

int LinuxThermalDriver::readAdcMillivolts(uint8_t channel) {
    struct thermal_channel_data data;
    data.channel = channel;
    data.value = 0;
    
    if (ioctl(fd_, THERMAL_GET_ADC_MV, &data) < 0) {
        cerr << "IOCTL THERMAL_GET_ADC_MV failed\n";
        return 0;
    }
    return static_cast<int>(data.value);
}

void LinuxThermalDriver::writePwmDuty(uint8_t channel, uint16_t duty) {
    struct thermal_channel_data data;
    data.channel = channel;
    data.value = duty;
    
    if (ioctl(fd_, THERMAL_SET_PWM_DUTY, &data) < 0) {
        cerr << "IOCTL THERMAL_SET_PWM_DUTY failed\n";
    }
}

uint32_t LinuxThermalDriver::readTachometerPulses(uint8_t channel) {
    struct thermal_channel_data data;
    data.channel = channel;
    data.value = 0;
    
    if (ioctl(fd_, THERMAL_GET_TACH_PULSES, &data) < 0) {
        cerr << "IOCTL THERMAL_GET_TACH_PULSES failed\n";
        return 0;
    }
    return data.value;
}

void LinuxThermalDriver::setSimulatedLoad(uint8_t channel, double loadCps) {
    struct thermal_channel_data data;
    data.channel = channel;
    // Cast double to an integer representation (e.g., 4.0 -> 4)
    data.value = static_cast<uint32_t>(loadCps);
    
    if (ioctl(fd_, THERMAL_SET_SIM_LOAD, &data) < 0) {
        cerr << "IOCTL THERMAL_SET_SIM_LOAD failed\n";
    }
}
