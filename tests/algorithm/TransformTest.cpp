#include <raze/algorithm/transform/Transform.h>

#include <vector>
#include <random>
#include <type_traits>
#include <algorithm>
#include <cassert>

template <typename T>
struct IntDistributionType {
    using type = T;
};

template <>
struct IntDistributionType<char> {
    using type = int;
};

template <>
struct IntDistributionType<signed char> {
    using type = int;
};

template <>
struct IntDistributionType<unsigned char> {
    using type = int;
};

template <>
struct IntDistributionType<bool> {
    using type = int;
};

template <>
struct IntDistributionType<wchar_t> {
    using type = int;
};

template <>
struct IntDistributionType<char8_t> {
    using type = int;
};

template <>
struct IntDistributionType<char16_t> {
    using type = int;
};

template <>
struct IntDistributionType<char32_t> {
    using type = int;
};

template <typename T>
using int_distribution_type_t = typename IntDistributionType<T>::type;

template <typename T>
struct RandomGenerator {
    std::mt19937 gen;
    
    RandomGenerator(unsigned seed = 42) : gen(seed) {}
    
    T operator()() {
        if constexpr (std::is_integral_v<T>) {
            using DistType = int_distribution_type_t<T>;
            
            if constexpr (std::is_signed_v<T>) {
                std::uniform_int_distribution<DistType> dist(-1000, 1000);
                return static_cast<T>(dist(gen));
            } else {
                std::uniform_int_distribution<DistType> dist(0, 2000);
                return static_cast<T>(dist(gen));
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            std::uniform_real_distribution<T> dist(-1000.0, 1000.0);
            return dist(gen);
        }
    }
};

template <typename T>
std::vector<T> generate_random_vector(size_t size, unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::vector<T> vec(size);
    for (auto& elem : vec) {
        elem = gen();
    }
    return vec;
}

template <typename T>
void test_unary_transform_random(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i);
        
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);
        
        auto simd_result = raze::algorithm::transform(vec.begin(), vec.end(), simd_out.begin(),
            [](const T& x) { return static_cast<T>(x * 2); });
        auto std_result = std::ranges::transform(vec.begin(), vec.end(), std_out.begin(),
            [](const T& x) { return static_cast<T>(x * 2); });
        
        raze_assert(simd_result.in == vec.end());
        raze_assert(std_result.in == vec.end());
        raze_assert(simd_result.out == simd_out.end());
        raze_assert(std_result.out == std_out.end());
        raze_assert(simd_out == std_out);
    }
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 5000);
        
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);
        
        auto simd_result = raze::algorithm::transform(vec.begin(), vec.end(), simd_out.begin(),
            [](const T& x) { return static_cast<T>(x + 100); });
        auto std_result = std::ranges::transform(vec.begin(), vec.end(), std_out.begin(),
            [](const T& x) { return static_cast<T>(x + 100); });
        
        raze_assert(simd_out == std_out);
    }
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 10000);
        
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);
        
        auto simd_result = raze::algorithm::transform(vec.begin(), vec.end(), simd_out.begin(),
            [](const T& x) { return static_cast<T>(x / 0xFF); });
        auto std_result = std::ranges::transform(vec.begin(), vec.end(), std_out.begin(),
            [](const T& x) { return static_cast<T>(x / 0xFF); });
        
        raze_assert(simd_out == std_out);
    }
}

template <typename T>
void test_unary_transform_ranges(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 20000);
        
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);
        
        auto simd_result = raze::algorithm::transform(vec, simd_out.begin(),
            [](const T& x) { return static_cast<T>(x * 3); });
        auto std_result = std::ranges::transform(vec, std_out.begin(),
            [](const T& x) { return static_cast<T>(x * 3); });
        
        raze_assert(simd_out == std_out);
        raze_assert(simd_result.in == vec.end());
        raze_assert(simd_result.out == simd_out.end());
    }
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 30000);
        
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);
        
        auto simd_result = raze::algorithm::transform(vec, simd_out.begin(),
            [](const T& x) { return static_cast<T>(-x); });
        auto std_result = std::ranges::transform(vec, std_out.begin(),
            [](const T& x) { return static_cast<T>(-x); });
        
        raze_assert(simd_out == std_out);
    }
}

