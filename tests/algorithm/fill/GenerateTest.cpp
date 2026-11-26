#include <tests/rts/rts.h>
#include <raze/algorithm/fill/Generate.h>

#include <algorithm>
#include <array>
#include <list>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::generate", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::compact();

    auto make_gen = []() {
        int counter = 0;
        return [counter]() mutable -> T { return static_cast<T>((++counter) % 97); };
    };

    run<range, T>(cfg,
        [make_gen](auto c) { return raze::algorithm::generate(c.first(), c.last(), make_gen()); },
        [make_gen](auto c) { return std::ranges::generate(c.first(), c.last(), make_gen()); });

    run<range, T>(cfg,
        [make_gen](auto c) { return raze::algorithm::generate(c.range(), make_gen()); },
        [make_gen](auto c) { return std::ranges::generate(c.range(), make_gen()); });

    run<range, T>(cfg,
        [make_gen](auto c) { return raze::algorithm::generate[opt::fscalar](c.range(), make_gen()); },
        [make_gen](auto c) { return std::ranges::generate(c.range(), make_gen()); });

    run<range, T>(cfg,
        [make_gen](auto c) { return raze::algorithm::generate[opt::fstatic](c.first(), c.last(), make_gen()); },
        [make_gen](auto c) { return std::ranges::generate(c.first(), c.last(), make_gen()); });

    static_assert([]() constexpr {
        std::array<T, 6> a{};
        int n = 1;
        auto end = raze::algorithm::generate(a, [&n]() constexpr -> T { return static_cast<T>(n++); });
        return end == a.end() && a[0] == T(1) && a[5] == T(6);
    }());
};

RTTS_CASE("raze::algorithm::generate views, output iterators, and dangling") {
    // Non-common sentinel & writable view
    {
        std::vector<int> v(10, 0);
        int x = 10;
        auto end = raze::algorithm::generate(rtts::make_non_common(v), [&x]() { return x++; });
        RTTS_EXPECT(end == v.end());
        RTTS_EQUAL(v.front(), 10);
        RTTS_EQUAL(v.back(), 19);

        auto view = v | std::views::drop(2) | std::views::take(4);
        int y = 100;
        raze::algorithm::generate(view, [&y]() { return y++; });
        RTTS_EXPECT(v[2] == 100 && v[5] == 103 && v[6] == 16);
    }

    // Empty & rvalue dangling
    {
        std::vector<int> empty;
        int calls = 0;
        auto it = raze::algorithm::generate(empty, [&calls]() { return ++calls; });
        RTTS_EXPECT(it == empty.end() && calls == 0);

        auto dr = raze::algorithm::generate(std::vector<int>(3), []() { return 42; });
        static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
    }
};

