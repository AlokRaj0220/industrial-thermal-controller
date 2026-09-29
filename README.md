# Industrial Thermal Management & Dynamic Fan Controller

## 1. Project Title
Industrial Thermal Management & Dynamic Fan Controller

## 2. Project Overview
This project is an advanced, multi-zone thermal management system featuring a custom Linux character driver (LKM) and a C++ userspace application. It simulates the real-time thermal dynamics of industrial equipment without requiring physical hardware. The system coordinates temperature sensing, PWM-driven fan cooling, tachometer pulse accumulation, and intelligent hysteresis control to maintain thermal stability under dynamic loads.

## 3. Problem Statement
Modern industrial equipment requires reliable thermal regulation across multiple heat zones to prevent hardware damage. Developing and testing physical thermal controllers is expensive and carries the risk of damaging components. There is a need for a robust, software-simulated thermal physics environment that mirrors authentic hardware interactions via standard Linux system interfaces (VFS and IOCTL).

## 4. Objectives
- Design a virtual Linux character driver to expose standard file operations and IOCTL interfaces.
- Simulate NTC thermistor ADC outputs, fan PWM duties, and tachometer pulses purely in software.
- Develop a C++ userspace application with a Hardware Abstraction Layer (HAL) to orchestrate multi-zone cooling.
- Implement robust hysteresis logic to prevent fan short-cycling and ensure stable temperature equilibrium.

## 5. Architecture
The system employs a tightly decoupled architecture:
- **User-space C++ application**: Manages thermal coordination, dynamic scenario generation, and sensor/fan mathematics.
- **C++ Driver HAL**: Uses the `IDriver` interface and a concrete `LinuxThermalDriver` implementation to communicate with the kernel.
- **Linux VFS interface**: Exposes the kernel module to userspace through the `/dev/thermal_controller` character device.
- **Linux character driver**: A custom Loadable Kernel Module (LKM) implementing `open`, `read`, `write`, and `ioctl` operations.
- **Virtual hardware/register simulation**: Utilizes a real-time kernel timer and internal static arrays to simulate ADC changes and tachometer pulses based on applied PWM and thermal loads.

## 6. Project Directory Structure
```
Industrial_Thermal_Controller/
├── build/                # CMake build outputs
├── driver/               # Linux kernel module code
│   ├── thermal_controller.c
│   ├── thermal_controller_ioctl.h
│   └── ioctl_test.cpp
├── src/                  # Userspace C++ application
│   ├── driver_hal/       # IDriver HAL and Linux backend
│   ├── emulator/         # Local software emulator backend
│   ├── engine/           # Hysteresis and multi-zone coordination
│   ├── fans/             # Fan logic and math
│   ├── sensors/          # NTC thermistor mapping
│   └── main.cpp          # Application entry point and scenario runner
└── CMakeLists.txt        # Build configuration
```

## 7. Key Technologies
- **C**: Used for the Linux Loadable Kernel Module (LKM).
- **C++17**: Used for the userspace application, HAL, and physics logic.
- **Linux Kernel Headers**: Used for character device registration, spinlocks, and kernel timers.
- **CMake & Make**: Build orchestration for both userspace and kernel space.

## 8. Linux Character Driver
The LKM securely registers `/dev/thermal_controller` to accept VFS operations:
- **`open()`**: Allocates basic resources and prepares the device.
- **`read()`**: Acts as a generic loopback for reading isolated internal buffers.
- **`write()`**: Acts as a generic loopback for writing to internal buffers.
- **`ioctl()`**: The primary interface for controlling simulated hardware registers and legacy state structs.

## 9. IOCTL Interface
Defined in `thermal_controller_ioctl.h`, the driver supports the following commands:
- `THERMAL_GET_STATE`: Legacy API returning the core state struct safely.
- `THERMAL_SET_TARGET_TEMP`: Legacy API to update target thresholds.
- `THERMAL_SET_FAN_SPEED`: Legacy API to manually force a legacy fan speed.
- `THERMAL_RESET`: Legacy API to clear state flags.
- `THERMAL_GET_ADC_MV`: Reads the simulated millivolt value for a specific ADC channel.
- `THERMAL_SET_PWM_DUTY`: Sets the fan speed PWM (0-255) for a specific cooling channel.
- `THERMAL_GET_TACH_PULSES`: Reads and clears the accumulated tachometer pulses for RPM calculation.
- `THERMAL_SET_SIM_LOAD`: Injects a dynamic thermal load from the test scenario into the kernel physics simulation.

