#include <tests/rts/rts.h>
#include <raze/algorithm/order/IsPermutation.h>

#include <algorithm>
#include <array>
#include <forward_list>
#include <list>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::is_permutation", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(7), size_t(16), size_t(64) }) {
        auto a = rtts::random::vector<T>(sz, 200u + static_cast<unsigned>(sz));
        auto b = a;
        std::mt19937 rng(42u + static_cast<unsigned>(sz));
        std::ranges::shuffle(b, rng);

        RTTS_EXPECT(raze::algorithm::is_permutation(a, b));
        RTTS_EXPECT(raze::algorithm::is_permutation(a.begin(), a.end(), b.begin(), b.end()));
        RTTS_EXPECT(raze::algorithm::is_permutation[opt::fscalar](a, b));
        RTTS_EXPECT(raze::algorithm::is_permutation[opt::fstatic](a, b));

        if (sz > 0) {
            b[sz / 2] = static_cast<T>(b[sz / 2] + T(1));
            RTTS_EQUAL(raze::algorithm::is_permutation(a, b), std::ranges::is_permutation(a, b));
        }

        auto shorter = a;
        if (!shorter.empty()) {
            shorter.pop_back();
            RTTS_EXPECT(!raze::algorithm::is_permutation(a, shorter));
        }
    }

    static_assert([]() constexpr {
        std::array<T, 5> a{ T(1), T(2), T(2), T(3), T(4) };
        std::array<T, 5> b{ T(4), T(2), T(1), T(3), T(2) };
        std::array<T, 5> c{ T(4), T(2), T(1), T(3), T(3) };
        std::array<T, 0> e{};
        return raze::algorithm::is_permutation(a, b)
            && !raze::algorithm::is_permutation(a, c)
            && raze::algorithm::is_permutation(e, e);
    }());
};

RTTS_CASE("raze::algorithm::is_permutation projections, forward_list, and non-common") {
    const std::vector<rtts::test_item> a{ { 1, 10 }, { 2, 20 }, { 2, 30 }, { 3, 40 } };
    const std::vector<rtts::test_item> b{ { 2, 99 }, { 3, 88 }, { 1, 77 }, { 2, 66 } };
    RTTS_EXPECT(raze::algorithm::is_permutation(
        a, b, std::equal_to<>{}, &rtts::test_item::key, &rtts::test_item::key));

    std::forward_list<int> f1{ 3, 1, 4, 1, 5 };
    std::list<int> l2{ 5, 1, 3, 4, 1 };
    RTTS_EXPECT(raze::algorithm::is_permutation(f1, l2));
    RTTS_EXPECT(raze::algorithm::is_permutation(rtts::make_non_common(f1), rtts::make_non_common(l2)));
};

