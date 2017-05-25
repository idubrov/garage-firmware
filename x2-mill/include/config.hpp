#ifndef __CONFIG_HPP
#define __CONFIG_HPP

#include "hw/delay.hpp"
#include "hw/led.hpp"
#include "hw/encoder.hpp"
#include "hw/lcd.hpp"
#include "hw/hall.hpp"
#include "hw/driver.hpp"
#include "hw/stepper.hpp"

namespace config {
    using delay = ::hw::delay::timer_delay_base<TIM4_BASE>;
    //using delay = ::hw::delay::dwt_delay;
    using led = ::hw::led::led_t<GPIOC_BASE, GPIO_PIN_13, false>;

    using encoder = ::hw::encoder::encoder_base<TIM3_BASE, GPIOA_BASE, GPIO_PIN_5, GPIO_PIN_6 | GPIO_PIN_7>;
    // Data pins must be tolerant to 5V if busy flag is used!
    using lcd_hw = ::hw::lcd::lcd_params<delay,
            GPIOB_BASE, GPIO_PIN_1 /* RS */, GPIO_PIN_10 /* RW */, GPIO_PIN_11 /* E */,
            GPIOB_BASE, 12 /* we use pins 12, 13, 14 and 15 for transferring data */>;
    using hall = ::hw::hall::hall_t<TIM2_BASE, TIM2_IRQn, GPIOA_BASE, GPIO_PIN_0>;
    // Driver pins must be tolerant to 5V in case driver uses 5V inputs.
    using driver = ::hw::driver::driver_base<TIM1_BASE, TIM1_UP_IRQn, GPIOA_BASE,
            GPIO_PIN_8 /* STEP */, GPIO_PIN_9 /* DIR */, GPIO_PIN_10 /* ENABLE */, GPIO_PIN_11 /* RESET */>;
    using stepper_hw = ::hw::stepper::controller_base<driver, delay>;
}


#endif /* __CONFIG_HPP */