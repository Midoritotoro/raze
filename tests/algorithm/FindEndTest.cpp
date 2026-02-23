#include <raze/algorithm/find/FindEnd.h>
#include <algorithm>
#include <vector>
#include <string>
#include <list>
#include <deque>
#include <random>
#include <cctype>
#include <iostream>


template <typename It1, typename It2>
void check_find_end(It1 first1, It1 last1, It2 first2, It2 last2) {
    auto std_res = std::find_end(first1, last1, first2, last2);
    auto simd_res = raze::algorithm::find_end(first1, last1, first2, last2);
    raze_assert(std_res == simd_res);
}

template <typename It1, typename It2, typename Pred>
void check_find_end(It1 first1, It1 last1, It2 first2, It2 last2, Pred pred) {
    auto std_res = std::find_end(first1, last1, first2, last2, pred);
    auto simd_res = raze::algorithm::find_end(first1, last1, first2, last2, pred);
    raze_assert(std_res == simd_res);
}

template <class T>
void fill_random(std::vector<T>& v, int seed = 12345) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, 255);
    for (auto& x : v) x = static_cast<T>(dist(rng));
}

template <class _Type_>
void run_find_end_tests_for_type() {
    {
        std::vector<_Type_> a, b;
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,3,4,5 };
        std::vector<_Type_> b{ 1,2,3,4,5 };
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,3,4,5 };
        std::vector<_Type_> b{ 1,2 };
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,3,4,5 };
        std::vector<_Type_> b{ 4,5 };
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,3,4,5 };
        std::vector<_Type_> b{ 9,9 };
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,1,2,3,1,2,3,4 };
        std::vector<_Type_> b{ 1,2,3 };
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::list<_Type_> a{ 1,2,3,4,5,6 };
        std::list<_Type_> b{ 3,4 };
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a(10000, 1);
        a[5000] = 42; a[5001] = 43; a[5002] = 44;
        std::vector<_Type_> b{ 42,43,44 };
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,3 };
        std::vector<_Type_> b{ 1,2,3,4,5 };
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 7,7,7,7,7,7 };
        std::vector<_Type_> b{ 7,7,7 };
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,3,4,5 };
        std::vector<_Type_> b{ 3 };
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::deque<_Type_> a{ 10,20,30,40,50,60 };
        std::deque<_Type_> b{ 30,40,50 };
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a(200000, 3);
        a[199000] = 9; a[199001] = 10; a[199002] = 11;
        std::vector<_Type_> b{ 9,10,11 };
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<unsigned char> a(4096, 1);
        std::vector<unsigned char> b(128, 2);
        for (size_t i = 3000; i < 3000 + b.size(); ++i) a[i] = 2;
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a(1000, 1);
        std::vector<_Type_> b{ 2,2,2,2,2,2 };
        size_t pos = 400;
        for (size_t i = 0; i < 3; ++i) a[pos + i] = 2;
        for (size_t i = 3; i < 6; ++i) a[pos + i] = 3;
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a(5000);
        std::vector<_Type_> b{ 5,7,9,11 };
        for (size_t i = 0; i < a.size(); ++i) {
            if (i % 3 == 0) a[i] = 5;
            else if (i % 5 == 0) a[i] = 11;
            else a[i] = static_cast<_Type_>(100 + (i % 50));
        }
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a(2000), b(32);
        fill_random(a);
        fill_random(b);
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a(4096);
        for (size_t i = 0; i < a.size(); ++i) a[i] = (i % 4);
        std::vector<_Type_> b{ 1,2,3 };
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,3,4,5,6,7,8,9 };
        std::vector<_Type_> b{ 5 };
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a(2000, 1);
        std::vector<_Type_> b(64, 2);
        for (size_t i = 1500; i < 1500 + 64; ++i) a[i] = 2;
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a(3000, 7);
        std::vector<_Type_> b(32, 7);
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,1,1,1,1,1,1 };
        std::vector<_Type_> b{ 1,1,1,1 };
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a(4096, 1);
        std::vector<_Type_> b(128, 1);
        b[127] = 2;
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a(4096);
        std::vector<_Type_> b{ 5,6,7,8,9 };
        for (size_t i = 0; i < a.size(); ++i) {
            if (i % 2 == 0) a[i] = 5;
            else if (i % 3 == 0) a[i] = 9;
            else a[i] = 100;
        }
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::mt19937 rng(123456);
        std::uniform_int_distribution<int> len_dist(0, 2000);
        std::uniform_int_distribution<int> val_dist(-1000000, 1000000);

        const size_t iterations = 1'000'000;

        for (size_t i = 0; i < iterations; ++i) {
            size_t main_len = len_dist(rng);
            size_t sub_len = len_dist(rng);

            std::vector<_Type_> a(main_len);
            std::vector<_Type_> b(sub_len);

            for (auto& x : a) x = static_cast<_Type_>(val_dist(rng));
            for (auto& x : b) x = static_cast<_Type_>(val_dist(rng));

            auto std_res = std::find_end(a.begin(), a.end(), b.begin(), b.end());
            auto simd_res = raze::algorithm::find_end(a.begin(), a.end(), b.begin(), b.end());

            raze_assert(std_res == simd_res);
        }
    }

}

void run_find_end_tests_for_string() {
    {
        std::string s = "Hello World";
        std::string sub = "World";
        check_find_end(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::string s = "abcdef";
        std::string sub = "gh";
        check_find_end(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::string s = "CaseInsensitive";
        std::string sub = "insensitive";
        auto pred = [](char a, char b) { return std::tolower(a) == std::tolower(b); };
        check_find_end(s.begin(), s.end(), sub.begin(), sub.end(), pred);
    }
    {
        std::string s = "aaaaab";
        std::string sub = "aaab";
        check_find_end(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::string s = "абракадабра";
        std::string sub = "када";
        check_find_end(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::string s = "xyzxyzxyzxyz";
        std::string sub = "xyzxyz";
        check_find_end(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::string s = "pattern_at_the_end123";
        std::string sub = "end123";
        check_find_end(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::string s(512, 'a');
        for (size_t i = 256; i < 336; ++i) s[i] = 'b';
        std::string sub(80, 'b');
        check_find_end(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::u8string s = u8"Привет мир мир мир";
        std::u8string sub = u8"мир";
        check_find_end(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::u8string s = u8"abc🔥🔥🔥xyz🔥🔥";
        std::u8string sub = u8"🔥🔥";
        check_find_end(s.begin(), s.end(), sub.begin(), sub.end());
    }

}

int main() {
    run_find_end_tests_for_string();

    run_find_end_tests_for_type<char>();
    run_find_end_tests_for_type<short>();
    run_find_end_tests_for_type<int>();
    run_find_end_tests_for_type<long long>();

    run_find_end_tests_for_type<unsigned char>();
    run_find_end_tests_for_type<unsigned short>();
    run_find_end_tests_for_type<unsigned int>();
    run_find_end_tests_for_type<unsigned long long>();

    run_find_end_tests_for_type<float>();
    run_find_end_tests_for_type<double>();

    return 0;
}
