#include <cassert>
#include <cstdint>
#include <vector>
#include <list>
#include <deque>
#include <limits>
#include <type_traits>
#include <raze/algorithm/minmax/Min.h>

template <class T>
T ref_min_scalar(T a, T b) { return (a < b) ? a : b; }

template <class It>
typename std::iterator_traits<It>::value_type
ref_min_range(It first, It last) {
    using V = typename std::iterator_traits<It>::value_type;
    raze_assert(first != last && "ref_min_range requires non-empty range");
    V best = *first;
    for (++first; first != last; ++first)
        if (*first < best) best = *first;
    return best;
}

template <class T>
std::vector<T> make_data(std::size_t bytes) {
    std::size_t n = std::max<std::size_t>(1, bytes / sizeof(T));
    std::vector<T> v(n);
    for (size_t i = 0; i < n; ++i) v[i] = T(i % 97);
    return v;
}

template <class Cont, class T>
void test_container(std::size_t bytes) {
    using raze::algorithm::min_range;
    Cont c;
    auto v = make_data<T>(bytes);
    c.insert(c.end(), v.begin(), v.end());
    if (c.empty()) return;
    auto got = min_range(c.begin(), c.end());
    auto ref = ref_min_range(c.begin(), c.end());
    raze_assert(got == ref);
}

template <class T>
void run_tests_for_type() {
    using raze::algorithm::min;
    using raze::algorithm::min_range;

    raze_assert(min<T>(1, 2) == ref_min_scalar<T>(1, 2));
    raze_assert(min<T>(2, 1) == ref_min_scalar<T>(2, 1));
    raze_assert(min<T>(0, 0) == ref_min_scalar<T>(0, 0));
    raze_assert(min<T>(std::numeric_limits<T>::min(),
        std::numeric_limits<T>::max())
        == ref_min_scalar<T>(std::numeric_limits<T>::min(),
            std::numeric_limits<T>::max()));

    for (std::size_t sz : { 16u, 64u, 512u, 4000u}) {
        test_container<std::vector<T>, T>(sz);
        test_container<std::list<T>, T>(sz);
        test_container<std::deque<T>, T>(sz);
    }
}

int main() {
    run_tests_for_type<int8_t>();
    run_tests_for_type<uint8_t>();
    run_tests_for_type<int16_t>();
    run_tests_for_type<uint16_t>();
    run_tests_for_type<int32_t>();
    run_tests_for_type<uint32_t>();
    run_tests_for_type<int64_t>();
    run_tests_for_type<uint64_t>();
    return 0;
}
