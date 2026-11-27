#include <tests/rts/rts.h>
#include <raze/algorithm/sort/Sort.h>

#include <algorithm>
#include <array>
#include <deque>
#include <memory>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::sort", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(3), size_t(15), size_t(16), size_t(17), size_t(64), size_t(300) }) {
        const auto base = rtts::random::vector<T>(sz, 800u + static_cast<unsigned>(sz));
        auto exp = base;
        std::ranges::sort(exp);

        auto check = [&](auto fn) {
            auto v = base;
            auto end = fn(v);
            RTTS_EXPECT(end == v.end());
            RTTS_ALL_EQUAL(v, exp);
        };

        check([](auto& v) { return raze::algorithm::sort(v); });
        check([](auto& v) { return raze::algorithm::sort(v.begin(), v.end()); });
        check([](auto& v) { return raze::algorithm::sort[opt::fscalar](v); });
        check([](auto& v) { return raze::algorithm::sort[opt::fstatic](v); });
    }

    for (size_t sz : { size_t(64), size_t(257) }) {
        std::vector<T> rev(sz), same(sz, T(7)), pipe(sz);
        for (size_t i = 0; i < sz; ++i) {
            rev[i] = static_cast<T>((sz - i) % 100);
            pipe[i] = static_cast<T>((i < sz / 2 ? i : sz - i) % 100);
        }
        for (const auto& src : { rev, same, pipe }) {
            auto a = src, b = src;
            raze::algorithm::sort(a);
            std::ranges::sort(b);
            RTTS_ALL_EQUAL(a, b);
        }
    }

    static_assert([]() constexpr {
        std::array<T, 10> a{ T(9), T(1), T(5), T(3), T(8), T(2), T(7), T(4), T(6), T(0) };
        auto end = raze::algorithm::sort(a);
        return end == a.end() && std::ranges::is_sorted(a);
    }());
};

RTTS_CASE("raze::algorithm::sort comparator, projections, deque, move-only, and dangling") {
    std::deque<int> dq{ 5, 1, 9, 2, 8, 3, 7, 4, 6 };
    raze::algorithm::sort(dq, std::greater<>{});
    RTTS_EXPECT(std::ranges::is_sorted(dq, std::greater<>{}));

    std::vector<rtts::test_item> items{ { 4, 1 }, { 1, 2 }, { 3, 3 }, { 2, 4 } };
    raze::algorithm::sort(items, std::less<>{}, &rtts::test_item::key);
    RTTS_EXPECT(std::ranges::is_sorted(items, std::less<>{}, &rtts::test_item::key));

    std::vector<std::unique_ptr<int>> mp;
    for (int x : { 5, 2, 8, 1, 9, 3 }) mp.push_back(std::make_unique<int>(x));
    raze::algorithm::sort(mp, [](const auto& a, const auto& b) { return *a < *b; });
    for (size_t i = 1; i < mp.size(); ++i) RTTS_EXPECT(*mp[i - 1] <= *mp[i]);

    auto ni = raze::algorithm::sort(rtts::make_non_common(dq));
    RTTS_EXPECT(ni == dq.end() && std::ranges::is_sorted(dq));

    auto dr = raze::algorithm::sort(std::vector<int>{ 3, 1, 2 });
    static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void sort_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 351);
            std::vector<T> base(n);
            const unsigned shape = static_cast<unsigned>(it % 5);
            if (shape == 0u) {
                base = rtts::random::vector<T>(n, rng());
            }
            else if (shape == 1u) {
                for (size_t i = 0; i < n; ++i) base[i] = static_cast<T>(rng() % 4u);
            }
            else if (shape == 2u) {
                for (size_t i = 0; i < n; ++i) base[i] = static_cast<T>(rng() % 16u);
            }
            else if (shape == 3u) {
                base = rtts::random::vector<T>(n, rng());
                std::ranges::sort(base);
                for (size_t k = 0; k < 4 && n >= 2; ++k)
                    std::swap(base[static_cast<size_t>(rng() % n)], base[static_cast<size_t>(rng() % n)]);
            }
            else {
                base = rtts::random::vector<T>(n, rng());
                std::ranges::sort(base, std::greater<>{});
            }

            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            const bool desc = (it % 3 == 2);

            auto run_check = [&](auto comp) {
                auto exp = base;
                std::ranges::sort(exp.begin() + static_cast<std::ptrdiff_t>(lo),
                                  exp.begin() + static_cast<std::ptrdiff_t>(hi), comp);

                auto verify = [&](auto fn) {
                    auto v = base;
                    auto sub = std::ranges::subrange(v.begin() + static_cast<std::ptrdiff_t>(lo),
                                                     v.begin() + static_cast<std::ptrdiff_t>(hi));
                    auto end = fn(sub);
                    RTTS_EXPECT(end == sub.end());
                    RTTS_ALL_EQUAL(v, exp);
                };

                verify([&](auto sub) { return raze::algorithm::sort(sub, comp); });
                verify([&](auto sub) { return raze::algorithm::sort(sub.begin(), sub.end(), comp); });
                verify([&](auto sub) { return raze::algorithm::sort[opt::fscalar](sub, comp); });
                verify([&](auto sub) { return raze::algorithm::sort[opt::fstatic](sub, comp); });

                if (it % 8 == 0 && (hi - lo) <= 64) {
                    std::deque<T> dq(base.begin() + static_cast<std::ptrdiff_t>(lo),
                                     base.begin() + static_cast<std::ptrdiff_t>(hi));
                    auto end = raze::algorithm::sort(dq, comp);
                    RTTS_EXPECT(end == dq.end());
                    RTTS_EXPECT(std::ranges::equal(dq, std::ranges::subrange(
                        exp.begin() + static_cast<std::ptrdiff_t>(lo),
                        exp.begin() + static_cast<std::ptrdiff_t>(hi))));
                }
            };

            if (desc) run_check(std::greater<>{});
            else      run_check(std::less<>{});
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::sort fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        sort_fuzz<T>(seed, 30);
};
