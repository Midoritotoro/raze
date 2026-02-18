#include <cassert>
#include <cstdint>
#include <vector>
#include <list>
#include <deque>
#include <limits>
#include <type_traits>
#include <utility>
#include <raze/algorithm/minmax/MinMax.h>

template <class T>
std::pair<T, T> ref_minmax_scalar(T a, T b) {
    return (a < b) ? std::make_pair(a, b) : std::make_pair(b, a);
}

template <class It>
std::pair<typename std::iterator_traits<It>::value_type,
    typename std::iterator_traits<It>::value_type>
    ref_minmax_range(It first, It last) {
    using V = typename std::iterator_traits<It>::value_type;
    raze_assert(first != last && "ref_minmax_range requires non-empty range");
    V minv = *first, maxv = *first;
    for (++first; first != last; ++first) {
        if (*first < minv) minv = *first;
        if (maxv < *first) maxv = *first;
    }
    return std::make_pair(minv, maxv);
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
    using raze::algorithm::minmax_range;
    Cont c;
    auto v = make_data<T>(bytes);
    c.insert(c.end(), v.begin(), v.end());
    if (c.empty()) return;
    auto got = minmax_range<typename Cont::iterator>(c.begin(), c.end());
    auto ref = ref_minmax_range(c.begin(), c.end());
    raze_assert(got.first == ref.first);
    raze_assert(got.second == ref.second);
}

template <class T>
void run_tests_for_type() {
    using raze::algorithm::minmax;
    using raze::algorithm::minmax_range;

    auto p1 = minmax<T>(1, 2);
    auto r1 = ref_minmax_scalar<T>(1, 2);
    raze_assert(p1.first == r1.first && p1.second == r1.second);

    auto p2 = minmax<T>(2, 1);
    auto r2 = ref_minmax_scalar<T>(2, 1);
    raze_assert(p2.first == r2.first && p2.second == r2.second);

    auto p3 = minmax<T>(0, 0);
    auto r3 = ref_minmax_scalar<T>(0, 0);
    raze_assert(p3.first == r3.first && p3.second == r3.second);

    auto p4 = minmax<T>(std::numeric_limits<T>::min(),
        std::numeric_limits<T>::max());
    auto r4 = ref_minmax_scalar<T>(std::numeric_limits<T>::min(),
        std::numeric_limits<T>::max());
    raze_assert(p4.first == r4.first && p4.second == r4.second);

    for (std::size_t sz : {16u, 64u, 512u, 4000u}) {
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
