#define NOMINMAX

#include <raze/math/BitMath.h>
#include <src/raze/utility/Assert.h>

#include <iostream>
#include <limits>

void test_popcount_unsigned_int() {
    raze_assert(raze::math::population_count(0u) == 0);
    raze_assert(raze::math::population_count(1u) == 1);

    raze_assert(raze::math::population_count(2u) == 1);
    raze_assert(raze::math::population_count(3u) == 2);

    raze_assert(raze::math::population_count(0x80000000u) == 1);
    raze_assert(raze::math::population_count(0b00101000u) == 2);
    raze_assert(raze::math::population_count(0b10101010u) == 4);

    raze_assert(raze::math::population_count(std::numeric_limits<unsigned int>::max()) == std::numeric_limits<unsigned int>::digits); // 0b111...111
}

void test_popcount_unsigned_long_long() {
    raze_assert(raze::math::population_count(0ULL) == 0);
    raze_assert(raze::math::population_count(1ULL) == 1);

    raze_assert(raze::math::population_count(3ULL) == 2);
    raze_assert(raze::math::population_count(0x8000000000000000ULL) == 1);
    raze_assert(raze::math::population_count(0b0010100000000000000000000000000000000000000000000000000000000000ULL) == 2);

    raze_assert(raze::math::population_count(0b1101101101101101101101101101101101101101101101101101101101101101ULL) == 43);
    raze_assert(raze::math::population_count(std::numeric_limits<unsigned long long>::max()) == std::numeric_limits<unsigned long long>::digits); // 0b111...111
}

void test_popcount_signed_int_non_negative() {
    raze_assert(raze::math::population_count(0U) == 0);
    raze_assert(raze::math::population_count(7U) == 3);
    raze_assert(raze::math::population_count(0x40000001U) == 2);
}

int main() {
    test_popcount_unsigned_int();
    test_popcount_unsigned_long_long();
    test_popcount_signed_int_non_negative();

    return 0;
}