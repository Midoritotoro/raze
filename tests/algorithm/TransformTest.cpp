#include <vector>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>

#include <raze/algorithm/transform/Transform.h>


template <typename T, typename UnaryOp>
void test_unary(size_t bytes, UnaryOp op) {
    size_t N = bytes / sizeof(T);
    if (N == 0) return;

    std::vector<T> input(N), out_std(N), out_simd(N);

    for (size_t i = 0; i < N; ++i)
        input[i] = static_cast<T>(i % 17);
    
    const auto stdRet = std::transform(input.begin(), input.end(), out_std.begin(), op);
    const auto simdStlRet = raze::algorithm::transform(input.begin(), input.end(), out_simd.begin(), op);

    raze_assert(out_simd == out_std);
}

template <typename T, typename BinaryOp>
void test_binary(size_t bytes, BinaryOp op) {
    size_t N = bytes / sizeof(T);
    if (N == 0) return;

    std::vector<T> a(N), b(N), out_std(N), out_simd(N);

    for (size_t i = 0; i < N; ++i) {
        a[i] = static_cast<T>(i % 23);
        b[i] = static_cast<T>((N - i) % 19);
    }

    const auto stdRet =  std::transform(a.begin(), a.end(), b.begin(), out_std.begin(), op);
    const auto simdStlRet = raze::algorithm::transform(a.begin(), a.end(), b.begin(), out_simd.begin(), op);

    raze_assert(out_simd == out_std);
}

void testAll(uint8_t bytes) noexcept {
    test_unary<uint8_t>(bytes, [](uint8_t x) { return static_cast<uint8_t>(~x); });
    test_unary<uint8_t>(bytes, raze::type_traits::negate<>());
    test_binary<uint8_t>(bytes, raze::type_traits::plus<>());
    test_binary<uint8_t>(bytes, raze::type_traits::minus<>());
    test_binary<uint8_t>(bytes, raze::type_traits::multiplies<>());

    // --- uint16_t ---
    test_unary<uint16_t>(bytes, raze::type_traits::negate<>());
    test_binary<uint16_t>(bytes, raze::type_traits::plus<>());
    test_binary<uint16_t>(bytes, raze::type_traits::minus<>());
    test_binary<uint16_t>(bytes, raze::type_traits::multiplies<>());

    // --- uint32_t ---
    test_unary<uint32_t>(bytes, raze::type_traits::negate<>());
    test_binary<uint32_t>(bytes, raze::type_traits::plus<>());
    test_binary<uint32_t>(bytes, raze::type_traits::minus<>());
    test_binary<uint32_t>(bytes, raze::type_traits::multiplies<>());

    // --- uint64_t ---
    test_unary<uint64_t>(bytes, raze::type_traits::negate<>());
    test_binary<uint64_t>(bytes, raze::type_traits::plus<>());
    test_binary<uint64_t>(bytes, raze::type_traits::minus<>());
    test_binary<uint64_t>(bytes, raze::type_traits::multiplies<>());

    // --- float ---
    test_unary<float>(bytes, [](float x) { return x + 1.5f; });
    test_unary<float>(bytes, raze::type_traits::negate<>());
    test_binary<float>(bytes, raze::type_traits::plus<>());
    test_binary<float>(bytes, raze::type_traits::minus<>());
    test_binary<float>(bytes, raze::type_traits::multiplies<>());
    test_binary<float>(bytes, raze::type_traits::divides<>());

            // --- double ---
    test_unary<double>(bytes, [](double x) { return x * 2.0; });
    test_unary<double>(bytes, raze::type_traits::negate<>());
    test_binary<double>(bytes, raze::type_traits::plus<>());
    test_binary<double>(bytes, raze::type_traits::minus<>());
    test_binary<double>(bytes, raze::type_traits::multiplies<>());
    test_binary<double>(bytes, raze::type_traits::divides<>());
}

int main() {
    for (size_t bytes = 1; bytes <= 4000; bytes += 257)
        testAll(bytes);
    
    return 0;
}