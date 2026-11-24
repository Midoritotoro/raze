#include <tests/rts/rts.h>
#include <raze/algorithm/find/FindLastIf.h>
#include <src/raze/algorithm/EqualTo.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <forward_list>
#include <iterator>
#include <list>
#include <random>
#include <ranges>
#include <string>
#include <vector>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    struct find_last_if_item {
        int32_t key;
        int32_t value;
        bool operator==(const find_last_if_item&) const = default;
    };

    template <class T>
    struct equal_pred {
        T target;
        constexpr auto operator()(auto x) const { return x == target; }
    };

    struct false_pred {
        constexpr bool operator()(auto) const { return false; }
    };

    struct true_pred {
        constexpr bool operator()(auto) const { return true; }
    };

    template <std::forward_iterator It, std::sentinel_for<It> Sent, class Pred>
    constexpr std::ranges::subrange<It> ref_find_last_if(It first, Sent last, Pred pred) {
#if defined(__cpp_lib_ranges_find_last) && __cpp_lib_ranges_find_last >= 202207L
        return std::ranges::find_last_if(std::move(first), std::move(last), std::move(pred));
#else
        auto found = first;
        bool has_match = false;
        for (; first != last; ++first) {
            if (pred(*first)) {
                found = first;
                has_match = true;
            }
        }
        return { has_match ? found : first, first };
#endif
    }

    template <std::ranges::forward_range R, class Pred>
    constexpr std::ranges::borrowed_subrange_t<R> ref_find_last_if(R&& r, Pred pred) {
        return ref_find_last_if(std::ranges::begin(r), std::ranges::end(r), std::move(pred));
    }

    template <class SubA, class SubB>
    constexpr bool subrange_eq(const SubA& a, const SubB& b) {
        return a.begin() == b.begin() && a.end() == b.end();
    }

    template <class T>
    void check_find_last_if_variants(const std::vector<T>& input, T target) {
        auto pred = [target](auto x) { return x == target; };
        auto scalar_pred = [target](T x) { return x == target; };
        auto vec_pred = raze::algorithm::equal_to(target);

        auto exp = ref_find_last_if(input, pred);
        const auto exp_off = exp.begin() - input.begin();
        const auto exp_end_off = exp.end() - input.begin();

        auto check = [&](auto sub) {
            RTTS_EXPECT(sub.begin() - input.begin() == exp_off);
            RTTS_EXPECT(sub.end() - input.begin() == exp_end_off);
        };

        check(raze::algorithm::find_last_if(input, pred));
        check(raze::algorithm::find_last_if(input.begin(), input.end(), pred));
        check(raze::algorithm::find_last_if(input, scalar_pred));
        check(raze::algorithm::find_last_if[raze::options::fscalar](input, pred));
        check(raze::algorithm::find_last_if[raze::options::unroll<2>](input, pred));
        check(raze::algorithm::find_last_if[raze::options::unroll<4>](input, pred));
        check(raze::algorithm::find_last_if[raze::options::fstatic](input, pred));

        check(raze::algorithm::find_last_if(input, vec_pred));
        check(raze::algorithm::find_last_if(input.begin(), input.end(), vec_pred));
        check(raze::algorithm::find_last_if[raze::options::fscalar](input, vec_pred));
        check(raze::algorithm::find_last_if[raze::options::unroll<2>](input, vec_pred));
        check(raze::algorithm::find_last_if[raze::options::unroll<4>](input, vec_pred));
        check(raze::algorithm::find_last_if[raze::options::fstatic](input, vec_pred));
    }

    template <class T>
    void find_last_if_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);
        for (size_t n : { size_t(255), size_t(256), size_t(257), size_t(511), size_t(512), size_t(513), size_t(1024) })
            sizes.push_back(n);

        for (size_t n : sizes) {
            std::vector<T> base(n);
            for (size_t i = 0; i < n; ++i)
                base[i] = static_cast<T>((i % 100) + 1);

            check_find_last_if_variants(base, T(0));

            if (n > 0) {
                std::vector<T> all_match(n, T(7));
                check_find_last_if_variants(all_match, T(7));
            }

            if (n > 0) {
                std::vector<size_t> positions;
                if (n <= 64) {
                    for (size_t p = 0; p < n; ++p)
                        positions.push_back(p);
                }
                else {
                    positions = { 0, 1, 15, 16, 31, 32, 63, 64, n / 2, n - 2, n - 1 };
                }

                for (size_t pos : positions) {
                    auto input = base;
                    input[pos] = T(0);
                    check_find_last_if_variants(input, T(0));

                    auto dup_input = input;
                    dup_input[0] = T(0);
                    check_find_last_if_variants(dup_input, T(0));
                }
            }
        }
    }

    template <class T>
    void find_last_if_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            const T target = n == 0 ? T(0) : input[rng() % n];

            auto vec_pred = raze::algorithm::equal_to(target);
            auto exp = ref_find_last_if(input, vec_pred);

            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if(input, vec_pred), exp));
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if[raze::options::fscalar](input.begin(), input.end(), vec_pred), exp));
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if[raze::options::unroll<4>](input, vec_pred), exp));
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if[raze::options::fstatic](input, vec_pred), exp));

            auto no_zero = input;
            for (auto& x : no_zero)
                if (x == T(0)) x = T(1);

            auto miss_pred = raze::algorithm::equal_to(T(0));
            const std::ranges::subrange<typename std::vector<T>::iterator> miss_exp{ no_zero.end(), no_zero.end() };
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if(no_zero, miss_pred), miss_exp));
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if[raze::options::fscalar](no_zero, miss_pred), miss_exp));
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if[raze::options::fstatic](no_zero, miss_pred), miss_exp));

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            auto sub_exp = ref_find_last_if(input.begin() + first, input.begin() + last, vec_pred);
            RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if(input.begin() + first, input.begin() + last, vec_pred), sub_exp));
        }
    }

    template <class T>
    void find_last_if_suite() {
        using namespace rtts::algorithm;
        auto cfg = config::thorough();

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                auto res = raze::algorithm::find_last_if(c.first(), c.last(), equal_pred<T>{ target });
                RTTS_EXPECT(res.end() == std::ranges::next(c.first(), c.last()));
                return res.begin();
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return ref_find_last_if(c.first(), c.last(), equal_pred<T>{ target }).begin();
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                auto res = raze::algorithm::find_last_if(c.range(), equal_pred<T>{ target });
                RTTS_EXPECT(res.end() == std::ranges::next(c.first(), c.last()));
                return res.begin();
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return ref_find_last_if(c.range(), equal_pred<T>{ target }).begin();
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::find_last_if[raze::options::fscalar](c.first(), c.last(), equal_pred<T>{ target }).begin();
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return ref_find_last_if(c.first(), c.last(), equal_pred<T>{ target }).begin();
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::find_last_if[raze::options::unroll<2>](c.range(), equal_pred<T>{ target }).begin();
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return ref_find_last_if(c.range(), equal_pred<T>{ target }).begin();
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::find_last_if[raze::options::fstatic](c.range(), equal_pred<T>{ target }).begin();
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return ref_find_last_if(c.range(), equal_pred<T>{ target }).begin();
            });

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::find_last_if(c.first(), c.last(), false_pred{}).begin(); },
            [](auto c) { return ref_find_last_if(c.first(), c.last(), false_pred{}).begin(); });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::find_last_if[raze::options::fautovec](c.first(), c.last(), equal_pred<T>{ target }).begin();
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return ref_find_last_if(c.first(), c.last(), equal_pred<T>{ target }).begin();
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                auto pred = [target](auto x) { if (x == target) return true; return false; };
                return raze::algorithm::find_last_if[raze::options::fautovec][raze::options::fstatic](c.range(), pred).begin();
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                auto pred = [target](auto x) { if (x == target) return true; return false; };
                return ref_find_last_if(c.range(), pred).begin();
            });

        {
            namespace opt = raze::options;
            auto acfg = config::compact();

            auto check_aligned = [&]<size_t Align>(auto algo) {
                run_aligned<Align, n, T>(acfg,
                    [algo](auto c) {
                        T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                        return algo(c.first(), c.last(), equal_pred<T>{ target }).begin();
                    },
                    [](auto c) {
                        T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                        return ref_find_last_if(c.first(), c.last(), equal_pred<T>{ target }).begin();
                    });

                run_aligned<Align, n, T>(acfg,
                    [algo](auto c) {
                        T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                        return algo(c.range(), equal_pred<T>{ target }).begin();
                    },
                    [](auto c) {
                        T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                        return ref_find_last_if(c.range(), equal_pred<T>{ target }).begin();
                    });
            };

            check_aligned.template operator()<16>(raze::algorithm::find_last_if[opt::aligned<16>]);
            check_aligned.template operator()<32>(raze::algorithm::find_last_if[opt::aligned<32>]);
            check_aligned.template operator()<32>(raze::algorithm::find_last_if[opt::aligned<32>][opt::unroll<2>]);
            check_aligned.template operator()<32>(raze::algorithm::find_last_if[opt::aligned<32>][opt::fstatic]);
            check_aligned.template operator()<64>(raze::algorithm::find_last_if[opt::aligned<64>]);
            check_aligned.template operator()<64>(raze::algorithm::find_last_if[opt::aligned<64>][opt::fautovec]);
            check_aligned.template operator()<64>(raze::algorithm::find_last_if[opt::aligned<64>][opt::fscalar]);
            check_aligned.template operator()<1>(raze::algorithm::find_last_if[opt::aligned<opt::unaligned>]);
            check_aligned.template operator()<1>(raze::algorithm::find_last_if[opt::aligned<64>][opt::aligned<opt::unaligned>]);
        }

        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::find_last_if(c.first(), c.last(), true_pred{}).begin(); },
            [](auto c) { return ref_find_last_if(c.first(), c.last(), true_pred{}).begin(); }));

        static_assert(constexpr_run<T, n, 5, 3>(
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::find_last_if(c.range(), equal_pred<T>{ target }).begin();
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return ref_find_last_if(c.range(), equal_pred<T>{ target }).begin();
            }));

        static_assert(constexpr_run<T, n, 5, 3>(
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                auto pred = [target](auto x) constexpr { if (x == target) return true; return false; };
                return raze::algorithm::find_last_if[raze::options::fautovec](c.range(), pred).begin();
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                auto pred = [target](auto x) constexpr { if (x == target) return true; return false; };
                return ref_find_last_if(c.range(), pred).begin();
            }));

        static_assert(constexpr_run<T, n, 5, 3>(
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::find_last_if[raze::options::aligned<64>](c.range(), equal_pred<T>{ target }).begin();
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return ref_find_last_if(c.range(), equal_pred<T>{ target }).begin();
            }));

        static_assert([]() constexpr {
            std::array<T, 0> a{};
            auto res = raze::algorithm::find_last_if(a, true_pred{});
            return res.begin() == a.end() && res.end() == a.end();
        }());

        static_assert([]() constexpr {
            std::array<T, 5> a{ T(1), T(3), T(2), T(3), T(4) };
            auto res = raze::algorithm::find_last_if(a, equal_pred<T>{ T(3) });
            return res.begin() == a.begin() + 3 && res.end() == a.end();
        }());

        static_assert([]() constexpr {
            std::array<T, 5> a{ T(1), T(2), T(3), T(4), T(5) };
            auto res = raze::algorithm::find_last_if[raze::options::fscalar](a, [](auto x) { return x > T(1); });
            return res.begin() == a.begin() + 4 && res.end() == a.end();
        }());

        static_assert([]() constexpr {
            std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
            auto res = raze::algorithm::find_last_if(a.begin() + 1, a.begin() + 3, equal_pred<T>{ T(4) });
            return res.begin() == a.begin() + 3 && res.end() == a.begin() + 3;
        }());
    }
}

