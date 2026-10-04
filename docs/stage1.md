# Stage 1: Project Introduction

## Industrial Thermal Management & Dynamic Fan Controller

**Domain:** Domain 5 — Robotics, Edge AI & Hardware Emulation

## 1. Project Idea and Objective

This project is a software-only industrial thermal-management and dynamic fan-controller environment.

The main architecture consists of:
- A Linux Loadable Kernel Module (LKM)
- A Linux character device exposed at `/dev/thermal_controller`
- A C++17 userspace application
- A C++ Driver HAL for interfacing with the kernel module
- Virtual hardware and register simulation
- Virtual ADC, PWM, tachometer, and thermal-load state
- Multi-zone thermal control coordination

The primary objective is to demonstrate thermal sensing, cooling control, feedback handling, and Linux userspace/kernel communication without the need for physical thermal hardware. The software models hardware-oriented interactions through standard Linux device interfaces.

## 2. Problem Statement

The project provides a repeatable software environment for developing and verifying thermal-control algorithms and Linux device-driver communication without requiring physical thermal-management hardware.

This project addresses the interaction between:
- Virtual temperature sensing
- Thermal control
- PWM-based fan actuation
- Tachometer feedback
- Changing simulated thermal load

This project does not control real physical fans or sensors.

## 3. Project Scope

**Included:**
- Linux character device and Loadable Kernel Module
- `/dev/thermal_controller`
- C++17 userspace application
- C++ Driver HAL
- `LinuxThermalDriver`
- `VirtualHardwareBus` / local emulator
- Virtual ADC/PWM/tachometer/thermal-load state
- NTC Beta-parameter temperature conversion
- Hysteresis-based thermal control
- Multi-zone coordination
- Fail-safe behavior
- Local Emulator Mode
- Linux LKM Integration Mode
- Build and functional/integration verification

**Not included / current limitations:**
- Physical sensors and fans
- GPIO
- I2C
- SPI
- MMIO
- Hardware interrupts
- PID control
- Physical hardware validation

## 4. Expected Outcome

The expected and achieved software outcomes for this project include:
- A working Linux character driver
- Registration and access to `/dev/thermal_controller`
- A C++17 thermal-control application
- Communication through the Driver HAL and IOCTL interface
- Virtual thermal sensing and fan-control behavior
- Observable temperature, PWM, and RPM behavior scaling appropriately under load
- Successful build and integration verification

No physical hardware validation is claimed or expected as an outcome.

## 5. Applications

This project serves several realistic applications, such as:
- Learning Linux character-device and LKM development
- Understanding userspace/kernel communication interfaces
- Experimenting with and verifying thermal-control algorithms
- Validating software thermal-control logic safely before physical hardware integration
- Serving as a software foundation for future hardware integration (which remains a future scope)

## 6. Progress Evidence

Based on the project's repository history and planning:
- Initial thermal-controller prototype successfully developed.
- Linux driver successfully integrated with the C++ controller logic.
- Driver safety and state-copy improvements implemented.
- README and foundational project documentation added.
- Repository structure cleaned and organized.
- Successful driver and device verification.
- Successful IOCTL regression testing via isolated tests.
- Successful 60-second Linux LKM integration simulation, including simulated load injection, cooling response, fan RPM behavior, and recovery after load removal.

## 7. Roadmap for Stage 2

The project's subsequent phase will focus on:
- Defining functional, technical, and interface requirements
- Documenting requirements traceability
- Defining project phases and verification targets
- Documenting the relationship between requirements, implementation, and evidence
