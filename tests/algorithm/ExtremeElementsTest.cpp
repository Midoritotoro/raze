#include <raze/algorithm/extreme/ExtremeElement.h>
#include <raze/algorithm/extreme/ExtremeElements.h>

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
void test_extreme_element_empty() {
    std::vector<T> vec;

    auto res_min = raze::algorithm::extreme_element(vec.begin(), vec.end(), std::less<>{});
    auto res_max = raze::algorithm::extreme_element(vec.begin(), vec.end(), std::greater<>{});

    raze_assert(res_min == vec.end());
    raze_assert(res_max == vec.end());
}

template <typename T>
void test_extreme_element_single() {
    std::vector<T> vec = { T(42) };

    auto res_min = raze::algorithm::extreme_element(vec.begin(), vec.end(), std::less<>{});
    auto res_max = raze::algorithm::extreme_element(vec.begin(), vec.end(), std::greater<>{});
    raze_assert(*res_min == T(42));
    raze_assert(*res_max == T(42));
}

template <typename T>
void test_extreme_element_min_basic() {
    std::vector<T> vec = { T(10), T(5), T(20), T(1) };

    auto res = raze::algorithm::extreme_element(vec.begin(), vec.end(), std::less<>{});
    raze_assert(*res == T(1));
}

template <typename T>
void test_extreme_element_max_basic() {
    std::vector<T> vec = { T(10), T(50), T(5), T(100) };

    auto res = raze::algorithm::extreme_element(vec.begin(), vec.end(), std::greater<>{});
    raze_assert(*res == T(100));
}

template <typename T>
void test_extreme_element_random_min(unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(1, 1000);

    for (int i = 0; i < 5000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i);

        auto raze_res = raze::algorithm::extreme_element(vec.begin(), vec.end(), std::less<>{});
        auto std_res = std::ranges::min_element(vec);
        raze_assert(raze_res == std_res);
    }
}

template <typename T>
void test_extreme_element_random_max(unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(1, 1000);

    for (int i = 0; i < 5000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i);

        auto raze_res = raze::algorithm::extreme_element(vec.begin(), vec.end(), std::greater<>{});
        auto std_res = std::ranges::max_element(vec);
        raze_assert(raze_res == std_res);
    }
}

template <typename T>
void test_extreme_element_ranges(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 30000);

        {
            auto raze_res_min = raze::algorithm::extreme_element(vec, std::less<>{});
            auto std_res_min = std::ranges::min_element(vec);
            raze_assert(raze_res_min == std_res_min);
        }

        {
            auto raze_res_max = raze::algorithm::extreme_element(vec, std::greater<>{});
            auto std_res_max = std::ranges::max_element(vec);
            raze_assert(raze_res_max == std_res_max);
        }
    }
}

struct Person {
    std::string name;
    int age;
};

template <typename T>
void test_extreme_element_with_projection() {
    std::vector<Person> people = {
        {"Alice", 30},
        {"Bob", 25},
        {"Charlie", 35},
        {"David", 20}
    };

    auto proj = [](const Person& p) { return p.age; };

    {
        auto res_min = raze::algorithm::extreme_element(people.begin(), people.end(), std::less<>{}, proj);
        auto std_res_min = std::ranges::min_element(people.begin(), people.end(), {}, proj);
        raze_assert(res_min == std_res_min);
    }

    {
        auto res_max = raze::algorithm::extreme_element(people.begin(), people.end(), std::greater<>{}, proj);
        auto std_res_max = std::ranges::max_element(people.begin(), people.end(), {}, proj);
        raze_assert(res_max == std_res_max);
    }
}

template <typename T>
void test_extreme_elements_empty() {
    std::vector<T> vec;

    //auto res = raze::algorithm::extreme_elements(vec.begin(), vec.end(), std::less<>{});

    //raze_assert(!res.first.has_value());
    //raze_assert(!res.second.has_value());
}

template <typename T>
void test_extreme_elements_single() {
    std::vector<T> vec = { T(42) };

    //auto res = raze::algorithm::extreme_elements(vec.begin(), vec.end(), std::less<>{});

    //raze_assert(res.first.has_value());
    //raze_assert(res.second.has_value());
    //raze_assert(*res.first == T(42));
    //raze_assert(*res.second == T(42));
}

