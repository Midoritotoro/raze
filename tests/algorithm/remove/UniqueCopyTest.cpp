#include <tests/rts/rts.h>
#include <raze/algorithm/remove/UniqueCopy.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <list>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::unique_copy", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

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
    {
        const std::vector<int> src{ 1, 1, 2, 2, 2, 3, 4, 4, 1 };
        rtts::single_pass_input_range<int> sp(src);
        std::vector<int> out;
        auto r = raze::algorithm::unique_copy(sp, std::back_inserter(out));
        RTTS_EXPECT(r.in == sp.end());
        RTTS_EQUAL(sp.reads(), src.size());
        RTTS_EXPECT((out == std::vector<int>{ 1, 2, 3, 4, 1 }));
    }

    {
        const std::vector<rtts::test_item> src{ { 1, 10 }, { 1, 20 }, { 2, 30 }, { 3, 40 }, { 3, 50 } };
        std::vector<rtts::test_item> out;
        raze::algorithm::unique_copy(src, std::back_inserter(out), std::equal_to<>{}, &rtts::test_item::key);
        RTTS_EQUAL(out.size(), size_t(3));
        RTTS_EXPECT(out[0].payload == 10 && out[1].payload == 30 && out[2].payload == 40);
    }

    {
        std::vector<int> out;
        auto dr = raze::algorithm::unique_copy(std::vector<int>{ 1, 1, 2 }, std::back_inserter(out));
        static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
        RTTS_EXPECT((out == std::vector<int>{ 1, 2 }));
    }
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void unique_copy_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 257);
            std::vector<T> src(n);
            if ((it % 3) == 0) {
                src = rtts::random::vector<T>(n, rng());
            }
            else {
                const unsigned alpha = (it % 3 == 1) ? 3u : 10u;
                for (size_t i = 0; i < n; ++i)
                    src[i] = static_cast<T>(rng() % alpha);
            }

            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            const size_t len = hi - lo;
            auto sub = std::ranges::subrange(src.begin() + static_cast<std::ptrdiff_t>(lo),
                                             src.begin() + static_cast<std::ptrdiff_t>(hi));

            std::vector<T> exp(len);
            auto re = std::ranges::unique_copy(sub, exp.begin());
            const auto exp_n = re.out - exp.begin();

            auto verify = [&](auto fn) {
                std::vector<T> out(len);
                auto r = fn(sub, out.begin());
                RTTS_EXPECT(r.in == sub.end());
                RTTS_EXPECT(r.out - out.begin() == exp_n);
                RTTS_EXPECT(std::equal(out.begin(), r.out, exp.begin()));
            };

            verify([](auto s, auto o) { return raze::algorithm::unique_copy(s, o); });
            verify([](auto s, auto o) { return raze::algorithm::unique_copy(s.begin(), s.end(), o); });
            verify([](auto s, auto o) { return raze::algorithm::unique_copy[opt::fscalar](s, o); });
            verify([](auto s, auto o) { return raze::algorithm::unique_copy[opt::fstatic](s, o); });

            if (it % 8 == 0 && len <= 64) {
                std::vector<T> sub_vec(sub.begin(), sub.end());
                rtts::single_pass_input_range<T> sp(sub_vec);
                std::vector<T> out;
                auto r = raze::algorithm::unique_copy(sp, std::back_inserter(out));
                RTTS_EXPECT(r.in == sp.end());
                RTTS_EQUAL(sp.reads(), sub_vec.size());
                RTTS_EXPECT(static_cast<std::ptrdiff_t>(out.size()) == exp_n);
                RTTS_EXPECT(std::equal(out.begin(), out.end(), exp.begin()));
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::unique_copy fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        unique_copy_fuzz<T>(seed, 40);
};
