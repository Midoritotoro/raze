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
    check(replace_if[opt::unroll<1>]);
    check(replace_if[opt::unroll<2>]);

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
};

RTTS_CASE_TPL("raze::algorithm::replace_if.projection", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    using raze::algorithm::replace_if;
    namespace opt = raze::options;

    auto cfg = rtts::algorithm::config::thorough();

    const T threshold = rtts::random::generator<T>(12345)();
    const T new_val = rtts::random::generator<T>(67890)();

    auto pred = [threshold](const T& x) { return x > threshold; };
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
};