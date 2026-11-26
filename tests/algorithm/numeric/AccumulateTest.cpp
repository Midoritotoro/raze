#include <tests/rts/rts.h>
#include <raze/algorithm/numeric/Accumulate.h>

#include <array>
#include <list>
#include <numeric>
#include <ranges>
#include <string>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::accumulate", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32), size_t(128) }) {
        std::vector<T> v(sz);
        for (size_t i = 0; i < sz; ++i)
            v[i] = static_cast<T>(i & 1);

        const T exp = std::accumulate(v.begin(), v.end(), T(3));
        RTTS_EQUAL(raze::algorithm::accumulate(v, T(3)), exp);
        RTTS_EQUAL(raze::algorithm::accumulate(v.begin(), v.end(), T(3)), exp);
        RTTS_EQUAL(raze::algorithm::accumulate[opt::fscalar](v, T(3)), exp);
        RTTS_EQUAL(raze::algorithm::accumulate[opt::fstatic](v, T(3)), exp);
    }

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        return raze::algorithm::accumulate(a, T(0)) == T(10)
            && raze::algorithm::accumulate(a, T(1), std::multiplies<>{}) == T(24);
    }());
};

RTTS_CASE("raze::algorithm::accumulate custom op, single-pass input, and non-common") {
    const std::vector<int> v{ 1, 2, 3, 4 };
    RTTS_EQUAL(raze::algorithm::accumulate(v, 20, std::minus<>{}), 10);

    rtts::single_pass_input_range<int> sp(v);
    RTTS_EQUAL(raze::algorithm::accumulate(sp, 0), 10);
    RTTS_EQUAL(raze::algorithm::accumulate(rtts::make_non_common(v), 1, std::multiplies<>{}), 24);

    const std::list<std::string> words{ "a", "b", "c" };
    RTTS_EQUAL(raze::algorithm::accumulate(words, std::string("")), std::string("abc"));
};

