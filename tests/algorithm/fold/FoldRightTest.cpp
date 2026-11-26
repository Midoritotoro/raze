#include <tests/rts/rts.h>
#include <raze/algorithm/fold/FoldRight.h>

#include <algorithm>
#include <array>
#include <list>
#include <ranges>
#include <string>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::fold_right", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32) }) {
        std::vector<T> v(sz);
        for (size_t i = 0; i < sz; ++i)
            v[i] = static_cast<T>((i & 1) ? 1 : 2);

        const auto exp = std::ranges::fold_right(v, std::int64_t{ 5 }, std::plus<>{});
        RTTS_EQUAL(raze::algorithm::fold_right(v, std::int64_t{ 5 }, std::plus<>{}), exp);
        RTTS_EQUAL(raze::algorithm::fold_right[opt::fscalar](v.begin(), v.end(), std::int64_t{ 5 }, std::plus<>{}), exp);
        RTTS_EQUAL(raze::algorithm::fold_right[opt::fstatic](v, std::int64_t{ 5 }, std::plus<>{}), exp);
    }

    static_assert([]() constexpr {
        std::array<T, 3> a{ T(1), T(2), T(3) };
        // 1 - (2 - (3 - 10)) = 1 - (2 - (-7)) = 1 - 9 = -8
        auto sub = [](T x, std::int64_t acc) constexpr -> std::int64_t {
            return static_cast<std::int64_t>(x) - acc;
        };
        return raze::algorithm::fold_right(a, std::int64_t{ 10 }, sub) == -8;
    }());
};

RTTS_CASE("raze::algorithm::fold_right right-to-left order, P2248 braced-init, list, and non-common") {
    struct fold_box {
        int v = 0;
    };
    const std::list<int> lst{ 1, 2, 3 };
    RTTS_EQUAL(raze::algorithm::fold_right(lst, 10, std::minus<>{}), -8);
    const std::list<fold_box> blst{ { 1 }, { 2 }, { 3 } };
    auto rb = raze::algorithm::fold_right(blst, { 10 },
        [](fold_box x, fold_box acc) { return fold_box{ x.v + acc.v }; });
    RTTS_EQUAL(rb.v, 16);
    RTTS_EQUAL(raze::algorithm::fold_right(rtts::make_non_common(lst), 10, std::minus<>{}), -8);

    const std::vector<std::string> words{ "a", "b", "c" };
    auto s = raze::algorithm::fold_right(words, std::string("]"),
        [](const std::string& elem, std::string acc) { return elem + acc; });
    RTTS_EQUAL(s, std::string("abc]"));
};

