#include "hw/driver.hpp"

extern void Error_Handler(void);

using namespace hw::driver;


void detail::initialize(TIM_TypeDef *const timer, IRQn_Type IRQn, GPIO_TypeDef *const port,
                        uint16_t const step_pin, uint16_t const dir_pin, uint16_t const enable_pin, uint16_t const reset_pin) {

    // Initialize port

    // Control port, STEP pin (timer controlled)
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = step_pin;
    // Use open-drain, IM483 allows inputs to be left floating (which is considered as '1')
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(port, &gpio);

    // Control port (manually controlled)
    gpio.Pin = dir_pin | enable_pin | reset_pin;
    // Use open-drain, IM483 allows inputs to be left floating (which is considered as '1')
    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(port, &gpio);

    // Start in reset mode
    port->BRR = reset_pin;
    port->BRR = enable_pin;

    // Initialize timer

    TIM_HandleTypeDef init = {0};
    init.Instance = timer;
    init.Init.Prescaler = (HAL_RCC_GetHCLKFreq() / TickFrequency) - 1;
    init.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    init.Init.CounterMode = TIM_COUNTERMODE_UP;
    init.Init.Period = 0; // Configured when stepper is running

    // FIXME: remove deinit?
    if (HAL_TIM_Base_DeInit(&init) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_Base_Init(&init) != HAL_OK) {
        Error_Handler();
    }

    timer->CR2 &= ~TIM_CR2_CCPC; // Do not preload channel enable bit (CCxE)
    // We preload ARR and CCR1 with next delay value (which gets loaded once timer update event triggers)
    timer->CR1 |= TIM_CR1_ARPE;
    timer->CCMR1 = TIM_CCMR1_OC1PE;

    // Configure PWM
    TIM_OC_InitTypeDef ocInit = {0};
    // FIXME: flag: low step vs high step, TIC_OCMODE_PWM2 is low step
    ocInit.OCMode = TIM_OCMODE_PWM2; // inactive till CCR1, then active
    ocInit.Pulse = 0; // set by set_delay when driver is running
    ocInit.OCPolarity = TIM_OCPOLARITY_LOW; // Step pulse is '0'
    ocInit.OCIdleState = TIM_OCIDLESTATE_SET; // Set STEP to '1' when idle
    HAL_TIM_OC_ConfigChannel(&init, &ocInit, TIM_CHANNEL_1);

    timer->CCER |= TIM_CCER_CC1E; // Enable channel 1
    timer->BDTR |= TIM_BDTR_MOE;  // Enable PWM outputs

    // Only counter overflow/underflow generates an update interrupt
    // reload_timer() should not generate an event.
    timer->CR1 |= TIM_CR1_URS;

    // Enable interrupts
    timer->SR = ~TIM_FLAG_UPDATE;
    timer->DIER |= TIM_IT_UPDATE;

    // Enable NVIC interrupts
    HAL_NVIC_SetPriority(IRQn, 3, 0); // FIXME: priority?
    HAL_NVIC_ClearPendingIRQ(IRQn);
    HAL_NVIC_EnableIRQ(IRQn);
}
