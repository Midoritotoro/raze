#include <cAssert>
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

#include <raze/algorithm/swap/Swap.h>

struct Custom {
    int x;
    Custom(int v = 0) : x(v) {}
    bool operator==(const Custom& other) const { return x == other.x; }
};

int main() {
    {
        int a = 1, b = 2;
        raze::algorithm::swap(a, b);
        raze_assert(a == 2 && b == 1);
    }

    {
        double a = 3.14, b = 2.71;
        raze::algorithm::swap(a, b);
        raze_assert(a == 2.71 && b == 3.14);
    }

    {
        Custom c1(10), c2(20);
        raze::algorithm::swap(c1, c2);
        raze_assert(c1.x == 20 && c2.x == 10);
    }

    {
        int arr1[3] = { 1,2,3 };
        int arr2[3] = { 4,5,6 };
        raze::algorithm::swap(arr1, arr2);
        raze_assert((arr1[0] == 4 && arr1[1] == 5 && arr1[2] == 6));
        raze_assert((arr2[0] == 1 && arr2[1] == 2 && arr2[2] == 3));
    }

    {
        int arr1[1] = { 42 };
        int arr2[1] = { 99 };
        raze::algorithm::swap(arr1, arr2);
        raze_assert(arr1[0] == 99 && arr2[0] == 42);
    }

    {
        int a = 123;
        raze::algorithm::swap(a, a);
        raze_assert(a == 123);
    }

    {
        constexpr size_t N = 1'000'0;
        int v1[N];
        std::fill(v1, v1 + N, 1);

        int v2[N];
        std::fill(v2, v2 + N, 2);

        raze::algorithm::swap(v1, v2);

        raze_assert(std::all_of(v1, v1 + N, [](int x) { return x == 2; }));
        raze_assert(std::all_of(v2, v2 + N, [](int x) { return x == 1; }));
    }

    {
        int a1[5] = { 1,2,3,4,5 };
        int a2[5] = { 6,7,8,9,10 };

        int b1[5] = { 1,2,3,4,5 };
        int b2[5] = { 6,7,8,9,10 };

        raze::algorithm::swap(a1, a2);
        std::swap(b1, b2);

        for (int i = 0; i < 5; ++i) {
            raze_assert(a1[i] == b1[i]);
            raze_assert(a2[i] == b2[i]);
        }
    }

    {
        // === ������� ���� ===
        std::vector<int> a = { 1,2,3 };
        std::vector<int> b = { 4,5,6 };

        raze::algorithm::swap_ranges(a.begin(), a.end(), b.begin());

        raze_assert((a == std::vector<int>{4, 5, 6}));
        raze_assert((b == std::vector<int>{1, 2, 3}));
    }

    {
        // === ���������������� ���� ===
        std::vector<Custom> a = { Custom(1), Custom(2) };
        std::vector<Custom> b = { Custom(9), Custom(8) };

        raze::algorithm::swap_ranges(a.begin(), a.end(), b.begin());

        raze_assert(a[0].x == 9 && a[1].x == 8);
        raze_assert(b[0].x == 1 && b[1].x == 2);
    }

    {
        // === ��������� �������� ===
        std::vector<int> a = { 1,2,3,4 };
        std::vector<int> b = { 9,8,7,6 };

        raze::algorithm::swap_ranges(a.begin() + 1, a.begin() + 3, b.begin() + 1);

        raze_assert((a == std::vector<int>{1, 8, 7, 4}));
        raze_assert((b == std::vector<int>{9, 2, 3, 6}));
    }

    {
        // === swap_ranges � ��������� ===
        int a[3] = { 10,20,30 };
        int b[3] = { 1,2,3 };

        raze::algorithm::swap_ranges(a, a + 3, b);

        raze_assert((a[0] == 1 && a[1] == 2 && a[2] == 3));
        raze_assert((b[0] == 10 && b[1] == 20 && b[2] == 30));
    }

    {
        // === ������� �������� ===
        std::vector<int> a = { 1,2,3 };
        std::vector<int> b = { 4,5,6 };

        raze::algorithm::swap_ranges(a.begin(), a.begin(), b.begin()); // ������ �� ��������

        raze_assert((a == std::vector<int>{1, 2, 3}));
        raze_assert((b == std::vector<int>{4, 5, 6}));
    }

    {
        // === ������� ��������� ===
        std::vector<int> a = { 1,2,3 };
        std::vector<int> b = { 4,5,6 };

        auto it = raze::algorithm::swap_ranges(a.begin(), a.end(), b.begin());
        raze_assert(it == b.end());
    }

    {
        // === ��������� � std::swap_ranges ===
        std::vector<int> a1 = { 1,2,3 };
        std::vector<int> b1 = { 4,5,6 };

        std::vector<int> a2 = a1;
        std::vector<int> b2 = b1;

        raze::algorithm::swap_ranges(a1.begin(), a1.end(), b1.begin());
        std::swap_ranges(a2.begin(), a2.end(), b2.begin());

        raze_assert(a1 == a2);
        raze_assert(b1 == b2);
    }

    {
        // === ������� ������� ===
        constexpr size_t N = 1'000'0;
        std::vector<int> a(N, 1);
        std::vector<int> b(N, 2);

        raze::algorithm::swap_ranges(a.begin(), a.end(), b.begin());

        raze_assert(std::all_of(a.begin(), a.end(), [](int x) { return x == 2; }));
        raze_assert(std::all_of(b.begin(), b.end(), [](int x) { return x == 1; }));
    }

    return 0;
}
