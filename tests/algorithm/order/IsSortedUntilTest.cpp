#include <tests/rts/rts.h>
#include <raze/algorithm/order/IsSortedUntil.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <random>
#include <ranges>
#include <vector>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    struct is_sorted_until_item {
        int32_t key;
        int32_t value;
        bool operator==(const is_sorted_until_item&) const = default;
    };

    template <class T>
    void is_sorted_until_boundary_sweep() {
        constexpr size_t MaxN = 140;
        for (size_t n = 0; n <= MaxN; ++n) {
            std::vector<T> sorted(n);
            for (size_t i = 0; i < n; ++i)
                sorted[i] = static_cast<T>(i / 2 + 1);

            auto check = [](auto& v) {
                const auto exp = std::ranges::is_sorted_until(v);
                RTTS_EXPECT(raze::algorithm::is_sorted_until(v) == exp);
                RTTS_EXPECT(raze::algorithm::is_sorted_until(v.begin(), v.end()) == exp);
                RTTS_EXPECT(raze::algorithm::is_sorted_until[raze::options::fscalar](v) == exp);
                RTTS_EXPECT(raze::algorithm::is_sorted_until[raze::options::unroll<2>](v) == exp);
                RTTS_EXPECT(raze::algorithm::is_sorted_until[raze::options::fstatic](v) == exp);
            };

            check(sorted);

            for (size_t p = 1; p < n; ++p) {
                auto bad = sorted;
                bad[p] = static_cast<T>(bad[p - 1] - 1);
                RTTS_EXPECT(std::ranges::is_sorted_until(bad) == bad.begin() + p);
                check(bad);
            }
        }
    }

    template <class T>
    void is_sorted_until_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);
        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto v = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            switch (i % 4) {
                case 1: std::ranges::sort(v); break;
                case 2: std::ranges::sort(v, std::greater<>{}); break;
                case 3: if (n > 1) { std::ranges::sort(v); std::swap(v[rng() % n], v[rng() % n]); } break;
                default: break;
            }
            RTTS_EXPECT(raze::algorithm::is_sorted_until(v) == std::ranges::is_sorted_until(v));
            RTTS_EXPECT(raze::algorithm::is_sorted_until(v, std::greater<>{}) == std::ranges::is_sorted_until(v, std::greater<>{}));
            RTTS_EXPECT(raze::algorithm::is_sorted_until[raze::options::fscalar](v) == std::ranges::is_sorted_until(v));
        }
    }

    template <class T>
    void is_sorted_until_struct_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 400);
        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto v = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            if (i % 2 == 0) std::ranges::sort(v, std::less<>{}, &T::key);
            RTTS_EXPECT(raze::algorithm::is_sorted_until(v, std::less<>{}, &T::key) ==
                        std::ranges::is_sorted_until(v, std::less<>{}, &T::key));
            RTTS_EXPECT(raze::algorithm::is_sorted_until[raze::options::fscalar](v, std::greater<>{}, &T::key) ==
                        std::ranges::is_sorted_until(v, std::greater<>{}, &T::key));
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::is_sorted_until", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    auto cfg = config::thorough();

    run<range, T>(cfg,
        [](auto c) { return std::ranges::distance(c.first(), raze::algorithm::is_sorted_until(c.first(), c.last())); },
        [](auto c) { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.first(), c.last())); });

    run<range, T>(cfg,
        [](auto c) { return std::ranges::distance(c.first(), raze::algorithm::is_sorted_until(c.range())); },
        [](auto c) { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.range())); });

    run<range, T>(cfg,
        [](auto c) { return std::ranges::distance(c.first(), raze::algorithm::is_sorted_until[raze::options::fscalar](c.first(), c.last())); },
        [](auto c) { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.first(), c.last())); });

    run<range, T>(cfg,
        [](auto c) { return std::ranges::distance(c.first(), raze::algorithm::is_sorted_until[raze::options::unroll<2>](c.range())); },
        [](auto c) { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.range())); });

    run<range, T>(cfg,
        [](auto c) { return std::ranges::distance(c.first(), raze::algorithm::is_sorted_until[raze::options::fstatic](c.range())); },
        [](auto c) { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.range())); });

    run<range, T>(cfg,
        [](auto c) { return std::ranges::distance(c.first(), raze::algorithm::is_sorted_until(c.range(), std::greater<>{})); },
        [](auto c) { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.range(), std::greater<>{})); });

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return std::ranges::distance(c.first(), raze::algorithm::is_sorted_until(c.first(), c.last())); },
        [](auto c) constexpr { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.first(), c.last())); }));

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return std::ranges::distance(c.first(), raze::algorithm::is_sorted_until[raze::options::fscalar](c.range())); },
        [](auto c) constexpr { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.range())); }));

    static_assert([]() constexpr {
        std::array<T, 0> a{};
        return raze::algorithm::is_sorted_until(a) == a.end();
        }());

    static_assert([]() constexpr {
        std::array<T, 1> a{ T(1) };
        return raze::algorithm::is_sorted_until(a) == a.end();
        }());

    static_assert([]() constexpr {
        std::array<T, 3> a{ T(1), T(2), T(3) };
        return raze::algorithm::is_sorted_until(a) == a.end();
        }());

    static_assert([]() constexpr {
        std::array<T, 3> a{ T(2), T(2), T(2) };
        return raze::algorithm::is_sorted_until(a) == a.end();
        }());

    static_assert([]() constexpr {
        std::array<T, 3> a{ T(3), T(1), T(2) };
        return raze::algorithm::is_sorted_until(a) == a.begin() + 1;
        }());

    static_assert([]() constexpr {
        std::array<T, 3> a{ T(1), T(3), T(2) };
        return raze::algorithm::is_sorted_until(a) == a.begin() + 2;
        }());
};

