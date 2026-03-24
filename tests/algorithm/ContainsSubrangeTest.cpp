#include <raze/algorithm/find/ContainsSubrange.h>
#include <algorithm>
#include <ranges>
#include <vector>
#include <string>
#include <list>
#include <deque>
#include <cctype>
#include <random>

template <typename It1, typename It2>
void check_contains(It1 first1, It1 last1, It2 first2, It2 last2) {
    bool std_res = std::ranges::contains_subrange(
        std::ranges::subrange(first1, last1),
        std::ranges::subrange(first2, last2)
    );

    bool simd_res = raze::algorithm::contains_subrange(first1, last1, first2, last2);

    raze_assert(std_res == simd_res);
}

template <typename It1, typename It2, typename Pred>
void check_contains(It1 first1, It1 last1, It2 first2, It2 last2, Pred pred) {
    bool std_res = std::ranges::contains_subrange(
        std::ranges::subrange(first1, last1),
        std::ranges::subrange(first2, last2),
        pred
    );

    bool simd_res = raze::algorithm::contains_subrange(first1, last1, first2, last2, pred);

    raze_assert(std_res == simd_res);
}

template <class _Type_>
void run_contains_tests_for_type() {
    {
        std::vector<_Type_> a, b;
        check_contains(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,3,4,5 };
        std::vector<_Type_> b{ 1,2,3,4,5 };
        check_contains(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,3,4,5 };
        std::vector<_Type_> b{ 1,2 };
        check_contains(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,3,4,5 };
        std::vector<_Type_> b{ 4,5 };
        check_contains(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,3,4,5 };
        std::vector<_Type_> b{ 9,9 };
        check_contains(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,1,2,2,3,1,1,3,1,3 };
        std::vector<_Type_> b{ 1,2,3 };
        check_contains(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::list<_Type_> a{ 1,2,3,4,5,6 };
        std::list<_Type_> b{ 3,4 };
        check_contains(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a(10000, 1);
        a[5000] = 42; a[5001] = 43; a[5002] = 44;
        std::vector<_Type_> b{ 42,43,44 };
        check_contains(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,3 };
        std::vector<_Type_> b{ 1,2,3,4,5 };
        check_contains(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 7,7,7,7,7,7 };
        std::vector<_Type_> b{ 7,7,7 };
        check_contains(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,3,4,5 };
        std::vector<_Type_> b{ 3 };
        check_contains(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::deque<_Type_> a{ 10,20,30,40,50,60 };
        std::deque<_Type_> b{ 30,40,50 };
        check_contains(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a(200000, 3);
        a[199000] = 9; a[199001] = 10; a[199002] = 11;
        std::vector<_Type_> b{ 9,10,11 };
        check_contains(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 'a','b','c','d','e' };
        std::vector<_Type_> b{ 'd','e','f' };
        check_contains(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a(4096, 1);
        std::vector<_Type_> b(128, 2);
        for (size_t i = 3000; i < 3000 + b.size(); ++i)
            a[i] = 2;
        check_contains(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a(69, 0);
        a[60] = 1; a[61] = 2; a[62] = 3; a[63] = 4; a[64] = 5;
        std::vector<_Type_> b{ 1,2,3,4,5 };
        check_contains(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a(79, 7);
        a[70] = 1; a[71] = 2; a[72] = 3; a[73] = 4;
        std::vector<_Type_> b{ 1,2,3,4 };
        check_contains(a.begin(), a.end(), b.begin(), b.end());
    }

    {
        std::mt19937 rng(123456);
        std::uniform_int_distribution<int> len_dist(0, 10000);
        std::uniform_int_distribution<int> val_dist(-1000000, 1000000);

        const size_t iterations = 1'000'000;

        for (size_t i = 0; i < iterations; ++i) {
            size_t main_len = len_dist(rng);
            size_t sub_len = len_dist(rng);

            std::vector<_Type_> a(main_len);
            std::vector<_Type_> b(sub_len);

            for (auto& x : a) x = static_cast<_Type_>(val_dist(rng));
            for (auto& x : b) x = static_cast<_Type_>(val_dist(rng));

            check_contains(a.begin(), a.end(), b.begin(), b.end());
        }
    }
}

void run_contains_tests_for_string() {
    {
        std::string s = "Hello World";
        std::string sub = "World";
        check_contains(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::string s = "abcdef";
        std::string sub = "gh";
        check_contains(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::string s = "CaseInsensitive";
        std::string sub = "insensitive";
        auto pred = [](char a, char b) { return std::tolower(a) == std::tolower(b); };
        check_contains(s.begin(), s.end(), sub.begin(), sub.end(), pred);
    }
    {
        std::string s = "aaaaab";
        std::string sub = "aaab";
        check_contains(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::string s = "абракадабра";
        std::string sub = "када";
        check_contains(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::string s = "xyzxyzxyzxyz";
        std::string sub = "xyzxyz";
        check_contains(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::string s = "pattern_at_the_end123";
        std::string sub = "end123";
        check_contains(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::string s(1024, 'a');
        for (size_t i = 256; i < 256 + 80; ++i)
            s[i] = 'b';
        std::string sub(80, 'b');
        check_contains(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::string s(97, 'x');
        s.replace(90, 7, "pattern");
        std::string sub = "pattern";
        check_contains(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::string s(79, 'q');
        s.replace(72, 5, "abcde");
        std::string sub = "abcde";
        check_contains(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::string s = "aaaaaaaaaaaaaaaa"
            "bbbbbbbbbbbbbbbb";
        std::string sub = "bbbb";
        check_contains(s.begin(), s.end(), sub.begin(), sub.end());
    }
}

int main() {
    run_contains_tests_for_string();

    run_contains_tests_for_type<char>();
    run_contains_tests_for_type<short>();
    run_contains_tests_for_type<int>();
    run_contains_tests_for_type<long long>();

    run_contains_tests_for_type<unsigned char>();
    run_contains_tests_for_type<unsigned short>();
    run_contains_tests_for_type<unsigned int>();
    run_contains_tests_for_type<unsigned long long>();

    run_contains_tests_for_type<float>();
    run_contains_tests_for_type<double>();

    return 0;
}
