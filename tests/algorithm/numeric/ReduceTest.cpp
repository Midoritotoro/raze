#include <tests/rts/rts.h>
#include <raze/algorithm/numeric/Reduce.h>

#include <array>
#include <list>
#include <numeric>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::reduce", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32), size_t(128) }) {
        std::vector<T> v(sz);
        for (size_t i = 0; i < sz; ++i)
            v[i] = static_cast<T>(i & 1);

        const T exp0 = std::reduce(v.begin(), v.end());
        const T exp5 = std::reduce(v.begin(), v.end(), T(5));

        RTTS_EQUAL(raze::algorithm::reduce(v), exp0);
        RTTS_EQUAL(raze::algorithm::reduce(v.begin(), v.end()), exp0);
        RTTS_EQUAL(raze::algorithm::reduce(v, T(5)), exp5);
        RTTS_EQUAL(raze::algorithm::reduce[opt::fscalar](v, T(5)), exp5);
        RTTS_EQUAL(raze::algorithm::reduce[opt::fstatic](v, T(5), std::plus<>{}), exp5);
    }

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        std::array<T, 0> e{};
        return raze::algorithm::reduce(a) == T(10)
            && raze::algorithm::reduce(e) == T(0)
            && raze::algorithm::reduce(a, T(1), std::multiplies<>{}) == T(24);
    }());
};

RTTS_CASE("raze::algorithm::reduce single-pass input and non-common sentinel") {
    const std::vector<int> v{ 2, 3, 5, 7 };
    rtts::single_pass_input_range<int> sp(v);
    RTTS_EQUAL(raze::algorithm::reduce(sp, 1, std::multiplies<>{}), 210);
    RTTS_EQUAL(raze::algorithm::reduce(rtts::make_non_common(v)), 17);
};

