#include <tests/rts/rts.h>
#include <raze/algorithm/find/LexicographicalCompareThreeWay.h>

#include <algorithm>
#include <array>
#include <compare>
#include <limits>
#include <list>
#include <ranges>
#include <string>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::lexicographical_compare_three_way", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::compact();

    run<range, T>(cfg,
        [](auto c) {
            return raze::algorithm::lexicographical_compare_three_way(c.first(), c.last(), c.first2(), c.last2());
        },
        [](auto c) {
            return std::lexicographical_compare_three_way(
                c.data.value.begin(), c.data.value.end(),
                c.data.second.begin(), c.data.second.end());
        });

    run<range, T>(cfg,
        [](auto c) {
            return raze::algorithm::lexicographical_compare_three_way[opt::fscalar](c.range(), c.range2());
        },
        [](auto c) {
            return std::lexicographical_compare_three_way(
                c.data.value.begin(), c.data.value.end(),
                c.data.second.begin(), c.data.second.end());
        });

    static_assert([]() constexpr {
        std::array<T, 3> a{ T(1), T(2), T(3) };
        std::array<T, 3> b{ T(1), T(2), T(4) };
        std::array<T, 2> p{ T(1), T(2) };
        std::array<T, 0> e{};
        return raze::algorithm::lexicographical_compare_three_way(a, b) < 0
            && raze::algorithm::lexicographical_compare_three_way(b, a) > 0
            && raze::algorithm::lexicographical_compare_three_way(a, a) == 0
            && raze::algorithm::lexicographical_compare_three_way(p, a) < 0
            && raze::algorithm::lexicographical_compare_three_way(a, p) > 0
            && raze::algorithm::lexicographical_compare_three_way(e, e) == 0;
    }());
};

RTTS_CASE("raze::algorithm::lexicographical_compare_three_way categories, NaN, and projections") {
    // Floating-point partial_ordering with NaN
    {
        const std::vector<double> a{ 1.0, std::numeric_limits<double>::quiet_NaN() };
        const std::vector<double> b{ 1.0, 2.0 };
        auto cmp = raze::algorithm::lexicographical_compare_three_way(a, b);
        static_assert(std::same_as<decltype(cmp), std::partial_ordering>);
        RTTS_EXPECT(cmp == std::partial_ordering::unordered);
    }

    // Projections, custom comparator, single-pass input, non-common
    {
        const std::vector<rtts::test_item> a{ { 1, 100 }, { 2, 200 } };
        const std::vector<rtts::test_item> b{ { 1, 1 }, { 2, 2 } };
        auto cmp = raze::algorithm::lexicographical_compare_three_way(
            a, b, std::compare_three_way{}, &rtts::test_item::key, &rtts::test_item::key);
        RTTS_EXPECT(cmp == std::strong_ordering::equal);

        const std::vector<int> v1{ 1, 2, 3 };
        const std::vector<int> v2{ 1, 2, 4 };
        rtts::single_pass_input_range<int> sp1(v1);
        rtts::single_pass_input_range<int> sp2(v2);
        RTTS_EXPECT(raze::algorithm::lexicographical_compare_three_way(sp1, sp2) == std::strong_ordering::less);
        RTTS_EXPECT(raze::algorithm::lexicographical_compare_three_way(
            rtts::make_non_common(v2), rtts::make_non_common(v1)) == std::strong_ordering::greater);
    }
};

