#ifndef __ALGO_STEPGEN_HPP
#define __ALGO_STEPGEN_HPP

#include <cstdint>
#include <atomic>

namespace algo {
    namespace stepgen {
        /**
         * Uses algorithm from "Generate stepper-motor speed pro les in real time" paper by David Austin.
         */
        class stepgen {
        public:
            // How many timer ticks it would take for one update
            constexpr static uint32_t TicksPerUpdate = 10;
        public:
            stepgen(uint32_t ticks_per_second) :
                    _step(0), _speed(0), _delay(0), _slewing_delay(0),
                    _ticks_per_second(ticks_per_second),
                    _first_delay(0), _tgt_step(0), _tgt_delay(0) {
            }

            stepgen &operator=(const stepgen &) = default;

            // Configuration methods. Should not be called while motor is running.


            /**
             * Set stepper motor acceleration. Note that this method is computation intensive,
             * so it's best to set acceleration once and never change it.
             * @param acceleration acceleration, in steps per second per second, in 24.8 format
             * @return false if acceleration is too slow and thus delay of the first
             * step would not fit into 16 bit timer counter.
             */
            bool set_acceleration(uint32_t acceleration);

            // Motor control API. Could be called while motor is running.

            /**
             * Returns '0' if should stop. Otherwise, returns timer delay in 24.8 format
             * @return timer delay in 24.8 format.
             */
            uint32_t next();


            /**
             * Set destination step for the stepper motor pulse generator. This is
             * the primary constraint used to determine necessary action to take.
             * If current step > target step, stepper motor would slow down until stop
             * if running or stay stopped if not running.
             * Setting target step to 0 will always force stepper to decelerate and stop.
             * @param step step to stop at
             */
            void set_target_step(uint32_t step) {
                _tgt_step.store(step, std::memory_order_relaxed);
            }

            /**
             * Set slew speed (maximum speed stepper motor would run). Note that stepper
             * motor would only reach this speed if target step is far enough, so there is
             * enough time for deceleration.
             * @param speed target slew speed to reach, in steps per second, 24.8 format
             * @return false if target speed is either too slow (doesn't fit into timer counter)
             * or too fast.
             */
            bool set_target_speed(uint32_t speed);

            // State query commands, should only be used for displaying purposes.
            // These shouldn't be used for making decisions, as they are not thread-safe

            inline uint32_t current_step() const {
                return _step.load(std::memory_order_relaxed);
            }

            inline uint32_t target_step() const {
                return _tgt_step.load(std::memory_order_relaxed);
            }

            /**
             * Estimated current speed
             * @return estimated current speed in 24.8 format
             */
            uint32_t current_speed() const;

            inline uint32_t target_delay() const {
                return _tgt_delay.load(std::memory_order_relaxed) >> 8;
            }

        private:
            static uint64_t sqrt(uint64_t x);

            void speedup();

            void slowdown();

        private:
            // State, updated in the IRQ handler
            std::atomic_uint_fast32_t _step;    // Current step

            // These two are not used outside of the IRQ handler
            uint32_t _speed; // Amount of acceleration steps we've taken so far
            uint32_t _delay; // Previously calculated delay, in 16.16 format

            // If slewing, this will be the slewing delay. Switched to this mode once
            // we overshoot target speed. 16.16 format.
            uint32_t _slewing_delay;

            // Parameters
            uint32_t _ticks_per_second; // Timer frequency
            uint32_t _first_delay; // First step delay, in 16.16 format

            // These two could be changed from outside
            std::atomic_uint_fast32_t _tgt_step; // Target step
            std::atomic_uint_fast32_t _tgt_delay; // Target speed delay, in 16.16 format
        };
    }
}

#endif /* __ALGO_STEPGEN_HPP */
