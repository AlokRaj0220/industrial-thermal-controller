# Stage 2: Requirements and Development Plan

## Industrial Thermal Management & Dynamic Fan Controller

**Domain:** Domain 5 — Robotics, Edge AI & Hardware Emulation

## 1. Functional Requirements

| ID | Functional Requirement | Status |
|---|---|---|
| FR-01 | Provide a Linux character device named `/dev/thermal_controller`. | Verified |
| FR-02 | Implement `open`, `read`, `write`, `ioctl`, and `release` file operations. | Verified |
| FR-03 | Provide the eight documented IOCTL commands. | Verified |
| FR-04 | Simulate ADC, PWM, tachometer, and thermal-load state. | Verified |
| FR-05 | Update simulated state using a 500 ms kernel timer. | Verified |
| FR-06 | Read ADC values and convert them to temperature in userspace. | Verified |
| FR-07 | Apply hysteresis thermal-control logic. | Verified |
| FR-08 | Coordinate multiple zones using maximum requested cooling. | Verified |
| FR-09 | Convert cooling demand to 8-bit PWM duty. | Verified |
| FR-10 | Convert tachometer pulses to RPM. | Verified |
| FR-11 | Treat invalid/non-finite temperature as 100% cooling demand. | Verified |
| FR-12 | Run the complete scenario for 60 seconds in LKM mode. | Verified by supplied test evidence |

## 2. Technical Requirements

| ID | Technical Requirement | Status |
|---|---|---|
| TR-01 | C++17 userspace application | Verified |
| TR-02 | C userspace/kernel interface using Linux IOCTLs | Verified |
| TR-03 | Linux character-device and LKM infrastructure | Verified |
| TR-04 | CMake userspace build | Verified |
| TR-05 | Linux Kbuild/driver Makefile for module build | Verified |
| TR-06 | WSL2 development environment with kernel build support | Verified |
| TR-07 | Git/GitHub repository for version control | Verified |
| TR-08 | No physical-hardware dependency | Verified |

## 3. Interface Requirements

| ID | Interface | Purpose / Specification | Status |
|---|---|---|---|
| IR-01 | Userspace HAL ↔ IDriver | Defines methods for ADC, PWM and tachometer access. | Verified |
| IR-02 | HAL ↔ Linux device | `LinuxThermalDriver` opens `/dev/thermal_controller` and issues IOCTLs. | Verified |
| IR-03 | HAL ↔ local emulator | `VirtualHardwareBus` implements `IDriver`. | Verified |
| IR-04 | Userspace ↔ kernel | Communication through `THERMAL_*` IOCTL commands. | Verified |
| IR-05 | Kernel simulated state ↔ ADC/PWM/tachometer/load arrays | State arrays protected by the driver spinlock. | Verified |

## 4. Scope, Modules and Features

The project modules and features include:
- Linux character driver / LKM
- Character-device operations
- IOCTL interface
- Virtual hardware/register simulation
- Kernel timer based state update
- C++17 Driver HAL
- `LinuxThermalDriver`
- `VirtualHardwareBus` / local emulator
- NTC temperature conversion
- PWM calculation
- Tachometer/RPM calculation
- Hysteresis controller
- Multi-zone thermal coordination
- Fail-safe invalid/non-finite temperature handling
- Local Emulator Mode
- Linux LKM Integration Mode

Physical sensors, physical fans, GPIO, I2C, SPI, MMIO, hardware interrupts, and PID control are not implemented in the current project.

## 5. Deliverables

- Linux kernel module source
- IOCTL header
- C++17 userspace application
- Driver HAL and emulator sources
- Thermal-control/sensor/fan/engine modules
- Build configuration
- Integration testing
- README and execution documentation
- Technical project documentation

## 6. Development Plan and Timeline

| Development Phase | Description |
|---|---|
| Requirements and Planning | Define functional, technical and interface requirements. |
| Architecture and UML Design | Define system architecture, components, interfaces and UML models. |
| Linux Character Driver | Implement the Linux character device, file operations and IOCTL interface. |
| Virtual Hardware and Simulation | Implement virtual ADC, PWM, tachometer and thermal-load state. |
| C++ Thermal Control | Implement HAL, temperature conversion, hysteresis, PWM/RPM calculation and multi-zone coordination. |
| Build and Integration | Build the kernel module and C++ application and integrate the Linux and emulator paths. |
| Testing and Verification | Perform build verification, IOCTL regression testing and 60-second LKM integration testing. |
| Documentation and Submission | Prepare README, stage documentation and final technical documentation. |

## 7. Requirements Traceability

| Requirement | Implementation / Design Element | Verification Evidence |
|---|---|---|
| FR-01, FR-02 | `thermal_controller.c` | Device verification |
| FR-03 | `thermal_controller_ioctl.h`, `thermal_controller.c` | IOCTL regression testing |
| FR-04, FR-05 | Virtual state and kernel timer in `thermal_controller.c` | End-to-end LKM test |
| FR-06 | `ntc_math.h`, `virtual_temp_sensor.h` | Runtime thermal behavior |
| FR-07 | `hysteresis_controller.h` | Thermal-control verification |
| FR-08 | `thermal_coordinator.h` | 60-second integration test |
| FR-09, FR-10 | `fan_math.h`, virtual fan components | 60-second integration test |
| FR-11 | Fail-safe thermal-control logic | Thermal-control verification |
| FR-12 | Complete LKM scenario | 60-second integration test |

## 8. Risks and Mitigation

- **Kernel/LKM build dependency on the matching WSL2 kernel environment:** Mitigated by using verified build procedures and providing clear README instructions.
- **Driver/kernel errors during development:** Mitigated by isolated and controlled IOCTL regression testing.
- **Incorrect simulated thermal behavior:** Mitigated by controlled testing in both local emulator and LKM integration modes.
- **Synchronization issues in shared simulated kernel state:** Mitigated by spinlock protection in the kernel module.
- **Accidental loss or inconsistency of project changes:** Mitigated by Git version control.

## 9. Roadmap for Stage 3

The next stage will focus on the following components:
- System architecture
- Component/interface relationships
- Important data structures
- UML class/component relationships
- Runtime sequence
- Thermal-control state machine
- End-to-end thermal workflow
