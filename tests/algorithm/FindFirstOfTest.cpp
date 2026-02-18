#include <cassert>
#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <iostream>
#include <cctype>

#include <raze/algorithm/find/FindFirstOf.h>

// ��������������� ������� ��� ��������
template <typename It1, typename It2>
void check_find_first_of(It1 first1, It1 last1, It2 first2, It2 last2) {
    auto std_res = std::find_first_of(first1, last1, first2, last2);
    auto simd_res = raze::algorithm::find_first_of(first1, last1, first2, last2);
    raze_assert((std_res == last1 && simd_res == last1) || (std_res - first1 == simd_res - first1));
}

template <typename It1, typename It2, typename Pred>
void check_find_first_of(It1 first1, It1 last1, It2 first2, It2 last2, Pred pred) {
    auto std_res = std::find_first_of(first1, last1, first2, last2, pred);
    auto simd_res = raze::algorithm::find_first_of(first1, last1, first2, last2, pred);
    raze_assert((std_res == last1 && simd_res == last1) || (std_res - first1 == simd_res - first1));
}

int main() {
    {
        std::vector<int> a, b;
        check_find_first_of(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a{ 1,2,3,4,5 };
        std::vector<int> b{ 3,4 };
        check_find_first_of(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a{ 1,2,3,4,5 };
        std::vector<int> b{ 5,4 };
        check_find_first_of(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a{ 1,2,3,4,5 };
        std::vector<int> b{ 9,8,7 };
        check_find_first_of(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a{ 1,2,3,4,5,6,7,8,9 };
        std::vector<int> b{ 7,8,9 };
        check_find_first_of(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::string s = "abcdefg";
        std::string chars = "xzef";
        check_find_first_of(s.begin(), s.end(), chars.begin(), chars.end());
    }
    {
        std::list<char> a{ 'a','b','c','d','e' };
        std::string b = "xyzde";
        check_find_first_of(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a{ 1,2,3,4,5 };
        std::vector<int> b{ 2,3,4 };
        auto pred = [](int x, int y) { return x % 3 == y % 3; };
        check_find_first_of(a.begin(), a.end(), b.begin(), b.end(), pred);
    }
    {
        std::string s = "caseinsensitive";
        std::string chars = "ei";
        auto pred = [](char a, char b) { return std::tolower(a) == std::tolower(b); };
        check_find_first_of(s.begin(), s.end(), chars.begin(), chars.end(), pred);
    }
    {
        std::vector<int> a(10000, 1);
        a[5000] = 42;
        std::vector<int> b{ 42, 99 };
        check_find_first_of(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a{ 1,2 };
        std::vector<int> b{ 1,2,3,4,5 };
        check_find_first_of(a.begin(), a.end(), b.begin(), b.end());
    }

    {
       // ���������� � ������ �������� �������
       std::vector<int> a(100000, 0);
       a[0] = 42;
       std::vector<int> b{ 42 };
       check_find_first_of(a.begin(), a.end(), b.begin(), b.end());
    }
    return 0;
}
