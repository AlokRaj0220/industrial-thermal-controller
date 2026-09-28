#pragma once
#include <cmath>
#include <algorithm>

using namespace std;

class HysteresisController {
public:
    struct Config {
        double tempHighC = 50.0;
        double tempLowC = 40.0;
        double tempCritC = 70.0; // At this temp, 100% speed
    };

    HysteresisController(const Config& config) : config_(config), isCooling_(false) {}

    int update(double currentTempC) {
        if (!isfinite(currentTempC)) return 100; // Fault state -> 100% cooling

        if (currentTempC > config_.tempHighC) {
            isCooling_ = true;
        } else if (currentTempC < config_.tempLowC) {
            isCooling_ = false;
        }

        if (isCooling_) {
            if (currentTempC >= config_.tempCritC) return 100;
            if (currentTempC <= config_.tempHighC) return 30; // Min fan speed when just above low threshold but cooling is ON
            
            // Linear scale between tempHighC and tempCritC
            double span = config_.tempCritC - config_.tempHighC;
            if (span <= 0) return 100;
            double pct = 30.0 + 70.0 * ((currentTempC - config_.tempHighC) / span);
            return static_cast<int>(min(100.0, pct));
        }
        return 0;
    }
    
    bool isCooling() const { return isCooling_; }

private:
    Config config_;
    bool isCooling_;
};
