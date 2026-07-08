#include <raze/algorithm/batch/ForEach.h>

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
struct SumAccumulator {
    long long sum = 0;
    size_t count = 0;
    
    void operator()(const T& val) {
        sum += static_cast<long long>(val);
        ++count;
    }
    
    bool operator==(const SumAccumulator& other) const = default;
};

template <typename T>
struct MultiplyBy {
    T factor;
    size_t count = 0;
    
    explicit MultiplyBy(T f) : factor(f) {}
    
    void operator()(T& val) {
        val *= factor;
        ++count;
    }
};

template <typename T>
struct CopyTo {
    T* dest;
    size_t index = 0;
    
    explicit CopyTo(T* d) : dest(d) {}
    
    void operator()(const T& val) {
        dest[index++] = val;
    }
};

template <typename T>
void test_for_each_random(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i);
        auto vec_copy = vec;
        
        SumAccumulator<T> simd_acc;
        SumAccumulator<T> std_acc;
        
        auto simd_result = raze::algorithm::for_each(vec.begin(), vec.end(), simd_acc);
        auto std_result = std::ranges::for_each(vec_copy.begin(), vec_copy.end(), std_acc);
        
        raze_assert(simd_result.fun.sum == std_result.fun.sum);
        raze_assert(simd_result.fun.count == std_result.fun.count);
        raze_assert(simd_result.fun.count == size);
        
        raze_assert(vec == vec_copy);
    }
}

template <typename T>
void test_for_each_ranges(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 100000);
        auto vec_copy = vec;
        
        SumAccumulator<T> simd_acc;
        SumAccumulator<T> std_acc;
        
        auto simd_result = raze::algorithm::for_each(vec, simd_acc);
        auto std_result = std::ranges::for_each(vec_copy, std_acc);
        
        raze_assert(simd_result.fun.sum == std_result.fun.sum);
        raze_assert(simd_result.fun.count == std_result.fun.count);
        raze_assert(simd_result.fun.count == size);
    }
}

template <typename T>
void test_for_each_modifying(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 200000);
        auto vec_copy = vec;
        
        auto simd_result = raze::algorithm::for_each(vec.begin(), vec.end(), 
            [](T& x) { x = static_cast<T>(x * 2); });
        auto std_result = std::ranges::for_each(vec_copy.begin(), vec_copy.end(), 
            [](T& x) { x = static_cast<T>(x * 2); });
        
        raze_assert(vec == vec_copy);
        raze_assert(simd_result.in == vec.end());
    }
    
    for (int i = 0; i < 500; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 300000);
        auto vec_copy = vec;
        
        raze::algorithm::for_each(vec.begin(), vec.end(), 
            [](T& x) { x = static_cast<T>(x + 1); });
        std::ranges::for_each(vec_copy.begin(), vec_copy.end(), 
            [](T& x) { x = static_cast<T>(x + 1); });
        
        raze_assert(vec == vec_copy);
    }
}

template <typename T>
void test_for_each_modifying_ranges(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 400000);
        auto vec_copy = vec;
        
        raze::algorithm::for_each(vec, [](T& x) { x = static_cast<T>(x * 3); });
        std::ranges::for_each(vec_copy, [](T& x) { x = static_cast<T>(x * 3); });
        
        raze_assert(vec == vec_copy);
    }
}

template <typename T>
void test_for_each_copy(unsigned seed = 42) {
    for (int i = 0; i < 500; ++i) {
        size_t size = 50 + (i % 200);
        auto vec = generate_random_vector<T>(size, seed + i + 500000);
        
        std::vector<T> simd_buf(size);
        std::vector<T> std_buf(size);
        
        CopyTo<T> simd_copier(simd_buf.data());
        CopyTo<T> std_copier(std_buf.data());
        
        auto simd_result = raze::algorithm::for_each(vec.begin(), vec.end(), simd_copier);
        auto std_result = std::ranges::for_each(vec.begin(), vec.end(), std_copier);
        
        raze_assert(simd_buf == std_buf);
        raze_assert(simd_buf == vec);
        raze_assert(simd_result.fun.index == size);
    }
}

