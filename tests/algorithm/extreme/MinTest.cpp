#include <tests/rts/rts.h>
#include <raze/algorithm/extreme/Min.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <random>
#include <ranges>
#include <vector>

namespace {
    struct non_default_item {
        int key;
        int id;
        explicit constexpr non_default_item(int k, int i = 0) noexcept : key(k), id(i) {}
        constexpr bool operator==(const non_default_item&) const = default;
    };

    template <std::input_iterator It, std::sentinel_for<It> Sent, class Comp = std::less<>, class Proj = std::identity>
    constexpr auto std_min_opt(It first, Sent last, Comp comp = {}, Proj proj = {})
        -> std::optional<std::iter_value_t<It>>
    {
        if (first == last) return std::nullopt;
        return std::ranges::min(std::ranges::subrange(first, last), comp, proj);
    }

    template <std::ranges::input_range R, class Comp = std::less<>, class Proj = std::identity>
    constexpr auto std_min_opt(R&& r, Comp comp = {}, Proj proj = {})
        -> std::optional<std::ranges::range_value_t<R>>
    {
        if (std::ranges::empty(r)) return std::nullopt;
        return std::ranges::min(std::forward<R>(r), comp, proj);
    }
}

RTTS_CASE_TPL("raze::algorithm::min", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::min(c.first(), c.last()); },
        [](auto c) { return std_min_opt(c.first(), c.last()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::min(c.range()); },
        [](auto c) { return std_min_opt(c.range()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::min[opt::fscalar](c.first(), c.last()); },
        [](auto c) { return std_min_opt(c.first(), c.last()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::min[opt::unroll<1>](c.range()); },
        [](auto c) { return std_min_opt(c.range()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::min[opt::unroll<2>](c.range()); },
        [](auto c) { return std_min_opt(c.range()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::min[opt::fstatic](c.range()); },
        [](auto c) { return std_min_opt(c.range()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::min(c.range(), std::greater<>{}); },
        [](auto c) { return std_min_opt(c.range(), std::greater<>{}); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::min(c.range(), std::ranges::less{}); },
        [](auto c) { return std_min_opt(c.range(), std::ranges::less{}); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::min(c.range(), std::less<T>{}); },
        [](auto c) { return std_min_opt(c.range(), std::less<T>{}); });

    auto cmp_branch = [](auto a, auto b) { if (a < b) return true; return false; };
    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::min[opt::fautovec](c.first(), c.last()); },
        [](auto c) { return std_min_opt(c.first(), c.last()); });

    run<range, T>(cfg,
        [cmp_branch](auto c) { return raze::algorithm::min[opt::fautovec](c.range(), cmp_branch); },
        [cmp_branch](auto c) { return std_min_opt(c.range(), cmp_branch); });

    run<range, T>(cfg,
        [cmp_branch](auto c) { return raze::algorithm::min[opt::fautovec][opt::fstatic](c.range(), cmp_branch); },
        [cmp_branch](auto c) { return std_min_opt(c.range(), cmp_branch); });

    {
        auto acfg = config::compact();
        auto check_aligned = [&]<size_t Align>(auto algo) {
            run_aligned<Align, range, T>(acfg,
                [algo](auto c) { return algo(c.first(), c.last()); },
                [](auto c) { return std_min_opt(c.first(), c.last()); });
            run_aligned<Align, range, T>(acfg,
                [algo](auto c) { return algo(c.range()); },
                [](auto c) { return std_min_opt(c.range()); });
        };

        check_aligned.template operator()<16>(raze::algorithm::min[opt::aligned<16>]);
        check_aligned.template operator()<32>(raze::algorithm::min[opt::aligned<32>]);
        check_aligned.template operator()<32>(raze::algorithm::min[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<32>(raze::algorithm::min[opt::aligned<32>][opt::fstatic]);
        check_aligned.template operator()<64>(raze::algorithm::min[opt::aligned<64>]);
        check_aligned.template operator()<64>(raze::algorithm::min[opt::aligned<64>][opt::fautovec]);
        check_aligned.template operator()<64>(raze::algorithm::min[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1>(raze::algorithm::min[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1>(raze::algorithm::min[opt::aligned<64>][opt::aligned<opt::unaligned>]);
    }

    // Vectorizable custom comparator (abs_less on signed/float, mod-order on unsigned)
    {
        auto custom_vec_cmp = [](auto a, auto b) { return a > b; };
        auto v = rtts::random::vector<T>(300, 12345u);
        RTTS_EQUAL(raze::algorithm::min(v, custom_vec_cmp), std_min_opt(v, custom_vec_cmp));
    }

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::min(c.first(), c.last()); },
        [](auto c) constexpr { return std_min_opt(c.first(), c.last()); }));

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::min[opt::fscalar](c.range()); },
        [](auto c) constexpr { return std_min_opt(c.range()); }));

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::min[opt::fautovec](c.range()); },
        [](auto c) constexpr { return std_min_opt(c.range()); }));

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::min[opt::aligned<64>](c.range()); },
        [](auto c) constexpr { return std_min_opt(c.range()); }));

    static_assert(raze::algorithm::min(std::initializer_list<T>{ T(5), T(2), T(8), T(1), T(4) }) == T(1));
    static_assert(!raze::algorithm::min(std::initializer_list<T>{}).has_value());
};

