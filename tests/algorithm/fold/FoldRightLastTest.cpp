#include <tests/rts/rts.h>
#include <raze/algorithm/fold/FoldRightLast.h>

#include <algorithm>
#include <array>
#include <list>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::fold_right_last", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32) }) {
        std::vector<T> v(sz);
        for (size_t i = 0; i < sz; ++i)
            v[i] = static_cast<T>((i & 1) ? 0 : 1);

        const auto exp = std::ranges::fold_right_last(v, std::plus<>{});
        RTTS_EXPECT(raze::algorithm::fold_right_last(v, std::plus<>{}) == exp);
        RTTS_EXPECT(raze::algorithm::fold_right_last[opt::fscalar](v.begin(), v.end(), std::plus<>{}) == exp);
    }

    static_assert([]() constexpr {
        std::array<T, 3> a{ T(1), T(2), T(10) };
        std::array<T, 0> e{};
        auto r1 = raze::algorithm::fold_right_last(a, [](T x, auto acc) constexpr -> int {
            return static_cast<int>(x) - static_cast<int>(acc);
        });
        auto r2 = raze::algorithm::fold_right_last(e, std::plus<>{});
        return r1 == 9 && !r2.has_value();
    }());
};

RTTS_CASE("raze::algorithm::fold_right_last list and non-common") {
    const std::list<int> lst{ 1, 2, 10 };
    RTTS_EXPECT(raze::algorithm::fold_right_last(lst, std::minus<>{}) == 9);
    RTTS_EXPECT(raze::algorithm::fold_right_last(rtts::make_non_common(lst), std::minus<>{}) == 9);
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void fold_right_last_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);
        const size_t max_n = (sizeof(T) == 1) ? 40u : 128u;

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % (max_n + 1));
            std::vector<T> v(n);
            for (size_t i = 0; i < n; ++i)
                v[i] = static_cast<T>(rng() % (sizeof(T) == 1 ? 3u : 11u));

            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            auto sub = std::ranges::subrange(v.begin() + static_cast<std::ptrdiff_t>(lo),
                                             v.begin() + static_cast<std::ptrdiff_t>(hi));

            const auto exp_sum = std::ranges::fold_right_last(sub, std::plus<>{});
            RTTS_EXPECT(raze::algorithm::fold_right_last(sub, std::plus<>{}) == exp_sum);
            RTTS_EXPECT(raze::algorithm::fold_right_last(sub.begin(), sub.end(), std::plus<>{}) == exp_sum);
            RTTS_EXPECT(raze::algorithm::fold_right_last[opt::fscalar](sub, std::plus<>{}) == exp_sum);
            RTTS_EXPECT(raze::algorithm::fold_right_last[opt::fstatic](sub, std::plus<>{}) == exp_sum);

            auto mix = [](T x, T acc) -> T {
                return static_cast<T>((static_cast<int>(acc) * 3 + static_cast<int>(x)) % 31);
            };
            const auto exp_mix = std::ranges::fold_right_last(sub, mix);
            RTTS_EXPECT(raze::algorithm::fold_right_last(sub, mix) == exp_mix);
            RTTS_EXPECT(raze::algorithm::fold_right_last[opt::fscalar](sub.begin(), sub.end(), mix) == exp_mix);

            if (it % 8 == 0 && (hi - lo) <= 64) {
                std::list<T> lst(sub.begin(), sub.end());
                RTTS_EXPECT(raze::algorithm::fold_right_last(lst, mix) == exp_mix);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::fold_right_last fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        fold_right_last_fuzz<T>(seed, 40);
};