template <typename T>
void test_unary_transform_in_place(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 40000);
        auto vec_copy = vec;
        
        raze::algorithm::transform(vec.begin(), vec.end(), vec.begin(),
            [](const T& x) { return static_cast<T>(x * 2); });
        std::ranges::transform(vec_copy.begin(), vec_copy.end(), vec_copy.begin(),
            [](const T& x) { return static_cast<T>(x * 2); });
        
        raze_assert(vec == vec_copy);
    }
    
    for (int i = 0; i < 500; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 50000);
        auto vec_copy = vec;
        
        raze::algorithm::transform(vec.begin(), vec.end(), vec.begin(),
            [](const T& x) { return static_cast<T>(x + 1); });
        std::ranges::transform(vec_copy.begin(), vec_copy.end(), vec_copy.begin(),
            [](const T& x) { return static_cast<T>(x + 1); });
        
        raze_assert(vec == vec_copy);
    }
}

template <typename T>
void test_unary_transform_with_projection(unsigned seed = 42) {
    for (int i = 0; i < 500; ++i) {
        size_t size = 50 + (i % 200);
        auto vec = generate_random_vector<T>(size, seed + i + 60000);
        
        std::vector<long long> simd_out(size);
        std::vector<long long> std_out(size);
        
        auto proj = [](const T& x) -> long long { return static_cast<long long>(x) * 2; };
        auto fn = [](long long x) -> long long { return x + 100; };
        
        raze::algorithm::transform(vec.begin(), vec.end(), simd_out.begin(), fn, proj);
        std::ranges::transform(vec.begin(), vec.end(), std_out.begin(), fn, proj);
        
        raze_assert(simd_out == std_out);
    }
}

template <typename T>
void test_unary_transform_with_projection_ranges(unsigned seed = 42) {
    for (int i = 0; i < 500; ++i) {
        size_t size = 50 + (i % 200);
        auto vec = generate_random_vector<T>(size, seed + i + 70000);
        
        std::vector<long long> simd_out(size);
        std::vector<long long> std_out(size);
        
        auto proj = [](const T& x) -> long long { return static_cast<long long>(x) + 100; };
        auto fn = [](long long x) -> long long { return x * 3; };
        
        raze::algorithm::transform(vec, simd_out.begin(), fn, proj);
        std::ranges::transform(vec, std_out.begin(), fn, proj);
        
        raze_assert(simd_out == std_out);
    }
}

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};

void test_unary_transform_struct_projection(unsigned seed = 42) {
    for (int i = 0; i < 100; ++i) {
        std::vector<Point> vec(100);
        RandomGenerator<int> gen(seed + i + 80000);
        for (auto& p : vec) {
            p.x = gen();
            p.y = gen();
        }
        
        std::vector<int> simd_out(100);
        std::vector<int> std_out(100);
        
        auto proj_x = [](const Point& p) -> int { return p.x; };
        auto fn = [](int x) -> int { return x * 2; };
        
        raze::algorithm::transform(vec.begin(), vec.end(), simd_out.begin(), fn, proj_x);
        std::ranges::transform(vec.begin(), vec.end(), std_out.begin(), fn, proj_x);
        
        raze_assert(simd_out == std_out);
        
        std::vector<int> simd_out2(100);
        std::vector<int> std_out2(100);
        
        auto proj_y = [](const Point& p) -> int { return p.y; };
        
        raze::algorithm::transform(vec, simd_out2.begin(), fn, proj_y);
        std::ranges::transform(vec, std_out2.begin(), fn, proj_y);
        
        raze_assert(simd_out2 == std_out2);
    }
}

