macro(stm32_setup FAMILY TYPE FLASH RAM)
    message(STATUS "Setting up for STM32${FAMILY} of type ${TYPE} with ${FLASH}iB flash and ${RAM}iB RAM")

    # Set global vars to be used by other targets
    set(STM32_FAMILY ${FAMILY})
    set(STM32_TYPE ${TYPE})
    set(STM32_FLASH_SIZE ${FLASH})
    set(STM32_RAM_SIZE ${RAM})
    string(TOLOWER ${STM32_FAMILY} STM32_FAMILY_LOWER)
    string(TOLOWER ${STM32_TYPE} STM32_TYPE_LOWER)

    # Figure out which CPU to set
    set(STM32_CPUS cortex-m0 cortex-m3)
    set(STM32_FAMILIES F0 F1)

    list(FIND STM32_FAMILIES ${STM32_FAMILY} STM32_FAMILY_IDX)
    if (${STM32_FAMILY_IDX} LESS 0)
        message(FATAL_ERROR "Unsupported STM32 STM32_FAMILY: ${STM32_FAMILY}. Supported families: ${STM32_FAMILIES}")
    endif ()
    list(GET STM32_CPUS ${STM32_FAMILY_IDX} STM32_CPU)

    # Compiler flags
    set(STM32_FLAGS "-mcpu=${STM32_CPU} -DSTM32${STM32_FAMILY} -DSTM32F${STM32_TYPE}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${STM32_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${STM32_FLAGS}")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${STM32_FLAGS}")

    # Linker flags
    # ld does not support search_paths_first and instead treats this as -Wl,-s
    if (APPLE)
        string(REPLACE "-Wl,-search_paths_first" "" CMAKE_C_LINK_FLAGS ${CMAKE_C_LINK_FLAGS})
        string(REPLACE "-Wl,-search_paths_first" "" CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS})
    endif ()


    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}\
        -Wl,-Map,${PROJECT_NAME}.map\
        -Wl,--gc-sections\
        -T ${CMAKE_SOURCE_DIR}/shared/ldscripts/mem.ld\
        -T ${CMAKE_SOURCE_DIR}/shared/ldscripts/sections.ld\
        -Xlinker --defsym=RAM_Size=${STM32_RAM_SIZE}\
        -Xlinker --defsym=FLASH_Size=${STM32_FLASH_SIZE}")

    set(STM32_SETUP TRUE)
endmacro()

function(stm32_targets TARGET)
    if(EXECUTABLE_OUTPUT_PATH)
        set(FILENAME "${EXECUTABLE_OUTPUT_PATH}/${TARGET}")
    else()
        set(FILENAME "${TARGET}")
    endif()
    add_custom_target(${TARGET}.dump DEPENDS ${TARGET} COMMAND ${CMAKE_OBJDUMP} -x -D -S -s ${FILENAME} | ${CMAKE_CPPFILT} > ${FILENAME}.dump)
    add_custom_target(${TARGET}.bin DEPENDS ${TARGET} ${TARGET}.dump COMMAND ${CMAKE_OBJCOPY} -Obinary ${FILENAME} ${FILENAME}.bin)
    add_custom_target(${TARGET}.program DEPENDS ${TARGET}.bin COMMAND st-flash write ${FILENAME}.bin 0x8000000)
    add_custom_command(TARGET ${TARGET}.program POST_BUILD COMMAND st-flash reset)
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_SIZE} ${FILENAME})

    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${FILENAME}.dump;${FILENAME}.hex;${FILENAME}.bin;${FILENAME}.map")
endfunction()