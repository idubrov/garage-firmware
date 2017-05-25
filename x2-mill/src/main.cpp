#include <stm32f103xb.h>
#include "stm32_hal.h"
#include "config.hpp"

// STM32F103C8T6 (LQFP-48, medium-density)
// 64kB flash, 20kB RAM


// Available timers
// TIM1 -- advanced
// TIM2 to TIM4 -- general purpose

// TIM1: PB13 (CH1N), PB14 (CH2N), PB15 (CH3N), PA8 (CH1), PA9 (CH2), PA10 (CH3), PA11 (CH4)
// TIM2: PA0 (CH1), PA1 (CH2), PA2 (CH3), PA3 (CH4)
// TIM3: PA6 (CH1), PA7 (CH2), PB0 (CH3), PB1 (CH4)
// TIM4: PB6 (CH1), PB7 (CH2), PB8 (CH3), PB9 (CH4)


// Debug interface
// PA13 SWDIO
// PA14 SWCLK
// PA15 JTDI
// PB3 JTDO
// PB4 NJTRST`

// PA9, PA10 -- USART


// Available pins
// PC13, PC14, PC15 (PC13 might be connected to LED!)
// PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7
// PB0, PB1
// FT: PB2, PB10, PB11, PB12, PB13, PB14, PB15
// FT: PA8, PA9, PA10, PA11, PA12,/* PA13, PA14,*/ PA15
// FT: PB3, PB4, PB5 (!FT), PB6, PB7, PB8, PB9

// RS -- GREEN, RW -- WHITE, E -- ORANGE
// Resource allocation
// LCD: RS: PA1, RW: PA2, E: PA3, DATA (4BIT): PB6, PB7, PB8, PB9
// QUAD: QUAD: (PA6, PA7, TIM3), BTN: PA5
// STEPPER:  DIR (PB12), STEP (PB13, TIM1/CH1N), ENABLE (PB14), RESET (PB15)
// SPINDLE: HALL: PA0, TIM2
// SWITCH: PB0, PB1
// DELAY: TIM4
// USART: PA9, PA10

// Data port should be 5v safe!

using namespace config;
using namespace hw::lcd;

::config::stepper_hw stepper;

/**
 *
 */
void SystemClock_Config() {
    // Enable HSE Oscillator and activate PLL with HSE as source
    RCC_OscInitTypeDef osc = {0};
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState = RCC_HSE_ON;
    osc.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    osc.PLL.PLLMUL = RCC_PLL_MUL9;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;

    if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
        while (true);
    }

    // Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers
    RCC_ClkInitTypeDef clkinitstruct = {0};
    clkinitstruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;
    clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
    // Flash latency: two wait states required for 72Mhz
    if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2) != HAL_OK) {
        while (true);
    }
}


void estop_init() {
    GPIO_InitTypeDef init = {0};
    init.Pin = GPIO_PIN_0;
    init.Mode = GPIO_MODE_IT_FALLING;
    init.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &init);

    HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

/**
 * Low priority ESTOP actions (like printing to the LCD) which should be executed from the main "thread".
 */
void estop_check() {
    if (!(GPIOB->IDR & GPIO_PIN_0)) {
        delay::ms(1); // Wait till power is back to normal
        led::on();
        lcd lcd = lcd_hw();
        lcd << ::hw::lcd::position(0, 0) << "*E-STOP*";
        lcd << ::hw::lcd::position(0, 1) << "        ";
        while (true);
    }

}

/**
 * @brief  Main program.
 * @param  None
 * @retval None
 */
int main() {
    SystemClock_Config();

    // Enable clocks for the used subsystems
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_TIM1_CLK_ENABLE(); // driver
    __HAL_RCC_TIM2_CLK_ENABLE(); // hall
    __HAL_RCC_TIM3_CLK_ENABLE(); // encoder
    __HAL_RCC_TIM4_CLK_ENABLE(); // delay
    __HAL_RCC_AFIO_CLK_ENABLE();


    // Initialize all resources
    delay::initialize();
    hall::initialize();
    led::initialize();

    lcd lcd = lcd_hw();
    encoder::initialize();
    lcd.initialize();


    // Setup LCD
    lcd.display(hw::lcd::DisplayOn, hw::lcd::CursorOff, hw::lcd::BlinkOff);
    lcd.entry_mode(hw::lcd::EntryRight, hw::lcd::NoShift);
    lcd.position(0, 0);

    // Setup E_STOP
    estop_init();


    // Setup encoder
    encoder::set_limit(50000);

    driver::initialize();
    driver::release();
    stepper.reset();
    stepper.set_speed((4 * 200 * 16) << 8);

    const uint16_t microsteps = 8;
    const uint16_t step_len_us = 1;
    // FIXME: why step delay is 50us?
    //driver::set_delay(1000000/(4*200*microsteps), step_len_us);
    //driver::reload_timer();
    //driver::start(false);

    bool sw = false;
    uint32_t pos = 0;
    while (true) {
        bool enc_sw = encoder::raw_pressed();
        if (sw && !enc_sw) {
            pos += 32000;
            stepper.move_to(pos);
        }
        sw = enc_sw;

        lcd << ::hw::lcd::position(0, 0) << encoder() << "       ";
        //lcd << ::hw::lcd::position(0, 1) << TIM1->ARR << "       ";
        //lcd << ::hw::lcd::position(0, 1) << ((GPIOB->IDR & GPIO_PIN_0) != 0) << "    ";
        lcd << ::hw::lcd::position(0, 1) << ((GPIOA->IDR & GPIO_PIN_0) != 0) << "!" << hall::RPM() << "     ";

        estop_check();
    }




    // FIXME: ???
    /*TIM2->DIER |= TIM_IT_UPDATE;
    TIM2->DIER |= TIM_IT_CC1;
    uint32_t counter = 0;
    while (true) {
        lcd << ::hw::lcd::position(0, 0) << encoder() << "       ";
        //lcd << ::hw::lcd::position(0, 1) << ((GPIOA->IDR & GPIO_PIN_0) ? "ON" : "OFF") << "      ";
        //lcd << ::hw::lcd::position(0, 0) << TIM2->CNT << "    ";
        //lcd << ::hw::lcd::position(0, 1) << TIM2->CCR1;
        lcd << ::hw::lcd::position(0, 1) << HAL_RCC_GetHCLKFreq() << "    ";
        counter++;
    }*/
}


void Error_Handler() {
    while (true);
}

extern "C" void
SysTick_Handler() {
}

extern "C" void
TIM2_IRQHandler() {
    hall::interrupt();
}

extern "C" void
TIM1_UP_IRQHandler() {
    if (TIM1->SR & TIM_FLAG_UPDATE) {
        TIM1->SR = ~TIM_FLAG_UPDATE;
        stepper.step_completed();
    }
}


extern "C" void
EXTI0_IRQHandler() {
    __HAL_GPIO_EXTI_CLEAR_FLAG(GPIO_PIN_0);

    // FIXME: should disable in a way that stepper does not re-enable it.
    driver::disable();
}