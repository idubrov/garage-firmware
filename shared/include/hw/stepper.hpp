#ifndef __HW_STEPPER_HPP
#define __HW_STEPPER_HPP

#include <atomic>

#include "hw/core.hpp"
#include "hw/driver.hpp"
#include "algo/stepgen.hpp"

namespace hw {
    namespace stepper {

        // Stepper mechanics configuration
        // FIXME: implement...

        // Stepper motor controller
        template<typename Driver, typename Delay,
                uint32_t StepPulseWidthNS = 75,
                uint32_t StepPulseSpacingNS = 100,
                uint32_t DirSetupNS = 100 /* FIXME: ? */,
                uint32_t DirHoldNS = 100>
        class controller_base {
        public:
            enum State {
                Stopped, Accelerating, Slewing, Decelerating
            };
        public:
            controller_base() : _step_pulse_ticks(hw::driver::ns2ticks(StepPulseWidthNS)),
                                _step_space_ticks(hw::driver::ns2ticks(StepPulseSpacingNS)),
                                _dir_setup_ticks(hw::driver::ns2ticks(DirSetupNS)),
                                _dir_hold_ticks(hw::driver::ns2ticks(DirHoldNS)),
                                reverse(false), stepgen(hw::driver::TickFrequency),
                                direction(true), base_step(0), position(0), stop_requested(false) {
            }

            void reset() {
                // Reload delays from the settings
                // FIXME: reloading...
                //step_len_ticks = hw::driver::ns2ticks(settings::StepLen.get(eeprom));
                //step_space_ticks = hw::driver::ns2ticks(settings::StepSpace.get(eeprom));
                //dir_setup_ns = settings::DirectionSetup.get(eeprom);
                //dir_hold_ns = settings::DirectionHold.get(eeprom);
                //reverse = settings::Reverse.get(eeprom);

                // FIXME: check return value
                //uint16_t accel = settings::Acceleration.get(eeprom);
                //uint16_t microsteps = settings::Microsteps.get(eeprom);

                // FIXME: real values...
                uint16_t accel = 1200;
                uint16_t microsteps = 16;
                stepgen.set_acceleration((static_cast<uint32_t>(accel) * microsteps) << 8);

                // FIXME: reset position
            }


            /**
             * Step complete handler, should be called from the timer update IRQ.
             * Does not reset interrupt handler (should be reset by the IRQ).
             */
            void step_completed() {
                if (Driver::check_stopped()) {
                    // If timer is stopped, disable the driver.
                    // FIXME: configurable behavior...
                    Driver::disable();
                    return;
                }

                if (stop_requested) {
                    stepgen.set_target_step(0);
                    stop_requested = false;
                }

                if (load_delay() == 0) {
                    // Stop on the next update, one pulse mode
                    // Note that load_delay() should have already loaded ARR and
                    // CCR1 with idle values.
                    Driver::set_last_pulse();
                }
            }

            void stop() {
                stop_requested = true;
            }

            inline bool is_stopped() {
                return Driver::check_stopped();
            }

            /**
             * Zero current position. This method could only be called while stepper motor is stopped.
             * @return false if motor is still running
             */
            bool zero_position() {
                if (!Driver::check_stopped()) {
                    return false;
                }

                base_step = stepgen.current_step();
                position = 0;
                return true;
            }

            /**
             * Move to given position. Note that no new move commands will be accepted
             * while stepper is running. However, other target parameter, target speed,
             * could be changed any time.
             * FIXME: technically, we can actually change target on the fly...
             * @param target
             * @return
             */
            bool move_to(int32_t target) {
                if (!Driver::check_stopped()) {
                    return false;
                }

                int32_t pos = update_position();
                uint32_t delta;
                if (pos < target) {
                    delta = target - pos;
                    set_direction(true);
                } else if (pos > target) {
                    delta = pos - target;
                    set_direction(false);
                } else {
                    // Nothing to do!
                    return true;
                }
                stepgen.set_target_step(base_step + delta);
                stop_requested = false;

                // Load first delay into ARR/CC1, if not stopped
                if (load_delay() == 0) {
                    // Not making even a single step
                    return false;
                }

                // FIXME: should be configurable... Enable driver outputs
                Driver::enable();
                Driver::reload_timer();

                // Load second delay into ARR & CC1.
                bool is_last = load_delay() == 0;

                // Start pulse generation
                Driver::start(is_last);

                return true;
            }

            /**
             * Set slew speed (maximum speed stepper motor would run). Note that stepper
             * motor would only reach this speed if target step is far enough, so there is
             * enough time for deceleration.
             * @param speed target slew speed to reach, in steps per second, 24.8 format
             * @return false if target speed is either too slow (doesn't fit into timer counter)
             * or too fast.
             */
            inline bool set_speed(uint32_t speed) {
                return stepgen.set_target_speed(speed);
            }

            /**
             * Return current stepper position in pulses
             * @return current stepper position in 24.8 format
             */
            inline int32_t current_position() {
                uint32_t offset = stepgen.current_step() - base_step;
                return position + (direction ? offset : -offset);
            }

            /**
             * Get the current stepper speed in pulses per second
             * @return current stepper speed in 24.8 format
             */
            inline uint32_t current_speed() {
                return stepgen.current_speed();
            }

            inline bool current_direction() {
                return direction;
            }

            /**
             * Print state for debugging purposes
             * @tparam S
             * @param sink
             */
            template<typename S>
            void print(S const &sink) const {
                sink << "S:" << stepgen._step();
                sink << " T:" << stepgen.target_step();
                sink << " Sp:" << stepgen._speed() << "TSp: " << stepgen.target_delay();
            }

        private:
            uint32_t load_delay() {
                uint32_t delay = stepgen.next();
                if (delay != 0) {
                    // Load new step into ARR, start pulse at the end
                    uint32_t d = (delay + 128) >> 8; // Delay is in 16.8 format
                    Driver::set_delay(d, _step_pulse_ticks);
                } else {
                    // FIXME: do we need this branch?
                    // Load idle values. This is important to do on the last update
                    // when timer is switched into one-pulse mode.
                    Driver::set_delay(1 /* FIXME: IDLE delay */, _step_pulse_ticks);
                }
                return delay;
            }


            // Incorporate outstanding steps from the stepgen into current position
            inline int32_t update_position() {
                uint32_t step = stepgen.current_step();
                uint32_t offset = step - base_step;
                base_step = step;
                if (direction) {
                    position += offset;
                } else {
                    position -= offset;
                }
                return position;
            }

            inline void set_direction(bool dir) {
                delay_ns(DirSetupNS);
                Driver::set_direction(reverse ? dir : !dir);
                direction = dir;
                delay_ns(DirHoldNS);
            }

            static inline void delay_ns(uint32_t ns) {
                Delay::us((ns + 999) / 1000);
            }

        private:
            // Delays required by the stepper driver, in ticks
            uint16_t _step_pulse_ticks;
            uint16_t _step_space_ticks;
            uint16_t _dir_setup_ticks;
            uint16_t _dir_hold_ticks;

            // If should reverse direction signal
            bool reverse;

            // Current state
            algo::stepgen::stepgen stepgen;
            bool direction;

            uint32_t base_step;
            int32_t position;

            // Stop signal
            std::atomic_bool stop_requested;
        };
    }
}

#endif /* __HW_STEPPER_HPP */
