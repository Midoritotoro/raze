#include <tests/rts/rts.h>
#include <raze/algorithm/set/Merge.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <list>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::merge", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t n1 : { size_t(0), size_t(1), size_t(7), size_t(32) }) {
        for (size_t n2 : { size_t(0), size_t(1), size_t(9), size_t(35) }) {
            auto a = rtts::random::vector<T>(n1, 600u + static_cast<unsigned>(n1));
            auto b = rtts::random::vector<T>(n2, 700u + static_cast<unsigned>(n2));
            std::ranges::sort(a);
            std::ranges::sort(b);

            std::vector<T> exp(n1 + n2);
            std::ranges::merge(a, b, exp.begin());

            std::vector<T> out1(n1 + n2), out2(n1 + n2);
            auto r1 = raze::algorithm::merge(a, b, out1.begin());
            auto r2 = raze::algorithm::merge[opt::fscalar](a.begin(), a.end(), b.begin(), b.end(), out2.begin());

            RTTS_EXPECT(r1.in1 == a.end() && r1.in2 == b.end() && r1.out == out1.end());
            RTTS_EXPECT(r2.in1 == a.end() && r2.in2 == b.end() && r2.out == out2.end());
            RTTS_ALL_EQUAL(out1, exp);
            RTTS_ALL_EQUAL(out2, exp);
        }
    }

    static_assert([]() constexpr {
        std::array<T, 3> a{ T(1), T(3), T(5) };
        std::array<T, 3> b{ T(2), T(3), T(6) };
        std::array<T, 6> out{};
        auto r = raze::algorithm::merge(a, b, out.begin());
        return r.in1 == a.end() && r.in2 == b.end() && r.out == out.end()
            && out == std::array<T, 6>{ T(1), T(2), T(3), T(3), T(5), T(6) };
    }());
};

RTTS_CASE("raze::algorithm::merge stability, projections, single-pass input, and dangling") {
    // Stability: elements from first range precede equivalent elements from second range
    {
        const std::vector<rtts::test_item> a{ { 1, 10 }, { 2, 11 }, { 3, 12 } };
        const std::vector<rtts::test_item> b{ { 2, 20 }, { 2, 21 }, { 4, 22 } };
        std::vector<rtts::test_item> out;
        raze::algorithm::merge(a, b, std::back_inserter(out), std::less<>{},
            &rtts::test_item::key, &rtts::test_item::key);
        RTTS_EQUAL(out.size(), size_t(6));
        RTTS_EXPECT(out[1].payload == 11 && out[2].payload == 20 && out[3].payload == 21);
    }

    // Single-pass input ranges & rvalue dangling
    {
        const std::vector<int> a{ 1, 4, 7 };
        const std::vector<int> b{ 2, 3, 8 };
        rtts::single_pass_input_range<int> sp1(a);
        rtts::single_pass_input_range<int> sp2(b);
        std::vector<int> out;
        raze::algorithm::merge(sp1, sp2, std::back_inserter(out));
        RTTS_EXPECT((out == std::vector<int>{ 1, 2, 3, 4, 7, 8 }));

        std::vector<int> o2;
        auto dr = raze::algorithm::merge(std::vector<int>{ 1, 3 }, std::vector<int>{ 2, 4 }, std::back_inserter(o2));
        static_assert(std::same_as<decltype(dr.in1), std::ranges::dangling>);
        static_assert(std::same_as<decltype(dr.in2), std::ranges::dangling>);
    }
};

