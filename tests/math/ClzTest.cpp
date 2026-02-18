#define NOMINMAX

#include <raze/math/BitMath.h>
#include <src/raze/utility/Assert.h>

#include <iostream>
#include <limits>

#include <bit>


void test_clz_unsigned_int() {
    raze_assert(raze::math::count_leading_zero_bits(0u) == std::numeric_limits<unsigned int>::digits);
    raze_assert(raze::math::count_leading_zero_bits(1u) == std::numeric_limits<unsigned int>::digits - 1);

    raze_assert(raze::math::count_leading_zero_bits(2u) == std::numeric_limits<unsigned int>::digits - 2);
    raze_assert(raze::math::count_leading_zero_bits(0x80000000u) == 0);

    raze_assert(raze::math::count_leading_zero_bits(0x40000000u) == 1);
    raze_assert(raze::math::count_leading_zero_bits(0b00101000u) == std::numeric_limits<unsigned int>::digits - 6);

    raze_assert(raze::math::count_leading_zero_bits(std::numeric_limits<unsigned int>::max()) == 0);
}

void test_clz_unsigned_long_long() {
    raze_assert(raze::math::count_leading_zero_bits(0ULL) == std::numeric_limits<unsigned long long>::digits);
    raze_assert(raze::math::count_leading_zero_bits(1ULL) == std::numeric_limits<unsigned long long>::digits - 1);

    raze_assert(raze::math::count_leading_zero_bits(2ULL) == std::numeric_limits<unsigned long long>::digits - 2);
    raze_assert(raze::math::count_leading_zero_bits(0x8000000000000000ULL) == 0);

    raze_assert(raze::math::count_leading_zero_bits(0x4000000000000000ULL) == 1);
    raze_assert(raze::math::count_leading_zero_bits(0b0010100000000000000000000000000000000000000000000000000000000000ULL) == 2);

    raze_assert(raze::math::count_leading_zero_bits(std::numeric_limits<unsigned long long>::max()) == 0);
}

void test_clz_signed_int_non_negative() {
    raze_assert(raze::math::count_leading_zero_bits(1U) == (std::numeric_limits<unsigned int>::digits - 1));
    raze_assert(raze::math::count_leading_zero_bits(8U) == std::numeric_limits<unsigned int>::digits - 4);
    raze_assert(raze::math::count_leading_zero_bits(0x40000000U) == 1);
}

int main() {
    test_clz_unsigned_int();
    test_clz_unsigned_long_long();
    test_clz_signed_int_non_negative();

    return 0;
}