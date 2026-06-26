#include <raze/algorithm/replace/ReplaceCopy.h>

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
            }
            else {
                std::uniform_int_distribution<DistType> dist(0, 2000);
                return static_cast<T>(dist(gen));
            }
        }
        else if constexpr (std::is_floating_point_v<T>) {
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
void test_replace_copy_random(unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);

    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i);
        auto vec_original = vec;
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);
        T old_val = gen();
        T new_val = gen();

        auto simd_result = raze::algorithm::replace_copy(vec.begin(), vec.end(), simd_out.begin(), old_val, new_val);
        auto std_result = std::ranges::replace_copy(vec.begin(), vec.end(), std_out.begin(), old_val, new_val);

        raze_assert(simd_out == std_out);
        raze_assert(vec == vec_original);
        raze_assert(simd_result.in == vec.end());
        raze_assert(simd_result.out == simd_out.end());
    }

    for (int i = 0; i < 500; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 10000);
        auto vec_original = vec;
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);

        T old_val = vec.empty() ? T(0) : vec[size / 2];
        T new_val = gen();
        new_val = new_val == old_val ? static_cast<T>(new_val - 1) : new_val;

        raze::algorithm::replace_copy(vec.begin(), vec.end(), simd_out.begin(), old_val, new_val);
        std::ranges::replace_copy(vec.begin(), vec.end(), std_out.begin(), old_val, new_val);

        raze_assert(simd_out == std_out);
        raze_assert(vec == vec_original);

        for (const auto& x : simd_out) {
            raze_assert(x != old_val);
        }
    }

    for (int i = 0; i < 500; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 20000);
        auto vec_original = vec;
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);

        T old_val = gen();
        T new_val = old_val;

        raze::algorithm::replace_copy(vec.begin(), vec.end(), simd_out.begin(), old_val, new_val);
        std::ranges::replace_copy(vec.begin(), vec.end(), std_out.begin(), old_val, new_val);

        raze_assert(simd_out == std_out);
        raze_assert(vec == vec_original);
    }
}

template <typename T>
void test_replace_copy_ranges(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    RandomGenerator<T> gen(seed);

    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 30000);
        auto vec_original = vec;
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);
        T old_val = gen();
        T new_val = gen();

        auto simd_result = raze::algorithm::replace_copy(vec, simd_out.begin(), old_val, new_val);
        auto std_result = std::ranges::replace_copy(vec, std_out.begin(), old_val, new_val);

        raze_assert(simd_out == std_out);
        raze_assert(vec == vec_original);
        raze_assert(simd_result.in == vec.end());
        raze_assert(simd_result.out == simd_out.end());
    }
}

template <typename T>
void test_replace_copy_if_random(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);

    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 40000);
        auto vec_original = vec;
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);
        T threshold = RandomGenerator<T>(seed + i + 40000)();

        auto pred = [threshold](const T& x) { return x > threshold; };
        T new_val = RandomGenerator<T>(seed + i + 50000)();

        raze::algorithm::replace_copy_if(vec.begin(), vec.end(), simd_out.begin(), pred, new_val);
        std::ranges::replace_copy_if(vec.begin(), vec.end(), std_out.begin(), pred, new_val);

        raze_assert(simd_out == std_out);
        raze_assert(vec == vec_original);
    }

    for (int i = 0; i < 500; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 60000);
        auto vec_original = vec;
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);
        T target = RandomGenerator<T>(seed + i + 60000)();

        auto pred = [target](const T& x) { return x == target; };
        T new_val = RandomGenerator<T>(seed + i + 70000)();

        raze::algorithm::replace_copy_if(vec.begin(), vec.end(), simd_out.begin(), pred, new_val);
        std::ranges::replace_copy_if(vec.begin(), vec.end(), std_out.begin(), pred, new_val);

        raze_assert(simd_out == std_out);
        raze_assert(vec == vec_original);
    }

    for (int i = 0; i < 100; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 80000);
        auto vec_original = vec;
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);

        auto pred = [](const T&) { return false; };
        T new_val = RandomGenerator<T>(seed + i + 90000)();

        raze::algorithm::replace_copy_if(vec.begin(), vec.end(), simd_out.begin(), pred, new_val);
        std::ranges::replace_copy_if(vec.begin(), vec.end(), std_out.begin(), pred, new_val);

        raze_assert(simd_out == std_out);
        raze_assert(simd_out == vec);
        raze_assert(vec == vec_original);
    }

    for (int i = 0; i < 100; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 100000);
        auto vec_original = vec;
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);

        auto pred = [](const T&) { return true; };
        T new_val = RandomGenerator<T>(seed + i + 110000)();

        raze::algorithm::replace_copy_if(vec.begin(), vec.end(), simd_out.begin(), pred, new_val);
        std::ranges::replace_copy_if(vec.begin(), vec.end(), std_out.begin(), pred, new_val);

        raze_assert(simd_out == std_out);
        raze_assert(vec == vec_original);

        for (const auto& x : simd_out) {
            raze_assert(x == new_val);
        }
    }
}

