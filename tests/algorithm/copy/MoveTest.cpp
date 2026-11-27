#include <tests/rts/rts.h>
#include <raze/algorithm/copy/Move.h>

#include <algorithm>
#include <array>
#include <memory>
#include <ranges>
#include <string>
#include <vector>
#include <deque>
#include <random>

RTTS_CASE_TPL("raze::algorithm::move", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::move(c.first(), c.last(), c.out()); },
        [](auto c) { return std::ranges::move(c.first(), c.last(), c.out()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::move(c.range(), c.out()); },
        [](auto c) { return std::ranges::move(c.range(), c.out()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::move[opt::fscalar](c.range(), c.out()); },
        [](auto c) { return std::ranges::move(c.range(), c.out()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::move[opt::fstatic](c.first(), c.last(), c.out()); },
        [](auto c) { return std::ranges::move(c.first(), c.last(), c.out()); });

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::move(c.first(), c.last(), c.out()); },
        [](auto c) constexpr { return std::ranges::move(c.first(), c.last(), c.out()); }));

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::move[opt::fscalar](c.range(), c.out()); },
        [](auto c) constexpr { return std::ranges::move(c.range(), c.out()); }));
};

RTTS_CASE("raze::algorithm::move move-only, overlap, output iterators, and dangling") {
    {
        std::vector<std::unique_ptr<int>> src;
        for (int i = 0; i < 8; ++i)
            src.push_back(std::make_unique<int>(i * 10));

        std::vector<std::unique_ptr<int>> dst(src.size());
        auto r = raze::algorithm::move(src, dst.begin());
        RTTS_EXPECT(r.in == src.end() && r.out == dst.end());
        for (int i = 0; i < 8; ++i) {
            RTTS_EXPECT(src[i] == nullptr);
            RTTS_EXPECT(dst[i] != nullptr && *dst[i] == i * 10);
        }
    }

    {
        std::vector<std::string> v{ "a", "b", "c", "d", "e" };
        auto r = raze::algorithm::move(v.begin() + 2, v.end(), v.begin());
        RTTS_EXPECT(r.in == v.end() && r.out == v.begin() + 3);
        RTTS_EXPECT(v[0] == "c" && v[1] == "d" && v[2] == "e");
    }

    {
        std::vector<std::string> src{ "one", "two", "three" };
        std::vector<std::string> out;
        raze::algorithm::move(rtts::make_non_common(src), std::back_inserter(out));
        RTTS_EXPECT((out == std::vector<std::string>{ "one", "two", "three" }));

        std::vector<int> dst(3);
        auto dr = raze::algorithm::move(std::vector<int>{ 7, 8, 9 }, dst.begin());
        static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
        RTTS_EXPECT((dst == std::vector<int>{ 7, 8, 9 }));
    }
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void move_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 513);
            const auto base = rtts::random::vector<T>(n, rng());
            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            const size_t len = hi - lo;

            {
                auto s1 = base, s2 = base, s3 = base, s4 = base, s_exp = base;
                std::vector<T> d1(len), d2(len), d3(len), d4(len), d_exp(len);
                auto re = std::ranges::move(s_exp.begin() + static_cast<std::ptrdiff_t>(lo),
                                            s_exp.begin() + static_cast<std::ptrdiff_t>(hi), d_exp.begin());
                auto r1 = raze::algorithm::move(s1.begin() + static_cast<std::ptrdiff_t>(lo),
                                                s1.begin() + static_cast<std::ptrdiff_t>(hi), d1.begin());
                auto r2 = raze::algorithm::move(std::ranges::subrange(s2.begin() + static_cast<std::ptrdiff_t>(lo),
                                                                      s2.begin() + static_cast<std::ptrdiff_t>(hi)), d2.begin());
                auto r3 = raze::algorithm::move[opt::fscalar](s3.begin() + static_cast<std::ptrdiff_t>(lo),
                                                              s3.begin() + static_cast<std::ptrdiff_t>(hi), d3.begin());
                auto r4 = raze::algorithm::move[opt::fstatic](s4.begin() + static_cast<std::ptrdiff_t>(lo),
                                                              s4.begin() + static_cast<std::ptrdiff_t>(hi), d4.begin());

                RTTS_EXPECT(r1.in - s1.begin() == re.in - s_exp.begin() && r1.out == d1.end());
                RTTS_EXPECT(r2.in - s2.begin() == re.in - s_exp.begin() && r2.out == d2.end());
                RTTS_EXPECT(r3.in - s3.begin() == re.in - s_exp.begin() && r3.out == d3.end());
                RTTS_EXPECT(r4.in - s4.begin() == re.in - s_exp.begin() && r4.out == d4.end());
                RTTS_ALL_EQUAL(d1, d_exp);
                RTTS_ALL_EQUAL(d2, d_exp);
                RTTS_ALL_EQUAL(d3, d_exp);
                RTTS_ALL_EQUAL(d4, d_exp);
            }

            if (lo > 0 && len > 0) {
                const size_t dst_off = static_cast<size_t>(rng() % (lo + 1));
                auto v1 = base, v2 = base, v_exp = base;
                auto re = std::ranges::move(v_exp.begin() + static_cast<std::ptrdiff_t>(lo),
                                            v_exp.begin() + static_cast<std::ptrdiff_t>(hi),
                                            v_exp.begin() + static_cast<std::ptrdiff_t>(dst_off));
                auto r1 = raze::algorithm::move(v1.begin() + static_cast<std::ptrdiff_t>(lo),
                                                v1.begin() + static_cast<std::ptrdiff_t>(hi),
                                                v1.begin() + static_cast<std::ptrdiff_t>(dst_off));
                auto r2 = raze::algorithm::move[opt::fscalar](v2.begin() + static_cast<std::ptrdiff_t>(lo),
                                                              v2.begin() + static_cast<std::ptrdiff_t>(hi),
                                                              v2.begin() + static_cast<std::ptrdiff_t>(dst_off));
                RTTS_EXPECT(r1.out - v1.begin() == re.out - v_exp.begin());
                RTTS_EXPECT(r2.out - v2.begin() == re.out - v_exp.begin());
                RTTS_ALL_EQUAL(v1, v_exp);
                RTTS_ALL_EQUAL(v2, v_exp);
            }

            if (it % 8 == 0) {
                std::deque<T> dq(base.begin(), base.end());
                std::deque<T> out(n);
                auto r = raze::algorithm::move(dq, out.begin());
                RTTS_EXPECT(r.in == dq.end() && r.out == out.end());
                RTTS_ALL_EQUAL(out, base);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::move fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        move_fuzz<T>(seed, 40);
};
