#ifndef __HW_HALL_HPP
#define __HW_HALL_HPP

#include "stm32_hal.h"
#include "hw/core.hpp"

namespace hw {
    namespace hall {
        namespace detail {
            void initialize(TIM_TypeDef *const timer,
                            IRQn_Type irqn,
                            GPIO_TypeDef *const port,
                            uint16_t const hall_pin,
                            uint32_t const tick_freq);
        }


        /**
         * @tparam TimerBase timer to use with hall sensor.
         * @tparam PortBase GPIO port where data pin of hall sensor is connected to.
         * @tparam ButtonPin mask for data pin
         */
        template<uint32_t TimerBase, IRQn_Type IRQn, uint32_t PortBase, uint16_t DataPin,
                uint32_t TickFreq = 100000 /* 0.01 ms */, uint32_t MaxRPM = 6000>
        struct hall_t {
            using port = core::port_ref<PortBase>;
            using timer = core::timer_ref<TimerBase>;
            static constexpr uint32_t MinPeriod = 60 * TickFreq / MaxRPM;

            inline static void initialize() {
                detail::initialize(timer(), IRQn, port(), DataPin, TickFreq);
            }

            inline static void interrupt() {
                TIM_TypeDef *tim = timer();
                if (tim->SR & TIM_FLAG_CC1) {
                    tim->SR = ~TIM_FLAG_CC1;

                    uint32_t lsb = TIM2->CCR1;
                    // Captured value is very low and overflow is pending -- need to account for "msb" increment, as it happened
                    // before the capture event. Otherwise, we don't care -- if overflow interrupt is pending, we will handle
                    // it on the next handler invocation.
                    if (lsb < MinPeriod && (tim->SR & TIM_FLAG_UPDATE)) {
                        tim->SR = ~TIM_FLAG_UPDATE;
                        // Capture happened just after the overflow: need to increment upper "msb"
                        _msb++;
                    }

                    uint32_t captured = (_msb << 16) | lsb;

                    // Capture only if period is long enough -- ignore the noise.
                    if (captured >= MinPeriod) {
                        _captured = captured;
                    }
                    _msb = 0;
                } else if (tim->SR & TIM_FLAG_UPDATE) {
                    tim->SR = ~TIM_FLAG_UPDATE;
                    _msb++;
                }
            }

            inline static uint32_t captured() {
                return _captured;
            }

            inline static uint32_t RPM() {
                return 60 * TickFreq / _captured;
            }

        private:
            static volatile uint32_t _captured;
            static volatile uint32_t _msb;

        };

        template<uint32_t TimerBase, IRQn_Type IRQn, uint32_t PortBase, uint16_t DataPin, uint32_t TickFreq, uint32_t MaxRPM>
        volatile uint32_t hall_t<TimerBase, IRQn, PortBase, DataPin, TickFreq, MaxRPM>::_captured = 0;

        template<uint32_t TimerBase, IRQn_Type IRQn, uint32_t PortBase, uint16_t DataPin, uint32_t TickFreq, uint32_t MaxRPM>
        volatile uint32_t hall_t<TimerBase, IRQn, PortBase, DataPin, TickFreq, MaxRPM>::_msb = 0;
    }
}

#endif /* __HW_HALL_HPP */
