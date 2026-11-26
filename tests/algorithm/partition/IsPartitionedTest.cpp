#include <tests/rts/rts.h>
#include <raze/algorithm/partition/IsPartitioned.h>

#include <algorithm>
#include <array>
#include <forward_list>
#include <list>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::is_partitioned", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::compact();
    auto pred = [](T x) { return x < T(50); };

    run<range, T>(cfg,
        [pred](auto c) { return raze::algorithm::is_partitioned(c.first(), c.last(), pred); },
        [pred](auto c) { return std::ranges::is_partitioned(c.first(), c.last(), pred); });

    run<range, T>(cfg,
        [pred](auto c) { return raze::algorithm::is_partitioned[opt::fscalar](c.range(), pred); },
        [pred](auto c) { return std::ranges::is_partitioned(c.range(), pred); });

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(15), size_t(64) }) {
        for (size_t cut : { size_t(0), sz / 2, sz }) {
            std::vector<T> v(sz);
            for (size_t i = 0; i < cut; ++i) v[i] = T(10);
            for (size_t i = cut; i < sz; ++i) v[i] = T(80);
            RTTS_EXPECT(raze::algorithm::is_partitioned(v, pred));
            if (cut > 0 && cut < sz) {
                std::swap(v[cut - 1], v[cut]);
                RTTS_EXPECT(!raze::algorithm::is_partitioned(v, pred));
            }
        }
    }

    static_assert([]() constexpr {
        std::array<T, 6> a{ T(2), T(4), T(6), T(1), T(3), T(5) };
        std::array<T, 6> b{ T(2), T(1), T(4), T(3), T(6), T(5) };
        std::array<T, 0> e{};
        auto is_even = [](T x) constexpr { return (static_cast<int>(x) % 2) == 0; };
        return raze::algorithm::is_partitioned(a, is_even)
            && !raze::algorithm::is_partitioned(b, is_even)
            && raze::algorithm::is_partitioned(e, is_even);
    }());
};

RTTS_CASE("raze::algorithm::is_partitioned projections, single-pass input, and non-common") {
    const std::vector<rtts::test_item> items{ { 1, 99 }, { 2, 88 }, { 10, 1 }, { 20, 2 } };
    RTTS_EXPECT(raze::algorithm::is_partitioned(items, [](int k) { return k < 5; }, &rtts::test_item::key));

    const std::vector<int> src{ 2, 4, 6, 7, 9 };
    rtts::single_pass_input_range<int> sp(src);
    RTTS_EXPECT(raze::algorithm::is_partitioned(sp, [](int x) { return x % 2 == 0; }));
    RTTS_EXPECT(raze::algorithm::is_partitioned(rtts::make_non_common(src), [](int x) { return x % 2 == 0; }));
};

