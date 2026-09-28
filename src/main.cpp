#include "emulator/hardware_emulator.h"
#include "engine/thermal_coordinator.h"
#include <iostream>
#include <iomanip>

using namespace std;

int main() {
    // 1. Initialize Emulated Hardware
    VirtualHardwareBus hardware(2, 1); // 2 zones, 1 fan
    
    // 2. Setup Drivers/Sensors
    sensors::NtcConfig ntcCfg; // defaults match emulator
    VirtualTempSensor ts1(&hardware, 0, ntcCfg);
    VirtualTempSensor ts2(&hardware, 1, ntcCfg);
    vector<VirtualTempSensor*> tempSensors = {&ts1, &ts2};
    
    VirtualFan f1(&hardware, 0);
    vector<VirtualFan*> fans = {&f1};
    
    // 3. Initialize Coordinator
    ThermalCoordinator coordinator(tempSensors, fans);
    
    double t = 0.0;
    double dt = 0.5; // 500ms ticks
    
    cout << "Starting Simulation...\n";
    cout << "Time(s)\tZ1(C)\tZ2(C)\tLoadZ1\tFanRPM\n";
    cout << "--------------------------------------------------------\n";
    
    while (t <= 60.0) {
        // Scenario timeline
        if (t >= 15.0 && t < 35.0) {
            hardware.setZoneLoad(0, 4.0); // Heavy load on Zone 1
        } else {
            hardware.setZoneLoad(0, 0.0); // Idle
        }
        
        // Step emulator physics
        hardware.updatePhysics(dt);
        
        // Control loop reads sensors and sets PWM
        coordinator.update(dt);
        
        // Read diagnostics via VirtualFan / IDriver
        uint16_t rpm = f1.readRpm(dt);
        
        // Read true temp to verify the simulation
        double z1Temp = hardware.getTrueTempC(0);
        double z2Temp = hardware.getTrueTempC(1);
        
        cout << fixed << setprecision(1)
                  << t << "\t"
                  << z1Temp << "\t"
                  << z2Temp << "\t"
                  << (t >= 15.0 && t < 35.0 ? "ON" : "OFF") << "\t"
                  << rpm << "\n";
                  
        t += dt;
    }
    
    return 0;
}
