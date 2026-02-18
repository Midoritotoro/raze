#include <cassert>
#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <iostream>
#include <cctype>

// ���������� ���� ����������
#include <raze/algorithm/find/StartsWith.h>
#include <raze/algorithm/find/EndsWith.h>

// ��������������� ������� ��� ��������
template <typename It1, typename It2>
void check_starts_with(It1 f1, It1 l1, It2 f2, It2 l2) {
    bool std_res = std::ranges::starts_with(f1, l1, f2, l2);
    bool simd_res = raze::algorithm::starts_with(f1, l1, f2, l2, [](auto&& a, auto&& b) { return a == b; });
    raze_assert(std_res == simd_res);
}

template <typename It1, typename It2, typename Pred>
void check_starts_with(It1 f1, It1 l1, It2 f2, It2 l2, Pred pred) {
    bool std_res = std::ranges::starts_with(f1, l1, f2, l2, pred);
    bool simd_res = raze::algorithm::starts_with(f1, l1, f2, l2, pred);
    raze_assert(std_res == simd_res);
}

template <typename It1, typename It2>
void check_ends_with(It1 f1, It1 l1, It2 f2, It2 l2) {
    bool std_res = std::ranges::ends_with(f1, l1, f2, l2);
    bool simd_res = raze::algorithm::ends_with(f1, l1, f2, l2, [](auto&& a, auto&& b) { return a == b; });
    raze_assert(std_res == simd_res);
}

template <typename It1, typename It2, typename Pred>
void check_ends_with(It1 f1, It1 l1, It2 f2, It2 l2, Pred pred) {
    bool std_res = std::ranges::ends_with(f1, l1, f2, l2, pred);
    bool simd_res = raze::algorithm::ends_with(f1, l1, f2, l2, pred);
    raze_assert(std_res == simd_res);
}

int main() {
    {
        std::vector<int> a, b;
        check_starts_with(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a{ 1,2,3,4,5 };
        std::vector<int> b{ 1,2,3 };
        check_starts_with(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a{ 1,2,3,4,5 };
        std::vector<int> b{ 2,3 };
        check_starts_with(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::string s = "HelloWorld";
        std::string pref = "Hello";
        check_starts_with(s.begin(), s.end(), pref.begin(), pref.end());
    }
    {
        std::string s = "CaseInsensitive";
        std::string pref = "case";
        auto pred = [](char a, char b) { return std::tolower(a) == std::tolower(b); };
        check_starts_with(s.begin(), s.end(), pref.begin(), pref.end(), pred);
    }
    {
        std::vector<int> a(100000, 1);
        std::vector<int> b(1000, 1);
        check_starts_with(a.begin(), a.end(), b.begin(), b.end());
    }

    {
        std::vector<int> a, b;
        check_ends_with(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a{ 1,2,3,4,5 };
        std::vector<int> b{ 4,5 };
        check_ends_with(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::vector<int> a{ 1,2,3,4,5 };
        std::vector<int> b{ 5 };
        check_ends_with(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::string s = "HelloWorld";
        std::string suf = "World";
        check_ends_with(s.begin(), s.end(), suf.begin(), suf.end());
    }
    {
        std::string s = "CaseInsensitive";
        std::string suf = "SITIVE";
        auto pred = [](char a, char b) { return std::tolower(a) == std::tolower(b); };
        check_ends_with(s.begin(), s.end(), suf.begin(), suf.end(), pred);
    }
    {
        std::vector<int> a(100000, 1);
        a.back() = 42;
        std::vector<int> b{ 42 };
        check_ends_with(a.begin(), a.end(), b.begin(), b.end());
    }
    {
        std::list<int> a{ 1,2,3,4,5 };
        std::list<int> b{ 2,3,4,5 };
        check_ends_with(a.begin(), a.end(), b.begin(), b.end());
    }


    return 0;
}
