#include <tests/rts/rts.h>
#include <raze/algorithm/order/IsPermutation.h>

#include <algorithm>
#include <array>
#include <forward_list>
#include <list>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::is_permutation", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(7), size_t(16), size_t(64) }) {
        auto a = rtts::random::vector<T>(sz, 200u + static_cast<unsigned>(sz));
        auto b = a;
        std::mt19937 rng(42u + static_cast<unsigned>(sz));
        std::ranges::shuffle(b, rng);

        RTTS_EXPECT(raze::algorithm::is_permutation(a, b));
        RTTS_EXPECT(raze::algorithm::is_permutation(a.begin(), a.end(), b.begin(), b.end()));
        RTTS_EXPECT(raze::algorithm::is_permutation[opt::fscalar](a, b));
        RTTS_EXPECT(raze::algorithm::is_permutation[opt::fstatic](a, b));

        if (sz > 0) {
            b[sz / 2] = static_cast<T>(b[sz / 2] + T(1));
            RTTS_EQUAL(raze::algorithm::is_permutation(a, b), std::ranges::is_permutation(a, b));
        }

        auto shorter = a;
        if (!shorter.empty()) {
            shorter.pop_back();
            RTTS_EXPECT(!raze::algorithm::is_permutation(a, shorter));
        }
    }

    static_assert([]() constexpr {
        std::array<T, 5> a{ T(1), T(2), T(2), T(3), T(4) };
        std::array<T, 5> b{ T(4), T(2), T(1), T(3), T(2) };
        std::array<T, 5> c{ T(4), T(2), T(1), T(3), T(3) };
        std::array<T, 0> e{};
        return raze::algorithm::is_permutation(a, b)
            && !raze::algorithm::is_permutation(a, c)
            && raze::algorithm::is_permutation(e, e);
    }());
};

RTTS_CASE("raze::algorithm::is_permutation projections, forward_list, and non-common") {
    const std::vector<rtts::test_item> a{ { 1, 10 }, { 2, 20 }, { 2, 30 }, { 3, 40 } };
    const std::vector<rtts::test_item> b{ { 2, 99 }, { 3, 88 }, { 1, 77 }, { 2, 66 } };
    RTTS_EXPECT(raze::algorithm::is_permutation(
        a, b, std::equal_to<>{}, &rtts::test_item::key, &rtts::test_item::key));

    std::forward_list<int> f1{ 3, 1, 4, 1, 5 };
    std::list<int> l2{ 5, 1, 3, 4, 1 };
    RTTS_EXPECT(raze::algorithm::is_permutation(f1, l2));
    RTTS_EXPECT(raze::algorithm::is_permutation(rtts::make_non_common(f1), rtts::make_non_common(l2)));
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void is_permutation_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);
        rtts::random::generator<T> val_gen(seed ^ 0x5A5A5A5Au);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 65);
            std::vector<T> a(n);
            if ((it & 1u) == 0u) {
                a = rtts::random::vector<T>(n, rng());
            }
            else {
                for (size_t i = 0; i < n; ++i)
                    a[i] = static_cast<T>(rng() % 8u);
            }

            std::vector<T> b = a;
            std::ranges::shuffle(b, rng);

            const unsigned mode = static_cast<unsigned>(it % 4);
            if (mode == 1u && n > 0) {
                b[static_cast<size_t>(rng() % n)] = val_gen();
            }
            else if (mode == 2u && n > 0) {
                if (rng() & 1u) b.pop_back();
                else b.push_back(val_gen());
            }
            else if (mode == 3u) {
                const size_t n2 = static_cast<size_t>(rng() % 65);
                b.resize(n2);
                for (size_t i = 0; i < n2; ++i)
                    b[i] = static_cast<T>(rng() % 8u);
            }

            const bool exp = std::ranges::is_permutation(a, b);
            RTTS_EQUAL(raze::algorithm::is_permutation(a, b), exp);
            RTTS_EQUAL(raze::algorithm::is_permutation(a.begin(), a.end(), b.begin(), b.end()), exp);
            RTTS_EQUAL(raze::algorithm::is_permutation[opt::fscalar](a, b), exp);
            RTTS_EQUAL(raze::algorithm::is_permutation[opt::fstatic](a, b), exp);

            if (it % 8 == 0 && n <= 32) {
                std::forward_list<T> fa(a.begin(), a.end());
                std::list<T> lb(b.begin(), b.end());
                RTTS_EQUAL(raze::algorithm::is_permutation(fa, lb), exp);
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::is_permutation fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        is_permutation_fuzz<T>(seed, 40);
};
