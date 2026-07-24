#include <raze/algorithm/find/Find.h>

#include <tests/utility/Random.h>
#include <tests/utility/TestArithmeticTypes.h>

#include <vector>
#include <algorithm>

using raze::tests::utility::RandomGenerator;
using raze::tests::utility::generate_random_vector;
using raze::tests::utility::run_for_arithmetic_types;

template <class T, class... Args>
void check_find(const std::vector<T>& vec, const Args&... args) {
    auto simd_result = raze::algorithm::find(vec.begin(), vec.end(), args...);
    auto std_result = std::ranges::find(vec.begin(), vec.end(), args...);
    raze_assert(simd_result == std_result);
}

template <class T, class Pred>
void check_find_if(const std::vector<T>& vec, Pred pred) {
    auto simd_result = raze::algorithm::find_if(vec.begin(), vec.end(), pred);
    auto std_result = std::ranges::find_if(vec.begin(), vec.end(), pred);
    raze_assert(simd_result == std_result);
}

template <class T, std::size_t N>
constexpr bool check_find_constexpr(std::array<T, N> arr, T value, std::ptrdiff_t expected) {
    auto simd_result = raze::algorithm::find(arr.begin(), arr.end(), value);
    auto std_result = std::ranges::find(arr.begin(), arr.end(), value);

    return simd_result - arr.begin() == expected
        && std_result - arr.begin() == expected;
}

template <class T, std::size_t N>
constexpr bool check_find_range_constexpr(std::array<T, N> arr, T value, std::ptrdiff_t expected) {
    auto simd_result = raze::algorithm::find(arr, value);
    auto std_result = std::ranges::find(arr, value);

    return simd_result - arr.begin() == expected
        && std_result - arr.begin() == expected;
}

template <class T, std::size_t N, class Pred>
constexpr bool check_find_if_constexpr(std::array<T, N> arr, Pred pred, std::ptrdiff_t expected) {
    auto simd_result = raze::algorithm::find_if(arr.begin(), arr.end(), pred);
    auto std_result = std::ranges::find_if(arr.begin(), arr.end(), pred);

    return simd_result - arr.begin() == expected
        && std_result - arr.begin() == expected;
}

static_assert(check_find_constexpr(std::array<int, 0>{}, 42, 0));
static_assert(check_find_constexpr(std::array{ 42 }, 42, 0));
static_assert(check_find_constexpr(std::array{ 42 }, 99, 1));
static_assert(check_find_constexpr(std::array{ 1, 42, 3, 42 }, 42, 1));
static_assert(check_find_constexpr(std::array{ 1, 2, 3, 42 }, 42, 3));

static_assert(check_find_range_constexpr(std::array{ 1, 2, 3 }, 2, 1));
static_assert(check_find_range_constexpr(std::array{ 1, 2, 3 }, 9, 3));

static_assert(check_find_if_constexpr(std::array<int, 0>{}, [](int) { return true; }, 0));
static_assert(check_find_if_constexpr(std::array{ 1, 2, 3 }, [](int x) { return x > 1; }, 1));
static_assert(check_find_if_constexpr(std::array{ 1, 2, 3 }, [](int x) { return x > 9; }, 3));
static_assert(check_find_if_constexpr(std::array{ 1, 2, 3 }, [](int) { return true; }, 0));
static_assert(check_find_if_constexpr(std::array{ 1, 2, 3 }, [](int) { return false; }, 3));

template <class T>
void test_find_random(unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);

    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(size_dist(size_gen), seed + i);
        check_find(vec, gen());
    }

    for (size_t pos : {0, 1, 7, 15, 31, 63, 127, 255, 511, 999}) {
        auto vec = raze::tests::utility::generate_vector_with_target<T>(1000, T(42), pos, seed);
        check_find(vec, T(42));
    }
}

template <class T>
void test_find_ranges(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 30000);
        T target = RandomGenerator<T>(seed + i + 30000)();

        auto simd_result = raze::algorithm::find(vec, target);
        auto std_result = std::ranges::find(vec, target);
        raze_assert(simd_result == std_result);
    }
}

template <class T>
void test_find_if_random(unsigned seed = 42) {
    auto run = [&](auto make_pred) {
        for (int i = 0; i < 1000; ++i) {
            auto vec = generate_random_vector<T>(100, seed + i + 20000);
            check_find_if(vec, make_pred(i));
        }
        };

    run([&](int i) { return [v = RandomGenerator<T>(seed + i + 40000)()](T x) { return x > v; }; });
    run([&](int i) { return [v = RandomGenerator<T>(seed + i + 50000)()](T x) { return x == v; }; });
    run([](int) { return [](T) { return true;  }; });
    run([](int) { return [](T) { return false; }; });
}

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};

void test_find_with_projection(unsigned seed = 42) {
    for (int i = 0; i < 100; ++i) {
        std::vector<Point> vec(100);
        RandomGenerator<int> gen(seed + i + 120000);
        for (auto& p : vec) { p.x = gen(); p.y = gen(); }

        int target_x = gen();

        auto simd_result = raze::algorithm::find_if(vec.begin(), vec.end(),
            [target_x](int p) { return p == target_x; },
            [](const Point& p) { return p.x; });

        auto std_result = std::ranges::find_if(vec.begin(), vec.end(),
            [target_x](const Point& p) { return p.x == target_x; });

        raze_assert(simd_result == std_result);
    }
}

template <class T>
void test_edge_cases() {
    check_find<T>({}, T(42));
    check_find<T>({ T(42) }, T(42));
    check_find<T>({ T(42) }, T(99));

    std::vector<T> first(100, T(0)), last(100, T(0));
    first.front() = T(42);
    last.back() = T(42);
    check_find(first, T(42));
    check_find(last, T(42));
}

int main() {
    run_for_arithmetic_types([] <class _Type_> () {
        test_find_random<_Type_>();
        test_find_ranges<_Type_>();
        test_find_if_random<_Type_>();
        test_edge_cases<_Type_>();
	});

    test_find_with_projection();
    return 0;
}