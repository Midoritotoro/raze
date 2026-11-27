#include <tests/rts/rts.h>
#include <raze/algorithm/fill/Generate.h>

#include <algorithm>
#include <array>
#include <list>
#include <ranges>
#include <vector>
#include <deque>
#include <random>

RTTS_CASE_TPL("raze::algorithm::generate", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    auto make_gen = []() {
        int counter = 0;
        return [counter]() mutable -> T { return static_cast<T>((++counter) % 97); };
    };

    run<range, T>(cfg,
        [make_gen](auto c) { return raze::algorithm::generate(c.first(), c.last(), make_gen()); },
        [make_gen](auto c) { return std::ranges::generate(c.first(), c.last(), make_gen()); });

    run<range, T>(cfg,
        [make_gen](auto c) { return raze::algorithm::generate(c.range(), make_gen()); },
        [make_gen](auto c) { return std::ranges::generate(c.range(), make_gen()); });

    run<range, T>(cfg,
        [make_gen](auto c) { return raze::algorithm::generate[opt::fscalar](c.range(), make_gen()); },
        [make_gen](auto c) { return std::ranges::generate(c.range(), make_gen()); });

    run<range, T>(cfg,
        [make_gen](auto c) { return raze::algorithm::generate[opt::fstatic](c.first(), c.last(), make_gen()); },
        [make_gen](auto c) { return std::ranges::generate(c.first(), c.last(), make_gen()); });

    static_assert([]() constexpr {
        std::array<T, 6> a{};
        int n = 1;
        auto end = raze::algorithm::generate(a, [&n]() constexpr -> T { return static_cast<T>(n++); });
        return end == a.end() && a[0] == T(1) && a[5] == T(6);
    }());
};

RTTS_CASE("raze::algorithm::generate views, output iterators, and dangling") {
    {
        std::vector<int> v(10, 0);
        int x = 10;
        auto end = raze::algorithm::generate(rtts::make_non_common(v), [&x]() { return x++; });
        RTTS_EXPECT(end == v.end());
        RTTS_EQUAL(v.front(), 10);
        RTTS_EQUAL(v.back(), 19);

        auto view = v | std::views::drop(2) | std::views::take(4);
        int y = 100;
        raze::algorithm::generate(view, [&y]() { return y++; });
        RTTS_EXPECT(v[2] == 100 && v[5] == 103 && v[6] == 16);
    }

    {
        std::vector<int> empty;
        int calls = 0;
        auto it = raze::algorithm::generate(empty, [&calls]() { return ++calls; });
        RTTS_EXPECT(it == empty.end() && calls == 0);

        auto dr = raze::algorithm::generate(std::vector<int>(3), []() { return 42; });
        static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
    }
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void generate_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 513);
            const auto base = rtts::random::vector<T>(n, rng());
            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            const unsigned gen_seed = rng();

            auto make_gen = [gen_seed]() {
                std::mt19937 g(gen_seed);
                return [g]() mutable -> T { return static_cast<T>(g() % 97u); };
            };

            auto v1 = base, v2 = base, v3 = base, v4 = base, v_exp = base;
            auto re = std::ranges::generate(v_exp.begin() + static_cast<std::ptrdiff_t>(lo),
                                            v_exp.begin() + static_cast<std::ptrdiff_t>(hi), make_gen());
            auto r1 = raze::algorithm::generate(v1.begin() + static_cast<std::ptrdiff_t>(lo),
                                                v1.begin() + static_cast<std::ptrdiff_t>(hi), make_gen());
            auto r2 = raze::algorithm::generate(std::ranges::subrange(v2.begin() + static_cast<std::ptrdiff_t>(lo),
                                                                      v2.begin() + static_cast<std::ptrdiff_t>(hi)), make_gen());
            auto r3 = raze::algorithm::generate[opt::fscalar](v3.begin() + static_cast<std::ptrdiff_t>(lo),
                                                              v3.begin() + static_cast<std::ptrdiff_t>(hi), make_gen());
            auto r4 = raze::algorithm::generate[opt::fstatic](v4.begin() + static_cast<std::ptrdiff_t>(lo),
                                                              v4.begin() + static_cast<std::ptrdiff_t>(hi), make_gen());

            RTTS_EXPECT(r1 - v1.begin() == re - v_exp.begin());
            RTTS_EXPECT(r2 - v2.begin() == re - v_exp.begin());
            RTTS_EXPECT(r3 - v3.begin() == re - v_exp.begin());
            RTTS_EXPECT(r4 - v4.begin() == re - v_exp.begin());
            RTTS_ALL_EQUAL(v1, v_exp);
            RTTS_ALL_EQUAL(v2, v_exp);
            RTTS_ALL_EQUAL(v3, v_exp);
            RTTS_ALL_EQUAL(v4, v_exp);

            if (it % 8 == 0 && n <= 64) {
                std::list<T> lst(base.begin(), base.end());
                std::list<T> lst_exp(base.begin(), base.end());
                auto rl = raze::algorithm::generate(lst, make_gen());
                std::ranges::generate(lst_exp, make_gen());
                RTTS_EXPECT(rl == lst.end());
                RTTS_ALL_EQUAL(lst, lst_exp);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::generate fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        generate_fuzz<T>(seed, 40);
};
