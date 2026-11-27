#include <tests/rts/rts.h>
#include <raze/algorithm/sort/NthElement.h>

#include <algorithm>
#include <array>
#include <deque>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::nth_element", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(3), size_t(7), size_t(31), size_t(128) }) {
        const auto base = rtts::random::vector<T>(sz, 840u + static_cast<unsigned>(sz));
        auto sorted = base;
        std::ranges::sort(sorted);

        for (size_t k : { size_t(0), sz / 4, sz / 2, sz > 0 ? sz - 1 : size_t(0), sz }) {
            auto check = [&](auto fn) {
                auto v = base;
                auto end = fn(v, v.begin() + static_cast<std::ptrdiff_t>(k));
                RTTS_EXPECT(end == v.end());
                RTTS_EXPECT(std::ranges::is_permutation(v, base));
                if (k < sz) {
                    RTTS_EQUAL(v[k], sorted[k]);
                    for (size_t i = 0; i < k; ++i) RTTS_EXPECT(!(v[k] < v[i]));
                    for (size_t i = k + 1; i < sz; ++i) RTTS_EXPECT(!(v[i] < v[k]));
                }
            };

            check([](auto& v, auto nth) { return raze::algorithm::nth_element(v, nth); });
            check([](auto& v, auto nth) { return raze::algorithm::nth_element(v.begin(), nth, v.end()); });
            check([](auto& v, auto nth) { return raze::algorithm::nth_element[opt::fscalar](v, nth); });
            check([](auto& v, auto nth) { return raze::algorithm::nth_element[opt::fstatic](v, nth); });
        }
    }

    static_assert([]() constexpr {
        std::array<T, 7> a{ T(7), T(2), T(9), T(1), T(5), T(3), T(8) };
        auto end = raze::algorithm::nth_element(a, a.begin() + 3);
        return end == a.end() && a[3] == T(5);
    }());
};

RTTS_CASE("raze::algorithm::nth_element comparator, projections, deque, and dangling") {
    std::vector<rtts::test_item> v{ { 5, 1 }, { 1, 2 }, { 9, 3 }, { 2, 4 }, { 7, 5 } };
    raze::algorithm::nth_element(v, v.begin() + 1, std::greater<>{}, &rtts::test_item::key);
    RTTS_EQUAL(v[1].key, 7);

    std::deque<int> dq{ 9, 4, 7, 1, 8, 2, 5 };
    auto ni = raze::algorithm::nth_element(rtts::make_non_common(dq), dq.begin() + 3);
    RTTS_EXPECT(ni == dq.end() && dq[3] == 5);

    std::vector<int> tmp{ 3, 1, 2 };
    auto nth = tmp.begin() + 1;
    auto dr = raze::algorithm::nth_element(std::move(tmp), nth);
    static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void nth_element_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 257);
            std::vector<T> base(n);
            const unsigned shape = static_cast<unsigned>(it % 4);
            if (shape == 0u) {
                base = rtts::random::vector<T>(n, rng());
            }
            else if (shape == 1u) {
                for (size_t i = 0; i < n; ++i) base[i] = static_cast<T>(rng() % 6u);
            }
            else if (shape == 2u) {
                base = rtts::random::vector<T>(n, rng());
                std::ranges::sort(base);
            }
            else {
                base = rtts::random::vector<T>(n, rng());
                std::ranges::sort(base, std::greater<>{});
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
                    auto nth = sub.begin() + static_cast<std::ptrdiff_t>(k);
                    auto end = fn(sub, nth);
                    RTTS_EXPECT(end == sub.end());
                    RTTS_EXPECT(std::equal(v.begin(), v.begin() + static_cast<std::ptrdiff_t>(lo), base.begin()));
                    RTTS_EXPECT(std::equal(v.begin() + static_cast<std::ptrdiff_t>(hi), v.end(),
                                           base.begin() + static_cast<std::ptrdiff_t>(hi)));
                    if (k < len) {
                        const T pivot = v[lo + k];
                        RTTS_EQUAL(pivot, sorted_sub[k]);
                        for (size_t i = 0; i < k; ++i)
                            RTTS_EXPECT(!comp(pivot, v[lo + i]));
                        for (size_t i = k + 1; i < len; ++i)
                            RTTS_EXPECT(!comp(v[lo + i], pivot));
                    }
                    std::vector<T> got_sorted(sub.begin(), sub.end());
                    std::ranges::sort(got_sorted, comp);
                    RTTS_ALL_EQUAL(got_sorted, sorted_sub);
                };

                verify([&](auto sub, auto nth) { return raze::algorithm::nth_element(sub, nth, comp); });
                verify([&](auto sub, auto nth) { return raze::algorithm::nth_element(sub.begin(), nth, sub.end(), comp); });
                verify([&](auto sub, auto nth) { return raze::algorithm::nth_element[opt::fscalar](sub, nth, comp); });
                verify([&](auto sub, auto nth) { return raze::algorithm::nth_element[opt::fstatic](sub, nth, comp); });

                if (it % 8 == 0 && len <= 64) {
                    std::deque<T> dq(base.begin() + static_cast<std::ptrdiff_t>(lo),
                                     base.begin() + static_cast<std::ptrdiff_t>(hi));
                    auto end = raze::algorithm::nth_element(dq, dq.begin() + static_cast<std::ptrdiff_t>(k), comp);
                    RTTS_EXPECT(end == dq.end());
                    if (k < len) RTTS_EQUAL(dq[k], sorted_sub[k]);
                }
            };

            if (desc) run_check(std::greater<>{});
            else      run_check(std::less<>{});
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::nth_element fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        nth_element_fuzz<T>(seed, 35);
};
