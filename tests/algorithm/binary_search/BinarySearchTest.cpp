#include <tests/rts/rts.h>
#include <raze/algorithm/binary_search/BinarySearch.h>

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

RTTS_CASE_TPL("raze::algorithm::binary_search", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(7), size_t(16), size_t(65), size_t(256) }) {
        auto v = rtts::random::vector<T>(sz, 530u + static_cast<unsigned>(sz));
        std::ranges::sort(v);

        std::vector<T> queries{ T(0), T(1), T(50), T(120) };
        if (!v.empty()) {
            queries.push_back(v.front());
            queries.push_back(v[sz / 2]);
            queries.push_back(v.back());
        }

        for (T q : queries) {
            const bool exp = std::ranges::binary_search(v, q);
            RTTS_EQUAL(raze::algorithm::binary_search(v, q), exp);
            RTTS_EQUAL(raze::algorithm::binary_search(v.begin(), v.end(), q), exp);
            RTTS_EQUAL(raze::algorithm::binary_search[opt::fscalar](v, q), exp);
            RTTS_EQUAL(raze::algorithm::binary_search[opt::fstatic](v, q), exp);
        }
    }

    static_assert([]() constexpr {
        std::array<T, 5> a{ T(1), T(3), T(5), T(7), T(9) };
        return raze::algorithm::binary_search(a, T(5))
            && !raze::algorithm::binary_search(a, T(4))
            && !raze::algorithm::binary_search(a, T(0))
            && !raze::algorithm::binary_search(a, T(10));
    }());
};

RTTS_CASE("raze::algorithm::binary_search P2248 braced-init, projections, forward_list, and non-common") {
    const std::vector<boxed_item> v{ { { 10 }, 1 }, { { 20 }, 2 }, { { 30 }, 3 } };
    RTTS_EXPECT(raze::algorithm::binary_search(v, { 20 }, std::less<>{}, &boxed_item::box));
    RTTS_EXPECT(!raze::algorithm::binary_search(v, { 25 }, std::less<>{}, &boxed_item::box));

    const std::forward_list<int> fl{ 1, 3, 5, 7, 9 };
    RTTS_EXPECT(raze::algorithm::binary_search(fl, 7));
    RTTS_EXPECT(!raze::algorithm::binary_search(fl, 6));

    const std::vector<int> desc{ 9, 7, 5, 3, 1 };
    RTTS_EXPECT(raze::algorithm::binary_search(rtts::make_non_common(desc), 5, std::greater<>{}));
};

