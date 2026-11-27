#include <tests/rts/rts.h>
#include <raze/algorithm/copy/MoveBackward.h>

#include <algorithm>
#include <array>
#include <list>
#include <memory>
#include <ranges>
#include <string>
#include <vector>
#include <deque>
#include <random>

RTTS_CASE_TPL("raze::algorithm::move_backward", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    run<range, T, bidirectional_iterator>(cfg,
        [](auto c) { return raze::algorithm::move_backward(c.first(), c.last(), c.data.output.end()); },
        [](auto c) { return std::ranges::move_backward(c.first(), c.last(), c.data.output.end()); });

    run<range, T, bidirectional_iterator>(cfg,
        [](auto c) { return raze::algorithm::move_backward(c.range(), c.data.output.end()); },
        [](auto c) { return std::ranges::move_backward(c.range(), c.data.output.end()); });

    run<range, T, bidirectional_iterator>(cfg,
        [](auto c) { return raze::algorithm::move_backward[opt::fscalar](c.range(), c.data.output.end()); },
        [](auto c) { return std::ranges::move_backward(c.range(), c.data.output.end()); });

    run<range, T, bidirectional_iterator>(cfg,
        [](auto c) { return raze::algorithm::move_backward[opt::fstatic](c.first(), c.last(), c.data.output.end()); },
        [](auto c) { return std::ranges::move_backward(c.first(), c.last(), c.data.output.end()); });

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::move_backward(c.first(), c.last(), c.data.output.end()); },
        [](auto c) constexpr { return std::ranges::move_backward(c.first(), c.last(), c.data.output.end()); }));

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::move_backward[opt::fscalar](c.range(), c.data.output.end()); },
        [](auto c) constexpr { return std::ranges::move_backward(c.range(), c.data.output.end()); }));
};

RTTS_CASE("raze::algorithm::move_backward move-only, overlap, non-common, and dangling") {
    {
        std::vector<std::unique_ptr<int>> src;
        for (int i = 0; i < 6; ++i)
            src.push_back(std::make_unique<int>(i + 100));

        std::vector<std::unique_ptr<int>> dst(src.size());
        auto r = raze::algorithm::move_backward(src, dst.end());
        RTTS_EXPECT(r.in == src.end() && r.out == dst.begin());
        for (int i = 0; i < 6; ++i) {
            RTTS_EXPECT(src[i] == nullptr);
            RTTS_EXPECT(dst[i] != nullptr && *dst[i] == i + 100);
        }
    }

    {
        std::vector<std::string> v{ "a", "b", "c", "d", "e" };
        auto r = raze::algorithm::move_backward(v.begin(), v.begin() + 3, v.end());
        RTTS_EXPECT(r.in == v.begin() + 3 && r.out == v.begin() + 2);
        RTTS_EXPECT(v[2] == "a" && v[3] == "b" && v[4] == "c");
    }

    {
        std::list<int> src{ 1, 2, 3, 4 };
        std::list<int> dst(4, 0);
        auto r = raze::algorithm::move_backward(rtts::make_non_common(src), dst.end());
        RTTS_EXPECT(r.in == src.end() && r.out == dst.begin());
        RTTS_EXPECT(dst == src);

        std::vector<int> out(3);
        auto dr = raze::algorithm::move_backward(std::vector<int>{ 4, 5, 6 }, out.end());
        static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
        RTTS_EXPECT((out == std::vector<int>{ 4, 5, 6 }));
    }
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void move_backward_fuzz(unsigned seed, size_t iters) {
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
                auto re = std::ranges::move_backward(s_exp.begin() + static_cast<std::ptrdiff_t>(lo),
                                                     s_exp.begin() + static_cast<std::ptrdiff_t>(hi), d_exp.end());
                auto r1 = raze::algorithm::move_backward(s1.begin() + static_cast<std::ptrdiff_t>(lo),
                                                         s1.begin() + static_cast<std::ptrdiff_t>(hi), d1.end());
                auto r2 = raze::algorithm::move_backward(std::ranges::subrange(s2.begin() + static_cast<std::ptrdiff_t>(lo),
                                                                               s2.begin() + static_cast<std::ptrdiff_t>(hi)), d2.end());
                auto r3 = raze::algorithm::move_backward[opt::fscalar](s3.begin() + static_cast<std::ptrdiff_t>(lo),
                                                                       s3.begin() + static_cast<std::ptrdiff_t>(hi), d3.end());
                auto r4 = raze::algorithm::move_backward[opt::fstatic](s4.begin() + static_cast<std::ptrdiff_t>(lo),
                                                                       s4.begin() + static_cast<std::ptrdiff_t>(hi), d4.end());

                RTTS_EXPECT(r1.in - s1.begin() == re.in - s_exp.begin() && r1.out == d1.begin());
                RTTS_EXPECT(r2.in - s2.begin() == re.in - s_exp.begin() && r2.out == d2.begin());
                RTTS_EXPECT(r3.in - s3.begin() == re.in - s_exp.begin() && r3.out == d3.begin());
                RTTS_EXPECT(r4.in - s4.begin() == re.in - s_exp.begin() && r4.out == d4.begin());
                RTTS_ALL_EQUAL(d1, d_exp);
                RTTS_ALL_EQUAL(d2, d_exp);
                RTTS_ALL_EQUAL(d3, d_exp);
                RTTS_ALL_EQUAL(d4, d_exp);
            }

            if (hi < n && len > 0) {
                const size_t dst_end = hi + static_cast<size_t>(rng() % (n - hi + 1));
                auto v1 = base, v2 = base, v_exp = base;
                auto re = std::ranges::move_backward(v_exp.begin() + static_cast<std::ptrdiff_t>(lo),
                                                     v_exp.begin() + static_cast<std::ptrdiff_t>(hi),
                                                     v_exp.begin() + static_cast<std::ptrdiff_t>(dst_end));
                auto r1 = raze::algorithm::move_backward(v1.begin() + static_cast<std::ptrdiff_t>(lo),
                                                         v1.begin() + static_cast<std::ptrdiff_t>(hi),
                                                         v1.begin() + static_cast<std::ptrdiff_t>(dst_end));
                auto r2 = raze::algorithm::move_backward[opt::fscalar](v2.begin() + static_cast<std::ptrdiff_t>(lo),
                                                                       v2.begin() + static_cast<std::ptrdiff_t>(hi),
                                                                       v2.begin() + static_cast<std::ptrdiff_t>(dst_end));
                RTTS_EXPECT(r1.out - v1.begin() == re.out - v_exp.begin());
                RTTS_EXPECT(r2.out - v2.begin() == re.out - v_exp.begin());
                RTTS_ALL_EQUAL(v1, v_exp);
                RTTS_ALL_EQUAL(v2, v_exp);
            }

            if (it % 8 == 0 && n <= 64) {
                std::list<T> lst(base.begin(), base.end());
                std::deque<T> dq(n);
                auto r = raze::algorithm::move_backward(lst, dq.end());
                RTTS_EXPECT(r.in == lst.end() && r.out == dq.begin());
                RTTS_ALL_EQUAL(dq, base);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::move_backward fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        move_backward_fuzz<T>(seed, 40);
};
