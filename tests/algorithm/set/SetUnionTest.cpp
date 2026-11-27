#include <tests/rts/rts.h>
#include <raze/algorithm/set/SetUnion.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <ranges>
#include <vector>
#include <random>

RTTS_CASE_TPL("raze::algorithm::set_union", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t n1 : { size_t(0), size_t(1), size_t(15), size_t(64) }) {
        for (size_t n2 : { size_t(0), size_t(1), size_t(17), size_t(64) }) {
            auto a = rtts::random::vector<T>(n1, 640u + static_cast<unsigned>(n1));
            auto b = rtts::random::vector<T>(n2, 650u + static_cast<unsigned>(n2));
            std::ranges::sort(a);
            std::ranges::sort(b);

            std::vector<T> exp, o1, o2;
            std::ranges::set_union(a, b, std::back_inserter(exp));
            auto r1 = raze::algorithm::set_union(a, b, std::back_inserter(o1));
            auto r2 = raze::algorithm::set_union[opt::fscalar](a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(o2));

            RTTS_EXPECT(r1.in1 == a.end() && r1.in2 == b.end());
            RTTS_EXPECT(r2.in1 == a.end() && r2.in2 == b.end());
            RTTS_ALL_EQUAL(o1, exp);
            RTTS_ALL_EQUAL(o2, exp);
        }
    }

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(2), T(4) };
        std::array<T, 4> b{ T(2), T(3), T(4), T(5) };
        std::array<T, 8> out{};
        auto r = raze::algorithm::set_union(a, b, out.begin());
        return r.in1 == a.end() && r.in2 == b.end() && r.out == out.begin() + 6
            && out[0] == T(1) && out[1] == T(2) && out[2] == T(2)
            && out[3] == T(3) && out[4] == T(4) && out[5] == T(5);
    }());
};

RTTS_CASE("raze::algorithm::set_union projections, single-pass input, and dangling") {
    const std::vector<rtts::test_item> a{ { 1, 10 }, { 2, 11 }, { 4, 12 } };
    const std::vector<rtts::test_item> b{ { 2, 20 }, { 3, 21 } };
    std::vector<rtts::test_item> out;
    raze::algorithm::set_union(a, b, std::back_inserter(out), std::less<>{},
        &rtts::test_item::key, &rtts::test_item::key);
    RTTS_EQUAL(out.size(), size_t(4));
    RTTS_EQUAL(out[1].payload, 11);

    const std::vector<int> v1{ 1, 3, 5 };
    const std::vector<int> v2{ 2, 3, 6 };
    rtts::single_pass_input_range<int> sp1(v1);
    rtts::single_pass_input_range<int> sp2(v2);
    std::vector<int> o2;
    raze::algorithm::set_union(sp1, sp2, std::back_inserter(o2));
    RTTS_EXPECT((o2 == std::vector<int>{ 1, 2, 3, 5, 6 }));

    std::vector<int> o3;
    auto dr = raze::algorithm::set_union(std::vector<int>{ 1, 2 }, std::vector<int>{ 2, 3 }, std::back_inserter(o3));
    static_assert(std::same_as<decltype(dr.in1), std::ranges::dangling>);
    static_assert(std::same_as<decltype(dr.in2), std::ranges::dangling>);
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void set_union_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n1 = static_cast<size_t>(rng() % 257);
            const size_t n2 = static_cast<size_t>(rng() % 257);
            std::vector<T> a(n1), b(n2);
            if ((it % 3) == 0) {
                a = rtts::random::vector<T>(n1, rng());
                b = rtts::random::vector<T>(n2, rng());
            }
            else {
                const unsigned alpha = (it % 3 == 1) ? 6u : 16u;
                for (size_t i = 0; i < n1; ++i) a[i] = static_cast<T>(rng() % alpha);
                for (size_t i = 0; i < n2; ++i) b[i] = static_cast<T>(rng() % alpha);
            }

            const bool desc = (it % 3 == 2);
            auto run_check = [&](auto comp) {
                std::ranges::sort(a, comp);
                std::ranges::sort(b, comp);

                std::vector<T> exp;
                std::ranges::set_union(a, b, std::back_inserter(exp), comp);

                auto verify_buf = [&](auto fn) {
                    std::vector<T> out(n1 + n2);
                    auto r = fn(out.begin());
                    RTTS_EXPECT(r.in1 == a.end() && r.in2 == b.end());
                    RTTS_EXPECT(static_cast<size_t>(r.out - out.begin()) == exp.size());
                    RTTS_EXPECT(std::equal(out.begin(), r.out, exp.begin()));
                };

                verify_buf([&](auto o) { return raze::algorithm::set_union(a, b, o, comp); });
                verify_buf([&](auto o) { return raze::algorithm::set_union(a.begin(), a.end(), b.begin(), b.end(), o, comp); });
                verify_buf([&](auto o) { return raze::algorithm::set_union[opt::fscalar](a, b, o, comp); });
                verify_buf([&](auto o) { return raze::algorithm::set_union[opt::fstatic](a, b, o, comp); });

                if (it % 8 == 0 && n1 <= 64 && n2 <= 64) {
                    rtts::single_pass_input_range<T> sp1(a);
                    rtts::single_pass_input_range<T> sp2(b);
                    std::vector<T> out;
                    raze::algorithm::set_union(sp1, sp2, std::back_inserter(out), comp);
                    RTTS_ALL_EQUAL(out, exp);
                }
            };

            if (desc) run_check(std::greater<>{});
            else      run_check(std::less<>{});
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::set_union fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        set_union_fuzz<T>(seed, 35);
};
