#pragma once
#include "hysteresis_controller.h"
#include "sensors/virtual_temp_sensor.h"
#include "fans/virtual_fan.h"
#include <vector>
#include <memory>
#include <iostream>

using namespace std;

class ThermalCoordinator {
public:
    ThermalCoordinator(vector<VirtualTempSensor*> sensors, vector<VirtualFan*> fans)
        : sensors_(sensors), fans_(fans) {
        for(size_t i = 0; i < sensors_.size(); ++i) {
            controllers_.push_back(HysteresisController(HysteresisController::Config{}));
        }
    }

    void update(double dtSeconds) {
        int maxFanSpeed = 0;
        
        for (size_t i = 0; i < sensors_.size(); ++i) {
            double tempC = sensors_[i]->readTempC();
            int requiredSpeed = controllers_[i].update(tempC);
            if (requiredSpeed > maxFanSpeed) {
                maxFanSpeed = requiredSpeed;
            }
        }
        
        for (auto fan : fans_) {
            fan->setSpeedPercent(maxFanSpeed);
        }
    }

private:
    vector<VirtualTempSensor*> sensors_;
    vector<VirtualFan*> fans_;
    vector<HysteresisController> controllers_;
};
