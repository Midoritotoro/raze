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

    // Standard URBG (range + iterator-sentinel + fscalar)
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

    // Standard distribution
    {
        std::vector<int> a(128), b(128);
        std::mt19937 g1(77), g2(77);
        std::uniform_int_distribution<int> d1(-50, 50), d2(-50, 50);
        auto it = raze::algorithm::generate_random(a, g1, d1);
        std::ranges::generate(b, [&]() { return d2(g2); });
        RTTS_EXPECT(it == a.end());
        RTTS_ALL_EQUAL(a, b);
    }

    // Bulk span & subrange URBG hooks (P1068)
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

    // Bulk span & subrange distribution hooks (P1068)
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

    // Empty, non-common sentinel, rvalue dangling
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

