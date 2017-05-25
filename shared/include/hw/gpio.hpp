#ifndef __HW_GPIO_HPP
#define __HW_GPIO_HPP

#include "stm32_hal.h"

namespace hw {
    namespace gpio {
        /**
         * "Passivate" pins (set them to input with pull down).
         * @return
         */
        inline void passivate(GPIO_TypeDef *port, uint32_t pins) {
            GPIO_InitTypeDef init;
            init.Pin = pins;
            init.Mode = GPIO_MODE_INPUT;
            init.Speed = GPIO_SPEED_FREQ_LOW;
            init.Pull = GPIO_PULLDOWN;
            HAL_GPIO_Init(port, &init);
        }
    }
}

#endif /* GPIO_HPP */