cmake_minimum_required(VERSION 3.10)
project(IndustrialThermalController)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(ThermalSimulator
    src/main.cpp
    src/emulator/hardware_emulator.cpp
    src/driver_hal/linux_thermal_driver.cpp
)
target_include_directories(ThermalSimulator PRIVATE src)
