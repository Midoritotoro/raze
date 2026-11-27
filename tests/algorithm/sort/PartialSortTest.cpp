#include <tests/rts/rts.h>
#include <raze/algorithm/sort/PartialSort.h>

#include <algorithm>
#include <array>
#include <deque>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::partial_sort", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(7), size_t(31), size_t(128) }) {
        const auto base = rtts::random::vector<T>(sz, 820u + static_cast<unsigned>(sz));

        for (size_t k : { size_t(0), size_t(1), sz / 3, sz / 2, sz }) {
            if (k > sz) continue;
            auto exp = base;
            std::ranges::partial_sort(exp, exp.begin() + static_cast<std::ptrdiff_t>(k));

            auto check = [&](auto fn) {
                auto v = base;
                auto end = fn(v, v.begin() + static_cast<std::ptrdiff_t>(k));
                RTTS_EXPECT(end == v.end());
                RTTS_EXPECT(std::ranges::equal(v.begin(), v.begin() + static_cast<std::ptrdiff_t>(k),
                    exp.begin(), exp.begin() + static_cast<std::ptrdiff_t>(k)));
                RTTS_EXPECT(std::ranges::is_permutation(v, base));
            };

            check([](auto& v, auto m) { return raze::algorithm::partial_sort(v, m); });
            check([](auto& v, auto m) { return raze::algorithm::partial_sort(v.begin(), m, v.end()); });
            check([](auto& v, auto m) { return raze::algorithm::partial_sort[opt::fscalar](v, m); });
            check([](auto& v, auto m) { return raze::algorithm::partial_sort[opt::fstatic](v, m); });
        }
    }

    static_assert([]() constexpr {
        std::array<T, 7> a{ T(7), T(2), T(9), T(1), T(5), T(3), T(8) };
        auto end = raze::algorithm::partial_sort(a, a.begin() + 3);
        return end == a.end() && a[0] == T(1) && a[1] == T(2) && a[2] == T(3);
    }());
};

RTTS_CASE("raze::algorithm::partial_sort comparator, projections, non-common, and dangling") {
    std::vector<rtts::test_item> v{ { 5, 1 }, { 1, 2 }, { 9, 3 }, { 2, 4 }, { 7, 5 } };
    raze::algorithm::partial_sort(v, v.begin() + 3, std::greater<>{}, &rtts::test_item::key);
    RTTS_EXPECT(v[0].key == 9 && v[1].key == 7 && v[2].key == 5);

    std::deque<int> dq{ 8, 3, 6, 1, 9, 2 };
    auto ni = raze::algorithm::partial_sort(rtts::make_non_common(dq), dq.begin() + 3);
    RTTS_EXPECT(ni == dq.end() && dq[0] == 1 && dq[1] == 2 && dq[2] == 3);

    std::vector<int> tmp{ 3, 1, 2 };
    auto m = tmp.begin() + 2;
    auto dr = raze::algorithm::partial_sort(std::move(tmp), m);
    static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void partial_sort_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 257);
            std::vector<T> base(n);
            if ((it & 1u) == 0u) {
                base = rtts::random::vector<T>(n, rng());
            }
            else {
                for (size_t i = 0; i < n; ++i)
                    base[i] = static_cast<T>(rng() % 8u);
            }

            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            const size_t len = hi - lo;
            const size_t k = len ? static_cast<size_t>(rng() % (len + 1)) : 0;
            const bool desc = (it % 3 == 2);

            auto run_check = [&](auto comp) {
                std::vector<T> sorted_sub(base.begin() + static_cast<std::ptrdiff_t>(lo),
                                          base.begin() + static_cast<std::ptrdiff_t>(hi));
                std::ranges::sort(sorted_sub, comp);

                auto verify = [&](auto fn) {
                    auto v = base;
                    auto sub = std::ranges::subrange(v.begin() + static_cast<std::ptrdiff_t>(lo),
                                                     v.begin() + static_cast<std::ptrdiff_t>(hi));
                    auto mid = sub.begin() + static_cast<std::ptrdiff_t>(k);
                    auto end = fn(sub, mid);
                    RTTS_EXPECT(end == sub.end());
                    RTTS_EXPECT(std::equal(sub.begin(), mid, sorted_sub.begin()));
                    RTTS_EXPECT(std::equal(v.begin(), v.begin() + static_cast<std::ptrdiff_t>(lo), base.begin()));
                    RTTS_EXPECT(std::equal(v.begin() + static_cast<std::ptrdiff_t>(hi), v.end(),
                                           base.begin() + static_cast<std::ptrdiff_t>(hi)));
                    std::vector<T> got_sorted(sub.begin(), sub.end());
                    std::ranges::sort(got_sorted, comp);
                    RTTS_ALL_EQUAL(got_sorted, sorted_sub);
                };

                verify([&](auto sub, auto m) { return raze::algorithm::partial_sort(sub, m, comp); });
                verify([&](auto sub, auto m) { return raze::algorithm::partial_sort(sub.begin(), m, sub.end(), comp); });
                verify([&](auto sub, auto m) { return raze::algorithm::partial_sort[opt::fscalar](sub, m, comp); });
                verify([&](auto sub, auto m) { return raze::algorithm::partial_sort[opt::fstatic](sub, m, comp); });

                if (it % 8 == 0 && len <= 64) {
                    std::deque<T> dq(base.begin() + static_cast<std::ptrdiff_t>(lo),
                                     base.begin() + static_cast<std::ptrdiff_t>(hi));
                    auto end = raze::algorithm::partial_sort(dq, dq.begin() + static_cast<std::ptrdiff_t>(k), comp);
                    RTTS_EXPECT(end == dq.end());
                    RTTS_EXPECT(std::equal(dq.begin(), dq.begin() + static_cast<std::ptrdiff_t>(k), sorted_sub.begin()));
                }
            };

            if (desc) run_check(std::greater<>{});
            else      run_check(std::less<>{});
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::partial_sort fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        partial_sort_fuzz<T>(seed, 35);
};
