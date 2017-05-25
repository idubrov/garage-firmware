#ifndef __HW_DELAY_HPP
#define __HW_DELAY_HPP

#include "stm32_hal.h"
#include "hw/core.hpp"

namespace hw {
    namespace delay {
        template<uint32_t DelayTimerBase>
        class timer_delay_base {
            using timer_ref = ::hw::core::timer_ref<DelayTimerBase>;
        public:
            inline static void initialize() {
                // Setup timer for delays
                TIM_HandleTypeDef init = {0};
                init.Instance = timer_ref();
                init.Init.Prescaler = (HAL_RCC_GetHCLKFreq() / 1000000) - 1; // Tick every 1us
                init.Init.CounterMode = TIM_COUNTERMODE_UP;
                init.Init.Period = 0;
                init.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
                init.Init.RepetitionCounter = 0;
                HAL_TIM_Base_Init(&init);

                // Delay timer runs in single pulse mode
                HAL_TIM_OnePulse_Init(&init, TIM_OPMODE_SINGLE);
            }

            inline static void us(uint32_t usec) {
                uint32_t msec = usec / 1000;
                usec -= msec * 1000;
                ms(msec);
                if (usec) {
                    us_(usec);
                }
            }

            inline static void ms(uint32_t msec) {
                while (msec--) {
                    us_(1000);
                }
            }
        private:
            inline static void us_(uint16_t usec) {
                TIM_TypeDef *timer = timer_ref();
                timer->ARR = usec;
                timer->SR = ~TIM_SR_UIF;
                timer->CR1 |= TIM_CR1_CEN;
                while (!(timer->SR & TIM_SR_UIF));
            }

        };

#ifdef STM32F1
        class dwt_delay {
        public:
            inline static void initialize() {
                CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
                DWT->CYCCNT = 0;
                DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
            }

            inline static void us(uint32_t usec) {
                // FIXME: Not checking for overflow!
                const uint32_t ticks_per_usec = (HAL_RCC_GetHCLKFreq() / 1000000);
                uint32_t start = DWT->CYCCNT;
                uint32_t total = (ticks_per_usec * usec);
                while ((DWT->CYCCNT - start) < total);
            }

            inline static void ms(uint32_t msec) {
                while (msec--) {
                    us(1000);
                }
            }
        };
#endif
    }
}

#endif /* __HW_DELAY_HPP */
