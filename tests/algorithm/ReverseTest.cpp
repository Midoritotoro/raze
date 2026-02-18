#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>
#include <array>
#include <deque>
#include <list>
#include <string>
#include <iostream>

#include <raze/algorithm/order/Reverse.h>


template <typename Container, typename T>
void fill_sequential(Container& c, T start = T(0)) {
    size_t i = 0;
    for (auto& x : c) x = static_cast<T>(start + T(i++));
}

template <typename Container>
void verify_equal(const Container& a, const Container& b) {
    raze_assert(a.size() == b.size());
    raze_assert(std::equal(a.begin(), a.end(), b.begin()));
}

template <typename Container>
void test_reverse_container(Container& c) {
    Container c_simd = c;
    Container c_ref = c;
    raze::algorithm::reverse(c_simd.begin(), c_simd.end());
    std::reverse(c_ref.begin(), c_ref.end());
    verify_equal<Container>(c_simd, c_ref);
}

template <typename T>
void test_reverse_large_vector() {
    const size_t target_bytes = 3072;
    const size_t N = std::max<size_t>(65 / sizeof(T) + 1, target_bytes / sizeof(T));
    std::vector<T> v(N);
    fill_sequential<std::vector<T>, T>(v);
    test_reverse_container<std::vector<T>>(v);
}

template <typename T, size_t N>
void test_reverse_large_array() {
    std::array<T, N> a{};
    static_assert(N * sizeof(T) > 64, "Array must exceed 64 bytes");
    fill_sequential<std::array<T, N>, T>(a);
    test_reverse_container<std::array<T, N>>(a);
}

template <typename T>
void test_reverse_large_deque() {
    const size_t target_bytes = 4096;
    const size_t N = std::max<size_t>(65 / sizeof(T) + 1, target_bytes / sizeof(T));
    std::deque<T> d(N);
    fill_sequential<std::deque<T>, T>(d);
    test_reverse_container<std::deque<T>>(d);
}

template <typename T>
void test_reverse_large_list() {
    const size_t target_bytes = 2048;
    const size_t N = std::max<size_t>(65 / sizeof(T) + 1, target_bytes / sizeof(T));
    std::list<T> l;
    for (size_t i = 0; i < N; ++i) l.push_back(static_cast<T>(i));
    test_reverse_container<std::list<T>>(l);
}

template <typename T>
void test_reverse_sizes() {
    // Sizes around SIMD thresholds and misaligned counts
    for (size_t bytes : {72u, 128u, 256u, 512u, 1024u, 2048u, 3992u}) {
        size_t N = std::max<size_t>(1, bytes / sizeof(T));
        std::vector<T> v(N);
        fill_sequential<std::vector<T>, T>(v);
        test_reverse_container<std::vector<T>>(v);
    }
}

template <typename T>
void test_reverse_edge_cases() {
    {
        size_t N = (64 / sizeof(T)) + 2;
        std::vector<T> v(N);
        fill_sequential<std::vector<T>, T>(v);
        test_reverse_container<std::vector<T>>(v);
    }

    {
        size_t N = 777 / sizeof(T) + 3;
        std::vector<T> v(N);
        fill_sequential<std::vector<T>, T>(v);
        test_reverse_container<std::vector<T>>(v);
    }

    {
        size_t N = 888 / sizeof(T) + 4;
        std::vector<T> v(N);
        fill_sequential<std::vector<T>, T>(v);
        test_reverse_container<std::vector<T>>(v);
    }

    {
        size_t N = 1024 / sizeof(T) + 8;
        std::vector<T> v(N, static_cast<T>(42));
        test_reverse_container<std::vector<T>>(v);
    }

    {
        size_t N = 1536 / sizeof(T) + 10;
        std::vector<T> v(N);
        for (size_t i = 0; i < N; ++i) v[i] = static_cast<T>(i % 7);
        test_reverse_container<std::vector<T>>(v);
    }
}

int main() {
    test_reverse_large_vector<uint8_t>();
    test_reverse_large_vector<int16_t>();
    test_reverse_large_vector<int32_t>();
    test_reverse_large_vector<int64_t>();
    test_reverse_large_vector<float>();
    test_reverse_large_vector<double>();

    test_reverse_large_array<uint8_t, 512>();
    test_reverse_large_array<int16_t, 256>();
    test_reverse_large_array<int32_t, 256>();
    test_reverse_large_array<int64_t, 128>();
    test_reverse_large_array<float, 256>();
    test_reverse_large_array<double, 128>();

    test_reverse_large_deque<int32_t>();
    test_reverse_large_deque<double>();

    test_reverse_large_list<int32_t>();
    test_reverse_large_list<uint8_t>();

    test_reverse_sizes<uint8_t>();
    test_reverse_sizes<int32_t>();
    test_reverse_sizes<double>();

    test_reverse_edge_cases<uint8_t>();
    test_reverse_edge_cases<int64_t>();
    test_reverse_edge_cases<float>();

    {
        const size_t N = 3000;
        std::vector<char> v(N);
        fill_sequential<std::vector<char>, char>(v, char(0));
        test_reverse_container<std::vector<char>>(v);
    }
    {
        std::deque<char> d(2000);
        fill_sequential<std::deque<char>, char>(d, char(5));
        test_reverse_container<std::deque<char>>(d);
    }

    return 0;
}