RTTS_CASE("raze::algorithm::min projections, ties, views, chunking, and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Non-identity projection MUST return original value, NOT projected value!
    {
        const std::vector<int> v = { 10, 30, 20, 50, 40 };
        auto neg_proj = [](int x) { return -x; };
        auto r = raze::algorithm::min(v, std::less<>{}, neg_proj);
        RTTS_EXPECT(r.has_value());
        RTTS_EQUAL(*r, 50);

        auto r_av = raze::algorithm::min[opt::fautovec](v, std::less<>{}, neg_proj);
        RTTS_EXPECT(r_av.has_value());
        RTTS_EQUAL(*r_av, 50);

        const std::vector<rtts::test_item> items = { { 5, 1 }, { 2, 10 }, { 2, 20 }, { 9, 3 } };
        auto ri = raze::algorithm::min(items, std::less<>{}, &rtts::test_item::key);
        RTTS_EXPECT(ri.has_value());
        RTTS_EQUAL(ri->key, 2);
        RTTS_EQUAL(ri->payload, 10); // First minimum!
    }

    // Custom vectorizable comparator with equivalence classes (first minimum preserved!)
    {
        auto mask_low = [](auto a, auto b) { return (a & 0xFF) < (b & 0xFF); };
        std::vector<int> v(130, 0x0010);
        v[17] = 0x1105; // first with low byte == 5
        v[85] = 0x2205; // second with low byte == 5
        v[129] = 0x3305; // tail with low byte == 5
        auto r = raze::algorithm::min(v, mask_low);
        RTTS_EXPECT(r.has_value());
        RTTS_EQUAL(*r, 0x1105);
    }

    // Large u8 chunking (> 255 * 64 elements) with custom vectorizable comparator
    {
        auto custom_cmp = [](auto a, auto b) { return a < b; };
        std::vector<uint8_t> big(20000, 200);
        big[18500] = 7;
        RTTS_EQUAL(raze::algorithm::min(big), std::optional<uint8_t>(7));
        RTTS_EQUAL(raze::algorithm::min(big, custom_cmp), std::optional<uint8_t>(7));
    }

    // Views and single-pass input range
    {
        RTTS_EQUAL(raze::algorithm::min(views::iota(5, 50)), std::optional<int>(5));
        RTTS_EQUAL(raze::algorithm::min(views::iota(5, 50) | views::reverse), std::optional<int>(5));

        const std::vector<int> src = { 9, 2, 7, 1, 8, 3, 6, 4 };
        RTTS_EQUAL(raze::algorithm::min(src | views::drop(1) | views::take(4)), std::optional<int>(1));
        RTTS_EQUAL(raze::algorithm::min(src | views::filter([](int x) { return x > 3; })), std::optional<int>(4));
        RTTS_EQUAL(raze::algorithm::min(src | views::transform([](int x) { return x * 10; })), std::optional<int>(10));
        RTTS_EQUAL(raze::algorithm::min[opt::fstatic](rtts::make_non_common(src)), std::optional<int>(1));

        rtts::single_pass_input_range<int> sp(src);
        RTTS_EQUAL(raze::algorithm::min(sp), std::optional<int>(1));
        RTTS_EQUAL(sp.reads(), src.size());
    }

    // Empty, single element, volatile, non-default-constructible
    {
        std::vector<int> empty_v;
        RTTS_EXPECT(!raze::algorithm::min(empty_v).has_value());
        RTTS_EQUAL(raze::algorithm::min(std::vector<int>{ 42 }), std::optional<int>(42));

        volatile int vbuf[16] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, -5, 11, 12, 13, 14 };
        RTTS_EQUAL(raze::algorithm::min(vbuf, vbuf + 16), std::optional<int>(-5));

        std::vector<non_default_item> nd;
        nd.emplace_back(5, 1);
        nd.emplace_back(2, 2);
        nd.emplace_back(2, 3);
        auto rnd = raze::algorithm::min(nd, std::less<>{}, &non_default_item::key);
        RTTS_EXPECT(rnd.has_value());
        RTTS_EQUAL(rnd->key, 2);
        RTTS_EQUAL(rnd->id, 2);
    }
};