template <typename T>
void test_unary_transform_edge_cases() {
    {
        std::vector<T> vec;
        std::vector<T> simd_out;
        std::vector<T> std_out;
        
        auto simd_result = raze::algorithm::transform(vec.begin(), vec.end(), simd_out.begin(),
            [](const T& x) { return x; });
        auto std_result = std::ranges::transform(vec.begin(), vec.end(), std_out.begin(),
            [](const T& x) { return x; });
        
        raze_assert(simd_result.in == vec.end());
        raze_assert(std_result.in == vec.end());
        raze_assert(simd_result.out == simd_out.end());
        raze_assert(std_result.out == std_out.end());
    }
    
    {
        std::vector<T> vec = {T(42)};
        std::vector<T> simd_out(1);
        std::vector<T> std_out(1);
        
        auto simd_result = raze::algorithm::transform(vec.begin(), vec.end(), simd_out.begin(),
            [](const T& x) { return x; });
        auto std_result = std::ranges::transform(vec.begin(), vec.end(), std_out.begin(),
            [](const T& x) { return x; });
        
        raze_assert(simd_out == std_out);
        raze_assert(simd_out[0] == T(42));
    }
    
    {
        std::vector<T> vec(1000, T(42));
        std::vector<T> simd_out(1000);
        std::vector<T> std_out(1000);
        
        raze::algorithm::transform(vec.begin(), vec.end(), simd_out.begin(),
            [](const T& x) { return static_cast<T>(x + 1); });
        std::ranges::transform(vec.begin(), vec.end(), std_out.begin(),
            [](const T& x) { return static_cast<T>(x + 1); });
        
        raze_assert(simd_out == std_out);
        for (const auto& x : simd_out) {
            raze_assert(x == T(43));
        }
    }
    
    {
        std::vector<T> vec = {T(1), T(2), T(3), T(4), T(5)};
        std::vector<T> simd_out(5);
        std::vector<T> std_out(5);
        
        auto simd_result = raze::algorithm::transform(vec.begin(), vec.end(), simd_out.begin(),
            [](const T& x) { return static_cast<T>(x * 10); });
        auto std_result = std::ranges::transform(vec.begin(), vec.end(), std_out.begin(),
            [](const T& x) { return static_cast<T>(x * 10); });
        
        raze_assert(simd_out == std_out);
        raze_assert(simd_result.in == vec.end());
        raze_assert(simd_result.out == simd_out.end());
    }
    
    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        std::vector<T> vec(size, T(1));
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);
        
        raze::algorithm::transform(vec.begin(), vec.end(), simd_out.begin(),
            [](const T& x) { return static_cast<T>(x * 2); });
        std::ranges::transform(vec.begin(), vec.end(), std_out.begin(),
            [](const T& x) { return static_cast<T>(x * 2); });
        
        raze_assert(simd_out == std_out);
    }
    
    {
        std::vector<T> vec = {T(1), T(2), T(3), T(4), T(5)};
        std::vector<T> simd_out(5);
        std::vector<T> std_out(5);
        
        raze::algorithm::transform(vec.begin(), vec.end(), simd_out.begin(),
            [](const T& x) { return x; });
        std::ranges::transform(vec.begin(), vec.end(), std_out.begin(),
            [](const T& x) { return x; });
        
        raze_assert(simd_out == vec);
        raze_assert(std_out == vec);
    }
}

template <typename T>
void test_unary_transform_return_value() {
    {
        std::vector<T> vec = {T(1), T(2), T(3)};
        std::vector<T> out(3);
        
        auto result = raze::algorithm::transform(vec.begin(), vec.end(), out.begin(),
            [](const T& x) { return x; });
        
        raze_assert(result.in == vec.end());
        raze_assert(result.out == out.end());
    }
    
    {
        std::vector<T> vec;
        std::vector<T> out;
        
        auto result = raze::algorithm::transform(vec.begin(), vec.end(), out.begin(),
            [](const T& x) { return x; });
        
        raze_assert(result.in == vec.end());
        raze_assert(result.out == out.end());
    }
    
    {
        std::vector<T> vec = {T(1), T(2), T(3)};
        std::vector<T> out(3);
        
        auto [in, out_it] = raze::algorithm::transform(vec.begin(), vec.end(), out.begin(),
            [](const T& x) { return x; });
        
        raze_assert(in == vec.end());
        raze_assert(out_it == out.end());
    }
}

