#include <tests/rts/rts.h>
#include <raze/algorithm/replace/Replace.h>

RTTS_CASE_TPL("raze::algorithm::replace", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    using raze::algorithm::replace;
    namespace opt = raze::options;

    auto cfg = rtts::algorithm::config::thorough();

    auto check = [&](auto algo) {
        {
            const T old_val = rtts::random::generator<T>(42)();
            const T new_val = rtts::random::generator<T>(1000)();

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, old_val, new_val](auto c) {
                    return algo(c.first(), c.last(), old_val, new_val);
                },
                [old_val, new_val](auto c) {
                    return std::ranges::replace(c.first(), c.last(), old_val, new_val);
                });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, old_val, new_val](auto c) {
                    return algo(c.range(), old_val, new_val);
                },
                [old_val, new_val](auto c) {
                    return std::ranges::replace(c.range(), old_val, new_val);
                });
        }

        {
            const T val = rtts::random::generator<T>(777)();

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, val](auto c) { return algo(c.first(), c.last(), val, val); },
                [val](auto c) { return std::ranges::replace(c.first(), c.last(), val, val); });

            rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
                [algo, val](auto c) { return algo(c.range(), val, val); },
                [val](auto c) { return std::ranges::replace(c.range(), val, val); });
        }
    };

    check(replace);
    check(replace[opt::fstatic]);
    check(replace[opt::fscalar]);
    check(replace[opt::fautovec]);
    check(replace[opt::unroll<1>]);
    check(replace[opt::unroll<2>]);

    {
        auto acfg = rtts::algorithm::config::compact();
        const T old_val = rtts::random::generator<T>(42)();
        const T new_val = rtts::random::generator<T>(1000)();

        auto check_aligned = [&]<size_t Align>(auto algo) {
            rtts::algorithm::run_aligned<Align, rtts::algorithm::range, T>(acfg,
                [algo, old_val, new_val](auto c) { return algo(c.first(), c.last(), old_val, new_val); },
                [old_val, new_val](auto c) { return std::ranges::replace(c.first(), c.last(), old_val, new_val); });

            rtts::algorithm::run_aligned<Align, rtts::algorithm::range, T>(acfg,
                [algo, old_val, new_val](auto c) { return algo(c.range(), old_val, new_val); },
                [old_val, new_val](auto c) { return std::ranges::replace(c.range(), old_val, new_val); });
        };

        check_aligned.template operator()<16>(replace[opt::aligned<16>]);
        check_aligned.template operator()<32>(replace[opt::aligned<32>]);
        check_aligned.template operator()<32>(replace[opt::aligned<32>][opt::fstatic]);
        check_aligned.template operator()<32>(replace[opt::aligned<32>][opt::unroll<2>]);
        check_aligned.template operator()<64>(replace[opt::aligned<64>]);
        check_aligned.template operator()<64>(replace[opt::aligned<64>][opt::fautovec]);
        check_aligned.template operator()<64>(replace[opt::aligned<64>][opt::fscalar]);
        check_aligned.template operator()<1>(replace[opt::aligned<opt::unaligned>]);
        check_aligned.template operator()<1>(replace[opt::aligned<64>][opt::aligned<opt::unaligned>]);
    }

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr { return raze::algorithm::replace(c.first(), c.last(), T(42), T(99)); },
        [](auto c) constexpr { return std::ranges::replace(c.first(), c.last(), T(42), T(99)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace[raze::options::fstatic](c.first(), c.last(), T(42), T(99));
        },
        [](auto c) constexpr { return std::ranges::replace(c.first(), c.last(), T(42), T(99)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace[raze::options::fscalar](c.range(), T(42), T(99));
        },
        [](auto c) constexpr { return std::ranges::replace(c.range(), T(42), T(99)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace[raze::options::fautovec](c.range(), T(42), T(99));
        },
        [](auto c) constexpr { return std::ranges::replace(c.range(), T(42), T(99)); }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace[raze::options::aligned<64>](c.range(), T(42), T(99));
        },
        [](auto c) constexpr { return std::ranges::replace(c.range(), T(42), T(99)); }));
};

