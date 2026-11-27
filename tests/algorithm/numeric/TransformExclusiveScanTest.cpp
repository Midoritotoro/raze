#include <tests/rts/rts.h>
#include <raze/algorithm/numeric/TransformExclusiveScan.h>

#include <array>
#include <iterator>
#include <numeric>
#include <ranges>
#include <vector>
#include <list>
#include <random>

RTTS_CASE_TPL("raze::algorithm::transform_exclusive_scan", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;
    auto uop = [](T x) constexpr { return static_cast<T>(x * T(2)); };

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32), size_t(64) }) {
        std::vector<T> src(sz);
        for (size_t i = 0; i < sz; ++i)
            src[i] = static_cast<T>(i & 1);

        std::vector<T> exp(sz), o1(sz), o2(sz);
        std::transform_exclusive_scan(src.begin(), src.end(), exp.begin(), T(5), std::plus<>{}, uop);

        auto r1 = raze::algorithm::transform_exclusive_scan(src, o1.begin(), T(5), std::plus<>{}, uop);
        auto r2 = raze::algorithm::transform_exclusive_scan[opt::fscalar](
            src.begin(), src.end(), o2.begin(), T(5), std::plus<>{}, uop);

        RTTS_EXPECT(r1.in == src.end() && r1.out == o1.end());
        RTTS_EXPECT(r2.in == src.end() && r2.out == o2.end());
        RTTS_ALL_EQUAL(o1, exp);
        RTTS_ALL_EQUAL(o2, exp);

        auto ip = src;
        raze::algorithm::transform_exclusive_scan(ip, ip.begin(), T(5), std::plus<>{}, uop);
        RTTS_ALL_EQUAL(ip, exp);
    }

    static_assert([]() constexpr {
        std::array<T, 4> src{ T(1), T(2), T(3), T(4) };
        std::array<T, 4> dst{};
        auto dbl = [](T x) constexpr { return static_cast<T>(x * T(2)); };
        auto r = raze::algorithm::transform_exclusive_scan(src, dst.begin(), T(10), std::plus<>{}, dbl);
        return r.in == src.end() && r.out == dst.end()
            && dst == std::array<T, 4>{ T(10), T(12), T(16), T(22) };
    }());
};

RTTS_CASE("raze::algorithm::transform_exclusive_scan single-pass input and dangling") {
    const std::vector<int> src{ 1, 2, 3 };
    rtts::single_pass_input_range<int> sp(src);
    std::vector<int> out;
    raze::algorithm::transform_exclusive_scan(sp, std::back_inserter(out), 100,
        std::plus<>{}, [](int x) { return x * 10; });
    RTTS_EXPECT((out == std::vector<int>{ 100, 110, 130 }));

    std::vector<int> o2(3);
    auto dr = raze::algorithm::transform_exclusive_scan(std::vector<int>{ 1, 2, 3 }, o2.begin(), 0,
        std::plus<>{}, [](int x) { return x * 2; });
    static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
    RTTS_EXPECT((o2 == std::vector<int>{ 0, 2, 6 }));
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void transform_exclusive_scan_fuzz(unsigned seed, size_t iters) {
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

            std::vector<T> exp(len), o1(len), o2(len), o3(len), o4(len);
            std::transform_exclusive_scan(sub.begin(), sub.end(), exp.begin(), init, std::plus<>{}, uop);

            auto r1 = raze::algorithm::transform_exclusive_scan(sub, o1.begin(), init, std::plus<>{}, uop);
            auto r2 = raze::algorithm::transform_exclusive_scan(sub.begin(), sub.end(), o2.begin(), init, std::plus<>{}, uop);
            auto r3 = raze::algorithm::transform_exclusive_scan[opt::fscalar](sub, o3.begin(), init, std::plus<>{}, uop);
            auto r4 = raze::algorithm::transform_exclusive_scan[opt::fstatic](sub, o4.begin(), init, std::plus<>{}, uop);

            RTTS_EXPECT(r1.in == sub.end() && r1.out == o1.end());
            RTTS_EXPECT(r2.in == sub.end() && r2.out == o2.end());
            RTTS_EXPECT(r3.in == sub.end() && r3.out == o3.end());
            RTTS_EXPECT(r4.in == sub.end() && r4.out == o4.end());
            RTTS_ALL_EQUAL(o1, exp);
            RTTS_ALL_EQUAL(o2, exp);
            RTTS_ALL_EQUAL(o3, exp);
            RTTS_ALL_EQUAL(o4, exp);

            auto ip = src;
            raze::algorithm::transform_exclusive_scan(
                ip.begin() + static_cast<std::ptrdiff_t>(lo),
                ip.begin() + static_cast<std::ptrdiff_t>(hi),
                ip.begin() + static_cast<std::ptrdiff_t>(lo), init, std::plus<>{}, uop);
            RTTS_EXPECT(std::equal(exp.begin(), exp.end(), ip.begin() + static_cast<std::ptrdiff_t>(lo)));

            if (it % 8 == 0 && len <= 64) {
                std::list<T> lst(sub.begin(), sub.end());
                std::vector<T> lout;
                raze::algorithm::transform_exclusive_scan(lst, std::back_inserter(lout), init, std::plus<>{}, uop);
                RTTS_ALL_EQUAL(lout, exp);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::transform_exclusive_scan fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        transform_exclusive_scan_fuzz<T>(seed, 40);
};
