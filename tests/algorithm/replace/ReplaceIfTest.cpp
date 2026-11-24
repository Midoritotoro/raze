#include <tests/rts/rts.h>
#include <raze/algorithm/replace/ReplaceIf.h>

RTTS_CASE_TPL("raze::algorithm::replace_if", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    using raze::algorithm::replace_if;
    namespace opt = raze::options;

    auto cfg = rtts::algorithm::config::thorough();

    auto check = [&](auto algo) {
        {
            const T threshold = rtts::random::generator<T>(1000)();
            const T new_val = rtts::random::generator<T>(2000)();
            auto pred = [threshold](const T& x) { return x > threshold; };

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, pred, new_val](auto c) { return algo(c.first(), c.last(), pred, new_val); },
                [pred, new_val](auto c) { return std::ranges::replace_if(c.first(), c.last(), pred, new_val); });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, pred, new_val](auto c) { return algo(c.range(), pred, new_val); },
                [pred, new_val](auto c) { return std::ranges::replace_if(c.range(), pred, new_val); });
        }

        {
            const T target = rtts::random::generator<T>(4000)();
            const T new_val = rtts::random::generator<T>(5000)();
            auto pred = [target](const T& x) { return x == target; };

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, pred, new_val](auto c) { return algo(c.first(), c.last(), pred, new_val); },
                [pred, new_val](auto c) { return std::ranges::replace_if(c.first(), c.last(), pred, new_val); });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, pred, new_val](auto c) { return algo(c.range(), pred, new_val); },
                [pred, new_val](auto c) { return std::ranges::replace_if(c.range(), pred, new_val); });
        }

        {
            const T new_val = rtts::random::generator<T>(6000)();
            auto always_false = [](const T&) { return false; };

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, always_false, new_val](auto c) { return algo(c.first(), c.last(), always_false, new_val); },
                [always_false, new_val](auto c) { return std::ranges::replace_if(c.first(), c.last(), always_false, new_val); });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, always_false, new_val](auto c) { return algo(c.range(), always_false, new_val); },
                [always_false, new_val](auto c) { return std::ranges::replace_if(c.range(), always_false, new_val); });
        }

        {
            const T new_val = rtts::random::generator<T>(7000)();
            auto always_true = [](const T&) { return true; };

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, always_true, new_val](auto c) { return algo(c.first(), c.last(), always_true, new_val); },
                [always_true, new_val](auto c) { return std::ranges::replace_if(c.first(), c.last(), always_true, new_val); });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, always_true, new_val](auto c) { return algo(c.range(), always_true, new_val); },
                [always_true, new_val](auto c) { return std::ranges::replace_if(c.range(), always_true, new_val); });
        }
    };

    check(replace_if);
    check(replace_if[opt::fstatic]);
    check(replace_if[opt::fscalar]);
    check(replace_if[opt::fautovec]);
    check(replace_if[opt::unroll<1>]);
    check(replace_if[opt::unroll<2>]);

    {
        auto acfg = rtts::algorithm::config::compact();
        const T threshold = rtts::random::generator<T>(1000)();
        const T new_val = rtts::random::generator<T>(2000)();
        auto pred_scalar = [threshold](const T& x) { return x > threshold; };
        auto pred_vector = [threshold](auto x) { return x > threshold; };
        auto pred_branching = [threshold](auto x) { if (x > threshold) return true; return false; };

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_branching, new_val](auto c) { return replace_if[opt::fautovec](c.range(), pred_branching, new_val); },
            [pred_branching, new_val](auto c) { return std::ranges::replace_if(c.range(), pred_branching, new_val); });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [pred_branching, new_val](auto c) { return replace_if[opt::fautovec][opt::fstatic](c.range(), pred_branching, new_val); },
            [pred_branching, new_val](auto c) { return std::ranges::replace_if(c.range(), pred_branching, new_val); });

        auto check_aligned = [&]<size_t Align>(auto algo, auto pred) {
            rtts::algorithm::run_aligned<Align, rtts::algorithm::range, T>(acfg,
                [algo, pred, new_val](auto c) { return algo(c.first(), c.last(), pred, new_val); },
                [pred, new_val](auto c) { return std::ranges::replace_if(c.first(), c.last(), pred, new_val); });

            rtts::algorithm::run_aligned<Align, rtts::algorithm::range, T>(acfg,
                [algo, pred, new_val](auto c) { return algo(c.range(), pred, new_val); },
                [pred, new_val](auto c) { return std::ranges::replace_if(c.range(), pred, new_val); });
        };

        check_aligned.template operator()<16>(replace_if[opt::aligned<16>], pred_vector);
        check_aligned.template operator()<32>(replace_if[opt::aligned<32>], pred_vector);
        check_aligned.template operator()<32>(replace_if[opt::aligned<32>][opt::fstatic], pred_vector);
        check_aligned.template operator()<32>(replace_if[opt::aligned<32>][opt::unroll<2>], pred_vector);
        check_aligned.template operator()<64>(replace_if[opt::aligned<64>], pred_vector);
        check_aligned.template operator()<64>(replace_if[opt::aligned<64>], pred_scalar);
        check_aligned.template operator()<64>(replace_if[opt::aligned<64>][opt::fautovec], pred_branching);
        check_aligned.template operator()<64>(replace_if[opt::aligned<64>][opt::fscalar], pred_vector);
        check_aligned.template operator()<1>(replace_if[opt::aligned<opt::unaligned>], pred_vector);
        check_aligned.template operator()<1>(replace_if[opt::aligned<64>][opt::aligned<opt::unaligned>], pred_vector);
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::replace_if(c.first(), c.last(), [](const T& x) constexpr { return x > T(0); }, T(99)); },
        [](auto c) constexpr { return std::ranges::replace_if(c.first(), c.last(), [](const T& x) constexpr { return x > T(0); }, T(99)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace_if[raze::options::fstatic](c.first(), c.last(), [](const T& x) constexpr { return x > T(0); }, T(99));
        },
        [](auto c) constexpr { return std::ranges::replace_if(c.first(), c.last(), [](const T& x) constexpr { return x > T(0); }, T(99)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace_if[raze::options::fscalar](c.range(), [](const T& x) constexpr { return x > T(0); }, T(99));
        },
        [](auto c) constexpr { return std::ranges::replace_if(c.range(), [](const T& x) constexpr { return x > T(0); }, T(99)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace_if[raze::options::fautovec](c.range(), [](auto x) constexpr { if (x > T(2)) return true; return false; }, T(99));
        },
        [](auto c) constexpr { return std::ranges::replace_if(c.range(), [](auto x) constexpr { if (x > T(2)) return true; return false; }, T(99)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace_if[raze::options::aligned<64>](c.range(), [](auto x) constexpr { return x > T(0); }, T(99));
        },
        [](auto c) constexpr { return std::ranges::replace_if(c.range(), [](auto x) constexpr { return x > T(0); }, T(99)); }));
};

RTTS_CASE_TPL("raze::algorithm::replace_if.projection", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    using raze::algorithm::replace_if;
    namespace opt = raze::options;

    auto cfg = rtts::algorithm::config::thorough();
    auto acfg = rtts::algorithm::config::compact();

    const T threshold = rtts::random::generator<T>(12345)();
    const T new_val = rtts::random::generator<T>(67890)();

    auto pred = [threshold](const T& x) { return x > threshold; };
    auto pred_branching = [threshold](auto x) { if (x > threshold) return true; return false; };
    auto proj = [](const T& x) -> const T& { return x; };

    auto check = [&](auto algo) {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [algo, pred, new_val, proj](auto c) {
                return algo(c.first(), c.last(), pred, new_val, proj);
            },
            [pred, new_val, proj](auto c) {
                return std::ranges::replace_if(c.first(), c.last(), pred, new_val, proj);
            });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [algo, pred, new_val, proj](auto c) {
                return algo(c.range(), pred, new_val, proj);
            },
            [pred, new_val, proj](auto c) {
                return std::ranges::replace_if(c.range(), pred, new_val, proj);
            });
    };

    check(replace_if);
    check(replace_if[opt::fstatic]);
    check(replace_if[opt::fscalar]);
    check(replace_if[opt::fautovec]);

    rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
        [pred_branching, new_val, proj](auto c) { return replace_if[opt::fautovec](c.range(), pred_branching, new_val, proj); },
        [pred_branching, new_val, proj](auto c) { return std::ranges::replace_if(c.range(), pred_branching, new_val, proj); });

    rtts::algorithm::run_aligned<64, rtts::algorithm::range, T>(acfg,
        [pred, new_val, proj](auto c) { return replace_if[opt::aligned<64>](c.range(), pred, new_val, proj); },
        [pred, new_val, proj](auto c) { return std::ranges::replace_if(c.range(), pred, new_val, proj); });

    rtts::algorithm::run_aligned<1, rtts::algorithm::range, T>(acfg,
        [pred, new_val, proj](auto c) { return replace_if[opt::aligned<opt::unaligned>](c.range(), pred, new_val, proj); },
        [pred, new_val, proj](auto c) { return std::ranges::replace_if(c.range(), pred, new_val, proj); });

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace_if(c.first(), c.last(),
                [](const T& x) constexpr { return x > T(0); }, T(99),
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::replace_if(c.first(), c.last(),
                [](const T& x) constexpr { return x > T(0); }, T(99),
                [](const T& x) constexpr -> const T& { return x; });
        }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace_if[raze::options::fscalar](c.range(),
                [](const T& x) constexpr { return x > T(0); }, T(99),
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::replace_if(c.range(),
                [](const T& x) constexpr { return x > T(0); }, T(99),
                [](const T& x) constexpr -> const T& { return x; });
        }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace_if[raze::options::fautovec][raze::options::aligned<64>](c.range(),
                [](auto x) constexpr { if (x > T(2)) return true; return false; }, T(99),
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::replace_if(c.range(),
                [](auto x) constexpr { if (x > T(2)) return true; return false; }, T(99),
                [](const T& x) constexpr -> const T& { return x; });
        }));
};


