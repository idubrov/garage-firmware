#ifndef __CONFIG_HPP
#define __CONFIG_HPP

#include "hw/delay.hpp"
#include "hw/led.hpp"
#include "hw/gpio.hpp"

namespace config {
    using delay = ::hw::delay::timer_delay_base<TIM3_BASE>;
    using led1 = ::hw::led::led_base<GPIOC_BASE, GPIO_PIN_8>;
    using led2 = ::hw::led::led_base<GPIOC_BASE, GPIO_PIN_9>;
}


#endif /* __CONFIG_HPP */