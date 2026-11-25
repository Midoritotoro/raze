#include <tests/rts/rts.h>
#include <raze/algorithm/extreme/MinmaxElement.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <random>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
    template <class It>
    constexpr auto to_offsets(It begin, std::ranges::minmax_element_result<It> res) {
        return std::pair{ std::ranges::distance(begin, res.min), std::ranges::distance(begin, res.max) };
    }
}

RTTS_CASE_TPL("raze::algorithm::minmax_element", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::thorough();

    run<range, T>(cfg,
        [](auto c) { return to_offsets(c.first(), raze::algorithm::minmax_element(c.first(), c.last())); },
        [](auto c) { return to_offsets(c.first(), std::ranges::minmax_element(c.first(), c.last())); });

    run<range, T>(cfg,
        [](auto c) { return to_offsets(c.first(), raze::algorithm::minmax_element(c.range())); },
        [](auto c) { return to_offsets(c.first(), std::ranges::minmax_element(c.range())); });

    run<range, T>(cfg,
        [](auto c) { return to_offsets(c.first(), raze::algorithm::minmax_element[opt::fscalar](c.first(), c.last())); },
        [](auto c) { return to_offsets(c.first(), std::ranges::minmax_element(c.first(), c.last())); });

    run<range, T>(cfg,
        [](auto c) { return to_offsets(c.first(), raze::algorithm::minmax_element[opt::unroll<2>](c.range())); },
        [](auto c) { return to_offsets(c.first(), std::ranges::minmax_element(c.range())); });

    run<range, T>(cfg,
        [](auto c) { return to_offsets(c.first(), raze::algorithm::minmax_element[opt::fstatic](c.range())); },
        [](auto c) { return to_offsets(c.first(), std::ranges::minmax_element(c.range())); });

    run<range, T>(cfg,
        [](auto c) { return to_offsets(c.first(), raze::algorithm::minmax_element(c.range(), std::greater<>{})); },
        [](auto c) { return to_offsets(c.first(), std::ranges::minmax_element(c.range(), std::greater<>{})); });

    run<range, T>(cfg,
        [](auto c) { return to_offsets(c.first(), raze::algorithm::minmax_element(c.range(), std::ranges::less{})); },
        [](auto c) { return to_offsets(c.first(), std::ranges::minmax_element(c.range(), std::ranges::less{})); });

    run<range, T>(cfg,
        [](auto c) { return to_offsets(c.first(), raze::algorithm::minmax_element(c.range(), std::less<T>{})); },
        [](auto c) { return to_offsets(c.first(), std::ranges::minmax_element(c.range(), std::less<T>{})); });

    auto cmp_branch = [](auto a, auto b) { if (a < b) return true; return false; };
    run<range, T>(cfg,
        [cmp_branch](auto c) { return to_offsets(c.first(), raze::algorithm::minmax_element[opt::fautovec](c.range(), cmp_branch)); },
        [cmp_branch](auto c) { return to_offsets(c.first(), std::ranges::minmax_element(c.range(), cmp_branch)); });

    {
        auto acfg = config::compact();
        auto check_aligned = [&]<size_t Align>(auto algo) {
            run_aligned<Align, range, T>(acfg,
                [algo](auto c) { return to_offsets(c.first(), algo(c.first(), c.last())); },
                [](auto c) { return to_offsets(c.first(), std::ranges::minmax_element(c.first(), c.last())); });
            run_aligned<Align, range, T>(acfg,
                [algo](auto c) { return to_offsets(c.first(), algo(c.range())); },
                [](auto c) { return to_offsets(c.first(), std::ranges::minmax_element(c.range())); });
        };

        check_aligned.template operator()<16>(raze::algorithm::minmax_element[opt::aligned<16>]);
        check_aligned.template operator()<32>(raze::algorithm::minmax_element[opt::aligned<32>]);
        check_aligned.template operator()<32>(raze::algorithm::minmax_element[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<32>(raze::algorithm::minmax_element[opt::aligned<32>][opt::fstatic]);
        check_aligned.template operator()<64>(raze::algorithm::minmax_element[opt::aligned<64>]);
        check_aligned.template operator()<64>(raze::algorithm::minmax_element[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1>(raze::algorithm::minmax_element[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1>(raze::algorithm::minmax_element[opt::aligned<64>][opt::aligned<opt::unaligned>]);
    }

    {
        for (size_t len : { size_t(1), size_t(2), size_t(7), size_t(16), size_t(31), size_t(32), size_t(64), size_t(65), size_t(130) }) {
            std::vector<T> eq(len, T(7));
            auto req = raze::algorithm::minmax_element(eq);
            RTTS_EQUAL(static_cast<size_t>(req.min - eq.begin()), size_t(0));
            RTTS_EQUAL(static_cast<size_t>(req.max - eq.begin()), len - 1);

            std::vector<T> v0(len, T(10));
            v0[0] = T(99);
            if (len > 2) v0[2] = T(99);
            auto rv0 = raze::algorithm::minmax_element(v0);
            auto ev0 = std::ranges::minmax_element(v0);
            RTTS_EQUAL(rv0.min - v0.begin(), ev0.min - v0.begin());
            RTTS_EQUAL(rv0.max - v0.begin(), ev0.max - v0.begin());

            if (len >= 4) {
                std::vector<T> vm(len, T(50));
                vm[1] = T(1);
                vm[len - 2] = T(1);
                vm[0] = T(100);
                vm[len - 1] = T(100);
                auto rvm = raze::algorithm::minmax_element(vm);
                auto evm = std::ranges::minmax_element(vm);
                RTTS_EQUAL(rvm.min - vm.begin(), evm.min - vm.begin());
                RTTS_EQUAL(rvm.max - vm.begin(), evm.max - vm.begin());
            }
        }
    }

    {
        auto custom_vec_cmp = [](auto a, auto b) { return a > b; };
        auto v = rtts::random::vector<T>(300, 33333u);
        auto got = raze::algorithm::minmax_element(v, custom_vec_cmp);
        auto exp = std::ranges::minmax_element(v, custom_vec_cmp);
        RTTS_EQUAL(got.min - v.begin(), exp.min - v.begin());
        RTTS_EQUAL(got.max - v.begin(), exp.max - v.begin());
    }

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return to_offsets(c.first(), raze::algorithm::minmax_element(c.first(), c.last())); },
        [](auto c) constexpr { return to_offsets(c.first(), std::ranges::minmax_element(c.first(), c.last())); }));

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return to_offsets(c.first(), raze::algorithm::minmax_element[opt::fscalar](c.range())); },
        [](auto c) constexpr { return to_offsets(c.first(), std::ranges::minmax_element(c.range())); }));

    static_assert(constexpr_run<T, range, 8>(
        [](auto c) constexpr { return to_offsets(c.first(), raze::algorithm::minmax_element[opt::aligned<64>](c.range())); },
        [](auto c) constexpr { return to_offsets(c.first(), std::ranges::minmax_element(c.range())); }));
};

