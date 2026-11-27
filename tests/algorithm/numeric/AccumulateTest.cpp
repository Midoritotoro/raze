#include <tests/rts/rts.h>
#include <raze/algorithm/numeric/Accumulate.h>

#include <array>
#include <list>
#include <numeric>
#include <ranges>
#include <string>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::accumulate", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32), size_t(128) }) {
        std::vector<T> v(sz);
        for (size_t i = 0; i < sz; ++i)
            v[i] = static_cast<T>(i & 1);

        const T exp = std::accumulate(v.begin(), v.end(), T(3));
        RTTS_EQUAL(raze::algorithm::accumulate(v, T(3)), exp);
        RTTS_EQUAL(raze::algorithm::accumulate(v.begin(), v.end(), T(3)), exp);
        RTTS_EQUAL(raze::algorithm::accumulate[opt::fscalar](v, T(3)), exp);
        RTTS_EQUAL(raze::algorithm::accumulate[opt::fstatic](v, T(3)), exp);
    }

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        return raze::algorithm::accumulate(a, T(0)) == T(10)
            && raze::algorithm::accumulate(a, T(1), std::multiplies<>{}) == T(24);
    }());
};

RTTS_CASE("raze::algorithm::accumulate custom op, single-pass input, and non-common") {
    const std::vector<int> v{ 1, 2, 3, 4 };
    RTTS_EQUAL(raze::algorithm::accumulate(v, 20, std::minus<>{}), 10);

    rtts::single_pass_input_range<int> sp(v);
    RTTS_EQUAL(raze::algorithm::accumulate(sp, 0), 10);
    RTTS_EQUAL(raze::algorithm::accumulate(rtts::make_non_common(v), 1, std::multiplies<>{}), 24);

    const std::list<std::string> words{ "a", "b", "c" };
    RTTS_EQUAL(raze::algorithm::accumulate(words, std::string("")), std::string("abc"));
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void accumulate_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);
        const size_t max_n = (sizeof(T) == 1) ? 40u : 256u;

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % (max_n + 1));
            std::vector<T> v(n);
            for (size_t i = 0; i < n; ++i) {
                if constexpr (sizeof(T) == 1) {
                    v[i] = static_cast<T>(rng() % 3u);
                }
                else {
                    const int val = static_cast<int>(rng() % 21u) - (std::is_signed_v<T> ? 10 : 0);
                    v[i] = static_cast<T>(val);
                }
            }

            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            auto sub = std::ranges::subrange(v.begin() + static_cast<std::ptrdiff_t>(lo),
                                             v.begin() + static_cast<std::ptrdiff_t>(hi));
            const T init = static_cast<T>(rng() % 21u);

            const T exp = std::accumulate(sub.begin(), sub.end(), init);
            RTTS_EQUAL(raze::algorithm::accumulate(sub, init), exp);
            RTTS_EQUAL(raze::algorithm::accumulate(sub.begin(), sub.end(), init), exp);
            RTTS_EQUAL(raze::algorithm::accumulate[opt::fscalar](sub, init), exp);
            RTTS_EQUAL(raze::algorithm::accumulate[opt::fstatic](sub, init), exp);

            auto mix = [](std::int64_t acc, T x) -> std::int64_t {
                return (acc * 3 + static_cast<std::int64_t>(x)) % 1000000007LL;
            };
            const std::int64_t init64 = static_cast<std::int64_t>(rng() % 101u) - 50;
            const auto exp_m = std::accumulate(sub.begin(), sub.end(), init64, mix);
            RTTS_EQUAL(raze::algorithm::accumulate(sub, init64, mix), exp_m);
            RTTS_EQUAL(raze::algorithm::accumulate[opt::fscalar](sub.begin(), sub.end(), init64, mix), exp_m);

            if (it % 8 == 0 && (hi - lo) <= 64) {
                std::list<T> lst(sub.begin(), sub.end());
                RTTS_EQUAL(raze::algorithm::accumulate(lst, init), exp);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::accumulate fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        accumulate_fuzz<T>(seed, 40);
};
