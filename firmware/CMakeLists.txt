##
 # @file  CMakeLists.txt
 # @brief Script used to compile the firmware using cmake and pico-sdk
 #
 # This program is distributed WITHOUT ANY WARRANTY.
##

cmake_minimum_required(VERSION 3.12)
set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 17)

# Include and initialize Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(cowprobe C CXX ASM)
pico_sdk_init()

# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME} 
	src/main.c
	src/ios.c
	src/serial.c
	src/usb.c
	src/log.c
	src/jtag.c
	src/cmsis.c
	src/swd.c
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
	./src/
	pico-sdk/lib/tinyusb/src
)

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME} 
	pico_stdlib
	hardware_pio
	tinyusb_device
	tinyusb_board
)
