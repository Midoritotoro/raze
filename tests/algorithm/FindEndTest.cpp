#include <cassert>
#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <iostream>
#include <cctype>
#include <random>

#include <raze/algorithm/find/FindEnd.h>

template <typename It1, typename It2>
void check_find_end(It1 first1, It1 last1, It2 first2, It2 last2) {
    auto std_res  = std::find_end(first1, last1, first2, last2);
    auto simd_res = raze::algorithm::find_end(first1, last1, first2, last2);
    raze_assert(std_res == simd_res);
}

template <typename It1, typename It2, typename Pred>
void check_find_end(It1 first1, It1 last1, It2 first2, It2 last2, Pred pred) {
    auto std_res  = std::find_end(first1, last1, first2, last2, pred);
    auto simd_res = raze::algorithm::find_end(first1, last1, first2, last2, pred);
    raze_assert(std_res == simd_res);
}

int main() {
    {
        std::vector<char> a;
        std::vector<char> b;
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<char> a(64, 'x');
        std::vector<char> b;
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<char> a;
        std::vector<char> b(64, 'y');
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<char> a(128, 'a');
        std::vector<char> b(128, 'a');
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<char> a(96, 'x');
        std::vector<char> b = {'x','x','x','x','x','x','x','x'};
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<char> a(128, 'z');
        std::vector<char> b(4, 'z');
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<char> a(128, 'a');
        std::vector<char> b(8, 'b');
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<char> a(64, 'a');
        std::vector<char> b(128, 'a');
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }

    {
        std::string s(64, 'A');
        std::string needle = "AAA";
        check_find_end(s.begin(), s.end(), needle.begin(), needle.end());
    }
    {
        std::string s = "helloHELLO";
        std::string needle = "hello";
        auto pred = [](char a, char b){ return std::tolower(a) == std::tolower(b); };
        check_find_end(s.begin(), s.end(), needle.begin(), needle.end(), pred);
    }
    {
        std::string s = "abc...abc...abc";
        std::string needle = "abc";
        check_find_end(s.begin(), s.end(), needle.begin(), needle.end());
    }

    {
        std::vector<int> a(64, 1);
        std::vector<int> b{1,1,1,1,1,1,1,1};
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a(64, 0);
        for (size_t i = 8; i + 4 <= a.size(); i += 16) {
            a[i+0] = 7; a[i+1] = 7; a[i+2] = 7; a[i+3] = 7;
        }
        std::vector<int> b{7,7,7,7};
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a(128, 2);
        std::vector<int> b(16, 3);
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a(64);
        std::vector<int> b(8);
        for (int i = 0; i < 64; ++i) a[i] = i % 10;
        for (int j = 0; j < 8; ++j)  b[j] = (j % 10);
        auto pred = [](int x, int y){ return (x % 10) == (y % 10); };
        check_find_end(a.begin(), a.end(), b.begin(), b.end(), pred);
    }

    {
        std::vector<uint64_t> a(16, 0xDEADBEEFDEADBEEFull);
        std::vector<uint64_t> b(16, 0xDEADBEEFDEADBEEFull);
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<uint64_t> a(32, 0);
        for (size_t i = 24; i < 32; ++i) a[i] = 42;
        std::vector<uint64_t> b(8, 42);
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<uint64_t> a(8, 1);
        std::vector<uint64_t> b(16, 1);
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }

    {
        std::vector<char> a(256, 'x');
        std::vector<char> b(64, 'x');
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<char> a(256, 'x');
        for (size_t i = 120; i < 140; ++i) a[i] = (i % 2) ? 'y' : 'x';
        std::vector<char> b = {'x','x','x','x','x','x','x','x','x','x','x','x'};
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }

    {
        std::list<int> a; a.assign(64, 5);
        std::list<int> b; b.assign(8, 5);
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::list<int> a; a.assign(64, 1);
        std::list<int> b; b.assign(8, 2);
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }

    {
        std::vector<int> a(100000, 0);
        std::vector<int> b{1,2,3,4,5,6,7,8};
        for (size_t i = 0; i + b.size() <= a.size(); i += 10000) {
            for (size_t j = 0; j < b.size(); ++j) a[i + j] = b[j];
        }
        check_find_end(a.begin(), a.end(), b.begin(), b.end());
    }
    return 0;
}