RTTS_CASE_TPL("raze::algorithm::find_last_if", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    find_last_if_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::find_last_if not_found", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (size_t n : { size_t(0), size_t(1), size_t(4), size_t(8), size_t(15), size_t(16), size_t(17),
                      size_t(31), size_t(32), size_t(33), size_t(63), size_t(64), size_t(65),
                      size_t(128), size_t(129), size_t(256), size_t(257), size_t(512), size_t(513) })
    {
        std::vector<T> v(n, T(1));
        auto miss = raze::algorithm::equal_to(T(0));
        const std::ranges::subrange<typename std::vector<T>::iterator> exp{ v.end(), v.end() };

        RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if(v, miss), exp));
        RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if(v.begin(), v.end(), miss), exp));
        RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if[raze::options::fscalar](v, miss), exp));
        RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if[raze::options::unroll<2>](v, miss), exp));
        RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if[raze::options::unroll<4>](v, miss), exp));
        RTTS_EXPECT(subrange_eq(raze::algorithm::find_last_if[raze::options::fstatic](v, miss), exp));
    }
};

RTTS_CASE_TPL("raze::algorithm::find_last_if boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    find_last_if_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::find_last_if fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        find_last_if_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::find_last_if structs and projections") {
    const std::vector<find_last_if_item> input{
        { 1, 10 }, { 3, 20 }, { 3, 30 }, { 4, 40 }, { 5, 10 }
    };

    auto res = raze::algorithm::find_last_if(
        input,
        [](int32_t k) { return k == 3; },
        [](const auto& item) { return item.key; });
    RTTS_EXPECT(res.begin() == input.begin() + 2 && res.end() == input.end());

    res = raze::algorithm::find_last_if[raze::options::fscalar](
        input,
        [](int32_t v) { return v == 10; },
        [](const auto& item) { return item.value; });
    RTTS_EXPECT(res.begin() == input.begin() + 4 && res.end() == input.end());

    res = raze::algorithm::find_last_if(
        input,
        [](int32_t v) { return v == 999; },
        [](const auto& item) { return item.value; });
    RTTS_EXPECT(res.begin() == input.end() && res.end() == input.end());

    const std::vector<std::string> words{ "alpha", "gamma", "beta", "gemma", "delta" };
    auto wres = raze::algorithm::find_last_if(words, [](const std::string& s) { return s.size() == 5 && s[0] == 'g'; });
    RTTS_EXPECT(wres.begin() == words.begin() + 3 && wres.end() == words.end());
};

