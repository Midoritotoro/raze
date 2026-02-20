#include <raze/algorithm/find/Search.h>
#include <algorithm>
#include <vector>
#include <string>
#include <list>
#include <deque>
#include <cctype>

template <typename It1, typename It2>
void check_search(It1 first1, It1 last1, It2 first2, It2 last2) {
    auto std_res = std::search(first1, last1, first2, last2);
    auto simd_res = raze::algorithm::search(first1, last1, first2, last2);
    raze_assert(std_res == simd_res);
}

template <typename It1, typename It2, typename Pred>
void check_search(It1 first1, It1 last1, It2 first2, It2 last2, Pred pred) {
    auto std_res = std::search(first1, last1, first2, last2, pred);
    auto simd_res = raze::algorithm::search(first1, last1, first2, last2, pred);
    raze_assert(simd_res == std_res);
}

template <class _Type_>
void run_tests_for_type() {
    {
        std::vector<_Type_> a, b;
        check_search(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,3,4,5 };
        std::vector<_Type_> b{ 1,2,3,4,5 };
        check_search(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,3,4,5 };
        std::vector<_Type_> b{ 1,2 };
        check_search(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,3,4,5 };
        std::vector<_Type_> b{ 4,5 };
        check_search(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,3,4,5 };
        std::vector<_Type_> b{ 9,9 };
        check_search(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,1,2,3,1,2,3,4 };
        std::vector<_Type_> b{ 1,2,3 };
        check_search(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::list<_Type_> a{ 1,2,3,4,5,6 };
        std::list<_Type_> b{ 3,4 };
        check_search(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a(10000, 1);
        a[5000] = 42; a[5001] = 43; a[5002] = 44;
        std::vector<_Type_> b{ 42,43,44 };
        check_search(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,3 };
        std::vector<_Type_> b{ 1,2,3,4,5 };
        check_search(a.begin(), a.end(), b.begin(), b.end());
    }

    {
        std::vector<_Type_> a{ 7,7,7,7,7,7 };
        std::vector<_Type_> b{ 7,7,7 };
        check_search(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 1,2,3,4,5 };
        std::vector<_Type_> b{ 3 };
        check_search(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::deque<_Type_> a{ 10,20,30,40,50,60 };
        std::deque<_Type_> b{ 30,40,50 };
        check_search(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a(200000, 3);
        a[199000] = 9; a[199001] = 10; a[199002] = 11;
        std::vector<_Type_> b{ 9,10,11 };
        check_search(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a{ 'a','b','c','d','e' };
        std::vector<_Type_> b{ 'd','e','f' };
        check_search(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<_Type_> a(4096, 1);
        std::vector<_Type_> b(128, 2);
        for (size_t i = 3000; i < 3000 + b.size(); ++i)
            a[i] = 2;
        check_search(a.begin(), a.end(), b.begin(), b.end());
    }

    {
        std::vector<_Type_> a(65, 0);
        a[60] = 1; a[61] = 2; a[62] = 3; a[63] = 4; a[64] = 5;
        std::vector<_Type_> b{ 1,2,3,4,5 };
        check_search(a.begin(), a.end(), b.begin(), b.end());
    }

    {
        std::vector<_Type_> a(79, 7);
        a[70] = 1; a[71] = 2; a[72] = 3; a[73] = 4;
        std::vector<_Type_> b{ 1,2,3,4 };
        check_search(a.begin(), a.end(), b.begin(), b.end());
    }
}

void run_tests_for_string() {
    {
        std::string s = "Hello World";
        std::string sub = "World";
        check_search(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::string s = "abcdef";
        std::string sub = "gh";
        check_search(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::string s = "CaseInsensitive";
        std::string sub = "insensitive";
        auto pred = [](char a, char b) { return std::tolower(a) == std::tolower(b); };
        check_search(s.begin(), s.end(), sub.begin(), sub.end(), pred);
    }

    {
        std::string s = "aaaaab";
        std::string sub = "aaab";
        check_search(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::string s = "абракадабра";
        std::string sub = "када";
        check_search(s.begin(), s.end(), sub.begin(), sub.end());
    }

    {
        std::string s = "xyzxyzxyzxyz";
        std::string sub = "xyzxyz";
        check_search(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::string s = "pattern_at_the_end123";
        std::string sub = "end123";
        check_search(s.begin(), s.end(), sub.begin(), sub.end());
    }
    {
        std::string s(1024, 'a');
        for (size_t i = 256; i < 256 + 80; ++i)
            s[i] = 'b'; std::string sub(80, 'b');
        check_search(s.begin(), s.end(), sub.begin(), sub.end());
    }

    {
        std::string s(97, 'x');
        s.replace(90, 7, "pattern");
        std::string sub = "pattern";
        check_search(s.begin(), s.end(), sub.begin(), sub.end());
    }

    {
        std::string s(79, 'q');
        s.replace(72, 5, "abcde");
        std::string sub = "abcde";
        check_search(s.begin(), s.end(), sub.begin(), sub.end());
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
