#include <tests/rts/rts.h>
#include <raze/algorithm/numeric/AdjacentDifference.h>

#include <array>
#include <iterator>
#include <numeric>
#include <ranges>
#include <vector>
#include <list>
#include <random>

RTTS_CASE_TPL("raze::algorithm::adjacent_difference", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(7), size_t(32), size_t(128) }) {
        std::vector<T> src(sz);
        for (size_t i = 0; i < sz; ++i)
            src[i] = static_cast<T>(i % 50);

        std::vector<T> exp(sz), o1(sz), o2(sz);
        std::adjacent_difference(src.begin(), src.end(), exp.begin());

        auto r1 = raze::algorithm::adjacent_difference(src, o1.begin());
        auto r2 = raze::algorithm::adjacent_difference[opt::fscalar](src.begin(), src.end(), o2.begin());

        RTTS_EXPECT(r1.in == src.end() && r1.out == o1.end());
        RTTS_EXPECT(r2.in == src.end() && r2.out == o2.end());
        RTTS_ALL_EQUAL(o1, exp);
        RTTS_ALL_EQUAL(o2, exp);

        auto ip = src;
        raze::algorithm::adjacent_difference(ip, ip.begin());
        RTTS_ALL_EQUAL(ip, exp);
    }

    static_assert([]() constexpr {
        std::array<T, 5> src{ T(2), T(4), T(7), T(11), T(16) };
        std::array<T, 5> dst{};
        auto r = raze::algorithm::adjacent_difference(src, dst.begin());
        return r.in == src.end() && r.out == dst.end()
            && dst == std::array<T, 5>{ T(2), T(2), T(3), T(4), T(5) };
    }());
};

RTTS_CASE("raze::algorithm::adjacent_difference custom op, single-pass input, and dangling") {
    const std::vector<int> src{ 1, 2, 3, 4 };
    rtts::single_pass_input_range<int> sp(src);
    std::vector<int> out;
    raze::algorithm::adjacent_difference(sp, std::back_inserter(out), std::plus<>{});
    RTTS_EXPECT((out == std::vector<int>{ 1, 3, 5, 7 }));

    std::vector<int> o2(3);
    auto dr = raze::algorithm::adjacent_difference(std::vector<int>{ 10, 15, 25 }, o2.begin());
    static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
    RTTS_EXPECT((o2 == std::vector<int>{ 10, 5, 10 }));
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void adjacent_difference_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 257);
            std::vector<T> src(n);
            for (size_t i = 0; i < n; ++i) {
                if constexpr (sizeof(T) == 1 && std::is_signed_v<T>)
                    src[i] = static_cast<T>(static_cast<int>(rng() % 101u) - 50);
                else if constexpr (sizeof(T) == 1)
                    src[i] = static_cast<T>(rng() % 101u);
                else
                    src[i] = static_cast<T>(static_cast<int>(rng() % 1001u) - (std::is_signed_v<T> ? 500 : 0));
            }

            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            const size_t len = hi - lo;
            auto sub = std::ranges::subrange(src.begin() + static_cast<std::ptrdiff_t>(lo),
                                             src.begin() + static_cast<std::ptrdiff_t>(hi));

            std::vector<T> exp(len), o1(len), o2(len), o3(len), o4(len);
            std::adjacent_difference(sub.begin(), sub.end(), exp.begin());

            auto r1 = raze::algorithm::adjacent_difference(sub, o1.begin());
            auto r2 = raze::algorithm::adjacent_difference(sub.begin(), sub.end(), o2.begin());
            auto r3 = raze::algorithm::adjacent_difference[opt::fscalar](sub, o3.begin());
            auto r4 = raze::algorithm::adjacent_difference[opt::fstatic](sub, o4.begin());

            RTTS_EXPECT(r1.in == sub.end() && r1.out == o1.end());
            RTTS_EXPECT(r2.in == sub.end() && r2.out == o2.end());
            RTTS_EXPECT(r3.in == sub.end() && r3.out == o3.end());
            RTTS_EXPECT(r4.in == sub.end() && r4.out == o4.end());
            RTTS_ALL_EQUAL(o1, exp);
            RTTS_ALL_EQUAL(o2, exp);
            RTTS_ALL_EQUAL(o3, exp);
            RTTS_ALL_EQUAL(o4, exp);

            auto ip = src;
            auto rip = raze::algorithm::adjacent_difference(
                ip.begin() + static_cast<std::ptrdiff_t>(lo),
                ip.begin() + static_cast<std::ptrdiff_t>(hi),
                ip.begin() + static_cast<std::ptrdiff_t>(lo));
            RTTS_EXPECT(rip.out == ip.begin() + static_cast<std::ptrdiff_t>(hi));
            RTTS_EXPECT(std::equal(exp.begin(), exp.end(), ip.begin() + static_cast<std::ptrdiff_t>(lo)));
            RTTS_EXPECT(std::equal(src.begin(), src.begin() + static_cast<std::ptrdiff_t>(lo), ip.begin()));
            RTTS_EXPECT(std::equal(src.begin() + static_cast<std::ptrdiff_t>(hi), src.end(),
                                   ip.begin() + static_cast<std::ptrdiff_t>(hi)));

            std::vector<T> exp_p(len), op_out(len);
            std::adjacent_difference(sub.begin(), sub.end(), exp_p.begin(), std::plus<>{});
            raze::algorithm::adjacent_difference(sub, op_out.begin(), std::plus<>{});
            RTTS_ALL_EQUAL(op_out, exp_p);

            if (it % 8 == 0 && len <= 64) {
                std::list<T> lst(sub.begin(), sub.end());
                std::vector<T> lout;
                raze::algorithm::adjacent_difference(lst, std::back_inserter(lout));
                RTTS_ALL_EQUAL(lout, exp);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::adjacent_difference fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        adjacent_difference_fuzz<T>(seed, 40);
};
