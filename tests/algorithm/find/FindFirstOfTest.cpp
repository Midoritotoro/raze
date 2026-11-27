#include <tests/rts/rts.h>
#include <raze/algorithm/find/FindFirstOf.h>

#include <algorithm>
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::find_first_of", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    auto make_set = [](auto& c) {
        auto beg = std::ranges::next(c.first2(), static_cast<std::ptrdiff_t>(c.count()));
        auto end = std::ranges::next(beg, 3, c.last2());
        return std::ranges::subrange(beg, end);
    };

    run<n, T>(cfg,
        [make_set](auto c) {
            auto s = make_set(c);
            return raze::algorithm::find_first_of(c.first(), c.last(), s.begin(), s.end());
        },
        [make_set](auto c) {
            auto s = make_set(c);
            return std::ranges::find_first_of(c.first(), c.last(), s.begin(), s.end());
        });

    run<n, T>(cfg,
        [make_set](auto c) { return raze::algorithm::find_first_of(c.range(), make_set(c)); },
        [make_set](auto c) { return std::ranges::find_first_of(c.range(), make_set(c)); });

    run<n, T>(cfg,
        [make_set](auto c) { return raze::algorithm::find_first_of[opt::fscalar](c.range(), make_set(c)); },
        [make_set](auto c) { return std::ranges::find_first_of(c.range(), make_set(c)); });

    run<n, T>(cfg,
        [make_set](auto c) { return raze::algorithm::find_first_of[opt::fstatic](c.range(), make_set(c)); },
        [make_set](auto c) { return std::ranges::find_first_of(c.range(), make_set(c)); });

    static_assert([]() constexpr {
        std::array<T, 6> a{ T(1), T(2), T(3), T(4), T(5), T(6) };
        std::array<T, 3> s{ T(9), T(5), T(3) };
        std::array<T, 0> empty{};
        auto r1 = raze::algorithm::find_first_of(a, s);
        auto r2 = raze::algorithm::find_first_of[opt::fscalar](a.begin(), a.end(), s.begin(), s.end());
        auto r3 = raze::algorithm::find_first_of(a, empty);
        auto r4 = raze::algorithm::find_first_of(empty, s);
        return r1 == a.begin() + 2 && r2 == a.begin() + 2 && r3 == a.end() && r4 == empty.end();
    }());
};

RTTS_CASE("raze::algorithm::find_first_of predicates, projections, views, and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    {
        const std::vector<rtts::test_item> items{ { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 } };
        const std::vector<int> targets{ 9, 3, 2 };
        auto it = raze::algorithm::find_first_of(items, targets, std::equal_to<>{}, &rtts::test_item::key);
        RTTS_EXPECT(it == items.begin() + 1);

        auto it2 = raze::algorithm::find_first_of(items, targets,
            [](int a, int b) { return a == b * 10; }, &rtts::test_item::payload);
        RTTS_EXPECT(it2 == items.begin() + 1);
    }

    {
        const std::vector<int> src{ 10, 20, 30, 40, 50 };
        const std::forward_list<int> set{ 99, 40, 30 };
        rtts::single_pass_input_range<int> sp(src);
        auto it = raze::algorithm::find_first_of(sp, set);
        RTTS_EXPECT(it != sp.end() && *it == 30);

        auto nc = rtts::make_non_common(src);
        auto it_nc = raze::algorithm::find_first_of[opt::fscalar](nc, set);
        RTTS_EXPECT(it_nc == src.begin() + 2);

        auto iv = views::iota(1, 20);
        auto it_v = raze::algorithm::find_first_of(iv, std::array<int, 2>{ 15, 7 });
        RTTS_EQUAL(*it_v, 7);

        auto rval = raze::algorithm::find_first_of(std::vector<int>{ 1, 2, 3 }, std::array<int, 1>{ 2 });
        static_assert(std::same_as<decltype(rval), std::ranges::dangling>);
    }

    {
        volatile int a[5] = { 1, 2, 3, 4, 5 };
        volatile int s[2] = { 9, 4 };
        auto it = raze::algorithm::find_first_of(a, a + 5, s, s + 2);
        RTTS_EXPECT(it == a + 3);
    }
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void find_first_of_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n1 = static_cast<size_t>(rng() % 257);
            const size_t n2 = static_cast<size_t>(rng() % 33);

            std::vector<T> a(n1), b(n2);
            if ((it & 1u) == 0u) {
                a = rtts::random::vector<T>(n1, rng());
                b = rtts::random::vector<T>(n2, rng());
            }
            else {
                const unsigned alpha = (it % 4 == 1) ? 16u : 64u;
                for (size_t i = 0; i < n1; ++i) a[i] = static_cast<T>(rng() % alpha);
                for (size_t i = 0; i < n2; ++i) b[i] = static_cast<T>(rng() % alpha);
            }

            const size_t lo1 = n1 ? static_cast<size_t>(rng() % (n1 + 1)) : 0;
            const size_t hi1 = n1 ? (lo1 + static_cast<size_t>(rng() % (n1 - lo1 + 1))) : 0;
            const size_t lo2 = n2 ? static_cast<size_t>(rng() % (n2 + 1)) : 0;
            const size_t hi2 = n2 ? (lo2 + static_cast<size_t>(rng() % (n2 - lo2 + 1))) : 0;

            auto s1 = std::ranges::subrange(a.begin() + static_cast<std::ptrdiff_t>(lo1),
                                            a.begin() + static_cast<std::ptrdiff_t>(hi1));
            auto s2 = std::ranges::subrange(b.begin() + static_cast<std::ptrdiff_t>(lo2),
                                            b.begin() + static_cast<std::ptrdiff_t>(hi2));

            const auto exp = std::ranges::find_first_of(s1, s2);
            RTTS_EXPECT(raze::algorithm::find_first_of(s1, s2) == exp);
            RTTS_EXPECT(raze::algorithm::find_first_of(s1.begin(), s1.end(), s2.begin(), s2.end()) == exp);
            RTTS_EXPECT(raze::algorithm::find_first_of[opt::fscalar](s1, s2) == exp);
            RTTS_EXPECT(raze::algorithm::find_first_of[opt::fstatic](s1, s2) == exp);

            auto pred = [](T x, T y) { return x < y; };
            const auto exp_p = std::ranges::find_first_of(s1, s2, pred);
            RTTS_EXPECT(raze::algorithm::find_first_of(s1, s2, pred) == exp_p);

            if (it % 8 == 0 && (hi1 - lo1) <= 64) {
                std::forward_list<T> fl1(s1.begin(), s1.end());
                std::list<T> l2(s2.begin(), s2.end());
                RTTS_EXPECT(raze::algorithm::find_first_of(fl1, l2) == std::ranges::find_first_of(fl1, l2));
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::find_first_of fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        find_first_of_fuzz<T>(seed, 40);
};
