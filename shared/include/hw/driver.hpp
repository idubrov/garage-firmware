#ifndef __HW_DRIVER_HPP
#define __HW_DRIVER_HPP

#include "stm32_hal.h"
#include "hw/core.hpp"

namespace hw {
    namespace driver {
        constexpr uint32_t TickFrequency = 1000000; /* 1us */

        /**
         * Convert nanoseconds to driver ticks, with ceil rounding.
         */
        inline static uint16_t ns2ticks(uint16_t ns) {
            constexpr uint32_t NanosecondsInTick = 1000000000 / TickFrequency;
            return (ns + NanosecondsInTick - 1) / NanosecondsInTick;
        }

        namespace detail {
            void initialize(TIM_TypeDef *const timer, IRQn_Type IRQn, GPIO_TypeDef *const port,
                            uint16_t const step_pin, uint16_t const dir_pin, uint16_t const enable_pin,
                            uint16_t const reset_pin);
        }

        // FIXME: Min delay? max speed?
        template<uint32_t TimerBase, IRQn_Type IRQn, uint32_t PortBase,
                uint16_t StepPin, uint16_t DirPin, uint16_t EnablePin, uint16_t ResetPin>
        class driver_base {
        public:
            using port = core::port_ref<PortBase>;
            using timer = core::timer_ref<TimerBase>;
        public:
            inline static void initialize() {
                detail::initialize(timer(), IRQn, port(), StepPin, DirPin, EnablePin, ResetPin);
            }

            inline static void set_direction(bool dir) {
                if (dir) {
                    port()->BSRR = DirPin;
                } else {
                    port()->BRR = DirPin;
                }
            }

            inline static void reset() {
                port()->BRR = ResetPin;
            }

            inline static void release() {
                port()->BSRR = ResetPin;
            }

            inline static void disable() {
                port()->BRR = EnablePin;
            }

            inline static void enable() {
                port()->BSRR = EnablePin;
            }

            inline static bool check_stopped() {
                TIM_TypeDef *tim = timer();

                // Step generation is still running
                if (tim->CR1 & TIM_CR1_CEN) {
                    return false;
                }

                // If there is a pending interrupt, wait until it is cleared.
                // (for instance, we got here just after last timer overflow and it wasn't processed yet).
                while (tim->SR & TIM_FLAG_UPDATE);
                return true;
            }

            inline static void set_last_pulse() {
                timer()->CR1 |= TIM_CR1_OPM;
            }

            inline static void set_delay(uint16_t delay, uint16_t step_len) {
                // FIXME: delay could be 0?
                TIM_TypeDef *tim = timer();
                tim->ARR = delay - 1;
                tim->CCR1 = (delay >= step_len) ? (delay - step_len) : 0;
            }

            inline static void start(bool is_last) {
                TIM_TypeDef *tim = timer();
                if (is_last) {
                    tim->CR1 |= TIM_CR1_OPM;
                } else {
                    tim->CR1 &= ~TIM_CR1_OPM;
                }

                tim->CR1 |= TIM_CR1_CEN; // Enable timer
            }

            /**
             * Generate event to reload timer values from preload registers.
             */
            inline static void reload_timer() {
                timer()->EGR = TIM_EVENTSOURCE_UPDATE;
            }
        };
    }
}

#endif /* __HW_DRIVER_HPP */
