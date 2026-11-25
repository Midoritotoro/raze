#include <tests/rts/rts.h>
#include <raze/algorithm/extreme/Minmax.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <random>
#include <ranges>
#include <utility>
#include <vector>

namespace {
    struct non_default_mm_item {
        int key;
        int id;
        explicit constexpr non_default_mm_item(int k, int i = 0) noexcept : key(k), id(i) {}
        constexpr bool operator==(const non_default_mm_item&) const = default;
    };

    template <class T>
    constexpr std::optional<std::pair<T, T>> to_pair(std::optional<std::ranges::minmax_result<T>> r) {
        if (!r.has_value()) return std::nullopt;
        return std::pair<T, T>{ r->min, r->max };
    }

    template <std::input_iterator It, std::sentinel_for<It> Sent, class Comp = std::less<>, class Proj = std::identity>
    constexpr auto std_minmax_pair(It first, Sent last, Comp comp = {}, Proj proj = {})
        -> std::optional<std::pair<std::iter_value_t<It>, std::iter_value_t<It>>>
    {
        if (first == last) return std::nullopt;
        auto r = std::ranges::minmax(std::ranges::subrange(first, last), comp, proj);
        return std::pair{ r.min, r.max };
    }

    template <std::ranges::input_range R, class Comp = std::less<>, class Proj = std::identity>
    constexpr auto std_minmax_pair(R&& r, Comp comp = {}, Proj proj = {})
        -> std::optional<std::pair<std::ranges::range_value_t<R>, std::ranges::range_value_t<R>>>
    {
        if (std::ranges::empty(r)) return std::nullopt;
        auto res = std::ranges::minmax(std::forward<R>(r), comp, proj);
        return std::pair{ res.min, res.max };
    }
}

