#include <tests/rts/rts.h>
#include <raze/algorithm/find/LexicographicalCompare.h>

#include <algorithm>
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <ranges>
#include <string>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::lexicographical_compare", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::compact();

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::lexicographical_compare(c.first(), c.last(), c.first2(), c.last2()); },
        [](auto c) { return std::ranges::lexicographical_compare(c.first(), c.last(), c.first2(), c.last2()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::lexicographical_compare(c.range(), c.range2()); },
        [](auto c) { return std::ranges::lexicographical_compare(c.range(), c.range2()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::lexicographical_compare[opt::fscalar](c.range(), c.range2()); },
        [](auto c) { return std::ranges::lexicographical_compare(c.range(), c.range2()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::lexicographical_compare[opt::fstatic](c.range(), c.range2(), std::greater<>{}); },
        [](auto c) { return std::ranges::lexicographical_compare(c.range(), c.range2(), std::greater<>{}); });

    static_assert([]() constexpr {
        std::array<T, 3> a{ T(1), T(2), T(3) };
        std::array<T, 3> b{ T(1), T(2), T(4) };
        std::array<T, 2> p{ T(1), T(2) };
        std::array<T, 0> e{};
        return raze::algorithm::lexicographical_compare(a, b)
            && !raze::algorithm::lexicographical_compare(b, a)
            && !raze::algorithm::lexicographical_compare(a, a)
            && raze::algorithm::lexicographical_compare(p, a)
            && !raze::algorithm::lexicographical_compare(a, p)
            && raze::algorithm::lexicographical_compare(e, p)
            && !raze::algorithm::lexicographical_compare(e, e);
    }());
};

RTTS_CASE("raze::algorithm::lexicographical_compare projections, views, and edge cases") {
    namespace views = std::views;

    // Projections & custom comparator
    {
        const std::vector<rtts::test_item> a{ { 1, 99 }, { 2, 88 } };
        const std::vector<rtts::test_item> b{ { 1, 1 }, { 3, 2 } };
        RTTS_EXPECT(raze::algorithm::lexicographical_compare(
            a, b, std::less<>{}, &rtts::test_item::key, &rtts::test_item::key));
        RTTS_EXPECT(!raze::algorithm::lexicographical_compare(
            a, b, std::less<>{}, &rtts::test_item::payload, &rtts::test_item::payload));
    }

    // Single-pass input ranges, non-common sentinels, strings
    {
        const std::vector<int> v1{ 1, 2, 3 };
        const std::vector<int> v2{ 1, 2, 3, 4 };
        rtts::single_pass_input_range<int> sp1(v1);
        rtts::single_pass_input_range<int> sp2(v2);
        RTTS_EXPECT(raze::algorithm::lexicographical_compare(sp1, sp2));

        RTTS_EXPECT(raze::algorithm::lexicographical_compare(
            rtts::make_non_common(v1), rtts::make_non_common(v2)));
        RTTS_EXPECT(raze::algorithm::lexicographical_compare(
            views::iota(0, 5), views::iota(0, 6)));
        RTTS_EXPECT(raze::algorithm::lexicographical_compare(
            std::string("apple"), std::string("apricot")));
    }
};

