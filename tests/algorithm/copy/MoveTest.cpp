#include <tests/rts/rts.h>
#include <raze/algorithm/copy/Move.h>

#include <algorithm>
#include <array>
#include <memory>
#include <ranges>
#include <string>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::move", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::compact();

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::move(c.first(), c.last(), c.out()); },
        [](auto c) { return std::ranges::move(c.first(), c.last(), c.out()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::move(c.range(), c.out()); },
        [](auto c) { return std::ranges::move(c.range(), c.out()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::move[opt::fscalar](c.range(), c.out()); },
        [](auto c) { return std::ranges::move(c.range(), c.out()); });

    run<range, T>(cfg,
        [](auto c) { return raze::algorithm::move[opt::fstatic](c.first(), c.last(), c.out()); },
        [](auto c) { return std::ranges::move(c.first(), c.last(), c.out()); });

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::move(c.first(), c.last(), c.out()); },
        [](auto c) constexpr { return std::ranges::move(c.first(), c.last(), c.out()); }));

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::move[opt::fscalar](c.range(), c.out()); },
        [](auto c) constexpr { return std::ranges::move(c.range(), c.out()); }));
};

RTTS_CASE("raze::algorithm::move move-only, overlap, output iterators, and dangling") {
    // Move-only unique_ptr
    {
        std::vector<std::unique_ptr<int>> src;
        for (int i = 0; i < 8; ++i)
            src.push_back(std::make_unique<int>(i * 10));

        std::vector<std::unique_ptr<int>> dst(src.size());
        auto r = raze::algorithm::move(src, dst.begin());
        RTTS_EXPECT(r.in == src.end() && r.out == dst.end());
        for (int i = 0; i < 8; ++i) {
            RTTS_EXPECT(src[i] == nullptr);
            RTTS_EXPECT(dst[i] != nullptr && *dst[i] == i * 10);
        }
    }

    // Overlapping left shift (valid for move when result not in [first, last))
    {
        std::vector<std::string> v{ "a", "b", "c", "d", "e" };
        auto r = raze::algorithm::move(v.begin() + 2, v.end(), v.begin());
        RTTS_EXPECT(r.in == v.end() && r.out == v.begin() + 3);
        RTTS_EXPECT(v[0] == "c" && v[1] == "d" && v[2] == "e");
    }

    // back_inserter, non-common sentinel, rvalue dangling
    {
        std::vector<std::string> src{ "one", "two", "three" };
        std::vector<std::string> out;
        raze::algorithm::move(rtts::make_non_common(src), std::back_inserter(out));
        RTTS_EXPECT((out == std::vector<std::string>{ "one", "two", "three" }));

        std::vector<int> dst(3);
        auto dr = raze::algorithm::move(std::vector<int>{ 7, 8, 9 }, dst.begin());
        static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
        RTTS_EXPECT((dst == std::vector<int>{ 7, 8, 9 }));
    }
};

