if (NOT DEFINED STM32_SETUP)
    message(FATAL_ERROR "Invoke stm32_setup macro first to setup variables!")
endif ()

if (NOT DEFINED STM32Cube${STM32_FAMILY}_DIR)
    message(WARNING "STM32Cube${STM32_FAMILY}_DIR is not set -- might not be able to find HAL sources!")
endif ()

find_path(STM32HAL${STM32_FAMILY}_INCLUDE_DIR stm32${STM32_FAMILY_LOWER}xx_hal.h
        HINTS ${STM32Cube${STM32_FAMILY}_DIR}/Drivers/STM32${STM32_FAMILY}xx_HAL_Driver/Inc
        CMAKE_FIND_ROOT_PATH_BOTH
        )

# Collect HAL sources
set(HAL_SRCS stm32${STM32_FAMILY_LOWER}xx_hal.c)
foreach (cmp ${STM32HAL_FIND_COMPONENTS})
    list(APPEND HAL_SRCS stm32${STM32_FAMILY_LOWER}xx_hal_${cmp}.c)
endforeach ()
list(REMOVE_DUPLICATES HAL_SRCS)

foreach (HAL_SRC ${HAL_SRCS})
    find_file(HAL${STM32_FAMILY}_${HAL_SRC}_FILE ${HAL_SRC}
            HINTS ${STM32Cube${STM32_FAMILY}_DIR}/Drivers/STM32${STM32_FAMILY}xx_HAL_Driver/Src
            CMAKE_FIND_ROOT_PATH_BOTH
            )
    list(APPEND STM32HAL${STM32_FAMILY}_SOURCES ${HAL${STM32_FAMILY}_${HAL_SRC}_FILE})
endforeach ()


find_path(STM32HAL${STM32_FAMILY}_SOURCES_DIR stm32${STM32_FAMILY_LOWER}xx_hal.c
        HINTS ${STM32Cube${STM32_FAMILY}_DIR}/Drivers/STM32${STM32_FAMILY}xx_HAL_Driver/Src
        CMAKE_FIND_ROOT_PATH_BOTH
        )

set(STM32HAL_INCLUDE_DIR ${STM32HAL${STM32_FAMILY}_INCLUDE_DIR})
set(STM32HAL_SOURCES ${STM32HAL${STM32_FAMILY}_SOURCES})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(STM32HAL DEFAULT_MSG STM32HAL_INCLUDE_DIR STM32HAL_SOURCES)