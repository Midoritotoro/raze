#include <tests/rts/rts.h>
#include <raze/algorithm/extreme/Max.h>

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
    struct non_default_max_item {
        int key;
        int id;
        explicit constexpr non_default_max_item(int k, int i = 0) noexcept : key(k), id(i) {}
        constexpr bool operator==(const non_default_max_item&) const = default;
    };

    template <std::input_iterator It, std::sentinel_for<It> Sent, class Comp = std::less<>, class Proj = std::identity>
    constexpr auto std_max_opt(It first, Sent last, Comp comp = {}, Proj proj = {})
        -> std::optional<std::iter_value_t<It>>
    {
        if (first == last) return std::nullopt;
        return std::ranges::max(std::ranges::subrange(first, last), comp, proj);
    }

    template <std::ranges::input_range R, class Comp = std::less<>, class Proj = std::identity>
    constexpr auto std_max_opt(R&& r, Comp comp = {}, Proj proj = {})
        -> std::optional<std::ranges::range_value_t<R>>
    {
        if (std::ranges::empty(r)) return std::nullopt;
        return std::ranges::max(std::forward<R>(r), comp, proj);
    }
}

RTTS_CASE_TPL("raze::algorithm::max", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    auto check_iter = [&](auto algo) {
        run<range, T>(cfg,
            [algo](auto c) { return algo(c.first(), c.last()); },
            [](auto c) { return std_max_opt(c.first(), c.last()); });
    };

    auto check_range = [&](auto algo) {
        run<range, T>(cfg,
            [algo](auto c) { return algo(c.range()); },
            [](auto c) { return std_max_opt(c.range()); });
    };

    auto check_cmp = [&](auto algo, auto cmp) {
        run<range, T>(cfg,
            [algo, cmp](auto c) { return algo(c.range(), cmp); },
            [cmp](auto c) { return std_max_opt(c.range(), cmp); });
    };

    check_iter(raze::algorithm::max);
    check_range(raze::algorithm::max);
    check_iter(raze::algorithm::max[opt::fscalar]);
    check_range(raze::algorithm::max[opt::unroll<1>]);
    check_range(raze::algorithm::max[opt::unroll<2>]);
    check_range(raze::algorithm::max[opt::fstatic]);
    check_cmp(raze::algorithm::max, std::greater<>{});
    check_cmp(raze::algorithm::max, std::ranges::greater{});
    check_cmp(raze::algorithm::max, std::less<T>{});

    auto cmp_branch = [](auto a, auto b) { if (a < b) return true; return false; };
    check_iter(raze::algorithm::max[opt::fautovec]);
    check_cmp(raze::algorithm::max[opt::fautovec], cmp_branch);
    check_cmp(raze::algorithm::max[opt::fautovec][opt::fstatic], cmp_branch);

    {
        auto acfg = config::compact();
        auto check_aligned = [&]<size_t Align>(auto algo) {
            run_aligned<Align, range, T>(acfg,
                [algo](auto c) { return algo(c.first(), c.last()); },
                [](auto c) { return std_max_opt(c.first(), c.last()); });
            run_aligned<Align, range, T>(acfg,
                [algo](auto c) { return algo(c.range()); },
                [](auto c) { return std_max_opt(c.range()); });
        };

        check_aligned.template operator()<16>(raze::algorithm::max[opt::aligned<16>]);
        check_aligned.template operator()<32>(raze::algorithm::max[opt::aligned<32>]);
        check_aligned.template operator()<32>(raze::algorithm::max[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<32>(raze::algorithm::max[opt::aligned<32>][opt::fstatic]);
        check_aligned.template operator()<64>(raze::algorithm::max[opt::aligned<64>]);
        check_aligned.template operator()<64>(raze::algorithm::max[opt::aligned<64>][opt::fautovec]);
        check_aligned.template operator()<64>(raze::algorithm::max[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1>(raze::algorithm::max[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1>(raze::algorithm::max[opt::aligned<64>][opt::aligned<opt::unaligned>]);
    }

    {
        auto custom_vec_cmp = [](auto a, auto b) { return a > b; };
        auto v = rtts::random::vector<T>(300, 54321u);
        RTTS_EQUAL(raze::algorithm::max(v, custom_vec_cmp), std_max_opt(v, custom_vec_cmp));
    }

    constexpr auto check_ce_range = [](auto algo) constexpr {
        return constexpr_run<T, range, 8>(
            [algo](auto c) constexpr { return algo(c.range()); },
            [](auto c) constexpr { return std_max_opt(c.range()); });
    };

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::max(c.first(), c.last()); },
        [](auto c) constexpr { return std_max_opt(c.first(), c.last()); }));
    static_assert(check_ce_range(raze::algorithm::max[opt::fscalar]));
    static_assert(check_ce_range(raze::algorithm::max[opt::fautovec]));
    static_assert(check_ce_range(raze::algorithm::max[opt::aligned<64>]));

    static_assert(raze::algorithm::max(std::initializer_list<T>{ T(5), T(2), T(8), T(1), T(4) }) == T(8));
    static_assert(!raze::algorithm::max(std::initializer_list<T>{}).has_value());
};

RTTS_CASE("raze::algorithm::max projections, ties, views, chunking, and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    {
        const std::vector<int> v = { 10, 30, 20, 50, 40 };
        auto neg_proj = [](int x) { return -x; };
        auto r = raze::algorithm::max(v, std::less<>{}, neg_proj);
        RTTS_EXPECT(r.has_value());
        RTTS_EQUAL(*r, 10);

        auto r_av = raze::algorithm::max[opt::fautovec](v, std::less<>{}, neg_proj);
        RTTS_EXPECT(r_av.has_value());
        RTTS_EQUAL(*r_av, 10);

        const std::vector<rtts::test_item> items = { { 5, 1 }, { 9, 10 }, { 9, 20 }, { 2, 3 } };
        auto ri = raze::algorithm::max(items, std::less<>{}, &rtts::test_item::key);
        RTTS_EXPECT(ri.has_value());
        RTTS_EQUAL(ri->key, 9);
        RTTS_EQUAL(ri->payload, 10);
    }

    {
        auto mask_low = [](auto a, auto b) { return (a & 0xFF) < (b & 0xFF); };
        std::vector<int> v(130, 0x0010);
        v[19] = 0x11F0;
        v[87] = 0x22F0;
        v[129] = 0x33F0;
        auto r = raze::algorithm::max(v, mask_low);
        RTTS_EXPECT(r.has_value());
        RTTS_EQUAL(*r, 0x11F0);
    }

    {
        auto custom_cmp = [](auto a, auto b) { return a < b; };
        std::vector<uint8_t> big(20000, 10);
        big[18500] = 250;
        RTTS_EQUAL(raze::algorithm::max(big), std::optional<uint8_t>(250));
        RTTS_EQUAL(raze::algorithm::max(big, custom_cmp), std::optional<uint8_t>(250));
    }

    {
        RTTS_EQUAL(raze::algorithm::max(views::iota(5, 50)), std::optional<int>(49));
        RTTS_EQUAL(raze::algorithm::max(views::iota(5, 50) | views::reverse), std::optional<int>(49));

        const std::vector<int> src = { 9, 2, 7, 1, 8, 3, 6, 4 };
        RTTS_EQUAL(raze::algorithm::max(src | views::drop(1) | views::take(4)), std::optional<int>(8));
        RTTS_EQUAL(raze::algorithm::max(src | views::filter([](int x) { return x < 7; })), std::optional<int>(6));
        RTTS_EQUAL(raze::algorithm::max(src | views::transform([](int x) { return x * 10; })), std::optional<int>(90));
        RTTS_EQUAL(raze::algorithm::max[opt::fstatic](rtts::make_non_common(src)), std::optional<int>(9));

        rtts::single_pass_input_range<int> sp(src);
        RTTS_EQUAL(raze::algorithm::max(sp), std::optional<int>(9));
        RTTS_EQUAL(sp.reads(), src.size());
    }

    {
        std::vector<int> empty_v;
        RTTS_EXPECT(!raze::algorithm::max(empty_v).has_value());
        RTTS_EQUAL(raze::algorithm::max(std::vector<int>{ 42 }), std::optional<int>(42));

        volatile int vbuf[16] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 99, 11, 12, 13, 14 };
        RTTS_EQUAL(raze::algorithm::max(vbuf, vbuf + 16), std::optional<int>(99));

        std::vector<non_default_max_item> nd;
        nd.emplace_back(5, 1);
        nd.emplace_back(9, 2);
        nd.emplace_back(9, 3);
        auto rnd = raze::algorithm::max(nd, std::less<>{}, &non_default_max_item::key);
        RTTS_EXPECT(rnd.has_value());
        RTTS_EQUAL(rnd->key, 9);
        RTTS_EQUAL(rnd->id, 2);
    }
};
