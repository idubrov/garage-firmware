#include <stm32_hal.h>

#include "config.hpp"

using namespace config;

int main() {
    // Enable clocks for the used subsystems
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_TIM3_CLK_ENABLE();

    hw::gpio::passivate(GPIOA, GPIO_PIN_All);
    hw::gpio::passivate(GPIOB, GPIO_PIN_All);
    hw::gpio::passivate(GPIOC, GPIO_PIN_All);
    hw::gpio::passivate(GPIOD, GPIO_PIN_All);

    delay::initialize();
    led1::initialize();
    led2::initialize();

    while(true) {
        led1::on();
        delay::ms(500);
        led1::off();
        delay::ms(500);
        led2::on();
        delay::ms(500);
        led2::off();
        delay::ms(500);
    }
}

extern "C"
void SysTick_Handler(void)
{
}