template <typename T>
void test_replace_copy_if_ranges(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);

    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 120000);
        auto vec_original = vec;
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);
        T threshold = RandomGenerator<T>(seed + i + 120000)();

        auto pred = [threshold](const T& x) { return x > threshold; };
        T new_val = RandomGenerator<T>(seed + i + 130000)();

        raze::algorithm::replace_copy_if(vec, simd_out.begin(), pred, new_val);
        std::ranges::replace_copy_if(vec, std_out.begin(), pred, new_val);

        raze_assert(simd_out == std_out);
        raze_assert(vec == vec_original);
    }
}

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};

template <typename T>
void test_replace_copy_with_projection(unsigned seed = 42) {
    for (int i = 0; i < 100; ++i) {
        std::vector<Point> vec(100);
        RandomGenerator<int> gen(seed + i + 140000);
        for (auto& p : vec) {
            p.x = gen();
            p.y = gen();
        }
        auto vec_original = vec;
        std::vector<Point> simd_out(100);
        std::vector<Point> std_out(100);

        int target_x = gen();
        Point new_point{ 999, 888 };

        auto proj = [](const Point& p) { return p.x; };

        raze::algorithm::replace_copy(vec.begin(), vec.end(), simd_out.begin(), target_x, new_point, proj);
        std::ranges::replace_copy(vec.begin(), vec.end(), std_out.begin(), target_x, new_point, proj);

        raze_assert(simd_out == std_out);
        raze_assert(vec == vec_original);
    }

    for (int i = 0; i < 100; ++i) {
        std::vector<Point> vec(100);
        RandomGenerator<int> gen(seed + i + 150000);
        for (auto& p : vec) {
            p.x = gen();
            p.y = gen();
        }
        auto vec_original = vec;
        std::vector<Point> simd_out(100);
        std::vector<Point> std_out(100);

        int threshold = gen();
        Point new_point{ 777, 666 };

        auto pred = [threshold](int x) { return x > threshold; };
        auto proj = [](const Point& p) { return p.x; };

        raze::algorithm::replace_copy_if(vec.begin(), vec.end(), simd_out.begin(), pred, new_point, proj);
        std::ranges::replace_copy_if(vec.begin(), vec.end(), std_out.begin(), pred, new_point, proj);

        raze_assert(simd_out == std_out);
        raze_assert(vec == vec_original);
    }
}

