#ifndef __HW_CORE_HPP
#define __HW_CORE_HPP

#include "stm32_hal.h"
#include <tuple>

namespace hw {
    namespace core {
        template<bool Set>
        void toggle(GPIO_TypeDef *port, uint16_t Pin);

        template<>
        inline void toggle<true>(GPIO_TypeDef *port, uint16_t Pin) {
            port->BSRR = Pin;
        }

        template<>
        inline void toggle<false>(GPIO_TypeDef *port, uint16_t Pin) {
            port->BRR = Pin;
        }


        template<uint32_t PortBase>
        struct port_ref {
            operator GPIO_TypeDef *() {
                return reinterpret_cast<GPIO_TypeDef *>(PortBase);
            }

            GPIO_TypeDef *operator->() {
                return reinterpret_cast<GPIO_TypeDef *>(PortBase);
            }
        };

        template<uint32_t TimerBase>
        struct timer_ref {
            operator TIM_TypeDef *() {
                return reinterpret_cast<TIM_TypeDef *>(TimerBase);
            }

            TIM_TypeDef *operator->() {
                return reinterpret_cast<TIM_TypeDef *>(TimerBase);
            }
        };
    }
}

#endif /* __HW_CORE_HPP */