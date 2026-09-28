#pragma once
#include <cmath>

using namespace std;

namespace sensors {

struct NtcConfig {
    double seriesResistorOhms = 470.0;
    double nominalResistanceOhms = 100000.0;
    double betaCoefficient = 3950.0;
    double referenceTempC = 25.0;
    double supplyVoltageMv = 3300.0;
    double disconnectedMarginMv = 2.0;
    double minReasonableResistanceOhms = 1.0;
    double maxReasonableResistanceOhms = 5000000.0;
    double maxValidTempC = 350.0;
    double minValidTempC = -40.0;
};

inline double ntcMillivoltsToTempC(int millivolts, const NtcConfig &cfg) {
    double vm = static_cast<double>(millivolts);
    double vcc = static_cast<double>(cfg.supplyVoltageMv);

    if (vm < cfg.disconnectedMarginMv) return NAN;

    double denominator = vcc - vm;
    if (denominator <= 0.0) return NAN;

    double rNtc = cfg.seriesResistorOhms * (vm / denominator);

    if (!isfinite(rNtc) || rNtc < cfg.minReasonableResistanceOhms ||
        rNtc > cfg.maxReasonableResistanceOhms) {
        return NAN;
    }

    constexpr double KELVIN_OFFSET = 273.15;
    double t0Kelvin = cfg.referenceTempC + KELVIN_OFFSET;
    double lnRatio = log(rNtc / cfg.nominalResistanceOhms);
    double inverseT = (1.0 / t0Kelvin) + (1.0 / cfg.betaCoefficient) * lnRatio;

    if (!isfinite(inverseT) || inverseT <= 0.0) return NAN;

    double tempKelvin = 1.0 / inverseT;
    double tempC = tempKelvin - KELVIN_OFFSET;

    if (tempC > cfg.maxValidTempC || tempC < cfg.minValidTempC) return NAN;

    return tempC;
}

}