template <typename T>
void test_for_each_with_projection(unsigned seed = 42) {
    for (int i = 0; i < 500; ++i) {
        size_t size = 50 + (i % 200);
        auto vec = generate_random_vector<T>(size, seed + i + 600000);
        auto vec_copy = vec;
        
        long long simd_sum = 0;
        long long std_sum = 0;
        
        auto simd_fn = [&simd_sum](long long x) { simd_sum += x; };
        auto std_fn = [&std_sum](long long x) { std_sum += x; };
        
        auto proj = [](const T& x) -> long long { return static_cast<long long>(x) * 2; };
        
        raze::algorithm::for_each(vec.begin(), vec.end(), simd_fn, proj);
        std::ranges::for_each(vec_copy.begin(), vec_copy.end(), std_fn, proj);
        
        raze_assert(simd_sum == std_sum);
        raze_assert(vec == vec_copy);
    }
}

template <typename T>
void test_for_each_with_projection_ranges(unsigned seed = 42) {
    for (int i = 0; i < 500; ++i) {
        size_t size = 50 + (i % 200);
        auto vec = generate_random_vector<T>(size, seed + i + 700000);
        
        long long simd_sum = 0;
        long long std_sum = 0;
        
        auto simd_fn = [&simd_sum](long long x) { simd_sum += x; };
        auto std_fn = [&std_sum](long long x) { std_sum += x; };
        
        auto proj = [](const T& x) -> long long { return static_cast<long long>(x) + 100; };
        
        raze::algorithm::for_each(vec, simd_fn, proj);
        std::ranges::for_each(vec, std_fn, proj);
        
        raze_assert(simd_sum == std_sum);
    }
}

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};

void test_for_each_struct_projection(unsigned seed = 42) {
    for (int i = 0; i < 100; ++i) {
        std::vector<Point> vec(100);
        RandomGenerator<int> gen(seed + i + 800000);
        for (auto& p : vec) {
            p.x = gen();
            p.y = gen();
        }
        auto vec_copy = vec;
        
        long long simd_sum_x = 0;
        long long std_sum_x = 0;
        
        auto simd_fn = [&simd_sum_x](int x) { simd_sum_x += x; };
        auto std_fn = [&std_sum_x](int x) { std_sum_x += x; };
        
        auto proj_x = [](const Point& p) -> int { return p.x; };
        
        raze::algorithm::for_each(vec.begin(), vec.end(), simd_fn, proj_x);
        std::ranges::for_each(vec_copy.begin(), vec_copy.end(), std_fn, proj_x);
        
        raze_assert(simd_sum_x == std_sum_x);
        
        long long simd_sum_y = 0;
        long long std_sum_y = 0;
        
        auto simd_fn_y = [&simd_sum_y](int y) { simd_sum_y += y; };
        auto std_fn_y = [&std_sum_y](int y) { std_sum_y += y; };
        
        auto proj_y = [](const Point& p) -> int { return p.y; };
        
        raze::algorithm::for_each(vec, simd_fn_y, proj_y);
        std::ranges::for_each(vec_copy, std_fn_y, proj_y);
        
        raze_assert(simd_sum_y == std_sum_y);
    }
}