template <typename T>
void test_unary_transform_large_vectors(unsigned seed = 42) {
    for (size_t size : {10000, 50000, 100000}) {
        auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size));
        
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);
        
        raze::algorithm::transform(vec.begin(), vec.end(), simd_out.begin(),
            [](const T& x) { return static_cast<T>(x * 2); });
        std::ranges::transform(vec.begin(), vec.end(), std_out.begin(),
            [](const T& x) { return static_cast<T>(x * 2); });
        
        raze_assert(simd_out == std_out);
    }
    
    for (size_t size : {10000, 50000}) {
        auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size) + 999);
        auto vec_copy = vec;
        
        raze::algorithm::transform(vec.begin(), vec.end(), vec.begin(),
            [](const T& x) { return static_cast<T>(x + 1); });
        std::ranges::transform(vec_copy.begin(), vec_copy.end(), vec_copy.begin(),
            [](const T& x) { return static_cast<T>(x + 1); });
        
        raze_assert(vec == vec_copy);
    }
}

template <typename T>
void test_binary_transform_random(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(1, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec1 = generate_random_vector<T>(size, seed + i + 100000);
        auto vec2 = generate_random_vector<T>(size, seed + i + 200000);
        
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);
        
        auto simd_result = raze::algorithm::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), simd_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        auto std_result = std::ranges::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), std_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        
        raze_assert(simd_out == std_out);
        raze_assert(simd_result.in1 == vec1.end());
        raze_assert(simd_result.in2 == vec2.end());
        raze_assert(simd_result.out == simd_out.end());
    }
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec1 = generate_random_vector<T>(size, seed + i + 300000);
        auto vec2 = generate_random_vector<T>(size, seed + i + 400000);
        
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);
        
        auto simd_result = raze::algorithm::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), simd_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a * b); });
        auto std_result = std::ranges::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), std_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a * b); });
        
        raze_assert(simd_out == std_out);
    }
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec1 = generate_random_vector<T>(size, seed + i + 500000);
        auto vec2 = generate_random_vector<T>(size, seed + i + 600000);
        
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);
        
        auto simd_result = raze::algorithm::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), simd_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a - b); });
        auto std_result = std::ranges::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), std_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a - b); });
        
        raze_assert(simd_out == std_out);
    }
}

template <typename T>
void test_binary_transform_ranges(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(1, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec1 = generate_random_vector<T>(size, seed + i + 700000);
        auto vec2 = generate_random_vector<T>(size, seed + i + 800000);
        
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);
        
        auto simd_result = raze::algorithm::transform(vec1, vec2, simd_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        auto std_result = std::ranges::transform(vec1, vec2, std_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        
        raze_assert(simd_out == std_out);
    }
}

template <typename T>
void test_binary_transform_different_sizes(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(1, 500);
    
    for (int i = 0; i < 500; ++i) {
        size_t size1 = size_dist(size_gen);
        size_t size2 = size_dist(size_gen);
        size_t min_size = std::min(size1, size2);
        
        auto vec1 = generate_random_vector<T>(size1, seed + i + 900000);
        auto vec2 = generate_random_vector<T>(size2, seed + i + 1000000);
        
        std::vector<T> simd_out(min_size);
        std::vector<T> std_out(min_size);
        
        auto simd_result = raze::algorithm::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), simd_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        auto std_result = std::ranges::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), std_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        
        raze_assert(simd_out == std_out);
    }
    
    for (int i = 0; i < 100; ++i) {
        size_t size1 = 100;
        size_t size2 = 50;
        size_t min_size = 50;
        
        auto vec1 = generate_random_vector<T>(size1, seed + i + 1100000);
        auto vec2 = generate_random_vector<T>(size2, seed + i + 1200000);
        
        std::vector<T> simd_out(min_size);
        std::vector<T> std_out(min_size);
        
        raze::algorithm::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), simd_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        std::ranges::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), std_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        
        raze_assert(simd_out == std_out);
    }
    
    for (int i = 0; i < 100; ++i) {
        size_t size1 = 50;
        size_t size2 = 100;
        size_t min_size = 50;
        
        auto vec1 = generate_random_vector<T>(size1, seed + i + 1300000);
        auto vec2 = generate_random_vector<T>(size2, seed + i + 1400000);
        
        std::vector<T> simd_out(min_size);
        std::vector<T> std_out(min_size);
        
        raze::algorithm::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), simd_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        std::ranges::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), std_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        
        raze_assert(simd_out == std_out);
    }
}

