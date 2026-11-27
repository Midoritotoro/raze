#include <tests/rts/rts.h>
#include <raze/algorithm/numeric/TransformInclusiveScan.h>

#include <array>
#include <iterator>
#include <numeric>
#include <ranges>
#include <vector>
#include <list>
#include <random>

RTTS_CASE_TPL("raze::algorithm::transform_inclusive_scan", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;
    auto uop = [](T x) constexpr { return static_cast<T>(x * T(2)); };

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32), size_t(64) }) {
        std::vector<T> src(sz);
        for (size_t i = 0; i < sz; ++i)
            src[i] = static_cast<T>(i & 1);

        std::vector<T> exp1(sz), exp2(sz), o1(sz), o2(sz);
        std::transform_inclusive_scan(src.begin(), src.end(), exp1.begin(), std::plus<>{}, uop);
        std::transform_inclusive_scan(src.begin(), src.end(), exp2.begin(), std::plus<>{}, uop, T(5));

        auto r1 = raze::algorithm::transform_inclusive_scan(src, o1.begin(), std::plus<>{}, uop);
        auto r2 = raze::algorithm::transform_inclusive_scan[opt::fscalar](
            src.begin(), src.end(), o2.begin(), std::plus<>{}, uop, T(5));

        RTTS_EXPECT(r1.in == src.end() && r1.out == o1.end());
        RTTS_EXPECT(r2.in == src.end() && r2.out == o2.end());
        RTTS_ALL_EQUAL(o1, exp1);
        RTTS_ALL_EQUAL(o2, exp2);

        auto ip = src;
        raze::algorithm::transform_inclusive_scan(ip, ip.begin(), std::plus<>{}, uop);
        RTTS_ALL_EQUAL(ip, exp1);
    }

    static_assert([]() constexpr {
        std::array<T, 4> src{ T(1), T(2), T(3), T(4) };
        std::array<T, 4> d1{}, d2{};
        auto dbl = [](T x) constexpr { return static_cast<T>(x * T(2)); };
        raze::algorithm::transform_inclusive_scan(src, d1.begin(), std::plus<>{}, dbl);
        raze::algorithm::transform_inclusive_scan(src, d2.begin(), std::plus<>{}, dbl, T(10));
        return d1 == std::array<T, 4>{ T(2), T(6), T(12), T(20) }
            && d2 == std::array<T, 4>{ T(12), T(16), T(22), T(30) };
    }());
};

RTTS_CASE("raze::algorithm::transform_inclusive_scan single-pass input and dangling") {
    const std::vector<int> src{ 1, 2, 3 };
    rtts::single_pass_input_range<int> sp(src);
    std::vector<int> out;
    raze::algorithm::transform_inclusive_scan(sp, std::back_inserter(out),
        std::plus<>{}, [](int x) { return x * 10; });
    RTTS_EXPECT((out == std::vector<int>{ 10, 30, 60 }));

    std::vector<int> o2(3);
    auto dr = raze::algorithm::transform_inclusive_scan(std::vector<int>{ 1, 2, 3 }, o2.begin(),
        std::plus<>{}, [](int x) { return x + 1; });
    static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
    RTTS_EXPECT((o2 == std::vector<int>{ 2, 5, 9 }));
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void transform_inclusive_scan_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);
        const size_t max_n = (sizeof(T) == 1) ? 30u : 256u;
        auto uop = [](T x) { return static_cast<T>(x * T(2)); };

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % (max_n + 1));
            std::vector<T> src(n);
            for (size_t i = 0; i < n; ++i) {
                if constexpr (sizeof(T) == 1)
                    src[i] = static_cast<T>(rng() % 2u);
                else
                    src[i] = static_cast<T>(static_cast<int>(rng() % 11u) - (std::is_signed_v<T> ? 5 : 0));
            }

            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            const size_t len = hi - lo;
            auto sub = std::ranges::subrange(src.begin() + static_cast<std::ptrdiff_t>(lo),
                                             src.begin() + static_cast<std::ptrdiff_t>(hi));
            const T init = static_cast<T>(rng() % 21u);

            std::vector<T> exp1(len), exp2(len), o1(len), o2(len), o3(len), o4(len);
            std::transform_inclusive_scan(sub.begin(), sub.end(), exp1.begin(), std::plus<>{}, uop);
            std::transform_inclusive_scan(sub.begin(), sub.end(), exp2.begin(), std::plus<>{}, uop, init);

            auto r1 = raze::algorithm::transform_inclusive_scan(sub, o1.begin(), std::plus<>{}, uop);
            auto r2 = raze::algorithm::transform_inclusive_scan(sub.begin(), sub.end(), o2.begin(), std::plus<>{}, uop, init);
            auto r3 = raze::algorithm::transform_inclusive_scan[opt::fscalar](sub, o3.begin(), std::plus<>{}, uop);
            auto r4 = raze::algorithm::transform_inclusive_scan[opt::fstatic](sub, o4.begin(), std::plus<>{}, uop, init);

            RTTS_EXPECT(r1.in == sub.end() && r1.out == o1.end());
            RTTS_EXPECT(r2.in == sub.end() && r2.out == o2.end());
            RTTS_EXPECT(r3.in == sub.end() && r3.out == o3.end());
            RTTS_EXPECT(r4.in == sub.end() && r4.out == o4.end());
            RTTS_ALL_EQUAL(o1, exp1);
            RTTS_ALL_EQUAL(o2, exp2);
            RTTS_ALL_EQUAL(o3, exp1);
            RTTS_ALL_EQUAL(o4, exp2);

            auto ip = src;
            raze::algorithm::transform_inclusive_scan(
                ip.begin() + static_cast<std::ptrdiff_t>(lo),
                ip.begin() + static_cast<std::ptrdiff_t>(hi),
                ip.begin() + static_cast<std::ptrdiff_t>(lo), std::plus<>{}, uop, init);
            RTTS_EXPECT(std::equal(exp2.begin(), exp2.end(), ip.begin() + static_cast<std::ptrdiff_t>(lo)));

            if (it % 8 == 0 && len <= 64) {
                std::list<T> lst(sub.begin(), sub.end());
                std::vector<T> lout;
                raze::algorithm::transform_inclusive_scan(lst, std::back_inserter(lout), std::plus<>{}, uop);
                RTTS_ALL_EQUAL(lout, exp1);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::transform_inclusive_scan fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        transform_inclusive_scan_fuzz<T>(seed, 40);
};