RTTS_CASE("raze::algorithm::replace_if views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Writable views: take, drop, counted, reverse, filter, non-common
    {
        std::vector<int> v1(40);
        std::iota(v1.begin(), v1.end(), -10);
        auto v2 = v1;

        auto pred_branch = [](auto x) { if (x < 0) return true; return false; };
        raze::algorithm::replace_if[opt::fautovec][opt::fstatic](v1 | views::drop(2) | views::take(20), pred_branch, 0);
        std::ranges::replace_if(v2 | views::drop(2) | views::take(20), pred_branch, 0);
        RTTS_EXPECT(v1 == v2);

        raze::algorithm::replace_if(v1 | views::reverse, [](auto x) { return x > 15; }, 100);
        std::ranges::replace_if(v2 | views::reverse, [](auto x) { return x > 15; }, 100);
        RTTS_EXPECT(v1 == v2);

        auto is_even = [](int x) { return (x % 2) == 0; };
        raze::algorithm::replace_if[opt::aligned<64>](v1 | views::filter(is_even), [](int x) { return x > 0; }, 77);
        std::ranges::replace_if(v2 | views::filter(is_even), [](int x) { return x > 0; }, 77);
        RTTS_EXPECT(v1 == v2);

        raze::algorithm::replace_if[opt::fstatic](rtts::make_non_common(v1), [](int x) { return x == 77; }, 88);
        std::ranges::replace_if(rtts::make_non_common(v2), [](int x) { return x == 77; }, 88);
        RTTS_EXPECT(v1 == v2);
    }

    // Empty, 1-elem, volatile, proxy references (vector<bool>), member projection
    {
        std::vector<int> empty_v;
        raze::algorithm::replace_if(empty_v, [](auto x) { return x > 0; }, 1);
        RTTS_EXPECT(empty_v.empty());

        std::vector<int> one_v = { -5 };
        raze::algorithm::replace_if(one_v, [](auto x) { return x < 0; }, 5);
        RTTS_EXPECT(one_v[0] == 5);

        volatile int vbuf[16];
        for (int i = 0; i < 16; ++i) vbuf[i] = i - 8;
        raze::algorithm::replace_if(vbuf, vbuf + 16, [](int x) { return x < 0; }, 0);
        for (int i = 0; i < 16; ++i) RTTS_EXPECT(vbuf[i] == (i < 8 ? 0 : i - 8));

        std::vector<bool> bits = { true, false, true, false, false };
        raze::algorithm::replace_if(bits, [](bool b) { return !b; }, true);
        for (bool b : bits) RTTS_EXPECT(b);

        std::vector<rtts::test_item> items = { { 1, 10 }, { 5, 50 }, { 2, 20 }, { 7, 70 } };
        raze::algorithm::replace_if(items, [](int k) { return k >= 5; }, rtts::test_item{ 0, 0 }, &rtts::test_item::key);
        RTTS_EXPECT(items[0].key == 1 && items[1].key == 0 && items[2].key == 2 && items[3].key == 0);
    }

    static_assert([]() constexpr {
        std::array<int, 6> a = { -2, -1, 0, 1, 2, 3 };
        raze::algorithm::replace_if[opt::fautovec][opt::fstatic](
            a | views::drop(1) | views::take(4), [](auto x) { if (x <= 0) return true; return false; }, 9);
        return a[0] == -2 && a[1] == 9 && a[2] == 9 && a[3] == 1 && a[5] == 3;
    }());
};