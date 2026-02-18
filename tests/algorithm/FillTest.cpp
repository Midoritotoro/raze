#include <cassert>
#include <algorithm>
#include <vector>
#include <iostream>

#include <raze/algorithm/fill/Fill.h>
#include <raze/algorithm/fill/FillN.h>
#include <chrono>

int main() {
   /* {
        std::vector<int> v1(5, 0);
        std::vector<int> v2(5, 0);

        raze::algorithm::fill(v1.begin(), v1.end(), 42);
        std::fill(v2.begin(), v2.end(), 42);

        raze_assert(v1 == v2);
    }

    {
        std::vector<int> v1;
        std::vector<int> v2;

        raze::algorithm::fill(v1.begin(), v1.end(), 7);
        std::fill(v2.begin(), v2.end(), 7);

        raze_assert(v1 == v2);
    }

    {
        std::vector<char> v1(10, 0);
        std::vector<char> v2(10, 0);

        raze::algorithm::fill_n(v1.begin(), 5, 99);
        std::fill_n(v2.begin(), 5, 99);

        raze_assert(v1 == v2);
    }

    {
        std::vector<int> v1(3, 1);
        std::vector<int> v2(3, 1);

        raze::algorithm::fill_n(v1.begin(), 0, 123);
        std::fill_n(v2.begin(), 0, 123);

        raze_assert(v1 == v2);
    }

    {
        std::vector<int> v(5, 0);
        auto it1 = raze::algorithm::fill_n(v.begin(), 3, 5);
        raze_assert(it1 == v.begin() + 3);
        raze_assert((v == std::vector<int>{5,5,5,0,0}));
    }
*/
    constexpr size_t N = 10'000'000;
    std::vector<char> v1(N), v2(N);

    {
        auto t1 = std::chrono::high_resolution_clock::now();
        raze::algorithm::fill(v1.begin(), v1.end(), 123);
        auto t2 = std::chrono::high_resolution_clock::now();

        auto t3 = std::chrono::high_resolution_clock::now();
        std::fill(v2.begin(), v2.end(), 123);
        auto t4 = std::chrono::high_resolution_clock::now();

        raze_assert(v1 == v2);

        std::cout << "fill raze: "
                  << std::chrono::duration<double, std::milli>(t2 - t1).count()
                  << " ms\n";
        std::cout << "fill std:      "
                  << std::chrono::duration<double, std::milli>(t4 - t3).count()
                  << " ms\n";
    }

    {
        std::vector<int> v3(N), v4(N);

        auto t1 = std::chrono::high_resolution_clock::now();
        raze::algorithm::fill_n(v3.begin(), N, 777);
        auto t2 = std::chrono::high_resolution_clock::now();

        auto t3 = std::chrono::high_resolution_clock::now();
        std::fill_n(v4.begin(), N, 777);
        auto t4 = std::chrono::high_resolution_clock::now();

        raze_assert(v3 == v4);

        std::cout << "fill_n raze: "
                  << std::chrono::duration<double, std::milli>(t2 - t1).count()
                  << " ms\n";
        std::cout << "fill_n std:      "
                  << std::chrono::duration<double, std::milli>(t4 - t3).count()
                  << " ms\n";
    }

    return 0;
}
