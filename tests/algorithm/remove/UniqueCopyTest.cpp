#include <tests/rts/rts.h>
#include <raze/algorithm/remove/UniqueCopy.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <list>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::unique_copy", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::compact();

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::unique_copy(c.first(), c.last(), c.out()); },
        [](auto c) { return std::ranges::unique_copy(c.first(), c.last(), c.out()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::unique_copy(c.range(), c.out()); },
        [](auto c) { return std::ranges::unique_copy(c.range(), c.out()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::unique_copy[opt::fscalar](c.range(), c.out()); },
        [](auto c) { return std::ranges::unique_copy(c.range(), c.out()); });

    static_assert([]() constexpr {
        std::array<T, 7> src{ T(1), T(1), T(2), T(2), T(3), T(3), T(2) };
        std::array<T, 7> dst{};
        auto r = raze::algorithm::unique_copy(src, dst.begin());
        return r.in == src.end() && r.out == dst.begin() + 4
            && dst[0] == T(1) && dst[1] == T(2) && dst[2] == T(3) && dst[3] == T(2);
    }());
};

RTTS_CASE("raze::algorithm::unique_copy input-only, projections, back_inserter, and dangling") {
    // Single-pass input range -> back_inserter (exercises input_iterator path!)
    {
        const std::vector<int> src{ 1, 1, 2, 2, 2, 3, 4, 4, 1 };
        rtts::single_pass_input_range<int> sp(src);
        std::vector<int> out;
        auto r = raze::algorithm::unique_copy(sp, std::back_inserter(out));
        RTTS_EXPECT(r.in == sp.end());
        RTTS_EQUAL(sp.reads(), src.size());
        RTTS_EXPECT((out == std::vector<int>{ 1, 2, 3, 4, 1 }));
    }

    // Projections & custom predicate
    {
        const std::vector<rtts::test_item> src{ { 1, 10 }, { 1, 20 }, { 2, 30 }, { 3, 40 }, { 3, 50 } };
        std::vector<rtts::test_item> out;
        raze::algorithm::unique_copy(src, std::back_inserter(out), std::equal_to<>{}, &rtts::test_item::key);
        RTTS_EQUAL(out.size(), size_t(3));
        RTTS_EXPECT(out[0].payload == 10 && out[1].payload == 30 && out[2].payload == 40);
    }

    // Empty & rvalue dangling
    {
        std::vector<int> out;
        auto dr = raze::algorithm::unique_copy(std::vector<int>{ 1, 1, 2 }, std::back_inserter(out));
        static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
        RTTS_EXPECT((out == std::vector<int>{ 1, 2 }));
    }
};

