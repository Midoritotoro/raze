#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>
#include <array>
#include <deque>
#include <list>
#include <iostream>

#include <raze/algorithm/batch/ForEach.h>
#include <raze/algorithm/batch/ForEachN.h>

template <typename Container, typename T>
void fill_sequential(Container& c, T start = T(1)) {
    size_t i = 0;
    for (auto& x : c) {
        x = static_cast<T>(start + T(i++));
    }
}

template <typename Container, typename T>
void test_for_each_container(Container& c) {
    Container c_simd = c;
    Container c_std = c;

    auto op = [](T& x) { x = static_cast<T>(x * 2); };


    std::for_each(c_std.begin(), c_std.end(), op);
    raze::algorithm::for_each(c_simd.begin(), c_simd.end(), op);

    raze_assert(std::equal(c_simd.begin(), c_simd.end(), c_std.begin()));
}

template <typename Container, typename T>
void test_for_each_n_container(Container& c, size_t n) {
    Container c_simd = c;
    Container c_std = c;

    auto op = [](T& x) { x = static_cast<T>(x + 5); };

    std::for_each_n(c_std.begin(), n, op);
    raze::algorithm::for_each_n(c_simd.begin(), n, op);

    raze_assert(std::equal(c_simd.begin(), c_simd.end(), c_std.begin()));
}

template <typename T>
void test_for_each_large() {
    const size_t total_bytes = 3000;
    const size_t N = std::max<size_t>(1, total_bytes / sizeof(T));

    {
        std::vector<T> v(N);
        fill_sequential<std::vector<T>, T>(v);
        test_for_each_container<std::vector<T>, T>(v);
        test_for_each_n_container<std::vector<T>, T>(v, N / 2);
    }

    {
        std::array<T, 1024> arr{};
        fill_sequential<std::array<T, 1024>, T>(arr);
        test_for_each_container<std::array<T, 1024>, T>(arr);
        test_for_each_n_container<std::array<T, 1024>, T>(arr, 512);
    }

    {
        std::deque<T> dq(N);
        fill_sequential<std::deque<T>, T>(dq);
        test_for_each_container<std::deque<T>, T>(dq);
        test_for_each_n_container<std::deque<T>, T>(dq, N / 3);
    }

    {
        std::list<T> lst;
        for (size_t i = 0; i < N; ++i) lst.push_back(static_cast<T>(i + 1));
        test_for_each_container<std::list<T>, T>(lst);
        test_for_each_n_container<std::list<T>, T>(lst, N / 4);
    }
}

int main() {
    test_for_each_large<int8_t>();
    test_for_each_large<int16_t>();
    test_for_each_large<int32_t>();
    test_for_each_large<int64_t>();
    test_for_each_large<float>();
    test_for_each_large<double>();

    return 0;
}