template <typename T>
void test_replace_copy_edge_cases() {
    {
        std::vector<T> vec;
        std::vector<T> simd_out;
        std::vector<T> std_out;

        raze::algorithm::replace_copy(vec.begin(), vec.end(), simd_out.begin(), T(42), T(99));
        std::ranges::replace_copy(vec.begin(), vec.end(), std_out.begin(), T(42), T(99));

        raze_assert(simd_out == std_out);
    }

    {
        std::vector<T> vec = { T(42) };
        std::vector<T> simd_out(1);
        std::vector<T> std_out(1);

        raze::algorithm::replace_copy(vec.begin(), vec.end(), simd_out.begin(), T(42), T(99));
        std::ranges::replace_copy(vec.begin(), vec.end(), std_out.begin(), T(42), T(99));

        raze_assert(simd_out == std_out);
        raze_assert(simd_out[0] == T(99));
        raze_assert(vec[0] == T(42));
    }

    {
        std::vector<T> vec = { T(42) };
        std::vector<T> simd_out(1);
        std::vector<T> std_out(1);

        raze::algorithm::replace_copy(vec.begin(), vec.end(), simd_out.begin(), T(99), T(0));
        std::ranges::replace_copy(vec.begin(), vec.end(), std_out.begin(), T(99), T(0));

        raze_assert(simd_out == std_out);
        raze_assert(simd_out[0] == T(42));
    }

    {
        std::vector<T> vec(100, T(42));
        std::vector<T> simd_out(100);
        std::vector<T> std_out(100);

        raze::algorithm::replace_copy(vec.begin(), vec.end(), simd_out.begin(), T(42), T(99));
        std::ranges::replace_copy(vec.begin(), vec.end(), std_out.begin(), T(42), T(99));

        raze_assert(simd_out == std_out);
        for (const auto& x : simd_out) {
            raze_assert(x == T(99));
        }
        for (const auto& x : vec) {
            raze_assert(x == T(42));
        }
    }

    {
        std::vector<T> vec = { T(1), T(2), T(3), T(2), T(1), T(2), T(3) };
        std::vector<T> simd_out(7);
        std::vector<T> std_out(7);

        raze::algorithm::replace_copy(vec.begin(), vec.end(), simd_out.begin(), T(2), T(99));
        std::ranges::replace_copy(vec.begin(), vec.end(), std_out.begin(), T(2), T(99));

        raze_assert(simd_out == std_out);
        raze_assert(simd_out[0] == T(1));
        raze_assert(simd_out[1] == T(99));
        raze_assert(simd_out[2] == T(3));
        raze_assert(simd_out[3] == T(99));
        raze_assert(simd_out[4] == T(1));
        raze_assert(simd_out[5] == T(99));
        raze_assert(simd_out[6] == T(3));
    }

    {
        std::vector<T> vec;
        std::vector<T> simd_out;
        std::vector<T> std_out;

        auto pred = [](const T&) { return true; };
        raze::algorithm::replace_copy_if(vec.begin(), vec.end(), simd_out.begin(), pred, T(42));
        std::ranges::replace_copy_if(vec.begin(), vec.end(), std_out.begin(), pred, T(42));

        raze_assert(simd_out == std_out);
    }

    {
        std::vector<T> vec = { T(1), T(2), T(3), T(4), T(5) };
        std::vector<T> simd_out(5);
        std::vector<T> std_out(5);

        auto pred = [](const T& x) { return x / 2 == 0; };
        raze::algorithm::replace_copy_if(vec.begin(), vec.end(), simd_out.begin(), pred, T(0));
        std::ranges::replace_copy_if(vec.begin(), vec.end(), std_out.begin(), pred, T(0));

        raze_assert(simd_out == std_out);
    }
}

