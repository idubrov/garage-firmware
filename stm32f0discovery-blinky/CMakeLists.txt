project(stm32f0discovery-blinky VERSION 0.1 LANGUAGES C CXX ASM)

# We use board based on STM32F051R8T6 ("STM32F0DISCOVERY"), 64Kib FLASH, 8Kib SRAM.
stm32_setup(F0 051x8 64K 8K)
find_package(CMSIS REQUIRED)
find_package(STM32HAL REQUIRED COMPONENTS gpio tim cortex pwr rcc REQUIRED)

include_directories(
        ${CMSIS_INCLUDE_DIRS}
        ${STM32HAL_INCLUDE_DIR}
        ${SHARED_INCLUDE_DIR}
        include
)

set(PROJECT_SOURCES
        src/main.cpp
        ../shared/src/syscalls.c
        )

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${CMSIS_SOURCES} ${STM32HAL_SOURCES})

stm32_targets(${PROJECT_NAME})
