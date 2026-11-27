#include <tests/rts/rts.h>
#include <raze/algorithm/fill/GenerateRandom.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <list>
#include <random>
#include <ranges>
#include <span>
#include <vector>
#include <deque>

namespace {
    struct constexpr_urbg {
        using result_type = std::uint32_t;
        std::uint32_t state = 1;

        static constexpr result_type min() noexcept { return 0; }
        static constexpr result_type max() noexcept { return 0xFFFFFFFFu; }

        constexpr result_type operator()() noexcept {
            state = state * 1664525u + 1013904223u;
            return state;
        }
    };

    struct bulk_span_urbg {
        using result_type = std::uint32_t;
        std::uint32_t state = 7;
        int span_calls = 0;

        static constexpr result_type min() noexcept { return 0; }
        static constexpr result_type max() noexcept { return 0xFFFFFFFFu; }

        result_type operator()() noexcept { return ++state; }

        void generate_random(std::span<std::uint32_t> sp) noexcept {
            ++span_calls;
            for (auto& x : sp) x = 1000u + (++state);
        }
    };

    struct bulk_subrange_urbg {
        using result_type = std::uint32_t;
        std::uint32_t state = 3;
        int subrange_calls = 0;

        static constexpr result_type min() noexcept { return 0; }
        static constexpr result_type max() noexcept { return 0xFFFFFFFFu; }

        result_type operator()() noexcept { return ++state; }

        template <class It, class Sent>
        void generate_random(std::ranges::subrange<It, Sent> r) noexcept {
            ++subrange_calls;
            for (auto& x : r) x = 2000u + (++state);
        }
    };

    struct bulk_dist {
        int span_calls = 0;
        int subrange_calls = 0;

        template <class G>
        int operator()(G& g) const {
            return static_cast<int>(g() % 100);
        }

        template <class G>
        void generate_random(std::span<int> sp, G& g) {
            ++span_calls;
            for (auto& x : sp) x = static_cast<int>(g() % 100) + 500;
        }

        template <class It, class Sent, class G>
        void generate_random(std::ranges::subrange<It, Sent> r, G& g) {
            ++subrange_calls;
            for (auto& x : r) x = static_cast<int>(g() % 100) + 900;
        }
    };
}

RTTS_CASE("raze::algorithm::generate_random URBG, distribution, bulk hooks, and constexpr") {
    namespace opt = raze::options;

    {
        std::vector<std::uint32_t> a(128), b(128), c(128);
        std::mt19937 g1(42), g2(42), g3(42);
        auto it1 = raze::algorithm::generate_random(a, g1);
        auto it2 = raze::algorithm::generate_random[opt::fscalar](b.begin(), b.end(), g2);
        std::ranges::generate(c, std::ref(g3));
        RTTS_EXPECT(it1 == a.end() && it2 == b.end());
        RTTS_ALL_EQUAL(a, c);
        RTTS_ALL_EQUAL(b, c);
    }

    {
        std::vector<int> a(128), b(128);
        std::mt19937 g1(77), g2(77);
        std::uniform_int_distribution<int> d1(-50, 50), d2(-50, 50);
        auto it = raze::algorithm::generate_random(a, g1, d1);
        std::ranges::generate(b, [&]() { return d2(g2); });
        RTTS_EXPECT(it == a.end());
        RTTS_ALL_EQUAL(a, b);
    }

    {
        std::vector<std::uint32_t> v(16);
        bulk_span_urbg bg;
        raze::algorithm::generate_random(v, bg);
        RTTS_EQUAL(bg.span_calls, 1);
        RTTS_EQUAL(v[0], 1008u);
        RTTS_EQUAL(v[15], 1023u);

        std::list<std::uint32_t> lst(8);
        bulk_subrange_urbg sg;
        raze::algorithm::generate_random(lst, sg);
        RTTS_EQUAL(sg.subrange_calls, 1);
        RTTS_EQUAL(lst.front(), 2004u);
    }

    {
        std::vector<int> v(10);
        std::mt19937 g(11);
        bulk_dist bd;
        raze::algorithm::generate_random(v, g, bd);
        RTTS_EQUAL(bd.span_calls, 1);
        RTTS_EXPECT(v[0] >= 500 && v[0] < 600);

        std::list<int> lst(6);
        raze::algorithm::generate_random(lst, g, bd);
        RTTS_EQUAL(bd.subrange_calls, 1);
        RTTS_EXPECT(lst.front() >= 900 && lst.front() < 1000);
    }

    {
        std::vector<std::uint32_t> empty;
        std::mt19937 g(99);
        RTTS_EXPECT(raze::algorithm::generate_random(empty, g) == empty.end());

        std::vector<std::uint32_t> v(8);
        auto nc_end = raze::algorithm::generate_random(rtts::make_non_common(v), g);
        RTTS_EXPECT(nc_end == v.end());

        auto dr = raze::algorithm::generate_random(std::vector<std::uint32_t>(4), g);
        static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
    }

    static_assert([]() constexpr {
        std::array<std::uint32_t, 5> a{};
        constexpr_urbg g1{};
        constexpr_urbg g2{};
        auto end = raze::algorithm::generate_random(a, g1);
        for (auto x : a) {
            if (x != g2()) return false;
        }
        return end == a.end();
    }());
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void generate_random_dist_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 513);
            const auto base = rtts::random::vector<T>(n, rng());
            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            const unsigned gseed = rng();

            auto dist = [](std::mt19937& g) -> T {
                return static_cast<T>(g() % 97u);
            };

            auto v_exp = base;
            std::mt19937 ge(gseed);
            auto re = std::ranges::generate(v_exp.begin() + static_cast<std::ptrdiff_t>(lo),
                                            v_exp.begin() + static_cast<std::ptrdiff_t>(hi),
                                            [&]() { return dist(ge); });
            const auto exp_off = re - v_exp.begin();
            const auto exp_next = ge();

            auto check_variant = [&](auto call) {
                auto v = base;
                std::mt19937 g(gseed);
                auto first = v.begin() + static_cast<std::ptrdiff_t>(lo);
                auto last = v.begin() + static_cast<std::ptrdiff_t>(hi);
                auto r = call(first, last, g);
                RTTS_EXPECT(r - v.begin() == exp_off);
                RTTS_ALL_EQUAL(v, v_exp);
                RTTS_EQUAL(g(), exp_next);
            };

            check_variant([&](auto f, auto l, auto& g) { return raze::algorithm::generate_random(f, l, g, dist); });
            check_variant([&](auto f, auto l, auto& g) { return raze::algorithm::generate_random(std::ranges::subrange(f, l), g, dist); });
            check_variant([&](auto f, auto l, auto& g) { return raze::algorithm::generate_random[opt::fscalar](f, l, g, dist); });
            check_variant([&](auto f, auto l, auto& g) { return raze::algorithm::generate_random[opt::fstatic](f, l, g, dist); });
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::generate_random distribution fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        generate_random_dist_fuzz<T>(seed, 30);
};

