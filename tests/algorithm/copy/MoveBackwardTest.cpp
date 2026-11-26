#include <tests/rts/rts.h>
#include <raze/algorithm/copy/MoveBackward.h>

#include <algorithm>
#include <array>
#include <list>
#include <memory>
#include <ranges>
#include <string>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::move_backward", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::compact();

    run<range, T, bidirectional_iterator>(cfg,
        [](auto c) { return raze::algorithm::move_backward(c.first(), c.last(), c.data.output.end()); },
        [](auto c) { return std::ranges::move_backward(c.first(), c.last(), c.data.output.end()); });

    run<range, T, bidirectional_iterator>(cfg,
        [](auto c) { return raze::algorithm::move_backward(c.range(), c.data.output.end()); },
        [](auto c) { return std::ranges::move_backward(c.range(), c.data.output.end()); });

    run<range, T, bidirectional_iterator>(cfg,
        [](auto c) { return raze::algorithm::move_backward[opt::fscalar](c.range(), c.data.output.end()); },
        [](auto c) { return std::ranges::move_backward(c.range(), c.data.output.end()); });

    run<range, T, bidirectional_iterator>(cfg,
        [](auto c) { return raze::algorithm::move_backward[opt::fstatic](c.first(), c.last(), c.data.output.end()); },
        [](auto c) { return std::ranges::move_backward(c.first(), c.last(), c.data.output.end()); });

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::move_backward(c.first(), c.last(), c.data.output.end()); },
        [](auto c) constexpr { return std::ranges::move_backward(c.first(), c.last(), c.data.output.end()); }));

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return raze::algorithm::move_backward[opt::fscalar](c.range(), c.data.output.end()); },
        [](auto c) constexpr { return std::ranges::move_backward(c.range(), c.data.output.end()); }));
};

RTTS_CASE("raze::algorithm::move_backward move-only, overlap, non-common, and dangling") {
    // Move-only unique_ptr
    {
        std::vector<std::unique_ptr<int>> src;
        for (int i = 0; i < 6; ++i)
            src.push_back(std::make_unique<int>(i + 100));

        std::vector<std::unique_ptr<int>> dst(src.size());
        auto r = raze::algorithm::move_backward(src, dst.end());
        RTTS_EXPECT(r.in == src.end() && r.out == dst.begin());
        for (int i = 0; i < 6; ++i) {
            RTTS_EXPECT(src[i] == nullptr);
            RTTS_EXPECT(dst[i] != nullptr && *dst[i] == i + 100);
        }
    }

    // Overlapping right shift
    {
        std::vector<std::string> v{ "a", "b", "c", "d", "e" };
        auto r = raze::algorithm::move_backward(v.begin(), v.begin() + 3, v.end());
        RTTS_EXPECT(r.in == v.begin() + 3 && r.out == v.begin() + 2);
        RTTS_EXPECT(v[2] == "a" && v[3] == "b" && v[4] == "c");
    }

    // Non-common sentinel, list, rvalue dangling
    {
        std::list<int> src{ 1, 2, 3, 4 };
        std::list<int> dst(4, 0);
        auto r = raze::algorithm::move_backward(rtts::make_non_common(src), dst.end());
        RTTS_EXPECT(r.in == src.end() && r.out == dst.begin());
        RTTS_EXPECT(dst == src);

        std::vector<int> out(3);
        auto dr = raze::algorithm::move_backward(std::vector<int>{ 4, 5, 6 }, out.end());
        static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
        RTTS_EXPECT((out == std::vector<int>{ 4, 5, 6 }));
    }
};