RTTS_CASE_TPL("raze::algorithm::is_sorted_until.projection", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    auto cfg = config::thorough();
    auto proj = [](auto x) { return x; };

    run<range, T>(cfg,
        [proj](auto c) { return std::ranges::distance(c.first(), raze::algorithm::is_sorted_until(c.range(), std::less<>{}, proj)); },
        [proj](auto c) { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.range(), std::less<>{}, proj)); });

    run<range, T>(cfg,
        [proj](auto c) { return std::ranges::distance(c.first(), raze::algorithm::is_sorted_until[raze::options::fscalar](c.first(), c.last(), std::less<>{}, proj)); },
        [proj](auto c) { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.first(), c.last(), std::less<>{}, proj)); });

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return std::ranges::distance(c.first(), raze::algorithm::is_sorted_until(c.range(), std::less<>{}, [](auto x) constexpr { return x; })); },
        [](auto c) constexpr { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.range(), std::less<>{}, [](auto x) constexpr { return x; })); }));
};

RTTS_CASE_TPL("raze::algorithm::is_sorted_until boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    is_sorted_until_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::is_sorted_until fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        is_sorted_until_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::is_sorted_until structs") {
    using namespace rtts::algorithm;
    auto cfg = config::thorough();

    run<range, is_sorted_until_item>(cfg,
        [](auto c) { return std::ranges::distance(c.first(), raze::algorithm::is_sorted_until(c.range(), std::less<>{}, &is_sorted_until_item::key)); },
        [](auto c) { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.range(), std::less<>{}, &is_sorted_until_item::key)); });

    run<range, is_sorted_until_item>(cfg,
        [](auto c) { return std::ranges::distance(c.first(), raze::algorithm::is_sorted_until[raze::options::fscalar](c.first(), c.last(), std::less<>{}, &is_sorted_until_item::key)); },
        [](auto c) { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.first(), c.last(), std::less<>{}, &is_sorted_until_item::key)); });

    static_assert(constexpr_run<is_sorted_until_item, range, 8>(
        [](auto c) constexpr { return std::ranges::distance(c.first(), raze::algorithm::is_sorted_until(c.range(), std::less<>{}, &is_sorted_until_item::key)); },
        [](auto c) constexpr { return std::ranges::distance(c.first(), std::ranges::is_sorted_until(c.range(), std::less<>{}, &is_sorted_until_item::key)); }));
};

RTTS_CASE("raze::algorithm::is_sorted_until structs fuzz") {
    for (unsigned seed : fuzz_seeds)
        is_sorted_until_struct_fuzz<is_sorted_until_item>(seed, 200);
};