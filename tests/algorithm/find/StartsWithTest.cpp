#include <tests/rts/rts.h>
#include <raze/algorithm/find/StartsWith.h>

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

    struct starts_with_item {
        int32_t key;
        int32_t value;
        bool operator==(const starts_with_item&) const = default;
    };

    template <class It1, class Sent1, class It2, class Sent2, class Pred = std::equal_to<>>
    constexpr bool ref_starts_with(It1 first1, Sent1 sent1, It2 first2, Sent2 sent2, Pred pred = {}) {
#if defined(__cpp_lib_ranges_starts_with) && __cpp_lib_ranges_starts_with >= 202207L
        return std::ranges::starts_with(std::move(first1), std::move(sent1),
            std::move(first2), std::move(sent2), std::move(pred));
#else
        for (; first2 != sent2; ++first1, ++first2) {
            if (first1 == sent1 || !pred(*first1, *first2))
                return false;
        }
        return true;
#endif
    }

    template <class R1, class R2, class Pred = std::equal_to<>>
    constexpr bool ref_starts_with(const R1& r1, const R2& r2, Pred pred = {}) {
        return ref_starts_with(std::ranges::begin(r1), std::ranges::end(r1),
            std::ranges::begin(r2), std::ranges::end(r2), std::move(pred));
    }

    template <class T>
    void check_starts_with_variants(const std::vector<T>& haystack, const std::vector<T>& needle) {
        const bool expected = ref_starts_with(haystack, needle);
        auto bin_pred = [](auto a, auto b) { return a == b; };

        RTTS_EXPECT(raze::algorithm::starts_with(haystack, needle) == expected);
        RTTS_EXPECT(raze::algorithm::starts_with(haystack.begin(), haystack.end(), needle.begin(), needle.end()) == expected);
        RTTS_EXPECT(raze::algorithm::starts_with[raze::options::fscalar](haystack, needle) == expected);
        RTTS_EXPECT(raze::algorithm::starts_with[raze::options::unroll<2>](haystack, needle) == expected);
        RTTS_EXPECT(raze::algorithm::starts_with[raze::options::unroll<4>](haystack, needle) == expected);
        RTTS_EXPECT(raze::algorithm::starts_with[raze::options::fstatic](haystack, needle) == expected);

        RTTS_EXPECT(raze::algorithm::starts_with(haystack, needle, bin_pred) == expected);
        RTTS_EXPECT(raze::algorithm::starts_with[raze::options::fscalar](haystack, needle, bin_pred) == expected);
        RTTS_EXPECT(raze::algorithm::starts_with[raze::options::fstatic](haystack, needle, bin_pred) == expected);
    }

    template <class T>
    void starts_with_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);
        for (size_t n : { size_t(255), size_t(256), size_t(257), size_t(511), size_t(512), size_t(513), size_t(1024) })
            sizes.push_back(n);

        for (size_t n : sizes) {
            std::vector<T> haystack(n);
            for (size_t i = 0; i < n; ++i)
                haystack[i] = static_cast<T>((i % 100) + 1);

            check_starts_with_variants(haystack, std::vector<T>{});

            std::vector<T> longer(n + 1, T(1));
            for (size_t i = 0; i < n; ++i)
                longer[i] = haystack[i];
            check_starts_with_variants(haystack, longer);

            if (n > 0) {
                check_starts_with_variants(haystack, haystack);

                std::vector<size_t> positions;
                if (n <= 64) {
                    for (size_t p = 0; p < n; ++p)
                        positions.push_back(p);
                }
                else {
                    positions = { 0, 1, 15, 16, 31, 32, 63, 64, n / 2, n - 2, n - 1 };
                }

                for (size_t pos : positions) {
                    auto broken = haystack;
                    broken[pos] = T(0);
                    check_starts_with_variants(haystack, broken);
                }
            }
        }

        for (size_t n : { size_t(65), size_t(128), size_t(300) }) {
            std::vector<T> haystack(n);
            for (size_t i = 0; i < n; ++i)
                haystack[i] = static_cast<T>((i % 100) + 1);

            for (size_t m : { size_t(1), size_t(3), size_t(15), size_t(16), size_t(17),
                              size_t(31), size_t(32), size_t(33), size_t(64), n - 1 })
            {
                std::vector<T> prefix(haystack.begin(), haystack.begin() + m);
                check_starts_with_variants(haystack, prefix);

                auto bad_first = prefix;
                bad_first[0] = T(0);
                check_starts_with_variants(haystack, bad_first);

                auto bad_last = prefix;
                bad_last[m - 1] = T(0);
                check_starts_with_variants(haystack, bad_last);
            }
        }
    }

    template <class T>
    void starts_with_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n1 = sz(rng);
            const size_t n2 = sz(rng);
            auto h = rtts::random::vector<T>(n1, seed + static_cast<unsigned>(i));
            auto needle = (n2 <= n1 && (rng() % 2 == 0))
                ? std::vector<T>(h.begin(), h.begin() + n2)
                : rtts::random::vector<T>(n2, seed + static_cast<unsigned>(i) + 1000u);

            if (n2 > 0 && n2 <= n1 && (rng() % 4 == 0))
                needle[rng() % n2] = static_cast<T>(needle[0] + T(1));

            const bool exp = ref_starts_with(h, needle);
            RTTS_EXPECT(raze::algorithm::starts_with(h, needle) == exp);
            RTTS_EXPECT(raze::algorithm::starts_with[raze::options::fscalar](h.begin(), h.end(), needle.begin(), needle.end()) == exp);
            RTTS_EXPECT(raze::algorithm::starts_with[raze::options::unroll<4>](h, needle) == exp);
            RTTS_EXPECT(raze::algorithm::starts_with[raze::options::fstatic](h, needle) == exp);
        }
    }

    template <class T>
    void starts_with_suite() {
        using namespace rtts::algorithm;
        auto cfg = config::thorough();

        run<n, T>(cfg,
            [](auto c) {
                auto p_end = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::starts_with(c.first(), c.last(), c.first(), p_end);
            },
            [](auto c) {
                auto p_end = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return ref_starts_with(c.first(), c.last(), c.first(), p_end);
            });

        run<n, T>(cfg,
            [](auto c) {
                auto p_end = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::starts_with(c.range(), std::ranges::subrange(c.first(), p_end));
            },
            [](auto c) {
                auto p_end = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return ref_starts_with(c.range(), std::ranges::subrange(c.first(), p_end));
            });

        run<n, T>(cfg,
            [](auto c) {
                auto p_end = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::starts_with[raze::options::fscalar](c.first(), c.last(), c.first(), p_end);
            },
            [](auto c) {
                auto p_end = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return ref_starts_with(c.first(), c.last(), c.first(), p_end);
            });

        run<n, T>(cfg,
            [](auto c) {
                auto p_end = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::starts_with[raze::options::unroll<2>](c.range(), std::ranges::subrange(c.first(), p_end));
            },
            [](auto c) {
                auto p_end = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return ref_starts_with(c.range(), std::ranges::subrange(c.first(), p_end));
            });

        run<n, T>(cfg,
            [](auto c) {
                auto p_end = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::starts_with[raze::options::fstatic](c.range(), std::ranges::subrange(c.first(), p_end));
            },
            [](auto c) {
                auto p_end = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return ref_starts_with(c.range(), std::ranges::subrange(c.first(), p_end));
            });

        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::starts_with(c.range(), c.range()); },
            [](auto c) { return ref_starts_with(c.range(), c.range()); }));

        static_assert(constexpr_run<T, n, 5, 3>(
            [](auto c) {
                auto p_end = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::starts_with(c.range(), std::ranges::subrange(c.first(), p_end));
            },
            [](auto c) {
                auto p_end = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return ref_starts_with(c.range(), std::ranges::subrange(c.first(), p_end));
            }));

        static_assert([]() constexpr {
            std::array<T, 0> empty{};
            std::array<T, 2> tiny{ T(1), T(2) };
            return raze::algorithm::starts_with(empty, empty) &&
                   raze::algorithm::starts_with(tiny, empty) &&
                   !raze::algorithm::starts_with(empty, tiny);
        }());

        static_assert([]() constexpr {
            std::array<T, 5> a{ T(1), T(2), T(3), T(4), T(5) };
            std::array<T, 3> p{ T(1), T(2), T(3) };
            std::array<T, 3> bad{ T(1), T(9), T(3) };
            std::array<T, 6> too_long{ T(1), T(2), T(3), T(4), T(5), T(6) };
            return raze::algorithm::starts_with(a, p) &&
                   !raze::algorithm::starts_with(a, bad) &&
                   !raze::algorithm::starts_with(a, too_long) &&
                   raze::algorithm::starts_with[raze::options::fscalar](a.begin(), a.end(), p.begin(), p.end());
        }());
    }
}