RTTS_CASE_TPL("raze::algorithm::minmax", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    run<range, T>(cfg,
        [](auto c) { return to_pair(raze::algorithm::minmax(c.first(), c.last())); },
        [](auto c) { return std_minmax_pair(c.first(), c.last()); });

    run<range, T>(cfg,
        [](auto c) { return to_pair(raze::algorithm::minmax(c.range())); },
        [](auto c) { return std_minmax_pair(c.range()); });

    run<range, T>(cfg,
        [](auto c) { return to_pair(raze::algorithm::minmax[opt::fscalar](c.first(), c.last())); },
        [](auto c) { return std_minmax_pair(c.first(), c.last()); });

    run<range, T>(cfg,
        [](auto c) { return to_pair(raze::algorithm::minmax[opt::unroll<1>](c.range())); },
        [](auto c) { return std_minmax_pair(c.range()); });

    run<range, T>(cfg,
        [](auto c) { return to_pair(raze::algorithm::minmax[opt::unroll<2>](c.range())); },
        [](auto c) { return std_minmax_pair(c.range()); });

    run<range, T>(cfg,
        [](auto c) { return to_pair(raze::algorithm::minmax[opt::fstatic](c.range())); },
        [](auto c) { return std_minmax_pair(c.range()); });

    run<range, T>(cfg,
        [](auto c) { return to_pair(raze::algorithm::minmax(c.range(), std::greater<>{})); },
        [](auto c) { return std_minmax_pair(c.range(), std::greater<>{}); });

    run<range, T>(cfg,
        [](auto c) { return to_pair(raze::algorithm::minmax(c.range(), std::ranges::less{})); },
        [](auto c) { return std_minmax_pair(c.range(), std::ranges::less{}); });

    auto cmp_branch = [](auto a, auto b) { if (a < b) return true; return false; };
    run<range, T>(cfg,
        [](auto c) { return to_pair(raze::algorithm::minmax[opt::fautovec](c.first(), c.last())); },
        [](auto c) { return std_minmax_pair(c.first(), c.last()); });

    run<range, T>(cfg,
        [cmp_branch](auto c) { return to_pair(raze::algorithm::minmax[opt::fautovec](c.range(), cmp_branch)); },
        [cmp_branch](auto c) { return std_minmax_pair(c.range(), cmp_branch); });

    run<range, T>(cfg,
        [cmp_branch](auto c) { return to_pair(raze::algorithm::minmax[opt::fautovec][opt::fstatic](c.range(), cmp_branch)); },
        [cmp_branch](auto c) { return std_minmax_pair(c.range(), cmp_branch); });

    {
        auto acfg = config::compact();
        auto check_aligned = [&]<size_t Align>(auto algo) {
            run_aligned<Align, range, T>(acfg,
                [algo](auto c) { return to_pair(algo(c.first(), c.last())); },
                [](auto c) { return std_minmax_pair(c.first(), c.last()); });
            run_aligned<Align, range, T>(acfg,
                [algo](auto c) { return to_pair(algo(c.range())); },
                [](auto c) { return std_minmax_pair(c.range()); });
        };

        check_aligned.template operator()<16>(raze::algorithm::minmax[opt::aligned<16>]);
        check_aligned.template operator()<32>(raze::algorithm::minmax[opt::aligned<32>]);
        check_aligned.template operator()<32>(raze::algorithm::minmax[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<32>(raze::algorithm::minmax[opt::aligned<32>][opt::fstatic]);
        check_aligned.template operator()<64>(raze::algorithm::minmax[opt::aligned<64>]);
        check_aligned.template operator()<64>(raze::algorithm::minmax[opt::aligned<64>][opt::fautovec]);
        check_aligned.template operator()<64>(raze::algorithm::minmax[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1>(raze::algorithm::minmax[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1>(raze::algorithm::minmax[opt::aligned<64>][opt::aligned<opt::unaligned>]);
    }

    {
        auto custom_vec_cmp = [](auto a, auto b) { return a > b; };
        auto v = rtts::random::vector<T>(300, 77777u);
        RTTS_EQUAL(to_pair(raze::algorithm::minmax(v, custom_vec_cmp)), std_minmax_pair(v, custom_vec_cmp));
    }

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return to_pair(raze::algorithm::minmax(c.first(), c.last())); },
        [](auto c) constexpr { return std_minmax_pair(c.first(), c.last()); }));

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return to_pair(raze::algorithm::minmax[opt::fscalar](c.range())); },
        [](auto c) constexpr { return std_minmax_pair(c.range()); }));

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return to_pair(raze::algorithm::minmax[opt::fautovec](c.range())); },
        [](auto c) constexpr { return std_minmax_pair(c.range()); }));

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return to_pair(raze::algorithm::minmax[opt::aligned<64>](c.range())); },
        [](auto c) constexpr { return std_minmax_pair(c.range()); }));

    static_assert(to_pair(raze::algorithm::minmax(std::initializer_list<T>{ T(5), T(2), T(8), T(1), T(4) }))
        == std::pair<T, T>{ T(1), T(8) });
    static_assert(!raze::algorithm::minmax(std::initializer_list<T>{}).has_value());
};

