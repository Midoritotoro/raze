#include <tests/rts/rts.h>
#include <raze/algorithm/binary_search/LowerBound.h>

#include <algorithm>
#include <array>
#include <forward_list>
#include <list>
#include <ranges>
#include <vector>

namespace {
    struct key_box {
        int val;
        constexpr bool operator<(const key_box& o) const noexcept { return val < o.val; }
    };
    struct boxed_item {
        key_box box;
        int id;
    };
}

RTTS_CASE_TPL("raze::algorithm::lower_bound", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(7), size_t(16), size_t(65), size_t(256) }) {
        auto v = rtts::random::vector<T>(sz, 500u + static_cast<unsigned>(sz));
        std::ranges::sort(v);

        std::vector<T> queries{ T(0), T(1), T(50), T(120) };
        if (!v.empty()) {
            queries.push_back(v.front());
            queries.push_back(v[sz / 2]);
            queries.push_back(v.back());
        }

        for (T q : queries) {
            const auto exp = std::ranges::lower_bound(v, q);
            RTTS_EXPECT(raze::algorithm::lower_bound(v, q) == exp);
            RTTS_EXPECT(raze::algorithm::lower_bound(v.begin(), v.end(), q) == exp);
            RTTS_EXPECT(raze::algorithm::lower_bound[opt::fscalar](v, q) == exp);
            RTTS_EXPECT(raze::algorithm::lower_bound[opt::fstatic](v, q) == exp);
        }
    }

    static_assert([]() constexpr {
        std::array<T, 6> a{ T(1), T(2), T(2), T(2), T(5), T(8) };
        return raze::algorithm::lower_bound(a, T(2)) == a.begin() + 1
            && raze::algorithm::lower_bound(a, T(3)) == a.begin() + 4
            && raze::algorithm::lower_bound(a, T(0)) == a.begin()
            && raze::algorithm::lower_bound(a, T(9)) == a.end();
    }());
};

RTTS_CASE("raze::algorithm::lower_bound P2248 braced-init, projections, forward_list, and dangling") {
    // C++26 P2248: braced-init-list with projection
    {
        const std::vector<boxed_item> v{ { { 10 }, 1 }, { { 20 }, 2 }, { { 20 }, 3 }, { { 30 }, 4 } };
        auto it = raze::algorithm::lower_bound(v, { 20 }, std::less<>{}, &boxed_item::box);
        RTTS_EXPECT(it == v.begin() + 1 && it->id == 2);
    }

    // Forward list, non-common sentinel, descending comparator, rvalue dangling
    {
        const std::forward_list<int> fl{ 1, 3, 3, 5, 7 };
        auto it = raze::algorithm::lower_bound(fl, 3);
        RTTS_EQUAL(*it, 3);

        const std::vector<int> desc{ 9, 7, 5, 5, 2, 1 };
        auto dit = raze::algorithm::lower_bound(rtts::make_non_common(desc), 5, std::greater<>{});
        RTTS_EXPECT(dit == desc.begin() + 2);

        auto dr = raze::algorithm::lower_bound(std::vector<int>{ 1, 2, 3 }, 2);
        static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
    }
};

