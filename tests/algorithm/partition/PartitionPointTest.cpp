#include <tests/rts/rts.h>
#include <raze/algorithm/partition/PartitionPoint.h>

#include <algorithm>
#include <array>
#include <forward_list>
#include <list>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::partition_point", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;
    auto pred = [](T x) { return x < T(50); };

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(7), size_t(16), size_t(65) }) {
        for (size_t cut : { size_t(0), size_t(1), sz / 2, sz }) {
            if (cut > sz) continue;
            std::vector<T> v(sz);
            for (size_t i = 0; i < cut; ++i) v[i] = T(10);
            for (size_t i = cut; i < sz; ++i) v[i] = T(80);

            RTTS_EXPECT(raze::algorithm::partition_point(v, pred) == v.begin() + static_cast<std::ptrdiff_t>(cut));
            RTTS_EXPECT(raze::algorithm::partition_point(v.begin(), v.end(), pred) == v.begin() + static_cast<std::ptrdiff_t>(cut));
            RTTS_EXPECT(raze::algorithm::partition_point[opt::fscalar](v, pred) == v.begin() + static_cast<std::ptrdiff_t>(cut));
            RTTS_EXPECT(raze::algorithm::partition_point[opt::fstatic](v, pred) == v.begin() + static_cast<std::ptrdiff_t>(cut));
        }
    }

    static_assert([]() constexpr {
        std::array<T, 6> a{ T(2), T(4), T(6), T(7), T(9), T(11) };
        auto it = raze::algorithm::partition_point(a, [](T x) constexpr { return x < T(7); });
        return it == a.begin() + 3;
    }());
};

RTTS_CASE("raze::algorithm::partition_point forward_list, projections, non-common, and dangling") {
    std::forward_list<int> fl{ 1, 2, 3, 10, 20, 30 };
    auto it = raze::algorithm::partition_point(fl, [](int x) { return x < 10; });
    RTTS_EXPECT(it != fl.end() && *it == 10);

    const std::vector<rtts::test_item> items{ { 1, 90 }, { 2, 80 }, { 10, 10 }, { 20, 5 } };
    auto pi = raze::algorithm::partition_point(items, [](int k) { return k < 5; }, &rtts::test_item::key);
    RTTS_EXPECT(pi == items.begin() + 2);

    const std::vector<int> v{ 1, 2, 10, 20 };
    auto ni = raze::algorithm::partition_point(rtts::make_non_common(v), [](int x) { return x < 5; });
    RTTS_EXPECT(ni == v.begin() + 2);

    auto dr = raze::algorithm::partition_point(std::vector<int>{ 1, 10 }, [](int x) { return x < 5; });
    static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
};