RTTS_CASE_TPL("raze::algorithm::replace.projection", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    using raze::algorithm::replace;
    namespace opt = raze::options;

    auto cfg = rtts::algorithm::config::thorough();
    auto acfg = rtts::algorithm::config::compact();

    const T old_val = rtts::random::generator<T>(12345)();
    const T new_val = rtts::random::generator<T>(67890)();
    auto proj = [](const T& x) -> const T& { return x; };

    auto check = [&](auto algo) {
        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [algo, old_val, new_val, proj](auto c) {
                return algo(c.first(), c.last(), old_val, new_val, proj);
            },
            [old_val, new_val, proj](auto c) {
                return std::ranges::replace(c.first(), c.last(), old_val, new_val, proj);
            });

        rtts::algorithm::run<rtts::algorithm::range, T>(cfg,
            [algo, old_val, new_val, proj](auto c) {
                return algo(c.range(), old_val, new_val, proj);
            },
            [old_val, new_val, proj](auto c) {
                return std::ranges::replace(c.range(), old_val, new_val, proj);
            });
    };

    check(replace);
    check(replace[opt::fstatic]);
    check(replace[opt::fscalar]);
    check(replace[opt::fautovec]);

    rtts::algorithm::run_aligned<64, rtts::algorithm::range, T>(acfg,
        [old_val, new_val, proj](auto c) { return replace[opt::aligned<64>](c.range(), old_val, new_val, proj); },
        [old_val, new_val, proj](auto c) { return std::ranges::replace(c.range(), old_val, new_val, proj); });

    rtts::algorithm::run_aligned<1, rtts::algorithm::range, T>(acfg,
        [old_val, new_val, proj](auto c) { return replace[opt::aligned<opt::unaligned>](c.range(), old_val, new_val, proj); },
        [old_val, new_val, proj](auto c) { return std::ranges::replace(c.range(), old_val, new_val, proj); });

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace(c.first(), c.last(), T(42), T(99),
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::replace(c.first(), c.last(), T(42), T(99),
                [](const T& x) constexpr -> const T& { return x; });
        }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace[raze::options::fscalar](c.range(), T(42), T(99),
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::replace(c.range(), T(42), T(99),
                [](const T& x) constexpr -> const T& { return x; });
        }));

    static_assert(rtts::algorithm::constexpr_run<T, rtts::algorithm::range, 8>(
        [](auto c) constexpr {
            return raze::algorithm::replace[raze::options::fautovec][raze::options::aligned<64>](c.range(), T(42), T(99),
                [](const T& x) constexpr -> const T& { return x; });
        },
        [](auto c) constexpr {
            return std::ranges::replace(c.range(), T(42), T(99),
                [](const T& x) constexpr -> const T& { return x; });
        }));
};


RTTS_CASE("raze::algorithm::replace views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Writable views: take, drop, counted, span, reverse, filter, non-common
    {
        std::vector<int> v1 = { 1, 2, 3, 2, 4, 2, 5, 2, 6, 2, 7, 2, 8, 2, 9, 2, 10, 2, 11, 2 };
        auto v2 = v1;

        raze::algorithm::replace(v1 | views::drop(2) | views::take(12), 2, 99);
        std::ranges::replace(v2 | views::drop(2) | views::take(12), 2, 99);
        RTTS_EXPECT(v1 == v2);

        raze::algorithm::replace[opt::fautovec][opt::fstatic](views::counted(v1.begin(), 10), 99, 77);
        std::ranges::replace(views::counted(v2.begin(), 10), 99, 77);
        RTTS_EXPECT(v1 == v2);

        raze::algorithm::replace(v1 | views::reverse, 2, 55);
        std::ranges::replace(v2 | views::reverse, 2, 55);
        RTTS_EXPECT(v1 == v2);

        auto is_big = [](int x) { return x > 10; };
        raze::algorithm::replace[opt::aligned<64>](v1 | views::filter(is_big), 77, 88);
        std::ranges::replace(v2 | views::filter(is_big), 77, 88);
        RTTS_EXPECT(v1 == v2);

        raze::algorithm::replace[opt::fstatic](rtts::make_non_common(v1), 55, 66);
        std::ranges::replace(rtts::make_non_common(v2), 55, 66);
        RTTS_EXPECT(v1 == v2);
    }

    // Empty, 1-elem, volatile, pointer-to-member projection, constexpr views
    {
        std::vector<int> empty_v;
        raze::algorithm::replace(empty_v, 1, 2);
        RTTS_EXPECT(empty_v.empty());

        std::vector<int> one_v = { 5 };
        raze::algorithm::replace(one_v, 5, 9);
        RTTS_EXPECT(one_v[0] == 9);

        volatile int vbuf[16];
        for (int i = 0; i < 16; ++i) vbuf[i] = (i % 3 == 0) ? 7 : i;
        raze::algorithm::replace(vbuf, vbuf + 16, 7, 42);
        for (int i = 0; i < 16; ++i) RTTS_EXPECT(vbuf[i] == ((i % 3 == 0) ? 42 : i));

        std::vector<rtts::test_item> items = { { 1, 10 }, { 2, 20 }, { 1, 30 } };
        raze::algorithm::replace(items, 1, rtts::test_item{ 9, 90 }, &rtts::test_item::key);
        RTTS_EXPECT(items[0].key == 9 && items[1].key == 2 && items[2].key == 9);
    }

    static_assert([]() constexpr {
        std::array<int, 6> a = { 1, 2, 3, 2, 4, 2 };
        raze::algorithm::replace[opt::fautovec][opt::fstatic](a | views::drop(1) | views::take(4), 2, 9);
        return a[0] == 1 && a[1] == 9 && a[3] == 9 && a[5] == 2;
    }());
};