RTTS_CASE("raze::algorithm::find_last_if sentinel") {
    for (size_t n : { size_t(1), size_t(3), size_t(16), size_t(64), size_t(300) }) {
        std::vector<int> v(n, 1);
        v[0] = 42;

        auto first = std::counted_iterator(v.begin(), static_cast<std::ptrdiff_t>(v.size()));
        auto tail_res = raze::algorithm::find_last_if(first, std::default_sentinel, raze::algorithm::equal_to(42));
        RTTS_EXPECT(tail_res.begin().base() == v.begin());
        RTTS_EXPECT(tail_res.end().base() == v.end() && tail_res.end() == std::default_sentinel);

        if (n > 1) {
            v[n - 1] = 42;
            auto last_res = raze::algorithm::find_last_if(first, std::default_sentinel, raze::algorithm::equal_to(42));
            RTTS_EXPECT(last_res.begin().base() == v.begin() + (n - 1));
            RTTS_EXPECT(last_res.end().base() == v.end() && last_res.end() == std::default_sentinel);
        }

        auto miss = raze::algorithm::find_last_if(first, std::default_sentinel, raze::algorithm::equal_to(777));
        RTTS_EXPECT(miss.begin() == std::default_sentinel && miss.end() == std::default_sentinel);
        RTTS_EXPECT(miss.begin().base() == v.end() && miss.end().base() == v.end());
    }
};


