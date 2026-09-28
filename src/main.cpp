#include "emulator/hardware_emulator.h"
#include "engine/thermal_coordinator.h"
#include "driver_hal/linux_thermal_driver.h"
#include <iostream>
#include <iomanip>
#include <memory>
#include <cstring>
#include <thread>
#include <chrono>

using namespace std;

int main(int argc, char* argv[]) {
    bool use_lkm = false;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--lkm") == 0) {
            use_lkm = true;
        }
    }

    // 1. Initialize Emulated Hardware
    std::unique_ptr<IDriver> driver;
    VirtualHardwareBus* vbus = nullptr;

    LinuxThermalDriver* lkm_driver = nullptr;
    if (use_lkm) {
        cout << "Initializing LinuxThermalDriver (/dev/thermal_controller)...\n";
        auto lkm = std::make_unique<LinuxThermalDriver>();
        lkm_driver = lkm.get();
        driver = std::move(lkm);
    } else {
        cout << "Initializing VirtualHardwareBus (Local Emulator)...\n";
        auto bus = std::make_unique<VirtualHardwareBus>(2, 1);
        vbus = bus.get();
        driver = std::move(bus);
    }
    
    // 2. Setup Drivers/Sensors
    sensors::NtcConfig ntcCfg; // defaults match emulator
    VirtualTempSensor ts1(driver.get(), 0, ntcCfg);
    VirtualTempSensor ts2(driver.get(), 1, ntcCfg);
    vector<VirtualTempSensor*> tempSensors = {&ts1, &ts2};
    
    VirtualFan f1(driver.get(), 0);
    vector<VirtualFan*> fans = {&f1};
    
    // 3. Initialize Coordinator
    ThermalCoordinator coordinator(tempSensors, fans);
    
    double t = 0.0;
    double dt = 0.5; // 500ms ticks
    
    cout << "Starting Simulation...\n";
    cout << "Time(s)\tZ1(C)\tZ2(C)\tLoadZ1\tFanRPM\n";
    cout << "--------------------------------------------------------\n";
    
    while (t <= 60.0) {
        // Scenario timeline for local emulator and LKM
        if (t >= 15.0 && t < 35.0) {
            if (vbus) vbus->setZoneLoad(0, 4.0); // Heavy load on Zone 1
            if (lkm_driver) lkm_driver->setSimulatedLoad(0, 4.0);
        } else {
            if (vbus) vbus->setZoneLoad(0, 0.0); // Idle
            if (lkm_driver) lkm_driver->setSimulatedLoad(0, 0.0);
        }
        
        if (vbus) {
            // Step emulator physics
            vbus->updatePhysics(dt);
        }
        
        // Control loop reads sensors and sets PWM
        coordinator.update(dt);
        
        // Read diagnostics via VirtualFan / IDriver
        uint16_t rpm = f1.readRpm(dt);
        
        // Read true temp to verify the simulation (0.0 if using LKM)
        double z1Temp = vbus ? vbus->getTrueTempC(0) : ts1.readTempC();
        double z2Temp = vbus ? vbus->getTrueTempC(1) : ts2.readTempC();
        
        cout << fixed << setprecision(1)
                  << t << "\t"
                  << z1Temp << "\t"
                  << z2Temp << "\t"
                  << (t >= 15.0 && t < 35.0 ? "ON" : "OFF") << "\t"
                  << rpm << "\n";
                  
        t += dt;
        
        // Align userspace loops with real-time kernel timer ticks
        if (lkm_driver) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(dt * 1000)));
        }
    }
    
    return 0;
}