template <typename T>
void test_replace_copy_return_value() {
    {
        std::vector<T> vec = { T(1), T(2), T(3) };
        std::vector<T> out(3);

        auto result = raze::algorithm::replace_copy(vec.begin(), vec.end(), out.begin(), T(2), T(99));

        raze_assert(result.in == vec.end());
        raze_assert(result.out == out.end());
    }

    {
        std::vector<T> vec;
        std::vector<T> out;

        auto result = raze::algorithm::replace_copy(vec.begin(), vec.end(), out.begin(), T(42), T(99));

        raze_assert(result.in == vec.end());
        raze_assert(result.out == out.end());
    }

    {
        std::vector<T> vec = { T(1), T(2), T(3) };
        std::vector<T> out(3);

        auto [in, out_it] = raze::algorithm::replace_copy(vec.begin(), vec.end(), out.begin(), T(2), T(99));

        raze_assert(in == vec.end());
        raze_assert(out_it == out.end());
    }

    {
        std::vector<T> vec = { T(1), T(2), T(3) };
        std::vector<T> out(3);

        auto pred = [](const T& x) { return x == T(2); };
        auto result = raze::algorithm::replace_copy_if(vec.begin(), vec.end(), out.begin(), pred, T(99));

        raze_assert(result.in == vec.end());
        raze_assert(result.out == out.end());
    }
}

template <typename T>
void test_replace_copy_simd_boundaries(unsigned seed = 42) {
    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256, 511, 512}) {
        std::vector<T> vec(size, T(42));
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);

        raze::algorithm::replace_copy(vec.begin(), vec.end(), simd_out.begin(), T(42), T(99));
        std::ranges::replace_copy(vec.begin(), vec.end(), std_out.begin(), T(42), T(99));

        raze_assert(simd_out == std_out);
        for (const auto& x : simd_out) {
            raze_assert(x == T(99));
        }
    }

    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size));
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);

        T old_val = vec.empty() ? T(0) : vec[0];
        T new_val = static_cast<T>(old_val + T(1000));

        raze::algorithm::replace_copy(vec.begin(), vec.end(), simd_out.begin(), old_val, new_val);
        std::ranges::replace_copy(vec.begin(), vec.end(), std_out.begin(), old_val, new_val);

        raze_assert(simd_out == std_out);
    }

    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size) + 5000);
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);

        auto pred = [](const T& x) { return x > T(0); };
        T new_val = T(0);

        raze::algorithm::replace_copy_if(vec.begin(), vec.end(), simd_out.begin(), pred, new_val);
        std::ranges::replace_copy_if(vec.begin(), vec.end(), std_out.begin(), pred, new_val);

        raze_assert(simd_out == std_out);
    }
}

template <typename T>
void test_replace_copy_large_vectors(unsigned seed = 42) {
    for (size_t size : {10000, 50000, 100000}) {
        auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size));
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);

        T old_val = vec.empty() ? T(0) : vec[size / 3];
        T new_val = static_cast<T>(old_val + T(1));

        raze::algorithm::replace_copy(vec.begin(), vec.end(), simd_out.begin(), old_val, new_val);
        std::ranges::replace_copy(vec.begin(), vec.end(), std_out.begin(), old_val, new_val);

        raze_assert(simd_out == std_out);
    }

    for (size_t size : {10000, 50000, 100000}) {
        auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size) + 999);
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);

        auto pred = [](const T& x) { return x > T(0); };
        T new_val = T(0);

        raze::algorithm::replace_copy_if(vec.begin(), vec.end(), simd_out.begin(), pred, new_val);
        std::ranges::replace_copy_if(vec.begin(), vec.end(), std_out.begin(), pred, new_val);

        raze_assert(simd_out == std_out);
    }
}

template <typename T>
void test_replace_copy_all_same(unsigned seed = 42) {
    for (size_t size : {1, 10, 100, 1000}) {
        std::vector<T> vec(size, T(42));
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);

        raze::algorithm::replace_copy(vec.begin(), vec.end(), simd_out.begin(), T(42), T(99));
        std::ranges::replace_copy(vec.begin(), vec.end(), std_out.begin(), T(42), T(99));

        raze_assert(simd_out == std_out);

        for (const auto& x : simd_out) {
            raze_assert(x == T(99));
        }
    }

    for (size_t size : {1, 10, 100, 1000}) {
        std::vector<T> vec(size, T(42));
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);

        auto pred = [](const T& x) { return x == T(42); };
        raze::algorithm::replace_copy_if(vec.begin(), vec.end(), simd_out.begin(), pred, T(99));
        std::ranges::replace_copy_if(vec.begin(), vec.end(), std_out.begin(), pred, T(99));

        raze_assert(simd_out == std_out);

        for (const auto& x : simd_out) {
            raze_assert(x == T(99));
        }
    }
}

