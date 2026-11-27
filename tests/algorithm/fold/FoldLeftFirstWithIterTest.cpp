#include <tests/rts/rts.h>
#include <raze/algorithm/fold/FoldLeftFirstWithIter.h>

#include <algorithm>
#include <array>
#include <ranges>
#include <vector>
#include <list>
#include <random>

RTTS_CASE_TPL("raze::algorithm::fold_left_first_with_iter", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32) }) {
        std::vector<T> v(sz);
        for (size_t i = 0; i < sz; ++i)
            v[i] = static_cast<T>((i & 1) ? 0 : 1);

        const auto exp = std::ranges::fold_left_first_with_iter(v, std::plus<>{});
        auto r1 = raze::algorithm::fold_left_first_with_iter(v, std::plus<>{});
        auto r2 = raze::algorithm::fold_left_first_with_iter[opt::fscalar](v.begin(), v.end(), std::plus<>{});

        RTTS_EXPECT(r1.in == v.end() && r1.value == exp.value);
        RTTS_EXPECT(r2.in == v.end() && r2.value == exp.value);
    }

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        std::array<T, 0> e{};
        auto r1 = raze::algorithm::fold_left_first_with_iter(a, std::plus<>{});
        auto r2 = raze::algorithm::fold_left_first_with_iter(e, std::plus<>{});
        return r1.in == a.end() && r1.value.has_value() && *r1.value == T(10)
            && r2.in == e.end() && !r2.value.has_value();
    }());
};

RTTS_CASE("raze::algorithm::fold_left_first_with_iter order, single-pass, and dangling") {
    const std::vector<int> v{ 20, 5, 3 };
    auto r = raze::algorithm::fold_left_first_with_iter(v, std::minus<>{});
    RTTS_EXPECT(r.in == v.end() && r.value == 12);

    rtts::single_pass_input_range<int> sp(v);
    auto rs = raze::algorithm::fold_left_first_with_iter(sp, std::minus<>{});
    RTTS_EXPECT(rs.in == sp.end() && rs.value == 12);

    auto dr = raze::algorithm::fold_left_first_with_iter(std::vector<int>{ 1, 2, 3 }, std::plus<>{});
    static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
    RTTS_EXPECT(dr.value == 6);
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void fold_left_first_with_iter_fuzz(unsigned seed, size_t iters) {
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

            const auto exp = std::ranges::fold_left_first_with_iter(sub, std::plus<>{});
            auto r1 = raze::algorithm::fold_left_first_with_iter(sub, std::plus<>{});
            auto r2 = raze::algorithm::fold_left_first_with_iter(sub.begin(), sub.end(), std::plus<>{});
            auto r3 = raze::algorithm::fold_left_first_with_iter[opt::fscalar](sub, std::plus<>{});
            auto r4 = raze::algorithm::fold_left_first_with_iter[opt::fstatic](sub, std::plus<>{});

            RTTS_EXPECT(r1.in == exp.in && r1.value == exp.value);
            RTTS_EXPECT(r2.in == exp.in && r2.value == exp.value);
            RTTS_EXPECT(r3.in == exp.in && r3.value == exp.value);
            RTTS_EXPECT(r4.in == exp.in && r4.value == exp.value);

            auto mix = [](T acc, T x) -> T {
                return static_cast<T>((static_cast<int>(acc) * 3 + static_cast<int>(x)) % 31);
            };
            const auto exp_m = std::ranges::fold_left_first_with_iter(sub, mix);
            auto rm = raze::algorithm::fold_left_first_with_iter(sub, mix);
            RTTS_EXPECT(rm.in == exp_m.in && rm.value == exp_m.value);

            if (it % 8 == 0 && (hi - lo) <= 64) {
                std::list<T> lst(sub.begin(), sub.end());
                auto rl = raze::algorithm::fold_left_first_with_iter(lst, mix);
                RTTS_EXPECT(rl.in == lst.end() && rl.value == exp_m.value);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::fold_left_first_with_iter fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        fold_left_first_with_iter_fuzz<T>(seed, 40);
};
