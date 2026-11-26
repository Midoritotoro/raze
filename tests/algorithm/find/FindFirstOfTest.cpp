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

RTTS_CASE_TPL("raze::algorithm::find_first_of", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::compact();

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

    // Projections & custom predicate
    {
        const std::vector<rtts::test_item> items{ { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 } };
        const std::vector<int> targets{ 9, 3, 2 };
        auto it = raze::algorithm::find_first_of(items, targets, std::equal_to<>{}, &rtts::test_item::key);
        RTTS_EXPECT(it == items.begin() + 1);

        auto it2 = raze::algorithm::find_first_of(items, targets,
            [](int a, int b) { return a == b * 10; }, &rtts::test_item::payload);
        RTTS_EXPECT(it2 == items.begin() + 1);
    }

    // Single-pass input range, non-common sentinel, views, rvalue dangling
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

    // Volatile
    {
        volatile int a[5] = { 1, 2, 3, 4, 5 };
        volatile int s[2] = { 9, 4 };
        auto it = raze::algorithm::find_first_of(a, a + 5, s, s + 2);
        RTTS_EXPECT(it == a + 3);
    }
};

