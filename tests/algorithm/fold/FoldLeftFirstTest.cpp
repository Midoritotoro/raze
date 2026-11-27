#include <tests/rts/rts.h>
#include <raze/algorithm/fold/FoldLeftFirst.h>

#include <algorithm>
#include <array>
#include <ranges>
#include <vector>
#include <list>
#include <random>

RTTS_CASE_TPL("raze::algorithm::fold_left_first", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32) }) {
        std::vector<T> v(sz);
        for (size_t i = 0; i < sz; ++i)
            v[i] = static_cast<T>((i & 1) ? 0 : 1);

        const auto exp = std::ranges::fold_left_first(v, std::plus<>{});
        RTTS_EXPECT(raze::algorithm::fold_left_first(v, std::plus<>{}) == exp);
        RTTS_EXPECT(raze::algorithm::fold_left_first[opt::fscalar](v.begin(), v.end(), std::plus<>{}) == exp);
    }

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        std::array<T, 0> e{};
        return raze::algorithm::fold_left_first(a, std::plus<>{}) == T(10)
            && !raze::algorithm::fold_left_first(e, std::plus<>{}).has_value();
    }());
};

RTTS_CASE("raze::algorithm::fold_left_first order and single-pass") {
    const std::vector<int> v{ 20, 5, 3 };
    RTTS_EXPECT(raze::algorithm::fold_left_first(v, std::minus<>{}) == 12);

    rtts::single_pass_input_range<int> sp(v);
    RTTS_EXPECT(raze::algorithm::fold_left_first(sp, std::minus<>{}) == 12);
    RTTS_EXPECT(raze::algorithm::fold_left_first(rtts::make_non_common(v), std::minus<>{}) == 12);
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void fold_left_first_fuzz(unsigned seed, size_t iters) {
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

            const auto exp_sum = std::ranges::fold_left_first(sub, std::plus<>{});
            RTTS_EXPECT(raze::algorithm::fold_left_first(sub, std::plus<>{}) == exp_sum);
            RTTS_EXPECT(raze::algorithm::fold_left_first(sub.begin(), sub.end(), std::plus<>{}) == exp_sum);
            RTTS_EXPECT(raze::algorithm::fold_left_first[opt::fscalar](sub, std::plus<>{}) == exp_sum);
            RTTS_EXPECT(raze::algorithm::fold_left_first[opt::fstatic](sub, std::plus<>{}) == exp_sum);

            auto mix = [](T acc, T x) -> T {
                return static_cast<T>((static_cast<int>(acc) * 3 + static_cast<int>(x)) % 31);
            };
            const auto exp_mix = std::ranges::fold_left_first(sub, mix);
            RTTS_EXPECT(raze::algorithm::fold_left_first(sub, mix) == exp_mix);
            RTTS_EXPECT(raze::algorithm::fold_left_first[opt::fscalar](sub.begin(), sub.end(), mix) == exp_mix);

            if (it % 8 == 0 && (hi - lo) <= 64) {
                std::list<T> lst(sub.begin(), sub.end());
                RTTS_EXPECT(raze::algorithm::fold_left_first(lst, mix) == exp_mix);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::fold_left_first fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        fold_left_first_fuzz<T>(seed, 40);
};
