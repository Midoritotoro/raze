#include <tests/rts/rts.h>
#include <raze/algorithm/numeric/InnerProduct.h>

#include <array>
#include <list>
#include <numeric>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::inner_product", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32), size_t(100) }) {
        std::vector<T> a(sz), b(sz);
        for (size_t i = 0; i < sz; ++i) {
            a[i] = static_cast<T>(i & 1);
            b[i] = static_cast<T>(1);
        }

        const T exp1 = std::inner_product(a.begin(), a.end(), b.begin(), T(4));
        const T exp2 = std::inner_product(a.begin(), a.end(), b.begin(), T(4), std::plus<>{}, std::plus<>{});

        RTTS_EQUAL(raze::algorithm::inner_product(a, b, T(4)), exp1);
        RTTS_EQUAL(raze::algorithm::inner_product(a.begin(), a.end(), b.begin(), T(4)), exp1);
        RTTS_EQUAL(raze::algorithm::inner_product[opt::fscalar](a, b, T(4)), exp1);
        RTTS_EQUAL(raze::algorithm::inner_product[opt::fstatic](a, b, T(4), std::plus<>{}, std::plus<>{}), exp2);
    }

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        std::array<T, 4> b{ T(5), T(6), T(7), T(8) };
        return raze::algorithm::inner_product(a, b, T(0)) == T(70);
    }());
};

RTTS_CASE("raze::algorithm::inner_product custom ops, single-pass, and mismatched lengths") {
    const std::vector<int> a{ 1, 2, 3, 4, 5 };
    const std::vector<int> b{ 10, 20, 30 };
    RTTS_EQUAL(raze::algorithm::inner_product(a, b, 0), 140);

    rtts::single_pass_input_range<int> sp1(a);
    rtts::single_pass_input_range<int> sp2(b);
    RTTS_EQUAL(raze::algorithm::inner_product(sp1, sp2, 0), 140);

    RTTS_EQUAL(raze::algorithm::inner_product(
        rtts::make_non_common(b), rtts::make_non_common(b), 0, std::plus<>{}, std::equal_to<>{}), 3);
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void inner_product_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);
        const size_t max_n = (sizeof(T) == 1) ? 25u : 256u;

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
            const T exp1 = std::inner_product(a.begin(), a.begin() + static_cast<std::ptrdiff_t>(len), b.begin(), init);
            const T exp2 = std::inner_product(a.begin(), a.begin() + static_cast<std::ptrdiff_t>(len), b.begin(), init,
                                              std::plus<>{}, std::plus<>{});

            RTTS_EQUAL(raze::algorithm::inner_product(a, b, init), exp1);
            RTTS_EQUAL(raze::algorithm::inner_product(a.begin(), a.begin() + static_cast<std::ptrdiff_t>(len), b.begin(), init), exp1);
            RTTS_EQUAL(raze::algorithm::inner_product[opt::fscalar](a, b, init), exp1);
            RTTS_EQUAL(raze::algorithm::inner_product[opt::fstatic](a, b, init, std::plus<>{}, std::plus<>{}), exp2);

            if (it % 8 == 0 && n1 <= 64 && n2 <= 64) {
                std::list<T> la(a.begin(), a.end());
                std::list<T> lb(b.begin(), b.end());
                RTTS_EQUAL(raze::algorithm::inner_product(la, lb, init), exp1);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::inner_product fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        inner_product_fuzz<T>(seed, 40);
};
