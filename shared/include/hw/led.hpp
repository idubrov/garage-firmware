#ifndef __HW_LED_HPP
#define __HW_LED_HPP

#include "stm32_hal.h"
#include "core.hpp"

namespace hw {
    namespace led {
        template<uint32_t PortBase, uint16_t Pin, bool HighOn = true>
        class led_t {
            using port_ref = core::port_ref<PortBase>;
        public:
            static void initialize() {
                off();

                GPIO_InitTypeDef init = {0};
                init.Pin = Pin;
                init.Mode = GPIO_MODE_OUTPUT_PP;
                init.Speed = GPIO_SPEED_FREQ_LOW;
                HAL_GPIO_Init(port_ref(), &init);
            }

            inline static void on() {
                ::hw::core::toggle<HighOn>(port_ref(), Pin);
            }

            inline static void off() {
                ::hw::core::toggle<!HighOn>(port_ref(), Pin);
            }
        };
    }
}

#endif /* __HW_LED_HPP */