## 10. Virtual Hardware and Registers
Physical hardware is entirely omitted in favor of software simulation. Static integer arrays (`adc_millivolts`, `pwm_duty`, `tach_pulses`, `sim_zone_load`) model device registers. A kernel timer (`sim_timer`) running at a 500ms interval simulates real-time thermal drift, heating up the ADC registers under load and cooling them under PWM fan actuation.

## 11. C++ Thermal Control
The userspace application executes advanced thermal logic:
- **NTC temperature conversion**: Uses the Steinhart-Hart equation with Beta parameters to map 3000mV–3284mV signals to realistic Celsius temperatures.
- **PWM control**: Maps requested cooling percentages (0-100%) to 8-bit PWM duty cycles (0-255).
- **Tachometer/RPM calculation**: Translates accumulated pulses per interval into industry-standard RPMs (2 pulses per revolution).
- **Hysteresis**: Ensures cooling engages at an upper threshold (50°C), scales linearly to 100% at critical (70°C), and disengages safely only at a lower threshold (40°C).
- **Multi-zone coordination**: Monitors multiple sensors to find the maximum required cooling speed across all zones, applying that maximum speed across all active fans.

## 12. Kernel Synchronization and Error Handling
The kernel driver implements robust safety measures:
- A `spinlock_t` prevents race conditions between the asynchronous `sim_timer` physics loop and userspace `ioctl()` system calls.
- `copy_to_user()` operations occur securely against a local stack copy *after* releasing the spinlock, preventing sleeping-while-atomic kernel panics.
- Standard POSIX error codes (`-EINVAL` for bad channels, `-EFAULT` for copy failures) are strictly enforced.

## 13. Build Instructions
### Build the Linux Kernel Module
```bash
# Uses the provided WSL kernel source
make -C /home/alok/wsl-kernel-6.6.87.2 M=$PWD/driver modules
```

### Build the C++ Application
```bash
mkdir -p build && cd build
cmake ..
make
```

## 14. Running the Project
The project supports two backends for testing:

**Local Emulator Mode** (Instant C++ physics loop without kernel interaction):
```bash
./build/ThermalSimulator
```

**Linux LKM Mode** (Real-time 60s simulation communicating with `/dev/thermal_controller`):
```bash
sudo insmod driver/thermal_controller.ko
sudo ./build/ThermalSimulator --lkm
```

## 15. Example Simulation Behavior
A complete 60-second integration simulation demonstrates the following dynamically:
1. **Initial**: Zone 1 stays near an ambient 25.8°C.
2. **Heating**: A simulated load is applied to Zone 1 at t=15s. The temperature rapidly rises.
3. **Cooling**: Hysteresis detects temperatures above 50°C. The fan engages and RPM ramps up.
4. **Equilibrium**: Zone 1 stabilizes around 52-53°C while the load persists.
5. **Recovery**: The load drops at t=35s. The fan continues cooling to rapidly bring the temperature down.
6. **Ambient**: Once below the low threshold (40°C), the fan shuts off, and the system coasts back to ambient.

## 16. Testing
The repository includes an isolated IOCTL tester:
```bash
make -C driver/ ioctl_test
sudo ./driver/ioctl_test
```
This performs a rapid sanity check on `THERMAL_GET_STATE`, `THERMAL_SET_TARGET_TEMP`, and ADC read operations to guarantee kernel stability without running the full scenario engine.

## 17. Limitations
- **No physical fans/sensors**: All components exist in software. I2C, SPI, and GPIO pathways are completely unrepresented.
- **Software-emulated hardware/registers**: The physics models represent heuristic linear equations and simplified drift mechanisms rather than complex thermodynamic differential equations.

## 18. Future Scope
- Integration with an actual I2C/SPI hardware framework for live deployment.
- Expansion of the character driver into the official Linux `hwmon` or `thermal` subsystem frameworks.
- Complex PID loop controllers instead of linear hysteresis.

## 19. Author/Project Information
Developed as a virtual thermal controller college project emphasizing safe Linux Loadable Kernel Module design and decoupled C++ HAL architectures.
