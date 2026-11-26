#include <tests/rts/rts.h>
#include <raze/algorithm/fold/FoldLeftWithIter.h>

#include <algorithm>
#include <array>
#include <list>
#include <ranges>
#include <string>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::fold_left_with_iter", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32), size_t(128) }) {
        std::vector<T> v(sz);
        for (size_t i = 0; i < sz; ++i)
            v[i] = static_cast<T>((i & 1) ? 1 : 2);

        const auto exp = std::ranges::fold_left_with_iter(v, std::int64_t{ 10 }, std::plus<>{});
        auto r1 = raze::algorithm::fold_left_with_iter(v, std::int64_t{ 10 }, std::plus<>{});
        auto r2 = raze::algorithm::fold_left_with_iter[opt::fscalar](v.begin(), v.end(), std::int64_t{ 10 }, std::plus<>{});
        auto r3 = raze::algorithm::fold_left_with_iter[opt::fstatic](v, std::int64_t{ 10 }, std::plus<>{});

        RTTS_EXPECT(r1.in == v.end() && r1.value == exp.value);
        RTTS_EXPECT(r2.in == v.end() && r2.value == exp.value);
        RTTS_EXPECT(r3.in == v.end() && r3.value == exp.value);
    }

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        auto r = raze::algorithm::fold_left_with_iter(a, 0, std::plus<>{});
        return r.in == a.end() && r.value == 10;
    }());
};

RTTS_CASE("raze::algorithm::fold_left_with_iter order, P2248 braced-init, single-pass, and dangling") {
    // Non-commutative fold verifies left-to-right associativity: ((10 - 1) - 2) - 3 = 4
    {
        const std::vector<int> v{ 1, 2, 3 };
        auto r = raze::algorithm::fold_left_with_iter(v, 10, std::minus<>{});
        RTTS_EXPECT(r.in == v.end() && r.value == 4);
    }

    // C++26 P2248: default-typed braced-init init
    {
        struct fold_box {
            int v = 0;
        };
        const std::vector<fold_box> v{ { 1 }, { 2 }, { 3 }, { 4 } };
        auto r = raze::algorithm::fold_left_with_iter(v, { 5 },
            [](fold_box acc, fold_box x) { return fold_box{ acc.v + x.v }; });
        RTTS_EXPECT(r.in == v.end() && r.value.v == 15);
    }

    // Single-pass input, non-common sentinel, rvalue dangling
    {
        const std::vector<std::string> words{ "a", "b", "c" };
        rtts::single_pass_input_range<std::string> sp(words);
        auto r = raze::algorithm::fold_left_with_iter(sp, std::string("["),
            [](std::string acc, const std::string& s) { return acc + s; });
        RTTS_EXPECT(r.in == sp.end() && r.value == "[abc");

        auto dr = raze::algorithm::fold_left_with_iter(std::vector<int>{ 1, 2, 3 }, 0, std::plus<>{});
        static_assert(std::same_as<decltype(dr.in), std::ranges::dangling>);
        RTTS_EQUAL(dr.value, 6);
    }
};

