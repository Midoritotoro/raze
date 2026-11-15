#include <tests/rts/rts.h>
#include <raze/algorithm/order/Reverse.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <list>
#include <random>
#include <ranges>
#include <vector>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    struct reverse_item {
        int32_t key;
        int32_t value;
        bool operator==(const reverse_item&) const = default;
    };

    template <class T>
    void reverse_boundary_sweep() {
        constexpr size_t MaxN = 140;
        for (size_t n = 0; n <= MaxN; ++n) {
            std::vector<T> input(n);
            for (size_t i = 0; i < n; ++i)
                input[i] = static_cast<T>(i);

            auto exp = input;
            std::ranges::reverse(exp);

            auto v = input;
            auto it = raze::algorithm::reverse(v);
            RTTS_EXPECT(it == v.end());
            RTTS_EXPECT(v == exp);

            v = input;
            it = raze::algorithm::reverse(v.begin(), v.end());
            RTTS_EXPECT(it == v.end());
            RTTS_EXPECT(v == exp);

            v = input;
            it = raze::algorithm::reverse[raze::options::fscalar](v);
            RTTS_EXPECT(it == v.end());
            RTTS_EXPECT(v == exp);

            v = input;
            it = raze::algorithm::reverse[raze::options::unroll<2>](v);
            RTTS_EXPECT(it == v.end());
            RTTS_EXPECT(v == exp);

            v = input;
            it = raze::algorithm::reverse[raze::options::fstatic](v);
            RTTS_EXPECT(it == v.end());
            RTTS_EXPECT(v == exp);

            it = raze::algorithm::reverse(v);
            RTTS_EXPECT(it == v.end());
            RTTS_EXPECT(v == input);
        }
    }

    template <class T>
    void reverse_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);
        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            auto exp = input;
            std::ranges::reverse(exp);

            auto v = input;
            auto it = raze::algorithm::reverse(v);
            RTTS_EXPECT(it == v.end());
            RTTS_EXPECT(v == exp);

            v = input;
            it = raze::algorithm::reverse[raze::options::fscalar](v.begin(), v.end());
            RTTS_EXPECT(it == v.end());
            RTTS_EXPECT(v == exp);

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            v = input;
            exp = input;
            std::ranges::reverse(exp.begin() + first, exp.begin() + last);
            it = raze::algorithm::reverse(v.begin() + first, v.begin() + last);
            RTTS_EXPECT(it == v.begin() + last);
            RTTS_EXPECT(v == exp);
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::reverse", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    auto cfg = config::thorough();

    run<range, T, rtts::algorithm::bidirectional_iterator>(cfg,
        [](auto c) { return std::ranges::distance(c.first(), raze::algorithm::reverse(c.first(), c.last())); },
        [](auto c) { return std::ranges::distance(c.first(), std::ranges::reverse(c.first(), c.last())); });

    run<range, T, rtts::algorithm::bidirectional_iterator>(cfg,
        [](auto c) { return std::ranges::distance(c.first(), raze::algorithm::reverse(c.range())); },
        [](auto c) { return std::ranges::distance(c.first(), std::ranges::reverse(c.range())); });

    run<range, T, rtts::algorithm::bidirectional_iterator>(cfg,
        [](auto c) { return std::ranges::distance(c.first(), raze::algorithm::reverse[raze::options::fscalar](c.first(), c.last())); },
        [](auto c) { return std::ranges::distance(c.first(), std::ranges::reverse(c.first(), c.last())); });

    run<range, T, rtts::algorithm::bidirectional_iterator>(cfg,
        [](auto c) { return std::ranges::distance(c.first(), raze::algorithm::reverse[raze::options::unroll<2>](c.range())); },
        [](auto c) { return std::ranges::distance(c.first(), std::ranges::reverse(c.range())); });

    run<range, T, rtts::algorithm::bidirectional_iterator>(cfg,
        [](auto c) { return std::ranges::distance(c.first(), raze::algorithm::reverse[raze::options::fstatic](c.range())); },
        [](auto c) { return std::ranges::distance(c.first(), std::ranges::reverse(c.range())); });

    static_assert([]() constexpr {
        std::array<T, 0> a{};
        return raze::algorithm::reverse(a) == a.end();
        }());

    static_assert([]() constexpr {
        std::array<T, 1> a{ T(1) };
        return raze::algorithm::reverse(a) == a.end() && a == std::array<T, 1>{ T(1) };
        }());

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        return raze::algorithm::reverse(a) == a.end() && a == std::array<T, 4>{ T(4), T(3), T(2), T(1) };
        }());

    static_assert([]() constexpr {
        std::array<T, 5> a{ T(1), T(2), T(3), T(4), T(5) };
        return raze::algorithm::reverse[raze::options::fscalar](a) == a.end() &&
               a == std::array<T, 5>{ T(5), T(4), T(3), T(2), T(1) };
        }());

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        return raze::algorithm::reverse(a.begin() + 1, a.begin() + 3) == a.begin() + 3 &&
               a == std::array<T, 4>{ T(1), T(3), T(2), T(4) };
        }());
};

RTTS_CASE_TPL("raze::algorithm::reverse boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    reverse_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::reverse fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        reverse_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::reverse structs") {
    std::vector<reverse_item> input{
        { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 }
    };

    auto exp = input;
    std::ranges::reverse(exp);

    auto v = input;
    auto it = raze::algorithm::reverse(v);
    RTTS_EXPECT(it == v.end());
    RTTS_EXPECT(v == exp);

    v = input;
    it = raze::algorithm::reverse[raze::options::fscalar](v);
    RTTS_EXPECT(it == v.end());
    RTTS_EXPECT(v == exp);
};

RTTS_CASE("raze::algorithm::reverse bidirectional") {
    std::list<int> input{ 1, 2, 3, 4, 5 };
    auto exp = input;
    std::ranges::reverse(exp);

    auto v = input;
    auto it = raze::algorithm::reverse(v);
    RTTS_EXPECT(it == v.end());
    RTTS_EXPECT(v == exp);

    v = input;
    it = raze::algorithm::reverse[raze::options::fscalar](v.begin(), v.end());
    RTTS_EXPECT(it == v.end());
    RTTS_EXPECT(v == exp);
};