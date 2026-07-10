#include <raze/algorithm/extreme/Min.h> 
#include <raze/algorithm/extreme/Max.h>
#include <raze/algorithm/extreme/MinMax.h>

#include <vector>
#include <random>
#include <type_traits>
#include <algorithm>
#include <cassert>
#include <optional>

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
std::vector<T> generate_vector_with_target(size_t size, T target, size_t target_pos, unsigned seed = 42) {
    auto vec = generate_random_vector<T>(size, seed);
    if (target_pos < size) {
        vec[target_pos] = target;
    }
    return vec;
}

template <typename T>
void test_min_edge_cases() {
    {
        std::vector<T> vec;
        auto raze_res = raze::algorithm::min(vec.begin(), vec.end());
        auto std_res_iter = std::ranges::min_element(vec.begin(), vec.end());

        raze_assert(!raze_res.has_value());
        raze_assert(std_res_iter == vec.end());
    }

    {
        std::vector<T> vec = { T(42) };
        auto raze_res = raze::algorithm::min(vec.begin(), vec.end());
        auto std_res = std::ranges::min(vec);

        raze_assert(raze_res.has_value());
        raze_assert(raze_res.value() == std_res);
        raze_assert(raze_res.value() == T(42));
    }

    {
        std::vector<T> vec(100, T(5));
        auto raze_res = raze::algorithm::min(vec.begin(), vec.end());
        auto std_res = std::ranges::min(vec);

        raze_assert(raze_res.has_value());
        raze_assert(raze_res.value() == std_res);
        raze_assert(raze_res.value() == T(5));
    }

    {
        std::vector<T> vec = { T(1), T(10), T(20), T(30) };
        auto raze_res = raze::algorithm::min(vec.begin(), vec.end());
        auto std_res = std::ranges::min(vec);

        raze_assert(raze_res.has_value());
        raze_assert(raze_res.value() == std_res);
        raze_assert(raze_res.value() == T(1));
    }

    {
        std::vector<T> vec = { T(30), T(20), T(10), T(1) };
        auto raze_res = raze::algorithm::min(vec.begin(), vec.end());
        auto std_res = std::ranges::min(vec);

        raze_assert(raze_res.has_value());
        raze_assert(raze_res.value() == std_res);
        raze_assert(raze_res.value() == T(1));
    }

    if constexpr (std::is_signed_v<T>) {
        std::vector<T> vec = { T(10), T(-50), T(0), T(5) };
        auto raze_res = raze::algorithm::min(vec.begin(), vec.end());
        auto std_res = std::ranges::min(vec);

        raze_assert(raze_res.has_value());
        raze_assert(raze_res.value() == std_res);
        raze_assert(raze_res.value() == T(-50));
    }
}

template <typename T>
void test_max_edge_cases() {
    {
        std::vector<T> vec;
        auto raze_res = raze::algorithm::max(vec.begin(), vec.end());
        auto std_res_iter = std::ranges::max_element(vec.begin(), vec.end());

        raze_assert(!raze_res.has_value());
        raze_assert(std_res_iter == vec.end());
    }

    {
        std::vector<T> vec = { T(42) };
        auto raze_res = raze::algorithm::max(vec.begin(), vec.end());
        auto std_res = std::ranges::max(vec);

        raze_assert(raze_res.has_value());
        raze_assert(raze_res.value() == std_res);
        raze_assert(raze_res.value() == T(42));
    }

    {
        std::vector<T> vec = { T(100), T(10), T(20) };
        auto raze_res = raze::algorithm::max(vec.begin(), vec.end());
        auto std_res = std::ranges::max(vec);

        raze_assert(raze_res.has_value());
        raze_assert(raze_res.value() == std_res);
        raze_assert(raze_res.value() == T(100));
    }

    {
        std::vector<T> vec = { T(10), T(20), T(100) };
        auto raze_res = raze::algorithm::max(vec.begin(), vec.end());
        auto std_res = std::ranges::max(vec);

        raze_assert(raze_res.has_value());
        raze_assert(raze_res.value() == std_res);
        raze_assert(raze_res.value() == T(100));
    }
}

template <typename T>
void test_minmax_edge_cases() {
    {
        std::vector<T> vec;
        auto raze_res = raze::algorithm::minmax(vec);
        raze_assert(!raze_res.has_value());
    }

    {
        std::vector<T> vec = { T(42) };
        auto raze_res = raze::algorithm::minmax(vec.begin(), vec.end());
        auto std_res = std::ranges::minmax(vec);

        raze_assert(raze_res.has_value());
        raze_assert(raze_res->first == std_res.min);
        raze_assert(raze_res->second == std_res.max);
        raze_assert(raze_res->first == raze_res->second);
    }

    {
        std::vector<T> vec = { T(10), T(50), T(5), T(100), T(20) };
        auto raze_res = raze::algorithm::minmax(vec.begin(), vec.end());
        auto std_res = std::ranges::minmax(vec);

        raze_assert(raze_res.has_value());
        raze_assert(raze_res->first == std_res.min);
        raze_assert(raze_res->second == std_res.max);
        raze_assert(raze_res->first == T(5));
        raze_assert(raze_res->second == T(100));
    }
}