RTTS_CASE("raze::algorithm::find_last_if views and edge cases") {
    namespace opt = raze::options;
    namespace views = std::views;

    // Views: iota, take, drop, reverse, filter, transform, chained, non-common
    {
        auto pred_branch = [](auto x) { if ((x % 4) == 0) return true; return false; };
        auto iv = views::iota(1, 30);
        auto ri = raze::algorithm::find_last_if[opt::fautovec][opt::fstatic](iv, pred_branch);
        auto si = std::ranges::find_last_if(iv, pred_branch);
        RTTS_EXPECT(ri.begin() == si.begin() && ri.end() == si.end());

        const std::vector<int> src = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        auto chain = src | views::reverse | views::filter([](int x) { return x > 2; }) | views::transform([](int x) { return x * 3; });
        auto rc = raze::algorithm::find_last_if[opt::aligned<64>](chain, [](int x) { return (x % 2) == 0; });
        auto sc = std::ranges::find_last_if(chain, [](int x) { return (x % 2) == 0; });
        RTTS_EXPECT(rc.begin() == sc.begin() && rc.end() == sc.end());

        auto nc = rtts::make_non_common(src);
        auto rn = raze::algorithm::find_last_if(nc, [](auto x) { return x < 7; });
        auto sn = std::ranges::find_last_if(nc, [](int x) { return x < 7; });
        RTTS_EXPECT(rn.begin() == sn.begin() && rn.end() == sn.end());
    }

    // Empty, 1-elem, volatile, rvalue dangling, member projection
    {
        std::vector<int> empty_v;
        RTTS_EXPECT(raze::algorithm::find_last_if(empty_v, [](auto x) { return x > 0; }).empty());

        volatile int vbuf[16];
        for (int i = 0; i < 16; ++i) vbuf[i] = i;
        auto vr = raze::algorithm::find_last_if(vbuf, vbuf + 16, [](int x) { return x < 10; });
        RTTS_EXPECT(vr.begin() == vbuf + 9);

        auto rval = raze::algorithm::find_last_if(std::vector<int>{ 1, 2, 3 }, [](auto x) { return (x & 1) == 1; });
        static_assert(std::same_as<decltype(rval), std::ranges::dangling>);

        const std::vector<rtts::test_item> items = { { 1, 10 }, { 5, 50 }, { 6, 60 }, { 2, 20 } };
        auto ir = raze::algorithm::find_last_if(items, [](int k) { return k >= 5; }, &rtts::test_item::key);
        RTTS_EXPECT(ir.begin() == items.begin() + 2 && ir.begin()->payload == 60);
    }
};