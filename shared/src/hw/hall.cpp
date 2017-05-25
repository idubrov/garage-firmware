#include "hw/hall.hpp"

extern void Error_Handler(void);

using namespace hw::hall;

void ::hw::hall::detail::initialize(TIM_TypeDef *const timer,
                        IRQn_Type irqn,
                        GPIO_TypeDef *const port,
                        uint16_t const hall_pin,
                        uint32_t tick_freq) {
    // Data pin
    GPIO_InitTypeDef init = {0};
    init.Pin = hall_pin;
    init.Mode = GPIO_MODE_INPUT;
    init.Pull = GPIO_PULLUP; // FIXME: remove pullup once hardware debouncing is implemented
    HAL_GPIO_Init(port, &init);

    // Timer
    TIM_HandleTypeDef tim_init = {0};
    tim_init.Instance = timer;
    tim_init.Init.Prescaler = (HAL_RCC_GetHCLKFreq() / tick_freq) - 1;
    tim_init.Init.Period = 0xffffU;
    tim_init.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    tim_init.Init.CounterMode = TIM_COUNTERMODE_UP;
    if (HAL_TIM_IC_Init(&tim_init) != HAL_OK) {
        Error_Handler();
    }

    // Input capture
    TIM_IC_InitTypeDef ic_init = {0};
    ic_init.ICPolarity = TIM_ICPOLARITY_FALLING;
    ic_init.ICSelection = TIM_ICSELECTION_DIRECTTI;
    ic_init.ICPrescaler = TIM_ICPSC_DIV1;
    ic_init.ICFilter = 15; // FIXME: ???
    if (HAL_TIM_IC_ConfigChannel(&tim_init, &ic_init, TIM_CHANNEL_1) != HAL_OK) {
        Error_Handler();
    }

    // Slave mode -- reset on capture
    TIM_SlaveConfigTypeDef slave_init = {0};
    slave_init.SlaveMode = TIM_SLAVEMODE_RESET;
    slave_init.InputTrigger = TIM_TS_TI1FP1;
    slave_init.TriggerFilter = 15;
    slave_init.TriggerPrescaler = 0;
    slave_init.TriggerPolarity = TIM_TRIGGERPOLARITY_FALLING;
    HAL_TIM_SlaveConfigSynchronization(&tim_init, &slave_init);

    __HAL_TIM_CLEAR_FLAG(&tim_init, TIM_FLAG_UPDATE | TIM_FLAG_CC1);
    __HAL_TIM_ENABLE_IT(&tim_init, TIM_IT_UPDATE);

    if (HAL_TIM_IC_Start_IT(&tim_init, TIM_CHANNEL_1) != HAL_OK) {
        Error_Handler();
    }

    // Enable NVIC interrupts
    HAL_NVIC_SetPriority(irqn, 7, 0); // FIXME: priority?
    HAL_NVIC_EnableIRQ(irqn);
}
