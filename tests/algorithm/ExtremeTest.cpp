#include <raze/algorithm/extreme/Extreme.h>

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
void test_extreme_empty() {
    std::vector<T> vec;

    auto res_min = raze::algorithm::extreme(vec.begin(), vec.end(), std::less<>{});
    auto res_max = raze::algorithm::extreme(vec.begin(), vec.end(), std::greater<>{});

    raze_assert(!res_min.has_value());
    raze_assert(!res_max.has_value());
}

template <typename T>
void test_extreme_single_element() {
    std::vector<T> vec = { T(42) };

    auto res_min = raze::algorithm::extreme(vec.begin(), vec.end(), std::less<>{});
    auto res_max = raze::algorithm::extreme(vec.begin(), vec.end(), std::greater<>{});

    raze_assert(res_min.has_value());
    raze_assert(res_max.has_value());
    raze_assert(res_min.value() == T(42));
    raze_assert(res_max.value() == T(42));
}

template <typename T>
void test_extreme_custom_comparator_min() {
    std::vector<T> vec = { T(10), T(5), T(20), T(1) };

    auto res = raze::algorithm::extreme(vec.begin(), vec.end(), std::less<>{});

    raze_assert(res.has_value());
    raze_assert(res.value() == T(1));
}

template <typename T>
void test_extreme_custom_comparator_max() {
    std::vector<T> vec = { T(10), T(50), T(5), T(100) };

    auto res = raze::algorithm::extreme(vec.begin(), vec.end(), std::greater<>{});

    raze_assert(res.has_value());
    raze_assert(res.value() == T(100));
}

template <typename T>
void test_extreme_random_min(unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(1, 1000);

    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i);

        auto raze_res = raze::algorithm::extreme(vec.begin(), vec.end(), std::less<>{});
        auto std_res = *std::ranges::min_element(vec.begin(), vec.end());

        raze_assert(raze_res.has_value());
        raze_assert(raze_res.value() == std_res);
    }
}

template <typename T>
void test_extreme_random_max(unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(1, 1000);

    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i);

        auto raze_res = raze::algorithm::extreme(vec.begin(), vec.end(), std::greater<>{});
        auto std_res = *std::ranges::max_element(vec.begin(), vec.end());

        raze_assert(raze_res.has_value());
        raze_assert(raze_res.value() == std_res);
    }
}

template <typename T>
void test_extreme_ranges_min(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 30000);

        auto raze_res = raze::algorithm::extreme(vec, std::less<>{});
        auto std_res = *std::ranges::min_element(vec);

        raze_assert(raze_res.has_value());
        raze_assert(raze_res.value() == std_res);
    }
}

template <typename T>
void test_extreme_ranges_max(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 40000);

        auto raze_res = raze::algorithm::extreme(vec, std::greater<>{});
        auto std_res = *std::ranges::max_element(vec);

        raze_assert(raze_res.has_value());
        raze_assert(raze_res.value() == std_res);
    }
}

struct Person {
    std::string name;
    int age;
};

template <typename T>
void test_extreme_with_projection_min() {
    std::vector<Person> people = {
        {"Alice", 30},
        {"Bob", 25},
        {"Charlie", 35},
        {"David", 20}
    };

    auto proj = [](const Person& p) { return p.age; };
    auto comp = std::less<>{};

    auto raze_res = raze::algorithm::extreme(people.begin(), people.end(), comp, proj);
    auto std_res_iter = std::ranges::min_element(people.begin(), people.end(), {}, proj);

    raze_assert(raze_res.has_value());
    raze_assert(raze_res->age == std_res_iter->age);
    raze_assert(raze_res->age == 20);
}

template <typename T>
void test_extreme_with_projection_max() {
    std::vector<Person> people = {
        {"Alice", 30},
        {"Bob", 25},
        {"Charlie", 35},
        {"David", 20}
    };

    auto proj = [](const Person& p) { return p.age; };
    auto comp = std::greater<>{};

    auto raze_res = raze::algorithm::extreme(people.begin(), people.end(), comp, proj);
    auto std_res_iter = std::ranges::max_element(people.begin(), people.end(), {}, proj);

    raze_assert(raze_res.has_value());
    raze_assert(raze_res->age == std_res_iter->age);
    raze_assert(raze_res->age == 35);
}

template <typename T>
void test_extreme_custom_object() {
    std::vector<int> vec = { 1, 2, 3, 4, 5 };

    struct CustomLess {
        bool operator()(int a, int b) const { return a < b; }
    };

    auto res = raze::algorithm::extreme(vec.begin(), vec.end(), CustomLess{});

    raze_assert(res.has_value());
    raze_assert(res.value() == 1);
}

template <typename T>
void run_all_extreme_tests_for_type() {
    test_extreme_empty<T>();
    test_extreme_single_element<T>();
    test_extreme_custom_comparator_min<T>();
    test_extreme_custom_comparator_max<T>();

    test_extreme_random_min<T>();
    test_extreme_random_max<T>();

    test_extreme_ranges_min<T>();
    test_extreme_ranges_max<T>();
}

int main() {
    run_all_extreme_tests_for_type<int>();
    run_all_extreme_tests_for_type<short>();
    run_all_extreme_tests_for_type<long long>();
    run_all_extreme_tests_for_type<float>();
    run_all_extreme_tests_for_type<double>();

    test_extreme_with_projection_min<int>();
    test_extreme_with_projection_max<int>();
    test_extreme_custom_object<int>();

    return 0;
}