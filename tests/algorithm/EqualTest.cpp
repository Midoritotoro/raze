#include <cassert>
#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <iostream>
#include <raze/algorithm/find/Equal.h>


template <typename It1, typename It2>
void check_equal(It1 first1, It1 last1, It2 first2, It2 last2) {
    bool std_res = std::equal(first1, last1, first2, last2);
    bool simd_res = raze::algorithm::equal(first1, last1, first2, last2);
    raze_assert(std_res == simd_res);
}

template <typename It1, typename It2, typename Pred>
void check_equal(It1 first1, It1 last1, It2 first2, It2 last2, Pred pred) {
    bool std_res = std::equal(first1, last1, first2, last2, pred);
    bool simd_res = raze::algorithm::equal(first1, last1, first2, last2, pred);
    raze_assert(std_res == simd_res);
}

int main() {
    {
        std::vector<int> a, b;
        check_equal(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a{ 42 }, b{ 42 };
        check_equal(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a{ 42 }, b{ 43 };
        check_equal(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a{ 1,2,3 };
        std::vector<int> b{ 1,2 };
        check_equal(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a(1000, 7), b(1000, 7);
        check_equal(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a{ 0,1,2,3,4 };
        std::vector<int> b{ 9,1,2,3,4 };
        check_equal(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a{ 1,2,3,4,5 };
        std::vector<int> b{ 1,2,9,4,5 };
        check_equal(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a{ 1,2,3,4,5 };
        std::vector<int> b{ 1,2,3,4,9 };
        check_equal(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::string s1 = "abcdef";
        std::string s2 = "abcdef";
        check_equal(s1.begin(), s1.end(), s2.begin(), s2.end());
    }
    {
        std::string s1 = "abcdef";
        std::string s2 = "abcxef";
        check_equal(s1.begin(), s1.end(), s2.begin(), s2.end());
    }
    {
        std::vector<int> a{ 2,4,6 };
        std::vector<int> b{ 0,2,8 };
        auto pred = [](int x, int y) { return (x % 2) == (y % 2); };
        check_equal(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::list<int> a{ 1,2,3,4,5 };
        std::vector<int> b{ 1,2,3,4,5 };
        check_equal(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a(10000), b(10000);
        for (int i = 0; i < 10000; ++i) {
            a[i] = i % 123;
            b[i] = i % 123;
        }
        check_equal(a.begin(), a.end(), b.begin(), b.end());
    }

    return 0;
}