template <typename T>
void test_for_each_edge_cases() {
    {
        std::vector<T> vec;
        SumAccumulator<T> simd_acc;
        SumAccumulator<T> std_acc;
        
        auto simd_result = raze::algorithm::for_each(vec.begin(), vec.end(), simd_acc);
        auto std_result = std::ranges::for_each(vec.begin(), vec.end(), std_acc);
        
        raze_assert(simd_result.fun.sum == std_result.fun.sum);
        raze_assert(simd_result.fun.count == 0);
        raze_assert(std_result.fun.count == 0);
    }
    
    {
        std::vector<T> vec = {T(42)};
        SumAccumulator<T> simd_acc;
        SumAccumulator<T> std_acc;
        
        auto simd_result = raze::algorithm::for_each(vec.begin(), vec.end(), simd_acc);
        auto std_result = std::ranges::for_each(vec.begin(), vec.end(), std_acc);
        
        raze_assert(simd_result.fun.sum == std_result.fun.sum);
        raze_assert(simd_result.fun.count == 1);
    }
    
    {
        std::vector<T> vec(1000, T(42));
        SumAccumulator<T> simd_acc;
        SumAccumulator<T> std_acc;
        
        auto simd_result = raze::algorithm::for_each(vec.begin(), vec.end(), simd_acc);
        auto std_result = std::ranges::for_each(vec.begin(), vec.end(), std_acc);
        
        raze_assert(simd_result.fun.sum == std_result.fun.sum);
        raze_assert(simd_result.fun.count == 1000);
    }
    
    {
        std::vector<T> vec(500, T(0));
        SumAccumulator<T> simd_acc;
        SumAccumulator<T> std_acc;
        
        auto simd_result = raze::algorithm::for_each(vec.begin(), vec.end(), simd_acc);
        auto std_result = std::ranges::for_each(vec.begin(), vec.end(), std_acc);
        
        raze_assert(simd_result.fun.sum == 0);
        raze_assert(std_result.fun.sum == 0);
        raze_assert(simd_result.fun.count == 500);
    }
    
    {
        std::vector<T> vec = {T(1), T(2), T(3), T(4), T(5)};
        auto result = raze::algorithm::for_each(vec.begin(), vec.end(), [](T&) {});
        raze_assert(result.in == vec.end());
    }
    
    {
        std::vector<T> vec = {T(1), T(2), T(3), T(4), T(5)};
        auto vec_copy = vec;
        
        raze::algorithm::for_each(vec.begin(), vec.end(), [](T& x) { x = T(0); });
        std::ranges::for_each(vec_copy.begin(), vec_copy.end(), [](T& x) { x = T(0); });
        
        raze_assert(vec == vec_copy);
        for (const auto& x : vec) {
            raze_assert(x == T(0));
        }
    }
    
    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        std::vector<T> vec(size, T(1));
        auto vec_copy = vec;
        
        SumAccumulator<T> simd_acc;
        SumAccumulator<T> std_acc;
        
        raze::algorithm::for_each(vec.begin(), vec.end(), simd_acc);
        std::ranges::for_each(vec_copy.begin(), vec_copy.end(), std_acc);
        
        raze_assert(simd_acc.sum == std_acc.sum);
    }
}

template <typename T>
void test_for_each_return_value() {
    {
        std::vector<T> vec = {T(1), T(2), T(3)};
        SumAccumulator<T> acc;
        
        auto result = raze::algorithm::for_each(vec.begin(), vec.end(), acc);
        
        raze_assert(result.in == vec.end());
        raze_assert(result.fun.sum == 6);
        raze_assert(result.fun.count == 3);
    }
    
    {
        std::vector<T> vec;
        SumAccumulator<T> acc;
        
        auto result = raze::algorithm::for_each(vec.begin(), vec.end(), acc);
        
        raze_assert(result.in == vec.end());
        raze_assert(result.fun.sum == 0);
        raze_assert(result.fun.count == 0);
    }
}

template <typename T>
void test_for_each_large_vectors(unsigned seed = 42) {
    for (size_t size : {10000, 50000, 100000}) {
        auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size));
        auto vec_copy = vec;
        
        SumAccumulator<T> simd_acc;
        SumAccumulator<T> std_acc;
        
        raze::algorithm::for_each(vec.begin(), vec.end(), simd_acc);
        std::ranges::for_each(vec_copy.begin(), vec_copy.end(), std_acc);
        
        raze_assert(simd_acc.sum == std_acc.sum);
    }
    
    for (size_t size : {10000, 50000}) {
        auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size) + 999);
        auto vec_copy = vec;
        
        raze::algorithm::for_each(vec.begin(), vec.end(), [](auto& x) { x = x + 1; });
        std::ranges::for_each(vec_copy.begin(), vec_copy.end(), [](auto& x) { x = x + 1; });
        
        raze_assert(vec == vec_copy);
    }
}

template <typename T>
void run_all_tests_for_type() {
    test_for_each_random<T>();
    test_for_each_ranges<T>();
    test_for_each_modifying<T>();
    test_for_each_modifying_ranges<T>();
    test_for_each_copy<T>();
    test_for_each_with_projection<T>();
    test_for_each_with_projection_ranges<T>();
    test_for_each_edge_cases<T>();
    test_for_each_return_value<T>();
    test_for_each_large_vectors<T>();
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
    
    test_for_each_struct_projection();
    
    return 0;
}