RTTS_CASE("raze::algorithm::minmax_element chunking, signed zero, views, dangling, and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    {
        const size_t N = 20000;
        for (size_t pos : { size_t(0), size_t(253 * 16), size_t(254 * 16), size_t(254 * 32),
                            size_t(254 * 64 - 1), size_t(254 * 64), size_t(18765), N - 1 })
        {
            std::vector<uint8_t> v(N, 100);
            v[pos] = 2;
            if (pos + 50 < N) v[pos + 50] = 2;     
            if (pos >= 50) v[pos - 50] = 250;      
            const size_t max_pos = (pos + 1 < N) ? (pos + 1) : pos;
            if (max_pos != pos) {
                v[max_pos] = 250;                  
            }
            auto got = raze::algorithm::minmax_element(v);
            auto exp = std::ranges::minmax_element(v);
            RTTS_EQUAL(got.min - v.begin(), exp.min - v.begin());
            RTTS_EQUAL(got.max - v.begin(), exp.max - v.begin());
        }
    }

    {
        std::vector<float> vf(64, -0.0f);
        vf[63] = +0.0f;
        auto rf = raze::algorithm::minmax_element(vf);
        RTTS_EQUAL(rf.min - vf.begin(), 0);
        RTTS_EQUAL(rf.max - vf.begin(), 63);

        std::vector<double> vd(64, 1.0);
        vd[7] = -0.0;
        vd[55] = +0.0;
        auto rd = raze::algorithm::minmax_element(vd);
        RTTS_EQUAL(rd.min - vd.begin(), 7);
        RTTS_EQUAL(rd.max - vd.begin(), 63);  
    }

    {
        const std::vector<rtts::test_item> items = {
            { 2, 1 }, { 9, 10 }, { 2, 2 }, { 9, 20 }, { 5, 3 }
        };
        auto r = raze::algorithm::minmax_element(items, std::less<>{}, &rtts::test_item::key);
        RTTS_EQUAL(r.min - items.begin(), 0);  
        RTTS_EQUAL(r.max - items.begin(), 3);  

        const std::vector<int> src = { 9, 2, 7, 1, 8, 1, 9, 4 };
        auto fview = src | views::filter([](int x) { return x > 1 && x < 9; });
        auto rf = raze::algorithm::minmax_element(fview);
        RTTS_EQUAL(*rf.min, 2);
        RTTS_EQUAL(*rf.max, 8);

        auto rview = src | views::reverse;
        auto rr = raze::algorithm::minmax_element(rview);
        RTTS_EQUAL(rr.min - rview.begin(), 2);  
        RTTS_EQUAL(rr.max - rview.begin(), 7);  

        auto nc = rtts::make_non_common(src);
        auto rnc = raze::algorithm::minmax_element[opt::fstatic](nc);
        RTTS_EQUAL(rnc.min - src.begin(), 3);
        RTTS_EQUAL(rnc.max - src.begin(), 6);

        auto d = raze::algorithm::minmax_element(std::vector<int>{ 3, 1, 2 });
        static_assert(std::same_as<decltype(d), std::ranges::minmax_element_result<std::ranges::dangling>>);
        (void)d;

        std::vector<int> empty_v;
        auto rempty = raze::algorithm::minmax_element(empty_v);
        RTTS_EXPECT(rempty.min == empty_v.end() && rempty.max == empty_v.end());

        volatile int vbuf[16] = { 10, -5, 99, 7, 6, -5, 4, 3, 2, 1, 0, 99, 11, 12, 13, 14 };
        auto rv = raze::algorithm::minmax_element(vbuf, vbuf + 16);
        RTTS_EQUAL(rv.min - vbuf, 1);
        RTTS_EQUAL(rv.max - vbuf, 11);
    }
};

