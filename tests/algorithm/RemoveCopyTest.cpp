#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <random>
#include <type_traits>
#include <vector>
#include <raze/algorithm/remove/RemoveCopy.h>

template <typename T>
void fill_sequential(std::vector<T>& v, T start = T(1)) {
    for (size_t i = 0; i < v.size(); ++i) v[i] = static_cast<T>(start + T(i));
}

template <typename T>
size_t ref_remove_copy(const std::vector<T>& in, std::vector<T>& out, const T& value) {
    size_t write = 0;
    for (size_t i = 0; i < in.size(); ++i) {
        if (!(in[i] == value)) {
            out[write++] = in[i];
        }
    }
    return write;
}

template <typename T, typename Pred>
size_t ref_remove_copy_if(const std::vector<T>& in, std::vector<T>& out, Pred pred) {
    size_t write = 0;
    for (size_t i = 0; i < in.size(); ++i) {
        if (!pred(in[i])) {
            out[write++] = in[i];
        }
    }
    return write;
}

template <typename T>
void test_remove_copy_large() {
    const size_t total_bytes = 4000;
    const size_t N = std::max<size_t>(1, total_bytes / sizeof(T));
    std::vector<T> in(N);
    fill_sequential(in);
    T value_to_remove = in[N / 3];
    std::vector<T> out_simd(N, T{});
    std::vector<T> out_ref(N, T{});
    auto out_it = raze::algorithm::remove_copy(in.begin(), in.end(), out_simd.begin(), value_to_remove);
    size_t simd_written = std::distance(out_simd.begin(), out_it);
    size_t ref_written = ref_remove_copy(in, out_ref, value_to_remove);
    raze_assert(simd_written == ref_written);
    raze_assert(std::equal(out_simd.begin(), out_simd.begin() + simd_written, out_ref.begin()));
}

template <typename T>
void test_remove_copy_if_large() {
    const size_t total_bytes = 4000;
    const size_t N = std::max<size_t>(1, total_bytes / sizeof(T));
    std::vector<T> in(N);
    fill_sequential(in);
    auto pred = [&](const T& x) {
        size_t idx = &x - in.data();
        if constexpr (std::is_integral_v<T>)
            return ((idx & 3) == 0) || ((x & T(7)) == T(3));
        else
            return (idx % 5 == 0) || (x < T(0));
        };
    std::vector<T> out_simd(N, T{});
    std::vector<T> out_ref(N, T{});
    auto out_it = raze::algorithm::remove_copy_if(in.begin(), in.end(), out_simd.begin(), pred);
    size_t simd_written = std::distance(out_simd.begin(), out_it);
    size_t ref_written = ref_remove_copy_if<T>(in, out_ref, pred);
    raze_assert(simd_written == ref_written);
    raze_assert(std::equal(out_simd.begin(), out_simd.begin() + simd_written, out_ref.begin()));
}

template <typename T>
void test_alignment_variants_remove_copy() {
    const size_t total_bytes = 4000;
    const size_t N = std::max<size_t>(1, total_bytes / sizeof(T));
    std::vector<T> in(N);
    fill_sequential(in);
    std::vector<T> out_aligned(N, T{});
    std::vector<T> out_unaligned(N + 8, T{});
    T value_to_remove = in[N / 2];
    auto itA = raze::algorithm::remove_copy(in.begin(), in.end(), out_aligned.begin(), value_to_remove);
    size_t wA = std::distance(out_aligned.begin(), itA);
    T* out_unaligned_ptr = out_unaligned.data() + 1;
    auto itU = raze::algorithm::remove_copy(in.begin(), in.end(), out_unaligned_ptr, value_to_remove);
    size_t wU = static_cast<size_t>(itU - out_unaligned_ptr);
    std::vector<T> ref(N, T{});
    size_t wR = ref_remove_copy(in, ref, value_to_remove);
    raze_assert(wA == wR);
    raze_assert(wU == wR);
    raze_assert(std::equal(out_aligned.begin(), out_aligned.begin() + wA, ref.begin()));
    raze_assert(std::equal(out_unaligned_ptr, out_unaligned_ptr + wU, ref.begin()));
}

template <typename T>
void test_alignment_variants_remove_copy_if() {
    const size_t total_bytes = 4000;
    const size_t N = std::max<size_t>(1, total_bytes / sizeof(T));
    std::vector<T> in(N);
    fill_sequential(in);
    auto pred = [&](const T& x) {
        size_t idx = &x - in.data();
        if constexpr (std::is_integral_v<T>)
            return ((idx & 1) == 1) && ((x & T(0xFF)) > T(127));
        else
            return (idx % 3 == 2) || (x * x > T(10000));
        };
    std::vector<T> out_aligned(N, T{});
    std::vector<T> out_unaligned(N + 8, T{});
    auto itA = raze::algorithm::remove_copy_if(in.begin(), in.end(), out_aligned.begin(), pred);
    size_t wA = std::distance(out_aligned.begin(), itA);
    T* out_unaligned_ptr = out_unaligned.data() + 1;
    auto itU = raze::algorithm::remove_copy_if(in.begin(), in.end(), out_unaligned_ptr, pred);
    size_t wU = static_cast<size_t>(itU - out_unaligned_ptr);
    std::vector<T> ref(N, T{});
    size_t wR = ref_remove_copy_if(in, ref, pred);
    raze_assert(wA == wR);
    raze_assert(wU == wR);
    raze_assert(std::equal(out_aligned.begin(), out_aligned.begin() + wA, ref.begin()));
    raze_assert(std::equal(out_unaligned_ptr, out_unaligned_ptr + wU, ref.begin()));
}

int main() {
    test_remove_copy_large<uint8_t>();
    test_remove_copy_large<int32_t>();
    test_remove_copy_large<double>();
    test_remove_copy_if_large<uint16_t>();
    test_remove_copy_if_large<int64_t>();
    test_remove_copy_if_large<float>();
    test_alignment_variants_remove_copy<int32_t>();
    test_alignment_variants_remove_copy_if<double>();

    return 0;
}
