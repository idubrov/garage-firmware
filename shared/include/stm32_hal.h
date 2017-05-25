#ifndef __STM32_HAL_H
#define __STM32_HAL_H

// Replace UNUSED macro (produces warning on C++14)
template<typename T>
void UNUSED(T&& t){}

#if defined(STM32F0)
#include <stm32f0xx_hal_def.h>
#undef UNUSED
#include <stm32f0xx_hal.h>
#elif defined(STM32F1)
#include <stm32f1xx_hal_def.h>
#undef UNUSED
#include <stm32f1xx_hal.h>
#else
#error "One of STM32F0 or STM32F1 must be defined!"
#endif

#endif /* __STM32_HAL_H */