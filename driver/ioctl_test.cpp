#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "thermal_controller_ioctl.h"

using namespace std;

int main() {
    int fd = open("/dev/thermal_controller", O_RDWR);

    if (fd < 0) {
        perror("open");
        return 1;
    }

    thermal_state state{};

    if (ioctl(fd, THERMAL_GET_STATE, &state) < 0) {
        perror("THERMAL_GET_STATE");
        close(fd);
        return 1;
    }

    cout << "Initial State:\n";
    cout << "Temperature: " << state.temperature << "\n";
    cout << "Fan Speed: " << state.fan_speed << "\n";
    cout << "Target Temp: " << state.target_temp << "\n";
    cout << "Alarm: " << state.alarm_active << "\n";

    thermal_channel_data data{};
    data.channel = 0;
    if (ioctl(fd, THERMAL_GET_ADC_MV, &data) < 0) {
        perror("THERMAL_GET_ADC_MV");
        close(fd);
        return 1;
    }
    cout << "ADC Channel 0: " << data.value << " mV\n";

    __s32 new_target = 35000;

    if (ioctl(fd, THERMAL_SET_TARGET_TEMP, &new_target) < 0) {
        perror("THERMAL_SET_TARGET_TEMP");
        close(fd);
        return 1;
    }

    if (ioctl(fd, THERMAL_GET_STATE, &state) < 0) {
        perror("THERMAL_GET_STATE");
        close(fd);
        return 1;
    }

    cout << "\nAfter changing target:\n";
    cout << "Target Temp: " << state.target_temp << "\n";

    close(fd);

    return 0;
}
