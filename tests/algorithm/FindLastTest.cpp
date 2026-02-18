#include <cassert>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <cctype>
#include <raze/algorithm/find/FindLast.h>

template <typename It, typename T>
It stl_find_last(It first, It last, const T& value) {
    const auto cachedLast = last;

    while (last != first) {
        --last;

        if (*last == value)
            return last;
    }

    return cachedLast;
}

template <typename It, typename Pred>
It stl_find_last_if(It first, It last, Pred pred) {
    const auto cachedLast = last;

    while (last != first) {
        --last;

        if (pred(*last))
            return last;
    }

    return cachedLast;
}

template <typename It, typename Pred>
It stl_find_last_if_not(It first, It last, Pred pred) {
    const auto cachedLast = last;

    while (last != first) {
        --last;

        if (!pred(*last))
            return last;
    }

    return cachedLast;
}

template <typename T>
void run_tests_for_type() {
    {
        std::vector<T> v;
        auto simd = raze::algorithm::find_last(v.begin(), v.end(), T(42));
        auto stl = stl_find_last(v.begin(), v.end(), T(42));
        raze_assert(simd == stl);

        auto simd_if = raze::algorithm::find_last_if(v.begin(), v.end(), [](T x) { return x == T(42); });
        auto stl_if = stl_find_last_if(v.begin(), v.end(), [](T x) { return x == T(42); });
        raze_assert(simd_if == stl_if);

        auto simd_if_not = raze::algorithm::find_last_if_not(v.begin(), v.end(), [](T x) { return x == T(42); });
        auto stl_if_not = stl_find_last_if_not(v.begin(), v.end(), [](T x) { return x == T(42); });
        raze_assert(simd_if_not == stl_if_not);
    }

    {
        std::vector<T> v(64, T(0));
        v[15] = T(42);

        auto simd = raze::algorithm::find_last(v.begin(), v.end(), T(42));
        auto stl = stl_find_last(v.begin(), v.end(), T(42));
        raze_assert(simd == stl);

        auto simd_if = raze::algorithm::find_last_if(v.begin(), v.end(), [](T x) { return x == T(42); });
        auto stl_if = stl_find_last_if(v.begin(), v.end(), [](T x) { return x == T(42); });
        raze_assert(simd_if == stl_if);

        auto simd_if_not = raze::algorithm::find_last_if_not(v.begin(), v.end(), [](T x) { return x == T(42); });
        auto stl_if_not = stl_find_last_if_not(v.begin(), v.end(), [](T x) { return x == T(42); });
        raze_assert(simd_if_not == stl_if_not);
    }
}


int main() {
    run_tests_for_type<char>();
    run_tests_for_type<signed char>();
    run_tests_for_type<unsigned char>();
    run_tests_for_type<short>();
    run_tests_for_type<unsigned short>();
    run_tests_for_type<int>();
    run_tests_for_type<unsigned int>();
    run_tests_for_type<long>();
    run_tests_for_type<unsigned long>();
    run_tests_for_type<long long>();
    run_tests_for_type<unsigned long long>();


    {
        std::string s = "HelloHELLO";
        raze_assert(raze::algorithm::find_last(s.begin(), s.end(), 'L') == (s.begin() + 8));
        raze_assert(raze::algorithm::find_last_if(s.begin(), s.end(), [](char c) { return std::isupper(c); }) == s.end() - 1);
        raze_assert(raze::algorithm::find_last_if_not(s.begin(), s.end(), [](char c) { return std::isupper(c); }) == s.end() - 6);
    }

    {
        std::vector<uint64_t> v(16, 0xDEADBEEF);
        v[15] = 0xCAFEBABE;
        raze_assert(raze::algorithm::find_last(v.begin(), v.end(), 0xCAFEBABEull) == v.end() - 1);
    }

    return 0;
}
