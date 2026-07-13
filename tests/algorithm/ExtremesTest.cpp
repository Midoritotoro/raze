#include <raze/algorithm/extreme/Extremes.h>

#include <vector>
#include <random>
#include <type_traits>
#include <algorithm>
#include <cassert>
#include <optional>
#include <functional>

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
void test_extremes_empty() {
    std::vector<T> vec;

    auto res = raze::algorithm::extremes(vec.begin(), vec.end(), std::less<>{});

    raze_assert(!res.has_value());
}

template <typename T>
void test_extremes_single_element() {
    std::vector<T> vec = { T(42) };

    auto res = raze::algorithm::extremes(vec.begin(), vec.end(), std::less<>{});

    raze_assert(res.has_value());
    raze_assert(res.value().first == T(42));
    raze_assert(res.value().second == T(42));
}

template <typename T>
void test_extremes_custom_comparator_minmax() {
    std::vector<T> vec = { T(10), T(5), T(20), T(1), T(15) };

    auto res = raze::algorithm::extremes(vec.begin(), vec.end(), std::less<>{});

    raze_assert(res.has_value());
    raze_assert(res.value().first == T(1));
    raze_assert(res.value().second == T(20));
}

template <typename T>
void test_extremes_reverse_comparator() {
    std::vector<T> vec = { T(10), T(50), T(5), T(100), T(25) };

    auto res = raze::algorithm::extremes(vec.begin(), vec.end(), std::greater<>{});

    raze_assert(res.has_value());
    raze_assert(res.value().first == T(100));
    raze_assert(res.value().second == T(5));
}

template <typename T>
void test_extremes_random(unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(2, 1000);

    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i);

        auto raze_res = raze::algorithm::extremes(vec.begin(), vec.end(), std::less<>{});

        auto std_min = *std::ranges::min_element(vec.begin(), vec.end());
        auto std_max = *std::ranges::max_element(vec.begin(), vec.end());

        raze_assert(raze_res.has_value());
        raze_assert(raze_res.value().first == std_min);
        raze_assert(raze_res.value().second == std_max);
    }
}

template <typename T>
void test_extremes_ranges(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 30000);

        auto raze_res = raze::algorithm::extremes(vec, std::less<>{});

        auto std_min = *std::ranges::min_element(vec);
        auto std_max = *std::ranges::max_element(vec);

        raze_assert(raze_res.has_value());
        raze_assert(raze_res.value().first == std_min);
        raze_assert(raze_res.value().second == std_max);
    }
}

struct Person {
    std::string name;
    int age;
};

template <typename T>
void test_extremes_with_projection() {
    std::vector<Person> people = {
        {"Alice", 30},
        {"Bob", 25},
        {"Charlie", 35},
        {"David", 20},
        {"Eve", 40}
    };

    auto proj = [](const Person& p) { return p.age; };
    auto comp = std::less<>{};

    auto raze_res = raze::algorithm::extremes(people.begin(), people.end(), comp, proj);

    raze_assert(raze_res.has_value());
    raze_assert(raze_res->first.age == 20);
    raze_assert(raze_res->second.age == 40);
}

template <typename T>
void test_extremes_custom_object() {
    std::vector<int> vec = { 5, 2, 8, 1, 9, 3 };

    struct CustomLess {
        bool operator()(int a, int b) const { return a < b; }
    };

    auto res = raze::algorithm::extremes(vec.begin(), vec.end(), CustomLess{});

    raze_assert(res.has_value());
    raze_assert(res.value().first == 1);
    raze_assert(res.value().second == 9);
}

template <typename T>
void test_extremes_all_same_elements() {
    std::vector<T> vec(100, T(42));

    auto res = raze::algorithm::extremes(vec.begin(), vec.end(), std::less<>{});

    raze_assert(res.has_value());
    raze_assert(res.value().first == T(42));
    raze_assert(res.value().second == T(42));
}

template <typename T>
void test_extremes_two_elements() {
    std::vector<T> vec = { T(10), T(20) };

    auto res = raze::algorithm::extremes(vec.begin(), vec.end(), std::less<>{});

    raze_assert(res.has_value());
    raze_assert(res.value().first == T(10));
    raze_assert(res.value().second == T(20));
}

template <typename T>
void test_extremes_negative_values() {
    std::vector<T> vec = { T(-10), T(-50), T(-5), T(-100), T(-25) };

    auto res = raze::algorithm::extremes(vec.begin(), vec.end(), std::less<>{});

    raze_assert(res.has_value());
    raze_assert(res.value().first == T(-100));
    raze_assert(res.value().second == T(-5));
}

template <typename T>
void run_all_extremes_tests_for_type() {
    test_extremes_empty<T>();
    test_extremes_single_element<T>();
    test_extremes_custom_comparator_minmax<T>();
    test_extremes_reverse_comparator<T>();
    test_extremes_all_same_elements<T>();
    test_extremes_two_elements<T>();
    test_extremes_negative_values<T>();
    test_extremes_random<T>();
    test_extremes_ranges<T>();
}

int main() {
    run_all_extremes_tests_for_type<int>();
    run_all_extremes_tests_for_type<short>();
    run_all_extremes_tests_for_type<long long>();
    run_all_extremes_tests_for_type<float>();
    run_all_extremes_tests_for_type<double>();

    test_extremes_with_projection<int>();
    test_extremes_custom_object<int>();

    return 0;
}