RTTS_CASE("raze::algorithm::minmax projections, ties, views, chunking, and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    {
        const std::vector<int> v = { 10, 30, 20, 50, 40 };
        auto neg_proj = [](int x) { return -x; };
        auto r = raze::algorithm::minmax(v, std::less<>{}, neg_proj);
        RTTS_EXPECT(r.has_value());
        RTTS_EQUAL(r->min, 50);
        RTTS_EQUAL(r->max, 10);

        const std::vector<rtts::test_item> items = {
            { 2, 1 }, { 9, 10 }, { 2, 2 }, { 9, 20 }, { 5, 3 }
        };
        auto ri = raze::algorithm::minmax(items, std::less<>{}, &rtts::test_item::key);
        RTTS_EXPECT(ri.has_value());
        RTTS_EQUAL(ri->min.key, 2);
        RTTS_EQUAL(ri->min.payload, 1); 
        RTTS_EQUAL(ri->max.key, 9);
        RTTS_EQUAL(ri->max.payload, 20);  
    }

    {
        auto mask_low = [](auto a, auto b) { return (a & 0xFF) < (b & 0xFF); };
        std::vector<int> v(130, 0x0050);
        v[0]   = 0x1105; 
        v[40]  = 0x2205;  
        v[1]   = 0x11F0; 
        v[95]  = 0x22F0;  
        v[129] = 0x33F0;  
        auto r = raze::algorithm::minmax(v, mask_low);
        RTTS_EXPECT(r.has_value());
        RTTS_EQUAL(r->min, 0x1105);
        RTTS_EQUAL(r->max, 0x33F0);

        v.assign(128, 0x0050);
        v[0] = 0xAA99;
        v[2] = 0xBB99;
        auto r2 = raze::algorithm::minmax(v, mask_low);
        RTTS_EXPECT(r2.has_value());
        RTTS_EQUAL(r2->min, 0x0050);
        RTTS_EQUAL(r2->max, 0xBB99);
    }

    {
        auto custom_cmp = [](auto a, auto b) { return a < b; };
        std::vector<uint8_t> big(20000, 100);
        big[16300] = 3;
        big[19900] = 251;
        auto r1 = raze::algorithm::minmax(big);
        auto r2 = raze::algorithm::minmax(big, custom_cmp);
        RTTS_EXPECT(r1.has_value() && r2.has_value());
        RTTS_EQUAL(r1->min, uint8_t(3));
        RTTS_EQUAL(r1->max, uint8_t(251));
        RTTS_EQUAL(r2->min, uint8_t(3));
        RTTS_EQUAL(r2->max, uint8_t(251));
    }

    {
        auto r1 = raze::algorithm::minmax(views::iota(5, 50));
        RTTS_EXPECT(r1.has_value());
        RTTS_EQUAL(r1->min, 5);
        RTTS_EQUAL(r1->max, 49);

        const std::vector<int> src = { 9, 2, 7, 1, 8, 3, 6, 4 };
        auto r2 = raze::algorithm::minmax(src | views::filter([](int x) { return x % 2 == 0; }));
        RTTS_EXPECT(r2.has_value());
        RTTS_EQUAL(r2->min, 2);
        RTTS_EQUAL(r2->max, 8);

        auto r3 = raze::algorithm::minmax[opt::fstatic](rtts::make_non_common(src));
        RTTS_EXPECT(r3.has_value());
        RTTS_EQUAL(r3->min, 1);
        RTTS_EQUAL(r3->max, 9);

        rtts::single_pass_input_range<int> sp(src);
        auto r4 = raze::algorithm::minmax(sp);
        RTTS_EXPECT(r4.has_value());
        RTTS_EQUAL(r4->min, 1);
        RTTS_EQUAL(r4->max, 9);
        RTTS_EQUAL(sp.reads(), src.size());
    }

    {
        std::vector<int> empty_v;
        RTTS_EXPECT(!raze::algorithm::minmax(empty_v).has_value());
        auto r1 = raze::algorithm::minmax(std::vector<int>{ 42 });
        RTTS_EXPECT(r1.has_value());
        RTTS_EQUAL(r1->min, 42);
        RTTS_EQUAL(r1->max, 42);

        volatile int vbuf[16] = { 10, 9, 8, -3, 6, 5, 4, 3, 2, 1, 0, 99, 11, 12, 13, 14 };
        auto rv = raze::algorithm::minmax(vbuf, vbuf + 16);
        RTTS_EXPECT(rv.has_value());
        RTTS_EQUAL(rv->min, -3);
        RTTS_EQUAL(rv->max, 99);

        std::vector<non_default_mm_item> nd;
        nd.emplace_back(5, 1);
        nd.emplace_back(2, 2);
        nd.emplace_back(9, 3);
        nd.emplace_back(2, 4);
        nd.emplace_back(9, 5);
        auto rnd = raze::algorithm::minmax(nd, std::less<>{}, &non_default_mm_item::key);
        RTTS_EXPECT(rnd.has_value());
        RTTS_EQUAL(rnd->min.id, 2);
        RTTS_EQUAL(rnd->max.id, 5);
    }
};
