if (NOT DEFINED STM32_SETUP)
    message(FATAL_ERROR "Invoke stm32_setup macro first to setup variables!")
endif ()

if (NOT DEFINED STM32Cube${STM32_FAMILY}_DIR)
    message(WARNING "STM32Cube${STM32_FAMILY}_DIR is not set -- might not be able to find CMSIS sources!")
endif ()


set(CMSIS_COMMON_HEADERS
        arm_common_tables.h
        arm_const_structs.h
        arm_math.h
        core_cmFunc.h
        core_cmInstr.h
        core_cmSimd.h
        )

if (NOT CMSIS_STARTUP_SOURCE)
    set(CMSIS_STARTUP_SOURCE startup_stm32f${STM32_TYPE_LOWER}.s)
endif ()

find_path(CMSIS${STM32_FAMILY}_DEVICE_INCLUDE_DIR "system_stm32${STM32_FAMILY_LOWER}xx.h"
        HINTS ${STM32Cube${STM32_FAMILY}_DIR}/Drivers/CMSIS/Device/ST/STM32${STM32_FAMILY}xx/Include
        CMAKE_FIND_ROOT_PATH_BOTH
        )

find_path(CMSIS${STM32_FAMILY}_COMMON_INCLUDE_DIR cmsis_gcc.h
        HINTS ${STM32Cube${STM32_FAMILY}_DIR}/Drivers/CMSIS/Include/
        CMAKE_FIND_ROOT_PATH_BOTH
        )

set(CMSIS_INCLUDE_DIRS
        ${CMSIS${STM32_FAMILY}_DEVICE_INCLUDE_DIR}
        ${CMSIS${STM32_FAMILY}_COMMON_INCLUDE_DIR}
        )

find_file(CMSIS${STM32_FAMILY}_SYSTEM_SOURCE_FILE system_stm32${STM32_FAMILY_LOWER}xx.c
        HINTS ${STM32Cube${STM32_FAMILY}_DIR}/Drivers/CMSIS/Device/ST/STM32${STM32_FAMILY}xx/Source/Templates/
        CMAKE_FIND_ROOT_PATH_BOTH
        )
find_file(CMSIS${STM32_FAMILY}_STARTUP_SOURCE_FILE startup_stm32f${STM32_TYPE}.s
        HINTS ${STM32Cube${STM32_FAMILY}_DIR}/Drivers/CMSIS/Device/ST/STM32${STM32_FAMILY}xx/Source/Templates/gcc
        CMAKE_FIND_ROOT_PATH_BOTH
        )
set(CMSIS_SOURCES
        ${CMSIS${STM32_FAMILY}_SYSTEM_SOURCE_FILE}
        ${CMSIS${STM32_FAMILY}_STARTUP_SOURCE_FILE}
        )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CMSIS DEFAULT_MSG CMSIS_INCLUDE_DIRS CMSIS_SOURCES)