template <typename T>
void test_binary_transform_with_projection(unsigned seed = 42) {
    for (int i = 0; i < 500; ++i) {
        size_t size = 50 + (i % 200);
        auto vec1 = generate_random_vector<T>(size, seed + i + 1500000);
        auto vec2 = generate_random_vector<T>(size, seed + i + 1600000);
        
        std::vector<long long> simd_out(size);
        std::vector<long long> std_out(size);
        
        auto proj1 = [](const T& x) -> long long { return static_cast<long long>(x) * 2; };
        auto proj2 = [](const T& x) -> long long { return static_cast<long long>(x) + 10; };
        auto fn = [](long long a, long long b) -> long long { return a + b; };
        
        raze::algorithm::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), simd_out.begin(), fn, proj1, proj2);
        std::ranges::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), std_out.begin(), fn, proj1, proj2);
        
        raze_assert(simd_out == std_out);
    }
}

template <typename T>
void test_binary_transform_edge_cases() {
    {
        std::vector<T> vec1;
        std::vector<T> vec2;
        std::vector<T> simd_out;
        std::vector<T> std_out;
        
        auto simd_result = raze::algorithm::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), simd_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        auto std_result = std::ranges::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), std_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        
        raze_assert(simd_result.in1 == vec1.end());
        raze_assert(simd_result.in2 == vec2.end());
        raze_assert(std_result.in1 == vec1.end());
        raze_assert(std_result.in2 == vec2.end());
    }
    
    {
        std::vector<T> vec1 = {T(1), T(2), T(3)};
        std::vector<T> vec2;
        std::vector<T> simd_out;
        std::vector<T> std_out;
        
        auto simd_result = raze::algorithm::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), simd_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        auto std_result = std::ranges::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), std_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        
        raze_assert(simd_result.in1 == vec1.begin());
        raze_assert(simd_result.in2 == vec2.end());
    }
    
    {
        std::vector<T> vec1 = {T(1), T(2), T(3)};
        std::vector<T> vec2 = {T(10), T(20), T(30)};
        std::vector<T> simd_out(3);
        std::vector<T> std_out(3);
        
        raze::algorithm::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), simd_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        std::ranges::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), std_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        
        raze_assert(simd_out == std_out);
        raze_assert(simd_out[0] == T(11));
        raze_assert(simd_out[1] == T(22));
        raze_assert(simd_out[2] == T(33));
    }
    
    {
        std::vector<T> vec1(100, T(1));
        std::vector<T> vec2(100, T(2));
        std::vector<T> simd_out(100);
        std::vector<T> std_out(100);
        
        raze::algorithm::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), simd_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        std::ranges::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), std_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        
        raze_assert(simd_out == std_out);
        for (const auto& x : simd_out) {
            raze_assert(x == T(3));
        }
    }
    
    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        std::vector<T> vec1(size, T(2));
        std::vector<T> vec2(size, T(3));
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);
        
        raze::algorithm::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), simd_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a * b); });
        std::ranges::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), std_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a * b); });
        
        raze_assert(simd_out == std_out);
    }
    
    {
        std::vector<T> vec1 = {T(1), T(2), T(3), T(4), T(5)};
        std::vector<T> vec2 = {T(10), T(20)};
        std::vector<T> simd_out(2);
        std::vector<T> std_out(2);
        
        raze::algorithm::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), simd_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        std::ranges::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), std_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        
        raze_assert(simd_out == std_out);
        raze_assert(simd_out[0] == T(11));
        raze_assert(simd_out[1] == T(22));
    }
    
    {
        std::vector<T> vec1 = {T(10), T(20)};
        std::vector<T> vec2 = {T(1), T(2), T(3), T(4), T(5)};
        std::vector<T> simd_out(2);
        std::vector<T> std_out(2);
        
        raze::algorithm::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), simd_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        std::ranges::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), std_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        
        raze_assert(simd_out == std_out);
        raze_assert(simd_out[0] == T(11));
        raze_assert(simd_out[1] == T(22));
    }
}

