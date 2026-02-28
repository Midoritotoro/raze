#define _USE_STD_VECTOR_ALGORITHMS 0
#include <raze/algorithm/minmax/MinElement.h>
#include <algorithm>
#include <vector>
#include <string>
#include <list>
#include <deque>
#include <random>
#include <cctype>

template <typename It>
void check_min_element(It first, It last) {
    auto std_res = std::min_element(first, last);
    auto simd_res = raze::algorithm::min_element(first, last);
    raze_assert(std_res == simd_res);
}

template <typename It, typename Pred>
void check_min_element(It first, It last, Pred pred) {
    auto std_res = std::min_element(first, last, pred);
    auto simd_res = raze::algorithm::min_element(first, last, pred);
    raze_assert(std_res == simd_res);
}

template <class T>
void run_tests_for_type() {
    {
        std::vector<T> a;
        check_min_element(a.begin(), a.end());
    }
    {
        std::vector<T> a{ 1 };
        check_min_element(a.begin(), a.end());
    }
    {
        std::vector<T> a{ 1,2,3,4,5 };
        check_min_element(a.begin(), a.end());
    }
    {
        std::vector<T> a{ 5,4,3,2,1 };
        check_min_element(a.begin(), a.end());
    }
    {
        std::vector<T> a{ 1,1,1,1,1 };
        check_min_element(a.begin(), a.end());
    }
    {
        std::vector<T> a{ T(-5), T(-2),T(-9),T(-1),T(-3) };
        check_min_element(a.begin(), a.end());
    }
    {
        std::list<T> a{ 3,1,4,1,5,9 };
        check_min_element(a.begin(), a.end());
    }
    {
        std::deque<T> a{ 10,20,30,40,50 };
        check_min_element(a.begin(), a.end());
    }
    {
        std::vector<T> a(100, 5);
        a[50] = -999;
        check_min_element(a.begin(), a.end());
    }
    {
        std::vector<T> a(10000, 5);
        a[5000] = -999;
        check_min_element(a.begin(), a.end());
    }
    {
        std::vector<T> a(200000, 3);
        a[199999] = -777;
        check_min_element(a.begin(), a.end());
    }
    {
        std::vector<T> a{ 1,2,3,4,5 };
        auto pred = [](T a, T b) { return a > b; };
        check_min_element(a.begin(), a.end(), pred);
    }

    {
        std::mt19937 rng(123456);
        std::uniform_int_distribution<int> len_dist(0, 10000);
        std::uniform_int_distribution<int> val_dist(-1000000, 1000000);

        const size_t iterations = 1'000'000;

        for (size_t i = 0; i < iterations; ++i) {
            size_t len = len_dist(rng);
            std::vector<T> a(len);

            for (auto& x : a)
                x = static_cast<T>(val_dist(rng));

            auto std_res = std::min_element(a.begin(), a.end());
            auto simd_res = raze::algorithm::min_element(a.begin(), a.end());

            raze_assert(std_res == simd_res);
        }
    }
}

void run_tests_for_string() {
    {
        std::string s = "Hello";
        check_min_element(s.begin(), s.end());
    }
    {
        std::string s = "abcdef";
        check_min_element(s.begin(), s.end());
    }
    {
        std::string s = "CaseInsensitive";
        auto pred = [](char a, char b) { return std::tolower(a) < std::tolower(b); };
        check_min_element(s.begin(), s.end(), pred);
    }
    {
        std::string s(64, 'z');
        s[32] = 'a';
        check_min_element(s.begin(), s.end());
    }
    {
        std::string s(1024, 'z');
        s[512] = 'a';
        check_min_element(s.begin(), s.end());
    }
    {
        std::string s = "абракадабра";
        check_min_element(s.begin(), s.end());
    }
}

int main() {
    run_tests_for_string();

    run_tests_for_type<char>();
    run_tests_for_type<short>();
    run_tests_for_type<int>();
    run_tests_for_type<long long>();

    run_tests_for_type<unsigned char>();
    run_tests_for_type<unsigned short>();
    run_tests_for_type<unsigned int>();
    run_tests_for_type<unsigned long long>();

    run_tests_for_type<float>();
    run_tests_for_type<double>();

    return 0;
}
