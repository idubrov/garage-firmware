#ifndef __UTIL_HPP
#define __UTIL_HPP

#include <utility>
#include <limits>

namespace util {
    template<typename N, int R = 10>
    constexpr int digits(N value = ::std::numeric_limits<N>::max(), int result = 0) {
        return value == 0
               ? result
               : digits<N, R>(value / R, result + 1);
    }

    constexpr unsigned c_strlen(char const *str, unsigned count = 0) {
        return ('\0' == str[0])
               ? count
               : c_strlen(str + 1, count + 1);
    }

    template<typename T>
    constexpr T ns2us(T ns) {
        return (ns + 999) / 1000;
    }

    template<typename T>
    class ranged_value {
    public:
        ranged_value(T value, T min, T max, bool wrap) :
                value(value), min(min), max(max), wrap(wrap) {

        }

        explicit operator T() const {
            return value;
        }

        explicit operator T &() {
            return value;
        }

        T get() const {
            return value;
        }

        T &get() {
            return value;
        }

        // FIXME: document?
        template<typename D>
        ranged_value &operator+=(D d) {
            if (d == 0) {
                return *this;
            }

            d %= static_cast<D>(max - min + 1);
            if (d > 0) {
                if (d <= (max - value)) {
                    value += d;
                } else {
                    if (wrap) {
                        value = min + (d - (max - value) - 1);
                    } else {
                        value = max;
                    }
                }
            } else {
                d = -d;
                if (d <= value - min) {
                    value -= d;
                } else {
                    if (wrap) {
                        value = max - (d - (value - min) - 1);
                    } else {
                        value = min;
                    }
                }
            }
            return *this;
        }

    private:
        T value;
        T min;
        T max;
        bool wrap;
    };

    template<typename T>
    ranged_value<T> ranged(T value, T min, T max, bool wrap = true) {
        return
                {value, min, max, wrap};
    }
}

#endif /* __UTIL_HPP */
