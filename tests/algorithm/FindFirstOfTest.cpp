#include <cassert>
#include <vector>
#include <list>
#include <deque>
#include <array>
#include <string>
#include <map>
#include <algorithm>
#include <cctype>
#include <random>
#include <type_traits>
#include <raze/algorithm/find/FindFirstOf.h>

template <typename It1, typename It2>
void check_find_first_of(It1 first1, It1 last1, It2 first2, It2 last2) {
    auto std_res = std::find_first_of(first1, last1, first2, last2);
    auto simd_res = raze::algorithm::find_first_of(first1, last1, first2, last2);
    raze_assert(std_res == simd_res);
}

template <typename It1, typename It2, typename Pred>
void check_find_first_of(It1 first1, It1 last1, It2 first2, It2 last2, Pred pred) {
    auto std_res = std::find_first_of(first1, last1, first2, last2, pred);
    auto simd_res = raze::algorithm::find_first_of(first1, last1, first2, last2, pred);
    raze_assert(std_res == simd_res);
}

template <typename T>
void run_numeric_tests()
{
    {
        std::vector<T> a{ 1,2,3,4,5 };
        std::vector<T> b{ 3,5 };
        check_find_first_of(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::list<T> a{ 1,2,3,4,5 };
        std::list<T> b{ 4 };
        check_find_first_of(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::deque<T> a{ 10,20,30,40 };
        std::deque<T> b{ 30,100 };
        check_find_first_of(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::array<T, 6> a{ 5,4,3,2,1,0 };
        std::array<T, 3> b{ 7,4,9 };
        check_find_first_of(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::basic_string<T> a;
        if constexpr (std::is_same_v<T, char>)
            a = "abcdefg";
        else {
            a.resize(7);
            for (int i = 0; i < 7; ++i) a[i] = T(i + 1);
        }
        std::vector<T> b{ a[2], a[5] };
        check_find_first_of(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::map<T, T> m{ {T(1),T(10)}, {T(2),T(20)}, {T(3),T(30)} };
        std::vector<T> keys;
        for (auto& kv : m) keys.push_back(kv.first);
        std::vector<T> b{ T(2),T(9) };
        check_find_first_of(keys.begin(), keys.end(), b.begin(), b.end());
    }
    {
        if constexpr (!std::is_floating_point_v<T>) {
            std::vector<T> a{ 1,2,3,4,5 };
            std::vector<T> b{ 2,3,4 };
            auto pred = [](T x, T y) { return (x % 3) == (y % 3); };
            check_find_first_of(a.begin(), a.end(), b.begin(), b.end(), pred);
        }
    }
    {
        if constexpr (std::is_same_v<T, char>) {
            std::string s = "CaseInsensitive";
            std::string chars = "ei";
            auto pred = [](char a, char b) { return std::tolower(a) == std::tolower(b); };
            check_find_first_of(s.begin(), s.end(), chars.begin(), chars.end(), pred);
        }
    }
    {
        for (std::size_t n = 1; n < 200; ++n) {
            std::vector<T> a(n);
            for (std::size_t i = 0; i < n; ++i) a[i] = T(i % 11);
            std::vector<T> b{ T(3),T(7),T(9) };
            check_find_first_of(a.begin(), a.end(), b.begin(), b.end());
        }
    }
    {
        const std::size_t N = 200000;
        std::vector<T> a(N);
        for (std::size_t i = 0; i < N; ++i) a[i] = T(i % 17);
        std::vector<T> b{ T(5),T(11),T(16) };
        check_find_first_of(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::mt19937_64 rng(123456);
        std::uniform_int_distribution<uint64_t> dist_int;
        std::uniform_int_distribution<uint64_t> dist_length(0, 10000);
        std::uniform_real_distribution<double> dist_fp(-1000.0, 1000.0);

        for (int iter = 0; iter < 1'000'000; ++iter) {
            const std::size_t N = dist_length(rng);
            std::vector<T> a(N);

            if constexpr (std::is_floating_point_v<T>) {
                for (std::size_t i = 0; i < N; ++i) a[i] = T(dist_fp(rng));
            }
            else {
                for (std::size_t i = 0; i < N; ++i) a[i] = T(dist_int(rng));
            }

            std::vector<T> b;
            int bsize = (rng() % 8) + 1;
            b.reserve(bsize);

            for (int i = 0; i < bsize; ++i) {
                if constexpr (std::is_floating_point_v<T>)
                    b.push_back(T(dist_fp(rng)));
                else
                    b.push_back(T(dist_int(rng)));
            }

            check_find_first_of(a.begin(), a.end(), b.begin(), b.end());
        }
    }
}

int main() {
    run_numeric_tests<int8_t>();
    run_numeric_tests<uint8_t>();
    run_numeric_tests<int16_t>();
    run_numeric_tests<uint16_t>();
    run_numeric_tests<int32_t>();
    run_numeric_tests<uint32_t>();
    run_numeric_tests<int64_t>();
    run_numeric_tests<uint64_t>();
    run_numeric_tests<float>();
    run_numeric_tests<double>();
    run_numeric_tests<char>();
    return 0;
}
