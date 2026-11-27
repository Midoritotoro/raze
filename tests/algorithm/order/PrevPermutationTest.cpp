#include <tests/rts/rts.h>
#include <raze/algorithm/order/PrevPermutation.h>

#include <algorithm>
#include <array>
#include <list>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::prev_permutation", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    {
        std::vector<T> a{ T(4), T(3), T(2), T(1) };
        auto b = a;
        int count = 0;
        for (;;) {
            ++count;
            auto ra = raze::algorithm::prev_permutation(a);
            auto rb = std::ranges::prev_permutation(b);
            RTTS_EXPECT(ra.in == a.end());
            RTTS_EQUAL(ra.found, rb.found);
            RTTS_ALL_EQUAL(a, b);
            if (!ra.found) break;
        }
        RTTS_EQUAL(count, 24);
    }

    {
        std::vector<T> a{ T(2), T(2), T(1), T(1) };
        auto b = a;
        int count = 0;
        for (;;) {
            ++count;
            auto ra = raze::algorithm::prev_permutation[opt::fscalar](a.begin(), a.end());
            auto rb = std::ranges::prev_permutation(b);
            RTTS_EQUAL(ra.found, rb.found);
            RTTS_ALL_EQUAL(a, b);
            if (!ra.found) break;
        }
        RTTS_EQUAL(count, 6);
    }

    static_assert([]() constexpr {
        std::array<T, 3> a{ T(1), T(3), T(2) };
        auto r1 = raze::algorithm::prev_permutation(a);
        std::array<T, 3> first{ T(1), T(2), T(3) };
        auto r2 = raze::algorithm::prev_permutation(first);
        std::array<T, 0> empty{};
        auto r3 = raze::algorithm::prev_permutation(empty);
        return r1.found && a == std::array<T, 3>{ T(1), T(2), T(3) }
            && !r2.found && first == std::array<T, 3>{ T(3), T(2), T(1) }
            && !r3.found;
    }());
};

RTTS_CASE("raze::algorithm::prev_permutation list, projections, non-common, and dangling") {
    std::list<int> lst{ 3, 2, 1 };
    auto r = raze::algorithm::prev_permutation(lst);
    RTTS_EXPECT(r.found && r.in == lst.end());
    RTTS_EXPECT((lst == std::list<int>{ 3, 1, 2 }));

    std::vector<rtts::test_item> items{ { 3, 30 }, { 2, 20 }, { 1, 10 } };
    auto rp = raze::algorithm::prev_permutation(items, std::less<>{}, &rtts::test_item::key);
    RTTS_EXPECT(rp.found && items[1].key == 1 && items[2].key == 2);

    std::vector<int> v{ 2, 1, 3 };
    auto rn = raze::algorithm::prev_permutation(rtts::make_non_common(v));
    RTTS_EXPECT(rn.found && rn.in == v.end() && (v == std::vector<int>{ 1, 3, 2 }));

    auto dr = raze::algorithm::prev_permutation(std::vector<int>{ 2, 1 });
    static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
    RTTS_EXPECT(dr.found);
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void prev_permutation_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 65);
            std::vector<T> base(n);
            if ((it & 1u) == 0u) {
                base = rtts::random::vector<T>(n, rng());
            }
            else {
                const unsigned alpha = (it % 4 == 1) ? 3u : 8u;
                for (size_t i = 0; i < n; ++i)
                    base[i] = static_cast<T>(rng() % alpha);
            }
            if (it % 5 == 0)
                std::ranges::sort(base);

            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            const bool desc = (it % 3 == 2);

            auto run_steps = [&](auto comp) {
                auto v1 = base, v2 = base, v3 = base, v4 = base, ve = base;
                for (int step = 0; step < 5; ++step) {
                    auto se = std::ranges::subrange(ve.begin() + static_cast<std::ptrdiff_t>(lo),
                                                    ve.begin() + static_cast<std::ptrdiff_t>(hi));
                    auto s1 = std::ranges::subrange(v1.begin() + static_cast<std::ptrdiff_t>(lo),
                                                    v1.begin() + static_cast<std::ptrdiff_t>(hi));
                    auto s2 = std::ranges::subrange(v2.begin() + static_cast<std::ptrdiff_t>(lo),
                                                    v2.begin() + static_cast<std::ptrdiff_t>(hi));
                    auto s3 = std::ranges::subrange(v3.begin() + static_cast<std::ptrdiff_t>(lo),
                                                    v3.begin() + static_cast<std::ptrdiff_t>(hi));
                    auto s4 = std::ranges::subrange(v4.begin() + static_cast<std::ptrdiff_t>(lo),
                                                    v4.begin() + static_cast<std::ptrdiff_t>(hi));

                    auto re = std::ranges::prev_permutation(se, comp);
                    auto r1 = raze::algorithm::prev_permutation(s1, comp);
                    auto r2 = raze::algorithm::prev_permutation(s2.begin(), s2.end(), comp);
                    auto r3 = raze::algorithm::prev_permutation[opt::fscalar](s3, comp);
                    auto r4 = raze::algorithm::prev_permutation[opt::fstatic](s4, comp);

                    RTTS_EXPECT(r1.in == s1.end() && r1.found == re.found);
                    RTTS_EXPECT(r2.in == s2.end() && r2.found == re.found);
                    RTTS_EXPECT(r3.in == s3.end() && r3.found == re.found);
                    RTTS_EXPECT(r4.in == s4.end() && r4.found == re.found);
                    RTTS_ALL_EQUAL(v1, ve);
                    RTTS_ALL_EQUAL(v2, ve);
                    RTTS_ALL_EQUAL(v3, ve);
                    RTTS_ALL_EQUAL(v4, ve);
                }

                if (it % 8 == 0 && (hi - lo) <= 32) {
                    std::list<T> lst(base.begin() + static_cast<std::ptrdiff_t>(lo),
                                     base.begin() + static_cast<std::ptrdiff_t>(hi));
                    std::list<T> lst_exp = lst;
                    auto rl = raze::algorithm::prev_permutation(lst, comp);
                    auto rle = std::ranges::prev_permutation(lst_exp, comp);
                    RTTS_EXPECT(rl.in == lst.end() && rl.found == rle.found);
                    RTTS_ALL_EQUAL(lst, lst_exp);
                }
            };

            if (desc) run_steps(std::greater<>{});
            else      run_steps(std::less<>{});
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::prev_permutation fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        prev_permutation_fuzz<T>(seed, 35);
};
