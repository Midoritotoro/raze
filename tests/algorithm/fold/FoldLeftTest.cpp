#include <tests/rts/rts.h>
#include <raze/algorithm/fold/FoldLeft.h>

#include <algorithm>
#include <array>
#include <ranges>
#include <string>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::fold_left", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32), size_t(128) }) {
        std::vector<T> v(sz);
        for (size_t i = 0; i < sz; ++i)
            v[i] = static_cast<T>((i & 1) ? 1 : 2);

        const auto exp = std::ranges::fold_left(v, std::int64_t{ 5 }, std::plus<>{});
        RTTS_EQUAL(raze::algorithm::fold_left(v, std::int64_t{ 5 }, std::plus<>{}), exp);
        RTTS_EQUAL(raze::algorithm::fold_left[opt::fscalar](v.begin(), v.end(), std::int64_t{ 5 }, std::plus<>{}), exp);
        RTTS_EQUAL(raze::algorithm::fold_left[opt::fstatic](v, std::int64_t{ 5 }, std::plus<>{}), exp);
    }

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        return raze::algorithm::fold_left(a, 0, std::plus<>{}) == 10;
    }());
};

namespace {
    struct fold_box {
        int v = 0;
        constexpr bool operator==(const fold_box&) const = default;
    };
}

RTTS_CASE("raze::algorithm::fold_left order, P2248 braced-init, and single-pass") {
    const std::vector<int> v{ 1, 2, 3 };
    RTTS_EQUAL(raze::algorithm::fold_left(v, 10, std::minus<>{}), 4);

    const std::vector<fold_box> boxes{ { 1 }, { 2 }, { 3 } };
    auto rb = raze::algorithm::fold_left(boxes, { 10 },
        [](fold_box acc, fold_box x) { return fold_box{ acc.v + x.v }; });
    RTTS_EQUAL(rb.v, 16);

    rtts::single_pass_input_range<int> sp(v);
    RTTS_EQUAL(raze::algorithm::fold_left(sp, 0, std::plus<>{}), 6);
    RTTS_EQUAL(raze::algorithm::fold_left(rtts::make_non_common(v), 1, std::multiplies<>{}), 6);
};

