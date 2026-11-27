#include <tests/rts/rts.h>
#include <raze/algorithm/numeric/Reduce.h>

#include <array>
#include <list>
#include <numeric>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::reduce", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32), size_t(128) }) {
        std::vector<T> v(sz);
        for (size_t i = 0; i < sz; ++i)
            v[i] = static_cast<T>(i & 1);

        const T exp0 = std::reduce(v.begin(), v.end());
        const T exp5 = std::reduce(v.begin(), v.end(), T(5));

        RTTS_EQUAL(raze::algorithm::reduce(v), exp0);
        RTTS_EQUAL(raze::algorithm::reduce(v.begin(), v.end()), exp0);
        RTTS_EQUAL(raze::algorithm::reduce(v, T(5)), exp5);
        RTTS_EQUAL(raze::algorithm::reduce[opt::fscalar](v, T(5)), exp5);
        RTTS_EQUAL(raze::algorithm::reduce[opt::fstatic](v, T(5), std::plus<>{}), exp5);
    }

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        std::array<T, 0> e{};
        return raze::algorithm::reduce(a) == T(10)
            && raze::algorithm::reduce(e) == T(0)
            && raze::algorithm::reduce(a, T(1), std::multiplies<>{}) == T(24);
    }());
};

RTTS_CASE("raze::algorithm::reduce single-pass input and non-common sentinel") {
    const std::vector<int> v{ 2, 3, 5, 7 };
    rtts::single_pass_input_range<int> sp(v);
    RTTS_EQUAL(raze::algorithm::reduce(sp, 1, std::multiplies<>{}), 210);
    RTTS_EQUAL(raze::algorithm::reduce(rtts::make_non_common(v)), 17);
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void reduce_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);
        const size_t max_n = (sizeof(T) == 1) ? 40u : 256u;

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % (max_n + 1));
            std::vector<T> v(n);
            for (size_t i = 0; i < n; ++i) {
                if constexpr (sizeof(T) == 1)
                    v[i] = static_cast<T>(rng() % 3u);
                else
                    v[i] = static_cast<T>(static_cast<int>(rng() % 21u) - (std::is_signed_v<T> ? 10 : 0));
            }

            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            auto sub = std::ranges::subrange(v.begin() + static_cast<std::ptrdiff_t>(lo),
                                             v.begin() + static_cast<std::ptrdiff_t>(hi));
            const T init = static_cast<T>(rng() % 21u);

            const T exp0 = std::reduce(sub.begin(), sub.end());
            const T expi = std::reduce(sub.begin(), sub.end(), init);

            RTTS_EQUAL(raze::algorithm::reduce(sub), exp0);
            RTTS_EQUAL(raze::algorithm::reduce(sub.begin(), sub.end()), exp0);
            RTTS_EQUAL(raze::algorithm::reduce(sub, init), expi);
            RTTS_EQUAL(raze::algorithm::reduce[opt::fscalar](sub, init), expi);
            RTTS_EQUAL(raze::algorithm::reduce[opt::fstatic](sub, init, std::plus<>{}), expi);

            if (it % 8 == 0 && (hi - lo) <= 64) {
                std::list<T> lst(sub.begin(), sub.end());
                RTTS_EQUAL(raze::algorithm::reduce(lst, init), expi);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::reduce fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        reduce_fuzz<T>(seed, 40);
};
