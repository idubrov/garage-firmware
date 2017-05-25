set(CMAKE_SYSTEM_NAME Generic)


if(NOT DEFINED ARM_NONE_EABI_PREFIX)
    message(FATAL_ERROR "ARM_NONE_EABI_PREFIX not specified, should point to the location of GCC arm-none-eabi cross tooling!")
endif()

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_C_COMPILER ${ARM_NONE_EABI_PREFIX}/bin/arm-none-eabi-gcc${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_CXX_COMPILER ${ARM_NONE_EABI_PREFIX}/bin/arm-none-eabi-g++${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_ASM_COMPILER ${ARM_NONE_EABI_PREFIX}/bin/arm-none-eabi-gcc${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_OBJCOPY ${ARM_NONE_EABI_PREFIX}/bin/arm-none-eabi-objcopy${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "objcopy tool")
set(CMAKE_OBJDUMP ${ARM_NONE_EABI_PREFIX}/bin/arm-none-eabi-objdump${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "objdump tool")
set(CMAKE_SIZE ${ARM_NONE_EABI_PREFIX}/bin/arm-none-eabi-size${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "size tool")
set(CMAKE_DEBUGER ${ARM_NONE_EABI_PREFIX}/bin/arm-none-eabi-gdb${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "debuger")
set(CMAKE_CPPFILT ${ARM_NONE_EABI_PREFIX}/bin/arm-none-eabi-c++filt${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "C++filt")


set(CMAKE_FIND_ROOT_PATH ${ARM_NONE_EABI_PREFIX}/arm-none-eabi)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)


set(CMAKE_C_FLAGS_DEBUG "-Og -g" CACHE INTERNAL "c compiler flags debug")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g" CACHE INTERNAL "cxx compiler flags debug")
set(CMAKE_ASM_FLAGS_DEBUG "-g" CACHE INTERNAL "asm compiler flags debug")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE INTERNAL "linker flags debug")

set(CMAKE_C_FLAGS_RELEASE "-Os -flto" CACHE INTERNAL "c compiler flags release")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -flto" CACHE INTERNAL "cxx compiler flags release")
set(CMAKE_ASM_FLAGS_RELEASE "" CACHE INTERNAL "asm compiler flags release")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto" CACHE INTERNAL "linker flags release")


set(CMAKE_C_FLAGS "-mthumb -fno-builtin -Wall -std=gnu99 -ffunction-sections -fdata-sections -fomit-frame-pointer -mabi=aapcs" CACHE INTERNAL "c compiler flags")
set(CMAKE_CXX_FLAGS "-mthumb -fno-builtin -Wall -std=c++14 -ffunction-sections -fdata-sections -fomit-frame-pointer -mabi=aapcs -fno-exceptions -fno-rtti -fno-threadsafe-statics" CACHE INTERNAL "cxx compiler flags")
set(CMAKE_ASM_FLAGS "-mthumb -x assembler-with-cpp" CACHE INTERNAL "asm compiler flags")