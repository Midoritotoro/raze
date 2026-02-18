#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>
#include <array>
#include <deque>
#include <list>
#include <iostream>

#include <raze/algorithm/order/ReverseCopy.h>

template <typename Container, typename T>
void fill_sequential(Container& c, T start = T(0)) {
    size_t i = 0;
    for (auto& x : c) x = static_cast<T>(start + T(i++));
}

template <typename InContainer, typename OutContainer>
void test_reverse_copy_container(const InContainer& in) {
    OutContainer out_simd(in.size());
    OutContainer out_ref(in.size());

    raze::algorithm::reverse_copy(in.begin(), in.end(), out_simd.begin());
    std::reverse_copy(in.begin(), in.end(), out_ref.begin());

    raze_assert(std::equal(out_simd.begin(), out_simd.end(), out_ref.begin()));
}

template <typename T>
void test_reverse_copy_large_vector() {
    const size_t target_bytes = 3000;
    const size_t N = std::max<size_t>(65 / sizeof(T) + 1, target_bytes / sizeof(T));

    std::vector<T> v(N);
    fill_sequential<std::vector<T>, T>(v);

    test_reverse_copy_container<std::vector<T>, std::vector<T>>(v);
}

template <typename T, size_t N>
void test_reverse_copy_large_array() {
    static_assert(N * sizeof(T) > 64, "Array must exceed 64 bytes");
    std::array<T, N> a{};
    fill_sequential<std::array<T, N>, T>(a);
    test_reverse_copy_container<std::array<T, N>, std::vector<T>>(a);
}

template <typename T>
void test_reverse_copy_large_deque() {
    const size_t target_bytes = 2048;
    const size_t N = std::max<size_t>(65 / sizeof(T) + 1, target_bytes / sizeof(T));
    std::deque<T> d(N);
    fill_sequential<std::deque<T>, T>(d);
    test_reverse_copy_container<std::deque<T>, std::vector<T>>(d);
}

template <typename T>
void test_reverse_copy_large_list() {
    const size_t target_bytes = 1024;
    const size_t N = std::max<size_t>(65 / sizeof(T) + 1, target_bytes / sizeof(T));
    std::list<T> l;
    for (size_t i = 0; i < N; ++i) l.push_back(static_cast<T>(i));
    test_reverse_copy_container<std::list<T>, std::vector<T>>(l);
}

template <typename T>
void test_reverse_copy_edge_cases() {
    {
        std::vector<T> v;
        test_reverse_copy_container<std::vector<T>, std::vector<T>>(v);
    }
    {
        std::vector<T> v(1, static_cast<T>(42));
        test_reverse_copy_container<std::vector<T>, std::vector<T>>(v);
    }
    {
        std::vector<T> v(2, static_cast<T>(99));
        test_reverse_copy_container<std::vector<T>, std::vector<T>>(v);
    }
    {
        std::vector<T> v(100, static_cast<T>(7));
        test_reverse_copy_container<std::vector<T>, std::vector<T>>(v);
    }
    {
        std::vector<T> v(129);
        fill_sequential<std::vector<T>, T>(v);
        test_reverse_copy_container<std::vector<T>, std::vector<T>>(v);
    }
}

int main() {
    test_reverse_copy_large_vector<uint8_t>();
    test_reverse_copy_large_vector<int16_t>();
    test_reverse_copy_large_vector<int32_t>();
    test_reverse_copy_large_vector<int64_t>();
    test_reverse_copy_large_vector<float>();
    test_reverse_copy_large_vector<double>();

    test_reverse_copy_large_array<uint8_t, 512>();
    test_reverse_copy_large_array<int32_t, 256>();
    test_reverse_copy_large_array<double, 128>();

    test_reverse_copy_large_deque<int32_t>();
    test_reverse_copy_large_deque<double>();

    test_reverse_copy_large_list<int32_t>();
    test_reverse_copy_large_list<uint8_t>();

    test_reverse_copy_edge_cases<uint8_t>();
    test_reverse_copy_edge_cases<int32_t>();
    test_reverse_copy_edge_cases<double>();

    return 0;
}
