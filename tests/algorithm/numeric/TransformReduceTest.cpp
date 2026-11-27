#include <tests/rts/rts.h>
#include <raze/algorithm/numeric/TransformReduce.h>

#include <array>
#include <list>
#include <numeric>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::transform_reduce", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32), size_t(100) }) {
        std::vector<T> a(sz), b(sz);
        for (size_t i = 0; i < sz; ++i) {
            a[i] = static_cast<T>(i & 1);
            b[i] = static_cast<T>((i + 1) & 1);
        }

        auto sq = [](T x) { return static_cast<T>(x * x); };
        const T exp_u = std::transform_reduce(a.begin(), a.end(), T(2), std::plus<>{}, sq);
        RTTS_EQUAL(raze::algorithm::transform_reduce(a, T(2), std::plus<>{}, sq), exp_u);
        RTTS_EQUAL(raze::algorithm::transform_reduce[opt::fscalar](a.begin(), a.end(), T(2), std::plus<>{}, sq), exp_u);

        const T exp_b1 = std::transform_reduce(a.begin(), a.end(), b.begin(), T(3));
        const T exp_b2 = std::transform_reduce(a.begin(), a.end(), b.begin(), T(3), std::plus<>{}, std::plus<>{});
        RTTS_EQUAL(raze::algorithm::transform_reduce(a, b, T(3)), exp_b1);
        RTTS_EQUAL(raze::algorithm::transform_reduce(a.begin(), a.end(), b.begin(), T(3)), exp_b1);
        RTTS_EQUAL(raze::algorithm::transform_reduce[opt::fscalar](a, b, T(3), std::plus<>{}, std::plus<>{}), exp_b2);
    }

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        std::array<T, 4> b{ T(2), T(3), T(4), T(5) };
        return raze::algorithm::transform_reduce(a, b, T(0)) == T(40)
            && raze::algorithm::transform_reduce(a, T(0), std::plus<>{}, [](T x) constexpr { return static_cast<T>(x * T(2)); }) == T(20);
    }());
};

RTTS_CASE("raze::algorithm::transform_reduce single-pass input and non-common sentinel") {
    const std::vector<int> a{ 1, 2, 3, 4 };
    const std::vector<int> b{ 10, 20, 30, 40 };
    rtts::single_pass_input_range<int> sp1(a);
    rtts::single_pass_input_range<int> sp2(b);
    RTTS_EQUAL(raze::algorithm::transform_reduce(sp1, sp2, 0), 300);

    RTTS_EQUAL(raze::algorithm::transform_reduce(
        rtts::make_non_common(a), 0, std::plus<>{}, [](int x) { return x * x; }), 30);
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void transform_reduce_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);
        const size_t max_n = (sizeof(T) == 1) ? 25u : 256u;
        auto sq = [](T x) { return static_cast<T>(x * x); };

        for (size_t it = 0; it < iters; ++it) {
            const size_t n1 = static_cast<size_t>(rng() % (max_n + 1));
            const size_t n2 = (it % 3 == 0) ? n1 : static_cast<size_t>(rng() % (max_n + 1));
            const size_t len = (n1 < n2) ? n1 : n2;

            std::vector<T> a(n1), b(n2);
            for (size_t i = 0; i < n1; ++i) {
                if constexpr (sizeof(T) == 1) a[i] = static_cast<T>(rng() % 3u);
                else a[i] = static_cast<T>(static_cast<int>(rng() % 11u) - (std::is_signed_v<T> ? 5 : 0));
            }
            for (size_t i = 0; i < n2; ++i) {
                if constexpr (sizeof(T) == 1) b[i] = static_cast<T>(rng() % 3u);
                else b[i] = static_cast<T>(static_cast<int>(rng() % 11u) - (std::is_signed_v<T> ? 5 : 0));
            }

            const T init = static_cast<T>(rng() % 21u);

            const T exp_u = std::transform_reduce(a.begin(), a.end(), init, std::plus<>{}, sq);
            RTTS_EQUAL(raze::algorithm::transform_reduce(a, init, std::plus<>{}, sq), exp_u);
            RTTS_EQUAL(raze::algorithm::transform_reduce(a.begin(), a.end(), init, std::plus<>{}, sq), exp_u);
            RTTS_EQUAL(raze::algorithm::transform_reduce[opt::fscalar](a, init, std::plus<>{}, sq), exp_u);
            RTTS_EQUAL(raze::algorithm::transform_reduce[opt::fstatic](a, init, std::plus<>{}, sq), exp_u);

            const T exp_b1 = std::transform_reduce(a.begin(), a.begin() + static_cast<std::ptrdiff_t>(len), b.begin(), init);
            const T exp_b2 = std::transform_reduce(a.begin(), a.begin() + static_cast<std::ptrdiff_t>(len), b.begin(), init,
                                                   std::plus<>{}, std::plus<>{});
            RTTS_EQUAL(raze::algorithm::transform_reduce(a, b, init), exp_b1);
            RTTS_EQUAL(raze::algorithm::transform_reduce(a.begin(), a.begin() + static_cast<std::ptrdiff_t>(len), b.begin(), init), exp_b1);
            RTTS_EQUAL(raze::algorithm::transform_reduce[opt::fscalar](a, b, init, std::plus<>{}, std::plus<>{}), exp_b2);
            RTTS_EQUAL(raze::algorithm::transform_reduce[opt::fstatic](a, b, init), exp_b1);

            if (it % 8 == 0 && n1 <= 64 && n2 <= 64) {
                std::list<T> la(a.begin(), a.end());
                std::list<T> lb(b.begin(), b.end());
                RTTS_EQUAL(raze::algorithm::transform_reduce(la, lb, init), exp_b1);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::transform_reduce fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        transform_reduce_fuzz<T>(seed, 40);
};
