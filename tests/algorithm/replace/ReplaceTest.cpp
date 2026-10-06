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
    check(replace[opt::unroll<1>]);
    check(replace[opt::unroll<2>]);

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
};

RTTS_CASE_TPL("raze::algorithm::replace.projection", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    using raze::algorithm::replace;
    namespace opt = raze::options;

    auto cfg = rtts::algorithm::config::thorough();

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
};