template <typename T>
void test_binary_transform_return_value() {
    {
        std::vector<T> vec1 = {T(1), T(2), T(3)};
        std::vector<T> vec2 = {T(10), T(20), T(30)};
        std::vector<T> out(3);
        
        auto result = raze::algorithm::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        
        raze_assert(result.in1 == vec1.end());
        raze_assert(result.in2 == vec2.end());
        raze_assert(result.out == out.end());
    }
    
    {
        std::vector<T> vec1 = {T(1), T(2), T(3), T(4), T(5)};
        std::vector<T> vec2 = {T(10), T(20)};
        std::vector<T> out(2);
        
        auto result = raze::algorithm::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        
        raze_assert(result.in1 == vec1.begin() + 2);
        raze_assert(result.in2 == vec2.end());
        raze_assert(result.out == out.end());
    }
    
    {
        std::vector<T> vec1;
        std::vector<T> vec2;
        std::vector<T> out;
        
        auto result = raze::algorithm::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        
        raze_assert(result.in1 == vec1.end());
        raze_assert(result.in2 == vec2.end());
        raze_assert(result.out == out.end());
    }
    
    {
        std::vector<T> vec1 = {T(1), T(2), T(3)};
        std::vector<T> vec2 = {T(10), T(20), T(30)};
        std::vector<T> out(3);
        
        auto [in1, in2, out_it] = raze::algorithm::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        
        raze_assert(in1 == vec1.end());
        raze_assert(in2 == vec2.end());
        raze_assert(out_it == out.end());
    }
}

template <typename T>
void test_binary_transform_large_vectors(unsigned seed = 42) {
    for (size_t size : {10000, 50000, 100000}) {
        auto vec1 = generate_random_vector<T>(size, seed + static_cast<unsigned>(size));
        auto vec2 = generate_random_vector<T>(size, seed + static_cast<unsigned>(size) + 1);
        
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);
        
        raze::algorithm::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), simd_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        std::ranges::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), std_out.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        
        raze_assert(simd_out == std_out);
    }
}

template <typename T>
void test_binary_transform_in_place(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(1, 1000);
    
    for (int i = 0; i < 500; ++i) {
        size_t size = size_dist(size_gen);
        auto vec1 = generate_random_vector<T>(size, seed + i + 1700000);
        auto vec2 = generate_random_vector<T>(size, seed + i + 1800000);
        auto vec1_copy = vec1;
        auto vec2_copy = vec2;
        
        raze::algorithm::transform(vec1.begin(), vec1.end(),
            vec2.begin(), vec2.end(), vec1.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        std::ranges::transform(vec1_copy.begin(), vec1_copy.end(),
            vec2_copy.begin(), vec2_copy.end(), vec1_copy.begin(),
            [](const T& a, const T& b) { return static_cast<T>(a + b); });
        
        raze_assert(vec1 == vec1_copy);
    }
}

template <typename T>
void run_all_tests_for_type() {
    test_unary_transform_random<T>();
    test_unary_transform_ranges<T>();
    test_unary_transform_in_place<T>();
    test_unary_transform_with_projection<T>();
    test_unary_transform_with_projection_ranges<T>();
    test_unary_transform_edge_cases<T>();
    test_unary_transform_return_value<T>();
    test_unary_transform_large_vectors<T>();
    
    test_binary_transform_random<T>();
    test_binary_transform_ranges<T>();
    test_binary_transform_different_sizes<T>();
    test_binary_transform_with_projection<T>();
    test_binary_transform_edge_cases<T>();
    test_binary_transform_return_value<T>();
    test_binary_transform_large_vectors<T>();
    test_binary_transform_in_place<T>();
}

int main() {
    run_all_tests_for_type<int>();
    run_all_tests_for_type<short>();
    run_all_tests_for_type<long long>();
    run_all_tests_for_type<char>();
    
    run_all_tests_for_type<unsigned int>();
    run_all_tests_for_type<unsigned short>();
    run_all_tests_for_type<unsigned long long>();
    run_all_tests_for_type<unsigned char>();
    
    run_all_tests_for_type<float>();
    run_all_tests_for_type<double>();
    
    test_unary_transform_struct_projection();
    
    return 0;
}