#include <tests/rts/rts.h>
#include <raze/algorithm/order/NextPermutation.h>

#include <algorithm>
#include <array>
#include <list>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::next_permutation", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    // Exhaustive enumeration of all 4! = 24 permutations
    {
        std::vector<T> a{ T(1), T(2), T(3), T(4) };
        auto b = a;
        int count = 0;
        for (;;) {
            ++count;
            auto ra = raze::algorithm::next_permutation(a);
            auto rb = std::ranges::next_permutation(b);
            RTTS_EXPECT(ra.in == a.end());
            RTTS_EQUAL(ra.found, rb.found);
            RTTS_ALL_EQUAL(a, b);
            if (!ra.found) break;
        }
        RTTS_EQUAL(count, 24);
    }

    // With duplicates + fscalar + iterator-sentinel
    {
        std::vector<T> a{ T(1), T(1), T(2), T(2) };
        auto b = a;
        int count = 0;
        for (;;) {
            ++count;
            auto ra = raze::algorithm::next_permutation[opt::fscalar](a.begin(), a.end());
            auto rb = std::ranges::next_permutation(b);
            RTTS_EQUAL(ra.found, rb.found);
            RTTS_ALL_EQUAL(a, b);
            if (!ra.found) break;
        }
        RTTS_EQUAL(count, 6);
    }

    static_assert([]() constexpr {
        std::array<T, 3> a{ T(1), T(2), T(3) };
        auto r1 = raze::algorithm::next_permutation(a);
        std::array<T, 3> last{ T(3), T(2), T(1) };
        auto r2 = raze::algorithm::next_permutation(last);
        std::array<T, 0> empty{};
        auto r3 = raze::algorithm::next_permutation(empty);
        return r1.found && a == std::array<T, 3>{ T(1), T(3), T(2) }
            && !r2.found && last == std::array<T, 3>{ T(1), T(2), T(3) }
            && !r3.found;
    }());
};

RTTS_CASE("raze::algorithm::next_permutation list, projections, non-common, and dangling") {
    std::list<int> lst{ 1, 2, 3 };
    auto r = raze::algorithm::next_permutation(lst, std::greater<>{});
    RTTS_EXPECT(!r.found && r.in == lst.end());
    RTTS_EXPECT((lst == std::list<int>{ 3, 2, 1 }));

    std::vector<rtts::test_item> items{ { 1, 10 }, { 2, 20 }, { 3, 30 } };
    auto rp = raze::algorithm::next_permutation(items, std::less<>{}, &rtts::test_item::key);
    RTTS_EXPECT(rp.found && items[1].key == 3 && items[2].key == 2);

    std::vector<int> v{ 1, 3, 2 };
    auto rn = raze::algorithm::next_permutation(rtts::make_non_common(v));
    RTTS_EXPECT(rn.found && rn.in == v.end() && (v == std::vector<int>{ 2, 1, 3 }));

    auto dr = raze::algorithm::next_permutation(std::vector<int>{ 1, 2 });
    static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
    RTTS_EXPECT(dr.found);
};

