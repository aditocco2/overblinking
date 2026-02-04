# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.14)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico2 CACHE STRING "Board type")

if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()

# below 3 lines were added
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)

set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

if(NOT DEFINED PICO_SDK_PATH )
	set(PICO_SDK_PATH ${USERHOME}/.pico-sdk )
endif()

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(lightslop2 C CXX ASM)

pico_sdk_init()

set(FREERTOS_KERNEL_PATH "${USERHOME}/.FreeRTOS-Kernel")
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ/FreeRTOS_Kernel_import.cmake)

add_executable(lightslop2 main.c hub75.c)

pico_generate_pio_header(lightslop2 ${CMAKE_CURRENT_LIST_DIR}/hub75.pio)

pico_enable_stdio_uart(lightslop2 0)
pico_enable_stdio_usb(lightslop2 1)

target_link_libraries(lightslop2
    #FreeRTOS-Kernel-Heap4
    FreeRTOS-Kernel-Static
    pico_stdlib
    hardware_pio
    )

target_include_directories(lightslop2 PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(lightslop2)