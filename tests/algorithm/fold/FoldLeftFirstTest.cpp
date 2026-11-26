#include <tests/rts/rts.h>
#include <raze/algorithm/fold/FoldLeftFirst.h>

#include <algorithm>
#include <array>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::fold_left_first", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32) }) {
        std::vector<T> v(sz);
        for (size_t i = 0; i < sz; ++i)
            v[i] = static_cast<T>((i & 1) ? 0 : 1);

        const auto exp = std::ranges::fold_left_first(v, std::plus<>{});
        RTTS_EXPECT(raze::algorithm::fold_left_first(v, std::plus<>{}) == exp);
        RTTS_EXPECT(raze::algorithm::fold_left_first[opt::fscalar](v.begin(), v.end(), std::plus<>{}) == exp);
    }

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        std::array<T, 0> e{};
        return raze::algorithm::fold_left_first(a, std::plus<>{}) == T(10)
            && !raze::algorithm::fold_left_first(e, std::plus<>{}).has_value();
    }());
};

RTTS_CASE("raze::algorithm::fold_left_first order and single-pass") {
    const std::vector<int> v{ 20, 5, 3 };
    RTTS_EXPECT(raze::algorithm::fold_left_first(v, std::minus<>{}) == 12);

    rtts::single_pass_input_range<int> sp(v);
    RTTS_EXPECT(raze::algorithm::fold_left_first(sp, std::minus<>{}) == 12);
    RTTS_EXPECT(raze::algorithm::fold_left_first(rtts::make_non_common(v), std::minus<>{}) == 12);
};

