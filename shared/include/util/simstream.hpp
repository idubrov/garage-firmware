#ifndef __SIMSTREAM_HPP
#define __SIMSTREAM_HPP

#include "util.hpp"

template<typename S>
S const &operator<<(S const &sink, char c) {
    sink.write(c);
    return sink;
}

template<typename S>
S const &operator<<(S const &sink, char const *str) {
    while (*str) {
        sink << *str++;
    }
    return sink;
}

enum Alignment {
    Left, Right
};

template<typename N, int R, Alignment A, char Pad>
struct print_format {
    N value;
    int padding;
    int decimal;
};

template<int Radix = 10, Alignment Align = Left, char Pad = ' '>
print_format<int, Radix, Align, Pad> format(int value, int padding = 0,
                                            int decimal = 0) {
    return {value, padding, decimal};
}

template<typename S, typename N>
auto operator<<(S const &sink, N n) -> decltype((void) format<>(n), sink) {
    sink << format<>(n);
    return sink;
}

template<typename S, typename T>
S const &operator<<(S const &sink, util::ranged_value<T> n) {
    sink << format<>(n.get());
    return sink;
}

template<typename S, typename Printable>
auto operator<<(S const &sink, Printable const &printable) -> decltype((void) printable.print(sink), sink) {
    printable.print(sink);
    return sink;
}


template<typename N, int R>
constexpr int buf_size(N value = std::numeric_limits<int>::max(),
                       int result = 0) {
    return value == 0 ? result : buf_size<N, R>(value / R, result + 1);
}

template<typename S, typename N, int R = 10, Alignment A, char P>
S const &operator<<(S const &sink, print_format<N, R, A, P> nn) {
    constexpr int size = util::digits<N, R>();
    char buf[size];
    int pos = size;
    int n = nn.value;
    int padding = nn.padding;

    // FIXME: won't work for min_value
    bool negative = (n < 0);
    if (n < 0)
        n = -n;
    do {
        int m = n;
        n /= R;
        char c = m - R * n;
        buf[--pos] = c < 10 ? c + '0' : c + 'a' - 10;
    } while (n);

    int digits = size - pos;
    int printed = negative ? 1 : 0;
    if (nn.decimal > 0) {
        printed++; // Decimal dot
        printed += (digits <= nn.decimal ? (nn.decimal + 1) : digits); // plus leading zero
    } else
        printed += digits;
    if (A == Right) {
        for (int i = printed; i < padding; i++)
            sink << P;
    }
    if (negative)
        sink << '-';

    // Leading zeroes
    for (int i = 0; i <= nn.decimal - digits; i++) {
        if (i == 1)
            sink << '.';
        sink << '0';
    }
    while (pos < size) {
        if (pos == size - nn.decimal)
            sink << '.';
        sink << buf[pos++];
    }

    if (A == Left) {
        for (int i = printed; i < padding; i++)
            sink << P;
    }

    return sink;
}

struct print_blanks {
    unsigned blanks;
};

inline print_blanks blanks(unsigned blanks) {
    return
            {blanks};
}

template<typename S>
S const &operator<<(S const &sink, print_blanks b) {
    for (unsigned i = 0; i < b.blanks; i++)
        sink << ' ';
    return sink;
}

#endif /* __SIMSTREAM_HPP */
