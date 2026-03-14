#include <raze/algorithm/find/Contains.h>

#include <ranges>
#include <vector>
#include <array>
#include <numeric>
#include <iostream>
#include <random>


template <typename _IntType_>
void testContains() {
    {
        std::vector<_IntType_> v{ 1, 2, 3, 4, 5 };
        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 1) == true);
        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 5) == true);
        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 6) == false);
    }

    {
        std::vector<_IntType_> v{};
        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 1) == false);
    }

    {
        std::vector<_IntType_> v{ 42 };
        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 42) == true);
        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 0) == false);
    }

    {
        std::vector<_IntType_> v(128, 0);
        v[0] = 99;
        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 99) == true);
    }

    {
        std::vector<_IntType_> v(128, 0);
        v[127] = 77;
        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 77) == true);
    }

    {
        std::vector<_IntType_> v(128, 0);
        v[64] = 55;
        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 55) == true);
    }

    {
        std::vector<_IntType_> v(256, 123);
        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 999) == false);
    }

    {
        std::vector<_IntType_> v{ 1,2,3,4,5,6,7 };
        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 7) == true);
        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 8) == false);
    }

    {
        std::vector<_IntType_> v(1024, 0);
        v[0] = 11;
        v[777] = 22;
        v[1023] = 33;

        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 11) == true);
        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 22) == true);
        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 33) == true);
        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 44) == false);
    }

    {
        std::vector<_IntType_> v;
        v.reserve(512);
        for (int i = 0; i < 64; ++i)
        {
            v.push_back(1);
            v.push_back(2);
            v.push_back(3);
            v.push_back(4);
            v.push_back(5);
        }
        v.push_back(999);

        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 999) == true);
        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 12345) == false);
    }


    if constexpr (std::is_floating_point_v<_IntType_>)
    {
        std::vector<_IntType_> v{ 0.0, 1.0, -1.0, 3.14, 2.71 };

        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 3.14) == true);
        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 2.71) == true);
        raze_assert(raze::algorithm::contains(v.begin(), v.end(), 42.0) == false);

        v.push_back(std::numeric_limits<_IntType_>::quiet_NaN());
        raze_assert(raze::algorithm::contains(v.begin(), v.end(),
            std::numeric_limits<_IntType_>::quiet_NaN()) == false);
    }

    {
        std::mt19937_64 rng(0xBADC0FFEEULL);
        std::uniform_int_distribution<size_t> lenDist(0, 4096);
        std::uniform_int_distribution<long long> valDist(
            std::numeric_limits<long long>::min() / 2,
            std::numeric_limits<long long>::max() / 2
        );

        for (int iter = 0; iter < 1'000'000; ++iter)
        {
            size_t len = lenDist(rng);

            std::vector<_IntType_> vec;
            vec.reserve(len);

            for (size_t i = 0; i < len; ++i)
                vec.push_back(static_cast<_IntType_>(valDist(rng)));

            _IntType_ target = static_cast<_IntType_>(valDist(rng));

            auto simd_res = raze::algorithm::contains(vec.begin(), vec.end(), target);
            auto std_res = std::ranges::contains(vec.begin(), vec.end(), target);

            raze_assert(simd_res == std_res);
        }
    }
}

int main() {
    testContains<char>();
    testContains<unsigned char>();
    testContains<short>();
    testContains<unsigned short>();
    testContains<int>();
    testContains<unsigned int>();
    testContains<long long>();
    testContains<unsigned long long>();
    testContains<float>();
    testContains<double>();

    return 0;
}
