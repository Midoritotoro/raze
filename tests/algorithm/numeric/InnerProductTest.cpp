#include <tests/rts/rts.h>
#include <raze/algorithm/numeric/InnerProduct.h>

#include <array>
#include <list>
#include <numeric>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::inner_product", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32), size_t(100) }) {
        std::vector<T> a(sz), b(sz);
        for (size_t i = 0; i < sz; ++i) {
            a[i] = static_cast<T>(i & 1);
            b[i] = static_cast<T>(1);
        }

        const T exp1 = std::inner_product(a.begin(), a.end(), b.begin(), T(4));
        const T exp2 = std::inner_product(a.begin(), a.end(), b.begin(), T(4), std::plus<>{}, std::plus<>{});

        RTTS_EQUAL(raze::algorithm::inner_product(a, b, T(4)), exp1);
        RTTS_EQUAL(raze::algorithm::inner_product(a.begin(), a.end(), b.begin(), T(4)), exp1);
        RTTS_EQUAL(raze::algorithm::inner_product[opt::fscalar](a, b, T(4)), exp1);
        RTTS_EQUAL(raze::algorithm::inner_product[opt::fstatic](a, b, T(4), std::plus<>{}, std::plus<>{}), exp2);
    }

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        std::array<T, 4> b{ T(5), T(6), T(7), T(8) };
        return raze::algorithm::inner_product(a, b, T(0)) == T(70);
    }());
};

RTTS_CASE("raze::algorithm::inner_product custom ops, single-pass, and mismatched lengths") {
    const std::vector<int> a{ 1, 2, 3, 4, 5 };
    const std::vector<int> b{ 10, 20, 30 };
    RTTS_EQUAL(raze::algorithm::inner_product(a, b, 0), 140);

    rtts::single_pass_input_range<int> sp1(a);
    rtts::single_pass_input_range<int> sp2(b);
    RTTS_EQUAL(raze::algorithm::inner_product(sp1, sp2, 0), 140);

    RTTS_EQUAL(raze::algorithm::inner_product(
        rtts::make_non_common(b), rtts::make_non_common(b), 0, std::plus<>{}, std::equal_to<>{}), 3);
};

