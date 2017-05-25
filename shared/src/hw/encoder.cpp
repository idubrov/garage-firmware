#include "hw/encoder.hpp"

extern void Error_Handler(void);

using namespace hw::encoder;

void detail::initialize(TIM_TypeDef *const timer,
                GPIO_TypeDef *const port,
                uint16_t const button_pin,
                uint16_t const encoder_pins) {
    // Button
    GPIO_InitTypeDef init = {0};
    init.Pin = button_pin;
    init.Mode = GPIO_MODE_INPUT;
    init.Pull = GPIO_PULLUP; // FIXME: remove pullup once hardware debouncing is implemented
    HAL_GPIO_Init(port, &init);

    // Quadrature encoder on timer
    GPIO_InitTypeDef init2 = {0};
    init2.Pin = encoder_pins;
    init2.Mode = GPIO_MODE_INPUT;
    init2.Pull = GPIO_PULLUP; // FIXME: remove pullup once hardware debouncing is implemented
    HAL_GPIO_Init(port, &init2);

    // Configure alternative functions
    // We don't actually need to remap anything right now

    // Configure timer as rotary encoder
    TIM_HandleTypeDef tim_init = {0};
    tim_init.Instance = timer;
    tim_init.Init.Prescaler = 0;
    tim_init.Init.Period = 0;
    tim_init.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    tim_init.Init.CounterMode = TIM_COUNTERMODE_UP;


    TIM_Encoder_InitTypeDef enc_init = {0};
    enc_init.EncoderMode = TIM_ENCODERMODE_TI2;

    enc_init.IC1Polarity = TIM_ICPOLARITY_FALLING;
    enc_init.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    enc_init.IC1Prescaler = TIM_ICPSC_DIV8;
    enc_init.IC1Filter = 3;
    enc_init.IC2Polarity = TIM_ICPOLARITY_RISING;
    enc_init.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    enc_init.IC2Prescaler = TIM_ICPSC_DIV8;
    enc_init.IC2Filter = 3;

    if (HAL_TIM_Encoder_Init(&tim_init, &enc_init) != HAL_OK) {
        Error_Handler();
    }

    TIM_MasterConfigTypeDef master;
    master.MasterOutputTrigger = TIM_TRGO_RESET;
    master.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&tim_init, &master) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_TIM_Base_Start(&tim_init) != HAL_OK) {
        Error_Handler();
    }
}