RTTS_CASE_TPL("raze::algorithm::starts_with", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    starts_with_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::starts_with boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    starts_with_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::starts_with fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        starts_with_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::starts_with structs, projections, rvalues and predicates") {
    const std::vector<starts_with_item> input{
        { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }
    };
    const std::vector<int32_t> keys{ 1, 2, 3 };
    const std::vector<int32_t> bad_keys{ 1, 9, 3 };

    RTTS_EXPECT(raze::algorithm::starts_with(
        input, keys, std::equal_to<>{},
        [](const auto& item) { return item.key; }));

    RTTS_EXPECT(!raze::algorithm::starts_with(
        input, bad_keys, std::equal_to<>{},
        [](const auto& item) { return item.key; }));

    const std::vector<starts_with_item> other{ { 9, 100 }, { 8, 200 } };
    RTTS_EXPECT(raze::algorithm::starts_with[raze::options::fscalar](
        input, other,
        [](int32_t a, int32_t b) { return a * 10 == b; },
        [](const auto& x) { return x.value; },
        [](const auto& y) { return y.value; }));

    const std::vector<int32_t> nums{ 1, 2, 3, 4, 5, 6 };
    RTTS_EXPECT(raze::algorithm::starts_with(nums, std::vector<int32_t>{ 1, 2, 3 }));
    RTTS_EXPECT(!raze::algorithm::starts_with(nums, std::vector<int32_t>{ 1, 2, 9 }));
    RTTS_EXPECT(raze::algorithm::starts_with(std::vector<int32_t>{ 1, 2, 3, 4 }, std::vector<int32_t>{ 1, 2 }));

    const std::string s = "hello world";
    RTTS_EXPECT(raze::algorithm::starts_with(s, std::string_view("hello")));
    RTTS_EXPECT(!raze::algorithm::starts_with(s, std::string_view("world")));
};

