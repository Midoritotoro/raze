#include <raze/algorithm/copy/Copy.h>
#include <tests/utility/Random.h>
#include <tests/utility/TestArithmeticTypes.h>

#include <algorithm>

using raze::tests::utility::RandomGenerator;
using raze::tests::utility::generate_random_vector;
using raze::tests::utility::generate_random_list;
using raze::tests::utility::generate_random_forward_list;
using raze::tests::utility::run_for_arithmetic_types;
using raze::tests::utility::equal_ranges;

struct S1 { char a; friend bool operator==(const S1&, const S1&) = default; };
struct S2 { short a; friend bool operator==(const S2&, const S2&) = default; };
struct S4 { int a; friend bool operator==(const S4&, const S4&) = default; };
struct S8 { int x, y; friend bool operator==(const S8&, const S8&) = default; };
struct Point { int x, y; friend bool operator==(const Point&, const Point&) = default; };

template <typename T>
void test_copy_random(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
	auto run_tests = [&](auto container_gen) {
		for (int i = 0; i < 1000; ++i) {
			size_t size = size_dist(size_gen);
			auto src = container_gen(size, seed + i);
			std::vector<T> dst(size), dst_copy(size);

			auto simd_result = raze::algorithm::copy(src.begin(), src.end(), dst.begin());
			auto std_result = std::ranges::copy(src.begin(), src.end(), dst_copy.begin());

			raze_assert(equal_ranges(dst, dst_copy));
			raze_assert(simd_result.in == src.end());
			raze_assert(simd_result.out == dst.end());
			raze_assert(std_result.in == src.end());
			raze_assert(std_result.out == dst_copy.end());
		}
	};

	run_tests(&generate_random_vector<T>);
    run_tests(&generate_random_list<T>);
    run_tests(&generate_random_forward_list<T>);

    auto run_tests_ranges = [&](auto container_gen) {
		for (int i = 0; i < 1000; ++i) {
			size_t size = size_dist(size_gen);
			auto src = container_gen(size, seed + i);
			std::vector<T> dst(size), dst_copy(size);

			auto simd_result = raze::algorithm::copy(src, dst.begin());
			auto std_result = std::ranges::copy(src, dst_copy.begin());

			raze_assert(equal_ranges(dst, dst_copy));
			raze_assert(simd_result.in == src.end());
			raze_assert(simd_result.out == dst.end());
			raze_assert(std_result.in == src.end());
			raze_assert(std_result.out == dst_copy.end());
		}
	};

    run_tests_ranges(&generate_random_vector<T>);
    run_tests_ranges(&generate_random_list<T>);
    run_tests_ranges(&generate_random_forward_list<T>);
}

template <typename T>
void test_copy_edge_cases() {
    {
        std::vector<T> src, dst, dst_copy;
        auto r1 = raze::algorithm::copy(src.begin(), src.end(), dst.begin());
        auto r2 = std::ranges::copy(src.begin(), src.end(), dst_copy.begin());
        raze_assert(equal_ranges(dst, dst_copy));
        raze_assert(r1.in == src.end() && r1.out == dst.end());
        raze_assert(r2.in == src.end() && r2.out == dst_copy.end());
    }
    {
        std::vector<T> src = {T(42)};
        std::vector<T> dst(1), dst_copy(1);
        raze::algorithm::copy(src.begin(), src.end(), dst.begin());
        std::ranges::copy(src.begin(), src.end(), dst_copy.begin());
        raze_assert(equal_ranges(dst, dst_copy));
    }
    {
        std::vector<T> src(100, T(99));
        std::vector<T> dst(100), dst_copy(100);
        raze::algorithm::copy(src.begin(), src.end(), dst.begin());
        std::ranges::copy(src.begin(), src.end(), dst_copy.begin());
        raze_assert(equal_ranges(dst, dst_copy));
    }
}

template <typename T>
void test_copy_simd_boundaries(unsigned seed = 42) {
    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256, 511, 512}) {
        auto src = generate_random_vector<T>(size, seed + static_cast<unsigned>(size));
        std::vector<T> dst(size), dst_copy(size);
        
        raze::algorithm::copy(src.begin(), src.end(), dst.begin());
        std::ranges::copy(src.begin(), src.end(), dst_copy.begin());
        
        raze_assert(equal_ranges(dst, dst_copy));
    }
}

template <typename T>
void test_copy_overlapping(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        size_t size = 100;
        auto vec = generate_random_vector<T>(size, seed + i + 1600000);
        auto vec_copy = vec;
        
        raze::algorithm::copy(vec.begin(), vec.begin() + 50, vec.begin() + 50);
        std::ranges::copy(vec_copy.begin(), vec_copy.begin() + 50, vec_copy.begin() + 50);
        
        raze_assert(equal_ranges(vec, vec_copy));
    }
}

template <class Struct>
void test_copy_small_structs(unsigned seed = 42) {
    std::mt19937 size_gen(seed);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        std::vector<Struct> src(size), dst(size), dst_copy(size);
        for (auto& x : src) std::memset(&x, static_cast<unsigned char>(i % 256), sizeof(Struct));
        
        raze::algorithm::copy(src.begin(), src.end(), dst.begin());
        std::ranges::copy(src.begin(), src.end(), dst_copy.begin());
        raze_assert(equal_ranges(dst, dst_copy));
    }
    
    for (size_t size : {0, 1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        std::vector<Struct> src(size), dst(size), dst_copy(size);
        for (auto& x : src) std::memset(&x, 0xAB, sizeof(Struct));
        
        raze::algorithm::copy(src.begin(), src.end(), dst.begin());
        std::ranges::copy(src.begin(), src.end(), dst_copy.begin());
        raze_assert(equal_ranges(dst, dst_copy));
    }
}

void test_copy_struct(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        std::vector<Point> src(100), dst(100), dst_copy(100);
        for (auto& p : src) { p.x = i; p.y = i * 2; }
        
        raze::algorithm::copy(src.begin(), src.end(), dst.begin());
        std::ranges::copy(src.begin(), src.end(), dst_copy.begin());
        raze_assert(equal_ranges(dst, dst_copy));
    }
}

int main() {
	run_for_arithmetic_types([] <class _Type_> () {
        test_copy_random<_Type_>();
        test_copy_edge_cases<_Type_>();
        test_copy_simd_boundaries<_Type_>();
        test_copy_overlapping<_Type_>();
	});
    
    test_copy_struct();
    test_copy_small_structs<S1>();
    test_copy_small_structs<S2>();
    test_copy_small_structs<S4>();
    test_copy_small_structs<S8>();
    
    return 0;
}