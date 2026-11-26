#include <tests/rts/rts.h>
#include <raze/algorithm/fold/FoldRightLast.h>

#include <algorithm>
#include <array>
#include <list>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::fold_right_last", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32) }) {
        std::vector<T> v(sz);
        for (size_t i = 0; i < sz; ++i)
            v[i] = static_cast<T>((i & 1) ? 0 : 1);

        const auto exp = std::ranges::fold_right_last(v, std::plus<>{});
        RTTS_EXPECT(raze::algorithm::fold_right_last(v, std::plus<>{}) == exp);
        RTTS_EXPECT(raze::algorithm::fold_right_last[opt::fscalar](v.begin(), v.end(), std::plus<>{}) == exp);
    }

    static_assert([]() constexpr {
        std::array<T, 3> a{ T(1), T(2), T(10) };
        std::array<T, 0> e{};
        // 1 - (2 - 10) = 9
        auto r1 = raze::algorithm::fold_right_last(a, [](T x, auto acc) constexpr -> int {
            return static_cast<int>(x) - static_cast<int>(acc);
        });
        auto r2 = raze::algorithm::fold_right_last(e, std::plus<>{});
        return r1 == 9 && !r2.has_value();
    }());
};

RTTS_CASE("raze::algorithm::fold_right_last list and non-common") {
    const std::list<int> lst{ 1, 2, 10 };
    RTTS_EXPECT(raze::algorithm::fold_right_last(lst, std::minus<>{}) == 9);
    RTTS_EXPECT(raze::algorithm::fold_right_last(rtts::make_non_common(lst), std::minus<>{}) == 9);
};
