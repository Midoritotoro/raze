#include <tests/rts/rts.h>
#include <raze/algorithm/sort/NthElement.h>

#include <algorithm>
#include <array>
#include <deque>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::nth_element", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(3), size_t(7), size_t(31), size_t(128) }) {
        const auto base = rtts::random::vector<T>(sz, 840u + static_cast<unsigned>(sz));
        auto sorted = base;
        std::ranges::sort(sorted);

        for (size_t k : { size_t(0), sz / 4, sz / 2, sz > 0 ? sz - 1 : size_t(0), sz }) {
            auto check = [&](auto fn) {
                auto v = base;
                auto end = fn(v, v.begin() + static_cast<std::ptrdiff_t>(k));
                RTTS_EXPECT(end == v.end());
                RTTS_EXPECT(std::ranges::is_permutation(v, base));
                if (k < sz) {
                    RTTS_EQUAL(v[k], sorted[k]);
                    for (size_t i = 0; i < k; ++i) RTTS_EXPECT(!(v[k] < v[i]));
                    for (size_t i = k + 1; i < sz; ++i) RTTS_EXPECT(!(v[i] < v[k]));
                }
            };

            check([](auto& v, auto nth) { return raze::algorithm::nth_element(v, nth); });
            check([](auto& v, auto nth) { return raze::algorithm::nth_element(v.begin(), nth, v.end()); });
            check([](auto& v, auto nth) { return raze::algorithm::nth_element[opt::fscalar](v, nth); });
            check([](auto& v, auto nth) { return raze::algorithm::nth_element[opt::fstatic](v, nth); });
        }
    }

    static_assert([]() constexpr {
        std::array<T, 7> a{ T(7), T(2), T(9), T(1), T(5), T(3), T(8) };
        auto end = raze::algorithm::nth_element(a, a.begin() + 3);
        return end == a.end() && a[3] == T(5);
    }());
};

RTTS_CASE("raze::algorithm::nth_element comparator, projections, deque, and dangling") {
    std::vector<rtts::test_item> v{ { 5, 1 }, { 1, 2 }, { 9, 3 }, { 2, 4 }, { 7, 5 } };
    raze::algorithm::nth_element(v, v.begin() + 1, std::greater<>{}, &rtts::test_item::key);
    RTTS_EQUAL(v[1].key, 7);

    std::deque<int> dq{ 9, 4, 7, 1, 8, 2, 5 };
    auto ni = raze::algorithm::nth_element(rtts::make_non_common(dq), dq.begin() + 3);
    RTTS_EXPECT(ni == dq.end() && dq[3] == 5);

    std::vector<int> tmp{ 3, 1, 2 };
    auto nth = tmp.begin() + 1;
    auto dr = raze::algorithm::nth_element(std::move(tmp), nth);
    static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
};