template <typename T>
void test_min_random(unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(1, 1000);

    for (int i = 0; i < 5000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i);

        auto raze_res = raze::algorithm::min(vec.begin(), vec.end());
        auto std_res = std::ranges::min(vec);

        raze_assert(raze_res.has_value());
        raze_assert(raze_res.value() == std_res);
    }
}

template <typename T>
void test_max_random(unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(1, 1000);

    for (int i = 0; i < 5000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i);

        auto raze_res = raze::algorithm::max(vec.begin(), vec.end());
        auto std_res = std::ranges::max(vec);

        raze_assert(raze_res.has_value());
        raze_assert(raze_res.value() == std_res);
    }
}

template <typename T>
void test_minmax_random(unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(1, 1000);

    for (int i = 0; i < 5000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i);

        auto raze_res = raze::algorithm::minmax(vec.begin(), vec.end());
        auto std_res = std::ranges::minmax(vec);

        raze_assert(raze_res.has_value());
        raze_assert(raze_res->first == std_res.min);
        raze_assert(raze_res->second == std_res.max);
    }
}

template <typename T>
void test_min_ranges(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 30000);

        auto raze_res = raze::algorithm::min(vec);
        auto std_res = std::ranges::min(vec);

        raze_assert(raze_res.has_value());
        raze_assert(raze_res.value() == std_res);
    }
}

template <typename T>
void test_max_ranges(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 40000);

        auto raze_res = raze::algorithm::max(vec);
        auto std_res = std::ranges::max(vec);

        raze_assert(raze_res.has_value());
        raze_assert(raze_res.value() == std_res);
    }
}

template <typename T>
void test_minmax_ranges(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 50000);

        auto raze_res = raze::algorithm::minmax(vec);
        auto std_res = std::ranges::minmax(vec);

        raze_assert(raze_res.has_value());
        raze_assert(raze_res->first == std_res.min);
        raze_assert(raze_res->second == std_res.max);
    }
}

struct Person {
    std::string name;
    int age;
};

template <typename T>
void test_min_with_projection(unsigned seed = 42) {
    std::vector<Person> people = {
        {"Alice", 30},
        {"Bob", 25},
        {"Charlie", 35},
        {"David", 20}
    };

    auto proj = [](const Person& p) { return p.age; };

    auto raze_res = raze::algorithm::min(people.begin(), people.end(), proj);
    auto std_res_iter = std::ranges::min_element(people.begin(), people.end(), {}, proj);

    raze_assert(raze_res.has_value());
    raze_assert(raze_res->age == std_res_iter->age);
    raze_assert(raze_res->age == 20);
}

template <typename T>
void test_max_with_projection(unsigned seed = 42) {
    std::vector<Person> people = {
        {"Alice", 30},
        {"Bob", 25},
        {"Charlie", 35},
        {"David", 20}
    };

    auto proj = [](const Person& p) { return p.age; };

    auto raze_res = raze::algorithm::max(people.begin(), people.end(), proj);
    auto std_res_iter = std::ranges::max_element(people.begin(), people.end(), {}, proj);

    raze_assert(raze_res.has_value());
    raze_assert(raze_res->age == std_res_iter->age);
    raze_assert(raze_res->age == 35);
}

template <typename T>
void run_all_minmax_tests_for_type() {
    test_min_edge_cases<T>();
    test_max_edge_cases<T>();
    test_minmax_edge_cases<T>();

    test_min_random<T>();
    test_max_random<T>();
    test_minmax_random<T>();

    test_min_ranges<T>();
    test_max_ranges<T>();
    test_minmax_ranges<T>();
}

int main() {
    run_all_minmax_tests_for_type<int>();
    run_all_minmax_tests_for_type<short>();
    run_all_minmax_tests_for_type<long long>();
    run_all_minmax_tests_for_type<char>();

    run_all_minmax_tests_for_type<unsigned int>();
    run_all_minmax_tests_for_type<unsigned short>();
    run_all_minmax_tests_for_type<unsigned long long>();
    run_all_minmax_tests_for_type<unsigned char>();

    run_all_minmax_tests_for_type<float>();
    run_all_minmax_tests_for_type<double>();

    test_min_with_projection<int>();
    test_max_with_projection<int>();

    return 0;
}