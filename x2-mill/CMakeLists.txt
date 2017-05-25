project(x2-mill VERSION 0.1 LANGUAGES C CXX ASM)

# We use board based on STM32F103C8T6 ("Blue Pill"), 64Kib FLASH, 20Kib SRAM.
stm32_setup(F1 103xB 64K 20K)
find_package(CMSIS REQUIRED)
find_package(STM32HAL REQUIRED COMPONENTS gpio tim tim_ex cortex pwr rcc REQUIRED)

include_directories(
        ${CMSIS_INCLUDE_DIRS}
        ${STM32HAL_INCLUDE_DIR}
        ${SHARED_INCLUDE_DIR}
        include
)

set(PROJECT_SOURCES
        src/main.cpp
        ../shared/src/syscalls.c
        ../shared/src/hw/encoder.cpp
        ../shared/src/hw/lcd.cpp
        ../shared/src/hw/hall.cpp
        ../shared/src/hw/driver.cpp
        ../shared/src/algo/stepgen.cpp
        )

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${CMSIS_SOURCES} ${STM32HAL_SOURCES})

stm32_targets(${PROJECT_NAME})
