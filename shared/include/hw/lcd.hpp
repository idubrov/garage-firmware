#ifndef __HW_LCD_HPP
#define __HW_LCD_HPP

#include "stm32_hal.h"
#include "hw/core.hpp"
#include "util/simstream.hpp"

namespace hw {
    namespace lcd {
        // Constants
        enum FunctionMode {
            Bit4 = 0x00, // Send data 4 bits at the time
            Bit8 = 0x10 // Send data 8 bits at the time
        };
        enum FunctionDots {
            Dots5x8 = 0x00, Dots5x10 = 0x04
        };
        enum FunctionLine {
            Line1 = 0x00, Line2 = 0x08
        };
        enum DisplayBlink {
            BlinkOff = 0x00, BlinkOn = 0x01
        };
        enum DisplayCursor {
            CursorOff = 0x00, CursorOn = 0x02
        };
        enum DisplayMode {
            DisplayOff = 0x00, DisplayOn = 0x04
        };
        enum Direction {
            Left = 0x00, Right = 0x04
        };
        enum Scroll {
            CursorMove = 0x00, DisplayMove = 0x08
        };
        enum EntryModeDirection {
            EntryLeft = 0x00, EntryRight = 0x02
        };
        enum EntryModeShift {
            NoShift = 0x00, Shift = 0x01
        };

        enum Command {
            ClearDisplay = 0x01,
            ReturnHome = 0x02,
            EntryModeSet = 0x04,
            DisplayControl = 0x08,
            CursorShift = 0x10,
            FunctionSet = 0x20,
            SetCGRamAddr = 0x40,
            SetDDRamAddr = 0x80
        };

        // HD44780 LCD screen controller
        class HD44780 {
        public:
            /**
             * If use_busy is set to true, will ask display for readiness. Otherwise, uses delay to wait for command
             * completion. Note: set to 'true' only when data port is 5V tolerant!
             */
            constexpr HD44780(
                    void (*delay_us)(uint32_t),
                    GPIO_TypeDef *control_port, uint16_t rs_pin, uint16_t rw_pin, uint16_t e_pin,
                    GPIO_TypeDef *data_port, uint16_t data_shift, bool use_8bit, bool use_busy = false) :
                    _delay_us(delay_us),
                    _control_port(control_port), _rs_pin(rs_pin), _rw_pin(rw_pin), _e_pin(e_pin),
                    _data_port(data_port), _data_shift(data_shift), _use_8bit(use_8bit), _use_busy(use_busy) {
            }

            HD44780(HD44780 const &) = default;

            void initialize() const;

            inline void clear() const {
                command(ClearDisplay);
                // This command could take as long as 1.52ms to execute
                wait_ready(2000);
            }

            void home() const {
                command(ReturnHome);
                // This command could take as long as 1.52ms to execute
                wait_ready(2000);
            }

            void display(DisplayMode display, DisplayCursor cursor,
                         DisplayBlink blink) const {
                command(DisplayControl | display | cursor | blink);
            }

            void entry_mode(EntryModeDirection dir, EntryModeShift scroll) const {
                command(EntryModeSet | dir | scroll);
            }

            void scroll(Direction dir) const {
                command(CursorShift | DisplayMove | dir);
            }

            void cursor(Direction dir) const {
                command(CursorShift | CursorMove | dir);
            }

            void position(uint8_t col, uint8_t row) const {
                uint8_t offset = 0;
                switch (row) {
                    case 1:
                        offset = 0x40;
                        break;
                    case 2:
                        offset = 0x14;
                        break;
                    case 3:
                        offset = 0x54;
                        break;
                }
                command(SetDDRamAddr | (col + offset));
            }

            void write(uint8_t data) const {
                _control_port->BSRR = _rs_pin;
                wait_address(); // tAS
                send(data);
                wait_ready();
                // It takes 4us more (tADD) to update address counter
                _delay_us(5);
            }

            void upload_character(uint_fast8_t location, uint8_t map[8]) const {
                // Only 8 locations are available
                command(SetCGRamAddr | ((location & 0x7U) << 3));
                for (int i = 0; i < 8; i++) {
                    write(map[i]);
                }
            }

            unsigned lines() const {
                return 2; // Hard-coded for now.
            }

        private:
            void command(uint_fast8_t cmd) const {
                _control_port->BRR = _rs_pin;
                wait_address(); // tAS
                send(cmd);
                wait_ready();
            }

            // Typical command wait time is 37us
            void wait_ready(uint16_t delay = 50) const {
                if (_use_busy) {
                    wait_busy_flag();
                } else {
                    _delay_us(delay);
                }
            }

            inline void pulse_enable() const {
                _control_port->BSRR = _e_pin;
                _delay_us(1); // minimum delay is 450 ns
                _control_port->BRR = _e_pin;
            }

            inline void send(uint_fast8_t data) const {
                if (_use_8bit) {
                    send8bits(data);
                } else {
                    send4bits(data >> 4);
                    send4bits(data & 0xfU);
                }
            }

            inline void send8bits(uint_fast8_t data) const {
                _data_port->BSRR = (data & 0xffU) << _data_shift; // Set '1's
                _data_port->BRR = ((~data) & 0xffU) << _data_shift; // Clear '0's
                pulse_enable();
            }

            inline void send4bits(uint_fast8_t data) const {
                _data_port->BSRR = (data & 0xfU) << _data_shift; // Set '1's
                _data_port->BRR = ((~data) & 0xfU) << _data_shift; // Clear '0's
                pulse_enable();
            }

            void wait_busy_flag() const;

            inline void wait_address() const {
                // Address set up time is 40ns minimum (tAS)
                // Each our tick could be up to ~14ns (72Mhz)
                // So, let's wait for 4 cycles just to be sure
                __NOP();
                __NOP();
                __NOP();
                __NOP();
            }

        private:
            void (*const _delay_us)(uint32_t);

            GPIO_TypeDef *const _control_port;
            uint16_t const _rs_pin;
            uint16_t const _rw_pin;
            uint16_t const _e_pin;
            GPIO_TypeDef *const _data_port;
            unsigned const _data_shift;
            bool const _use_8bit;
            bool const _use_busy;
        };

        template<typename Delay,
                uint32_t ControlPortBase, uint16_t RSPin, uint16_t RWPin, uint16_t EPin,
                uint32_t DataPortBase, uint16_t DataShift, bool Use8Bit = false, bool UseBusyFlag = false>
        struct lcd_params {
        private:
            using control_port = ::hw::core::port_ref<ControlPortBase>;
            using data_port = ::hw::core::port_ref<DataPortBase>;
        public:
            operator HD44780() {
                return HD44780(&Delay::us,
                               control_port(), RSPin, RWPin, EPin,
                               data_port(), DataShift, Use8Bit, UseBusyFlag);
            }
        };
        using lcd = HD44780;

        // Stream API for LCDscreen
        struct __clear {
        };

        inline __clear clear() {
            return __clear();
        }

        inline HD44780 const &operator<<(HD44780 const &l, __clear) {
            l.clear();
            return l;
        }

        struct __position {
            uint8_t col;
            uint8_t row;
        };

        inline __position position(uint8_t col, uint8_t row) {
            return {col, row};
        }

        inline HD44780 const &operator<<(HD44780 const &l, __position p) {
            l.position(p.col, p.row);
            return l;
        }

    }
}

#endif /* __HW_LCD_HPP */
