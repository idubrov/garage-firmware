#include "hw/lcd.hpp"

void hw::lcd::HD44780::initialize() const {
    GPIO_InitTypeDef gpio = {0};

    // Set control pins to output
    gpio.Pin = _rs_pin | _rw_pin | _e_pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(_control_port, &gpio);

    // Set 4/8 data pins to output
    gpio.Pin = (_use_8bit ? 0xffU : 0xfU) << _data_shift;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(_data_port, &gpio);

    // Need to wait at least 40ms after Vcc rises to 2.7V
    _delay_us(50000);

    _control_port->BRR = _rs_pin;
    wait_address();
    if (_use_8bit) {
        // Run initialization procedure for the display (8-bit mode).

        // Set to 8-bit mode, 2 line, 5x10 font
        // Display off, clear, entry mode set
        send8bits(FunctionSet | Bit8 | Line2 | Dots5x10); // Send command for the first time
        _delay_us(4500); // Wait for more than 4.1ms

        pulse_enable(); // Repeat for the second time
        _delay_us(150); // Wait for more than 100us

        pulse_enable(); // Repeat for the third time
        wait_ready();
    } else {
        // Run initialization procedure for the display (4-bit mode).

        send4bits((FunctionSet | Bit8) >> 4);
        _delay_us(4500); // Wait for more than 4.1ms

        pulse_enable(); // Repeat for the second time
        _delay_us(150); // Wait for more than 100us

        pulse_enable(); // Repeat for the third time
        wait_ready(); // Wait fo FunctionSet to finish

        // Now we switch to 4-bit mode
        send4bits((FunctionSet | Bit4) >> 4);
        wait_ready(); // Wait for FunctionSet to finish
    }

    // Finally, set # lines, font size
    command(FunctionSet | (_use_8bit ? Bit8 : Bit4) | Line2 | Dots5x8);

    // Now display should be properly initialized, we can check BF now
    // Though if we are not checking BF, waiting time is longer
    display(DisplayOff, CursorOff, BlinkOff);
    clear();
    entry_mode(EntryRight, NoShift);
}

void hw::lcd::HD44780::wait_busy_flag() const {
    // LCD has OD output, set all to '0' just to be sure.
    _data_port->BRR = 0xffU << _data_shift;

    // First, set 4/8 data ports to input
    GPIO_InitTypeDef gpio;
    gpio.Pin = (_use_8bit ? 0xffU : 0xfU) << _data_shift;
    gpio.Mode = GPIO_MODE_INPUT; // LCD will pull-up to 5v
    // In order to sustain a voltage higher than VDD+0.3 the internal pull-up/pull-down resistors must be disabled
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(_data_port, &gpio);

    // Set R/W to '1', RS to '0'
    _control_port->BSRR = _rw_pin;
    _control_port->BRR = _rs_pin;
    wait_address(); // tAS
    bool busy;
    do {
        if (_use_8bit) {
            _control_port->BSRR = _e_pin;
            _delay_us(1); // minimum delay is 360+25 ns (tDDR + tER)
            busy = (_data_port->IDR & (0x80U << _data_shift)) != 0;
            _delay_us(1); // minimum delay is 450ns (PWen)
            _control_port->BRR = _e_pin;
        } else {
            // First read
            _control_port->BSRR = _e_pin;
            _delay_us(1); // minimum delay is 360+25 ns (tDDR + tER)
            busy = (_data_port->IDR & (0x8U << _data_shift)) != 0;
            _delay_us(1); // minimum delay is 450ns (PWen)
            _control_port->BRR = _e_pin;

            // Second read
            _control_port->BSRR = _e_pin;
            _delay_us(1); // minimum delay is 360+25+450 ns (tDDR + tER + PWEn)
            _control_port->BRR = _e_pin;
        }
    } while (busy);
    // tAH is 10ns, which is less than one cycle. So we don't have to wait.

    // Set R/W back to '0'
    _control_port->BRR = _rw_pin;

    // Reset data port to output
    gpio.Pin = (_use_8bit ? 0xffU : 0xfU) << _data_shift;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(_data_port, &gpio);
}