template <typename T>
void test_replace_copy_none_match(unsigned seed = 42) {
    for (size_t size : {1, 10, 100, 1000}) {
        std::vector<T> vec(size, T(42));
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);

        raze::algorithm::replace_copy(vec.begin(), vec.end(), simd_out.begin(), T(99), T(0));
        std::ranges::replace_copy(vec.begin(), vec.end(), std_out.begin(), T(99), T(0));

        raze_assert(simd_out == std_out);
        raze_assert(simd_out == vec);

        for (const auto& x : simd_out) {
            raze_assert(x == T(42));
        }
    }

    for (size_t size : {1, 10, 100, 1000}) {
        std::vector<T> vec(size, T(42));
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);

        auto pred = [](const T& x) { return x == T(99); };
        raze::algorithm::replace_copy_if(vec.begin(), vec.end(), simd_out.begin(), pred, T(0));
        std::ranges::replace_copy_if(vec.begin(), vec.end(), std_out.begin(), pred, T(0));

        raze_assert(simd_out == std_out);
        raze_assert(simd_out == vec);

        for (const auto& x : simd_out) {
            raze_assert(x == T(42));
        }
    }
}

template <typename T>
void test_replace_copy_mixed_pattern(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(10, 500);

    for (int i = 0; i < 500; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 200000);
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);

        T old_val = vec[size / 4];
        T new_val = RandomGenerator<T>(seed + i + 210000)();

        raze::algorithm::replace_copy(vec.begin(), vec.end(), simd_out.begin(), old_val, new_val);
        std::ranges::replace_copy(vec.begin(), vec.end(), std_out.begin(), old_val, new_val);

        raze_assert(simd_out == std_out);

        for (const auto& x : simd_out) {
            if (x == old_val && old_val != new_val) {
                raze_assert(false);
            }
        }
    }
}

template <typename T>
void test_replace_copy_source_unchanged(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(1, 500);

    for (int i = 0; i < 500; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 300000);
        auto vec_original = vec;
        std::vector<T> out(size);

        T old_val = vec.empty() ? T(0) : vec[0];
        T new_val = static_cast<T>(old_val + T(1));

        raze::algorithm::replace_copy(vec.begin(), vec.end(), out.begin(), old_val, new_val);

        raze_assert(vec == vec_original);
    }

    for (int i = 0; i < 500; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 400000);
        auto vec_original = vec;
        std::vector<T> out(size);

        auto pred = [](const T& x) { return x > T(0); };
        T new_val = T(0);

        raze::algorithm::replace_copy_if(vec.begin(), vec.end(), out.begin(), pred, new_val);

        raze_assert(vec == vec_original);
    }
}

template <typename T>
void run_all_tests_for_type() {
    test_replace_copy_random<T>();
    test_replace_copy_ranges<T>();
    test_replace_copy_if_random<T>();
    test_replace_copy_if_ranges<T>();
    test_replace_copy_with_projection<T>();
    test_replace_copy_edge_cases<T>();
    test_replace_copy_return_value<T>();
    test_replace_copy_simd_boundaries<T>();
    test_replace_copy_large_vectors<T>();
    test_replace_copy_all_same<T>();
    test_replace_copy_none_match<T>();
    test_replace_copy_mixed_pattern<T>();
    test_replace_copy_source_unchanged<T>();
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

    test_replace_copy_with_projection<int>();

    return 0;
}