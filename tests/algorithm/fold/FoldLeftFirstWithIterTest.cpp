#include <tests/rts/rts.h>
#include <raze/algorithm/fold/FoldLeftFirstWithIter.h>

#include <algorithm>
#include <array>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::fold_left_first_with_iter", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32) }) {
        std::vector<T> v(sz);
        for (size_t i = 0; i < sz; ++i)
            v[i] = static_cast<T>((i & 1) ? 0 : 1);

        const auto exp = std::ranges::fold_left_first_with_iter(v, std::plus<>{});
        auto r1 = raze::algorithm::fold_left_first_with_iter(v, std::plus<>{});
        auto r2 = raze::algorithm::fold_left_first_with_iter[opt::fscalar](v.begin(), v.end(), std::plus<>{});

        RTTS_EXPECT(r1.in == v.end() && r1.value == exp.value);
        RTTS_EXPECT(r2.in == v.end() && r2.value == exp.value);
    }

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        std::array<T, 0> e{};
        auto r1 = raze::algorithm::fold_left_first_with_iter(a, std::plus<>{});
        auto r2 = raze::algorithm::fold_left_first_with_iter(e, std::plus<>{});
        return r1.in == a.end() && r1.value.has_value() && *r1.value == T(10)
            && r2.in == e.end() && !r2.value.has_value();
    }());
};

RTTS_CASE("raze::algorithm::fold_left_first_with_iter order, single-pass, and dangling") {
    const std::vector<int> v{ 20, 5, 3 };
    auto r = raze::algorithm::fold_left_first_with_iter(v, std::minus<>{});
    RTTS_EXPECT(r.in == v.end() && r.value == 12); // (20 - 5) - 3

    rtts::single_pass_input_range<int> sp(v);
    auto rs = raze::algorithm::fold_left_first_with_iter(sp, std::minus<>{});
    RTTS_EXPECT(rs.in == sp.end() && rs.value == 12);

    auto dr = raze::algorithm::fold_left_first_with_iter(std::vector<int>{ 1, 2, 3 }, std::plus<>{});
    static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
    RTTS_EXPECT(dr.value == 6);
};

