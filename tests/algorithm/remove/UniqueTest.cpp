#include <tests/rts/rts.h>
#include <raze/algorithm/remove/Unique.h>

#include <algorithm>
#include <array>
#include <forward_list>
#include <list>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::unique", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::compact();

    auto make_dup_data = [](size_t n, unsigned seed) {
        std::mt19937 rng(seed);
        std::vector<T> v(n);
        for (size_t i = 0; i < n; ++i)
            v[i] = static_cast<T>(rng() % 5);
        return v;
    };

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(3), size_t(7), size_t(16), size_t(64), size_t(129) }) {
        const auto base = make_dup_data(sz, 100u + static_cast<unsigned>(sz));

        auto exp = base;
        auto er = std::ranges::unique(exp);
        exp.erase(er.begin(), er.end());

        auto check_mode = [&](auto fn) {
            auto v = base;
            auto r = fn(v);
            RTTS_EXPECT(r.end() == v.end());
            RTTS_EXPECT(std::ranges::equal(std::ranges::subrange(v.begin(), r.begin()), exp));
        };

        check_mode([](auto& v) { return raze::algorithm::unique(v); });
        check_mode([](auto& v) { return raze::algorithm::unique(v.begin(), v.end()); });
        check_mode([](auto& v) { return raze::algorithm::unique[opt::fscalar](v); });
        check_mode([](auto& v) { return raze::algorithm::unique[opt::fstatic](v); });
    }

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::unique(c.range()); },
        [](auto c) { return std::ranges::unique(c.range()); },
        prefix_result);

    static_assert([]() constexpr {
        std::array<T, 8> a{ T(1), T(1), T(2), T(2), T(2), T(3), T(1), T(1) };
        auto r = raze::algorithm::unique(a);
        return r.begin() == a.begin() + 4 && r.end() == a.end()
            && a[0] == T(1) && a[1] == T(2) && a[2] == T(3) && a[3] == T(1);
    }());
};

RTTS_CASE("raze::algorithm::unique predicates, projections, containers, and edge cases") {
    // Projections & custom predicate
    {
        std::vector<rtts::test_item> v{ { 1, 10 }, { 1, 20 }, { 2, 30 }, { 2, 40 }, { 3, 50 } };
        auto r = raze::algorithm::unique(v, std::equal_to<>{}, &rtts::test_item::key);
        v.erase(r.begin(), r.end());
        RTTS_EQUAL(v.size(), size_t(3));
        RTTS_EXPECT(v[0].payload == 10 && v[1].payload == 30 && v[2].payload == 50);
    }

    // Forward list, non-common sentinel, rvalue dangling
    {
        std::forward_list<int> fl{ 1, 1, 2, 3, 3, 3, 4 };
        auto r = raze::algorithm::unique(fl);
        std::vector<int> prefix(fl.begin(), r.begin());
        RTTS_EXPECT((prefix == std::vector<int>{ 1, 2, 3, 4 }));

        std::vector<int> v{ 5, 5, 6, 6, 7 };
        auto nr = raze::algorithm::unique(rtts::make_non_common(v));
        RTTS_EXPECT(nr.end() == v.end());
        RTTS_EXPECT((std::vector<int>(v.begin(), nr.begin()) == std::vector<int>{ 5, 6, 7 }));

        auto dr = raze::algorithm::unique(std::vector<int>{ 1, 1, 2 });
        static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
    }
};

