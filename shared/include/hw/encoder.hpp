#ifndef __HW_ENCODER_HPP
#define __HW_ENCODER_HPP

#include "stm32_hal.h"
#include "hw/core.hpp"

namespace hw {
    namespace encoder {
        namespace detail {
            void initialize(TIM_TypeDef *const timer,
                            GPIO_TypeDef *const port,
                            uint16_t const button_pin,
                            uint16_t const encoder_pins);
        }


        /**
         *
         * @tparam TimerBase timer to use with encoder interface
         * @tparam PortBase GPIO port where switch and encoder pins are connected to
         * @tparam ButtonPin mask for encoder switch pin
         * @tparam EncoderPins mask for encoder pins (should correspond to timer CH1 and CH2)
         */
        template<uint32_t TimerBase, uint32_t PortBase, uint16_t ButtonPin, uint16_t EncoderPins>
        class encoder_base {
            using port = core::port_ref<PortBase>;
            using timer = core::timer_ref<TimerBase>;
        public:
            inline static void initialize() {
                detail::initialize(timer(), port(), ButtonPin, EncoderPins);
            }

            inline static bool raw_pressed() {
                return !(port()->IDR & ButtonPin);
            }

            inline static uint_fast16_t raw_position() {
                return timer()->CNT >> 1;
            }

            inline static void set_position(uint_fast16_t pos) {
                timer()->CNT = (pos << 1);
            }

            inline static void set_limit(uint_fast16_t limit) {
                timer()->ARR = (limit << 1) - 1;
                timer()->CNT = 0;
            }

            inline static uint32_t get_state() {
                return (timer()->ARR << 16) | timer()->CNT;
            }

            inline static void set_state(uint32_t state) {
                timer()->ARR = state >> 16;
                timer()->CNT = state & 0xffff;
            }

            template<typename S>
            inline void print(S const &sink) const {
                sink << "E: " << (raw_pressed() ? 'P' : 'N') << ' ' << raw_position();
            }
        };
    }
}

#endif /* __HW_ENCODER_HPP */
