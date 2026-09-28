#ifndef THERMAL_CONTROLLER_IOCTL_H
#define THERMAL_CONTROLLER_IOCTL_H

#include <linux/ioctl.h>
#include <linux/types.h>

#define THERMAL_MAGIC 'T'

// Define the simulated thermal state structure
struct thermal_state {
    __s32 temperature;     // Simulated temperature (e.g., in millidegrees Celsius)
    __s32 fan_speed;       // Simulated fan speed (RPM)
    __s32 target_temp;     // Target temperature setpoint
    __s32 alarm_active;    // 1 if alarm is active, 0 otherwise
};

// IOCTL commands
#define THERMAL_GET_STATE _IOR(THERMAL_MAGIC, 1, struct thermal_state)
#define THERMAL_SET_TARGET_TEMP _IOW(THERMAL_MAGIC, 2, __s32)
#define THERMAL_SET_FAN_SPEED _IOW(THERMAL_MAGIC, 3, __s32)
#define THERMAL_RESET _IO(THERMAL_MAGIC, 4)

#endif // THERMAL_CONTROLLER_IOCTL_H
