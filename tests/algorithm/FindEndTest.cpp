#include <raze/algorithm/find/FindEnd.h>
#include <algorithm>
#include <vector>
#include <string>
#include <list>
#include <deque>
#include <cctype>

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
        for (size_t i = 3000; i < 3000 + b.size(); ++i)
            a[i] = 2;
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
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
        for (size_t i = 256; i < 256 + 80; ++i)
            s[i] = 'b';
        std::string sub(80, 'b');
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
