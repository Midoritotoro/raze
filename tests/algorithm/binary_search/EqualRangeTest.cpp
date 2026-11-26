#include <tests/rts/rts.h>
#include <raze/algorithm/binary_search/EqualRange.h>

#include <algorithm>
#include <array>
#include <forward_list>
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

RTTS_CASE_TPL("raze::algorithm::equal_range", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(7), size_t(16), size_t(65), size_t(256) }) {
        auto v = rtts::random::vector<T>(sz, 520u + static_cast<unsigned>(sz));
        std::ranges::sort(v);

        std::vector<T> queries{ T(0), T(1), T(50), T(120) };
        if (!v.empty()) {
            queries.push_back(v.front());
            queries.push_back(v[sz / 2]);
            queries.push_back(v.back());
        }

        for (T q : queries) {
            const auto exp = std::ranges::equal_range(v, q);
            auto eq = [&](auto r) { return r.begin() == exp.begin() && r.end() == exp.end(); };
            RTTS_EXPECT(eq(raze::algorithm::equal_range(v, q)));
            RTTS_EXPECT(eq(raze::algorithm::equal_range(v.begin(), v.end(), q)));
            RTTS_EXPECT(eq(raze::algorithm::equal_range[opt::fscalar](v, q)));
            RTTS_EXPECT(eq(raze::algorithm::equal_range[opt::fstatic](v, q)));
        }
    }

    static_assert([]() constexpr {
        std::array<T, 6> a{ T(1), T(2), T(2), T(2), T(5), T(8) };
        auto r = raze::algorithm::equal_range(a, T(2));
        auto m = raze::algorithm::equal_range(a, T(3));
        return r.begin() == a.begin() + 1 && r.end() == a.begin() + 4
            && m.begin() == a.begin() + 4 && m.end() == a.begin() + 4;
    }());
};

RTTS_CASE("raze::algorithm::equal_range P2248 braced-init, projections, forward_list, and dangling") {
    {
        const std::vector<boxed_item> v{ { { 10 }, 1 }, { { 20 }, 2 }, { { 20 }, 3 }, { { 30 }, 4 } };
        auto r = raze::algorithm::equal_range(v, { 20 }, std::less<>{}, &boxed_item::box);
        RTTS_EXPECT(r.begin() == v.begin() + 1 && r.end() == v.begin() + 3);
    }

    {
        const std::forward_list<int> fl{ 1, 3, 3, 5, 7 };
        auto r = raze::algorithm::equal_range(fl, 3);
        RTTS_EQUAL(std::ranges::distance(r), std::ptrdiff_t(2));

        const std::vector<int> desc{ 9, 7, 5, 5, 2, 1 };
        auto dr = raze::algorithm::equal_range(rtts::make_non_common(desc), 5, std::greater<>{});
        RTTS_EXPECT(dr.begin() == desc.begin() + 2 && dr.end() == desc.begin() + 4);

        auto dangling = raze::algorithm::equal_range(std::vector<int>{ 1, 2, 3 }, 2);
        static_assert(std::same_as<decltype(dangling), std::ranges::dangling>);
    }
};