RTTS_CASE("raze::algorithm::generate_random URBG and bulk hooks fuzz") {
    namespace opt = raze::options;

    for (unsigned seed : fuzz_seeds) {
        std::mt19937 rng(seed);
        for (size_t it = 0; it < 40; ++it) {
            const size_t n = static_cast<size_t>(rng() % 513);
            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            const unsigned gseed = rng();

            std::vector<std::uint32_t> v1(n, 0xCAFEu), v2(n, 0xCAFEu), ve(n, 0xCAFEu);
            std::mt19937 g1(gseed), g2(gseed), ge(gseed);
            raze::algorithm::generate_random(v1.begin() + static_cast<std::ptrdiff_t>(lo),
                                             v1.begin() + static_cast<std::ptrdiff_t>(hi), g1);
            raze::algorithm::generate_random[opt::fscalar](std::ranges::subrange(
                v2.begin() + static_cast<std::ptrdiff_t>(lo),
                v2.begin() + static_cast<std::ptrdiff_t>(hi)), g2);
            std::ranges::generate(ve.begin() + static_cast<std::ptrdiff_t>(lo),
                                  ve.begin() + static_cast<std::ptrdiff_t>(hi), std::ref(ge));
            RTTS_ALL_EQUAL(v1, ve);
            RTTS_ALL_EQUAL(v2, ve);

            std::vector<std::uint32_t> bvec(hi - lo);
            bulk_span_urbg bg;
            bg.state = gseed;
            auto bit = raze::algorithm::generate_random(bvec, bg);
            RTTS_EXPECT(bit == bvec.end());
            RTTS_EQUAL(bg.span_calls, 1);
            if (!bvec.empty()) {
                RTTS_EQUAL(bvec.front(), 1000u + gseed + 1u);
                RTTS_EQUAL(bvec.back(), 1000u + gseed + static_cast<std::uint32_t>(bvec.size()));
            }

            if ((hi - lo) <= 64) {
                std::list<std::uint32_t> blst(hi - lo);
                bulk_subrange_urbg sg;
                sg.state = gseed;
                auto sit = raze::algorithm::generate_random(blst, sg);
                RTTS_EXPECT(sit == blst.end());
                RTTS_EQUAL(sg.subrange_calls, 1);
            }
        }
    }
};