template <typename T>
void test_extreme_elements_basic() {
    std::vector<T> vec = { T(10), T(50), T(5), T(100), T(20) };

    //auto res = raze::algorithm::extreme_elements(vec.begin(), vec.end(), std::less<>{});

    //raze_assert(res.first.has_value());
    //raze_assert(res.second.has_value());
    //raze_assert(*res.first == T(5));
    //raze_assert(*res.second == T(100));
}

template <typename T>
void test_extreme_elements_reverse_comp() {
    std::vector<T> vec = { T(10), T(50), T(5), T(100), T(20) };

    //auto res = raze::algorithm::extreme_elements(vec.begin(), vec.end(), std::greater<>{});

    //raze_assert(res.first.has_value());
    //raze_assert(res.second.has_value());
    //raze_assert(*res.first == T(100));
    //raze_assert(*res.second == T(5));
}

template <typename T>
void test_extreme_elements_random(unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(1, 1000);

    for (int i = 0; i < 5000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i);

        //auto raze_res = raze::algorithm::extreme_elements(vec.begin(), vec.end(), std::less<>{});

        //if (vec.empty()) {
        //    raze_assert(!raze_res.first.has_value());
        //    raze_assert(!raze_res.second.has_value());
        //}
        //else {
        //    auto std_min = *std::ranges::min_element(vec);
        //    auto std_max = *std::ranges::max_element(vec);

        //    raze_assert(raze_res.first.has_value());
        //    raze_assert(raze_res.second.has_value());
        //    raze_assert(*raze_res.first == std_min);
        //    raze_assert(*raze_res.second == std_max);
        //}
    }
}

template <typename T>
void test_extreme_elements_ranges(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 50000);

        auto raze_res = raze::algorithm::extreme_elements(vec, std::less<>{});
        auto std_minmax = std::ranges::minmax_element(vec);

        raze_assert(raze_res.first == std_minmax.min);
        raze_assert(raze_res.second == std_minmax.max);
    }
}

template <typename T>
void test_extreme_elements_with_projection() {
    std::vector<Person> people = {
        {"Alice", 30},
        {"Bob", 25},
        {"Charlie", 35},
        {"David", 20},
        {"Eve", 40}
    };

    auto proj = [](const Person& p) { return p.age; };

    auto res = raze::algorithm::extreme_elements(people.begin(), people.end(), std::less<>{}, proj);

    raze_assert((*res.first).age == 20);
    raze_assert((*res.second).age == 40);
}

template <typename T>
void run_all_extreme_element_tests_for_type() {
    test_extreme_element_empty<T>();
    test_extreme_element_single<T>();
    test_extreme_element_min_basic<T>();
    test_extreme_element_max_basic<T>();
    test_extreme_element_random_min<T>();
    test_extreme_element_random_max<T>();
    test_extreme_element_ranges<T>();
}

template <typename T>
void run_all_extreme_elements_tests_for_type() {
    test_extreme_elements_empty<T>();
    test_extreme_elements_single<T>();
    test_extreme_elements_basic<T>();
    test_extreme_elements_reverse_comp<T>();
    test_extreme_elements_random<T>();
    test_extreme_elements_ranges<T>();
}

int main() {
    //run_all_extreme_element_tests_for_type<int>();
    //run_all_extreme_element_tests_for_type<short>();
    //run_all_extreme_element_tests_for_type<long long>();
    //run_all_extreme_element_tests_for_type<float>();
    //run_all_extreme_element_tests_for_type<double>();

    run_all_extreme_elements_tests_for_type<int>();
    run_all_extreme_elements_tests_for_type<short>();
    run_all_extreme_elements_tests_for_type<long long>();
    run_all_extreme_elements_tests_for_type<float>();
    run_all_extreme_elements_tests_for_type<double>();

    test_extreme_element_with_projection<int>();
    test_extreme_elements_with_projection<int>();

    return 0;
}