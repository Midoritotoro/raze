#pragma once 

#include <vector>
#include <random>
#include <type_traits>
#include <algorithm>
#include <cassert>
#include <list>
#include <forward_list>

namespace raze::tests::utility {

template <class T>
using int_distribution_type_t = std::conditional_t<std::is_integral_v<T> && sizeof(T) <= sizeof(int), int, T>;

template <class T>
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

template <class T>
std::vector<T> generate_random_vector(size_t size, unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::vector<T> vec(size);

    for (auto& elem : vec)
        elem = gen();
    
    return vec;
}

template <class T>
std::vector<T> generate_vector_with_target(size_t size, T target, size_t target_pos, unsigned seed = 42) {
    auto vec = generate_random_vector<T>(size, seed);

    if (target_pos < size) 
        vec[target_pos] = target;
    
    return vec;
}

template <class Range1, class Range2>
bool equal_ranges(const Range1& lhs, const Range2& rhs) {
    return std::ranges::equal(lhs, rhs, [](const auto& a, const auto& b) {
        if constexpr (std::is_floating_point_v<std::remove_cvref_t<decltype(a)>>) return (std::isnan(a) && std::isnan(b)) || (a == b);
        else return a == b;
    });
}

template <class T>
std::list<T> generate_random_list(size_t size, unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::list<T> lst;
    for (size_t i = 0; i < size; ++i) lst.push_back(gen());
    return lst;
}

template <class T>
std::forward_list<T> generate_random_forward_list(size_t size, unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::forward_list<T> flst;
    std::vector<T> temp;
    for (size_t i = 0; i < size; ++i) temp.push_back(gen());
    for (auto it = temp.rbegin(); it != temp.rend(); ++it) flst.push_front(*it);
    return flst;
}

} // namespace raze::tests::utility