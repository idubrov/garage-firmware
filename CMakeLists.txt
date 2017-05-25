cmake_minimum_required(VERSION 3.7)
project(garage-firmware NONE)
include(cmake/Toolchain-arm-none-eabi.cmake)
include(cmake/STM32.cmake)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")


#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti -fno-threadsafe-statics")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-exceptions -fno-threadsafe-statics")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map,${PROJECT_NAME}.map -fno-exceptions -fno-rtti -fno-threadsafe-statics")


if (NOT DEFINED STM32CubeF1_DIR)
    # Typical location for Mac OS X could be:
    # /Users/<username>/STM32Cube/Repository/STM32Cube_FW_F1_V1.4.0/
    message(WARNING
            "STM32CubeF1_DIR is not set, some projects might fail to build!\n"
            "Install STM32CubeF1 package (via STM32CubeMX) and point STM32CubeF1_DIR to it.")
endif ()

set(SHARED_INCLUDE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/include
        )
set(SHARED_SOURCES_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/src
        )

add_subdirectory(x2-mill)
add_subdirectory(stm32f0discovery-blinky)