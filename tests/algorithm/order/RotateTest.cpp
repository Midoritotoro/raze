#include <tests/rts/rts.h>
#include <raze/algorithm/order/Rotate.h>

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

    struct rotate_item {
        int32_t key;
        int32_t value;
        bool operator==(const rotate_item&) const = default;
    };

    struct rotate_odd_item {
        uint8_t bytes[7];
        bool operator==(const rotate_odd_item&) const = default;
    };

    struct rotate_large_item {
        int32_t data[160];
        bool operator==(const rotate_large_item&) const = default;
    };

    template <class T>
    void check_rotate_variants(const std::vector<T>& input, size_t mid) {
        const size_t n = input.size();
        auto exp = input;
        auto exp_res = std::ranges::rotate(exp, exp.begin() + mid);
        const auto exp_pos = exp_res.begin() - exp.begin();

        auto v = input;
        auto res = raze::algorithm::rotate(v, v.begin() + mid);
        RTTS_EXPECT(res.begin() - v.begin() == exp_pos);
        RTTS_EXPECT(res.end() == v.end());
        RTTS_EXPECT(v == exp);

        v = input;
        res = raze::algorithm::rotate(v.begin(), v.begin() + mid, v.end());
        RTTS_EXPECT(res.begin() - v.begin() == exp_pos);
        RTTS_EXPECT(res.end() == v.end());
        RTTS_EXPECT(v == exp);

        v = input;
        res = raze::algorithm::rotate[raze::options::fscalar](v, v.begin() + mid);
        RTTS_EXPECT(res.begin() - v.begin() == exp_pos);
        RTTS_EXPECT(res.end() == v.end());
        RTTS_EXPECT(v == exp);

        v = input;
        res = raze::algorithm::rotate[raze::options::unroll<2>](v, v.begin() + mid);
        RTTS_EXPECT(res.begin() - v.begin() == exp_pos);
        RTTS_EXPECT(res.end() == v.end());
        RTTS_EXPECT(v == exp);

        v = input;
        res = raze::algorithm::rotate[raze::options::fstatic](v, v.begin() + mid);
        RTTS_EXPECT(res.begin() - v.begin() == exp_pos);
        RTTS_EXPECT(res.end() == v.end());
        RTTS_EXPECT(v == exp);

        res = raze::algorithm::rotate(v, v.begin() + (n - mid));
        RTTS_EXPECT(res.begin() == v.begin() + mid);
        RTTS_EXPECT(res.end() == v.end());
        RTTS_EXPECT(v == input);
    }

    template <class T>
    void rotate_boundary_sweep() {
        std::vector<size_t> sizes;
        for (size_t n = 0; n <= 140; ++n)
            sizes.push_back(n);

        for (size_t n : {
            size_t(256), size_t(384), size_t(511), size_t(512), size_t(513),
            size_t(600), size_t(768), size_t(1023), size_t(1024), size_t(1025),
            size_t(1535), size_t(1536), size_t(1600), size_t(2048) })
        {
            sizes.push_back(n);
        }

        for (size_t n : sizes) {
            std::vector<T> input(n);
            for (size_t i = 0; i < n; ++i)
                input[i] = static_cast<T>(i);

            std::vector<size_t> mids;
            if (n <= 32) {
                for (size_t m = 0; m <= n; ++m)
                    mids.push_back(m);
            }
            else {
                for (size_t m : {
                    size_t(0), size_t(1), size_t(2),
                    size_t(127), size_t(128), size_t(129),
                    size_t(511), size_t(512), size_t(513),
                    n / 4, n / 3, n / 2, (2 * n) / 3, (3 * n) / 4,
                    n - 2, n - 1, n })
                {
                    if (m <= n)
                        mids.push_back(m);
                }
                if (n > 513) {
                    mids.push_back(n - 513);
                    mids.push_back(n - 512);
                    mids.push_back(n - 511);
                }
                std::ranges::sort(mids);
                mids.erase(std::ranges::unique(mids).begin(), mids.end());
            }

            for (size_t mid : mids)
                check_rotate_variants(input, mid);
        }
    }

    template <class T>
    void rotate_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 2500);

        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));

            const size_t mid = n == 0 ? 0 : rng() % (n + 1);
            auto exp = input;
            auto exp_res = std::ranges::rotate(exp, exp.begin() + mid);

            auto v = input;
            auto res = raze::algorithm::rotate(v, v.begin() + mid);
            RTTS_EXPECT(res.begin() - v.begin() == exp_res.begin() - exp.begin());
            RTTS_EXPECT(res.end() == v.end());
            RTTS_EXPECT(v == exp);

            v = input;
            res = raze::algorithm::rotate[raze::options::fscalar](v.begin(), v.begin() + mid, v.end());
            RTTS_EXPECT(res.begin() - v.begin() == exp_res.begin() - exp.begin());
            RTTS_EXPECT(res.end() == v.end());
            RTTS_EXPECT(v == exp);

            v = input;
            res = raze::algorithm::rotate[raze::options::fstatic](v, v.begin() + mid);
            RTTS_EXPECT(res.begin() - v.begin() == exp_res.begin() - exp.begin());
            RTTS_EXPECT(res.end() == v.end());
            RTTS_EXPECT(v == exp);

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            const size_t sub_mid = first + (last == first ? 0 : rng() % (last - first + 1));

            v = input;
            exp = input;
            exp_res = std::ranges::rotate(exp.begin() + first, exp.begin() + sub_mid, exp.begin() + last);
            res = raze::algorithm::rotate(v.begin() + first, v.begin() + sub_mid, v.begin() + last);
            RTTS_EXPECT(res.begin() - v.begin() == exp_res.begin() - exp.begin());
            RTTS_EXPECT(res.end() == v.begin() + last);
            RTTS_EXPECT(v == exp);
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::rotate", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    auto cfg = config::thorough();

    auto std_rotate_n = [](auto c) {
        auto mid = std::ranges::next(c.data.value.begin(), static_cast<std::ptrdiff_t>(c.count()));
        return std::ranges::rotate(c.data.value.begin(), mid, c.data.value.end());
    };

    run<n, T>(cfg,
        [](auto c) {
            auto mid = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
            return raze::algorithm::rotate(c.first(), mid, c.last());
        },
        std_rotate_n);

    run<n, T>(cfg,
        [](auto c) {
            auto mid = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
            return raze::algorithm::rotate(c.range(), mid);
        },
        std_rotate_n);

    run<n, T>(cfg,
        [](auto c) {
            auto mid = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
            return raze::algorithm::rotate[raze::options::fscalar](c.first(), mid, c.last());
        },
        std_rotate_n);

    run<n, T>(cfg,
        [](auto c) {
            auto mid = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
            return raze::algorithm::rotate[raze::options::unroll<2>](c.range(), mid);
        },
        std_rotate_n);

    run<n, T>(cfg,
        [](auto c) {
            auto mid = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
            return raze::algorithm::rotate[raze::options::fstatic](c.range(), mid);
        },
        std_rotate_n);

    run<range, T>(cfg,
        [](auto c) {
            auto mid = std::ranges::next(c.first(), c.last());
            return raze::algorithm::rotate(c.first(), mid, c.last());
        },
        [](auto c) {
            return std::ranges::rotate(c.data.value.begin(), c.data.value.end(), c.data.value.end());
        });

    static_assert(constexpr_run<T, range, 0>(
        [](auto c) { return raze::algorithm::rotate(c.first(), c.first(), c.last()); },
        [](auto c) { return std::ranges::rotate(c.data.value.begin(), c.data.value.begin(), c.data.value.end()); }));

    static_assert(constexpr_run<T, n, 5, 0>(
        [](auto c) {
            auto mid = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
            return raze::algorithm::rotate(c.first(), mid, c.last());
        },
        std_rotate_n));

    static_assert(constexpr_run<T, n, 5, 2>(
        [](auto c) {
            auto mid = std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
            return raze::algorithm::rotate(c.range(), mid);
        },
        std_rotate_n));

    static_assert([]() constexpr {
        std::array<T, 0> a{};
        auto res = raze::algorithm::rotate(a, a.begin());
        return res.begin() == a.end() && res.end() == a.end();
        }());

    static_assert([]() constexpr {
        std::array<T, 1> a{ T(1) };
        auto res = raze::algorithm::rotate(a, a.begin());
        return res.begin() == a.end() && res.end() == a.end() && a == std::array<T, 1>{ T(1) };
        }());

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        auto res = raze::algorithm::rotate(a, a.begin() + 1);
        return res.begin() == a.begin() + 3 && res.end() == a.end() &&
               a == std::array<T, 4>{ T(2), T(3), T(4), T(1) };
        }());

    static_assert([]() constexpr {
        std::array<T, 5> a{ T(1), T(2), T(3), T(4), T(5) };
        auto res = raze::algorithm::rotate[raze::options::fscalar](a, a.begin() + 3);
        return res.begin() == a.begin() + 2 && res.end() == a.end() &&
               a == std::array<T, 5>{ T(4), T(5), T(1), T(2), T(3) };
        }());

    static_assert([]() constexpr {
        std::array<T, 4> a{ T(1), T(2), T(3), T(4) };
        auto res = raze::algorithm::rotate(a, a.end());
        return res.begin() == a.begin() && res.end() == a.end() &&
               a == std::array<T, 4>{ T(1), T(2), T(3), T(4) };
        }());

    static_assert([]() constexpr {
        std::array<T, 5> a{ T(1), T(2), T(3), T(4), T(5) };
        auto res = raze::algorithm::rotate(a.begin() + 1, a.begin() + 3, a.begin() + 4);
        return res.begin() == a.begin() + 2 && res.end() == a.begin() + 4 &&
               a == std::array<T, 5>{ T(1), T(4), T(2), T(3), T(5) };
        }());
};

RTTS_CASE_TPL("raze::algorithm::rotate boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    rotate_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::rotate fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        rotate_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::rotate structs") {
    {
        std::vector<rotate_item> input{
            { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 }
        };

        auto exp = input;
        std::ranges::rotate(exp, exp.begin() + 2);

        auto v = input;
        auto res = raze::algorithm::rotate(v, v.begin() + 2);
        RTTS_EXPECT(res.begin() == v.begin() + 3);
        RTTS_EXPECT(res.end() == v.end());
        RTTS_EXPECT(v == exp);

        v = input;
        res = raze::algorithm::rotate[raze::options::fscalar](v, v.begin() + 2);
        RTTS_EXPECT(res.begin() == v.begin() + 3);
        RTTS_EXPECT(res.end() == v.end());
        RTTS_EXPECT(v == exp);
    }

    {
        std::vector<rotate_odd_item> input(250);
        for (size_t i = 0; i < input.size(); ++i)
            for (size_t j = 0; j < 7; ++j)
                input[i].bytes[j] = static_cast<uint8_t>((i * 7 + j) & 0xFF);

        for (size_t mid : { size_t(50), size_t(75), size_t(100), size_t(175) }) {
            auto exp = input;
            std::ranges::rotate(exp, exp.begin() + mid);

            auto v = input;
            auto res = raze::algorithm::rotate(v, v.begin() + mid);
            RTTS_EXPECT(res.begin() == v.begin() + (input.size() - mid));
            RTTS_EXPECT(res.end() == v.end());
            RTTS_EXPECT(v == exp);
        }
    }

    {
        std::vector<rotate_large_item> input(7);
        for (size_t i = 0; i < input.size(); ++i)
            input[i].data[0] = static_cast<int32_t>(i + 1);

        for (size_t mid : { size_t(2), size_t(3), size_t(5) }) {
            auto exp = input;
            std::ranges::rotate(exp, exp.begin() + mid);

            auto v = input;
            auto res = raze::algorithm::rotate(v, v.begin() + mid);
            RTTS_EXPECT(res.begin() == v.begin() + (input.size() - mid));
            RTTS_EXPECT(res.end() == v.end());
            RTTS_EXPECT(v == exp);
        }
    }

    {
        std::vector<std::string> input{
            "alpha", "beta", "gamma", "delta", "epsilon"
        };

        auto exp = input;
        std::ranges::rotate(exp, exp.begin() + 2);

        auto v = input;
        auto res = raze::algorithm::rotate(v, v.begin() + 2);
        RTTS_EXPECT(res.begin() == v.begin() + 3);
        RTTS_EXPECT(res.end() == v.end());
        RTTS_EXPECT(v == exp);
    }
};

RTTS_CASE("raze::algorithm::rotate bidirectional") {
    std::list<int> input{ 1, 2, 3, 4, 5 };

    for (size_t mid : { size_t(0), size_t(2), size_t(3), size_t(5) }) {
        auto exp = input;
        auto exp_res = std::ranges::rotate(exp, std::ranges::next(exp.begin(), static_cast<std::ptrdiff_t>(mid)));
        const auto exp_pos = std::ranges::distance(exp.begin(), exp_res.begin());

        auto v = input;
        auto res = raze::algorithm::rotate(v, std::ranges::next(v.begin(), static_cast<std::ptrdiff_t>(mid)));
        RTTS_EXPECT(std::ranges::distance(v.begin(), res.begin()) == exp_pos);
        RTTS_EXPECT(res.end() == v.end());
        RTTS_EXPECT(v == exp);

        v = input;
        res = raze::algorithm::rotate[raze::options::fscalar](
            v.begin(), std::ranges::next(v.begin(), static_cast<std::ptrdiff_t>(mid)), v.end());
        RTTS_EXPECT(std::ranges::distance(v.begin(), res.begin()) == exp_pos);
        RTTS_EXPECT(res.end() == v.end());
        RTTS_EXPECT(v == exp);
    }
};

RTTS_CASE("raze::algorithm::rotate forward") {
    std::forward_list<int> input{ 1, 2, 3, 4, 5 };

    for (size_t mid : { size_t(0), size_t(1), size_t(2), size_t(4), size_t(5) }) {
        auto exp = input;
        auto exp_res = std::ranges::rotate(exp, std::ranges::next(exp.begin(), static_cast<std::ptrdiff_t>(mid)));
        const auto exp_pos = std::ranges::distance(exp.begin(), exp_res.begin());

        auto v = input;
        auto res = raze::algorithm::rotate(v, std::ranges::next(v.begin(), static_cast<std::ptrdiff_t>(mid)));
        RTTS_EXPECT(std::ranges::distance(v.begin(), res.begin()) == exp_pos);
        RTTS_EXPECT(res.end() == v.end());
        RTTS_EXPECT(v == exp);

        v = input;
        res = raze::algorithm::rotate[raze::options::fscalar](
            v.begin(), std::ranges::next(v.begin(), static_cast<std::ptrdiff_t>(mid)), v.end());
        RTTS_EXPECT(std::ranges::distance(v.begin(), res.begin()) == exp_pos);
        RTTS_EXPECT(res.end() == v.end());
        RTTS_EXPECT(v == exp);
    }
};

RTTS_CASE("raze::algorithm::rotate sentinel") {
    {
        std::array<int, 5> input{ 1, 2, 3, 4, 5 };
        auto first = std::counted_iterator(input.begin(), 5);
        auto mid = std::counted_iterator(input.begin() + 2, 3);
        auto res = raze::algorithm::rotate(first, mid, std::default_sentinel);

        RTTS_EXPECT(res.begin().base() == input.begin() + 3);
        RTTS_EXPECT(res.end() == std::default_sentinel);
        RTTS_EXPECT((input == std::array<int, 5>{ 3, 4, 5, 1, 2 }));
    }

    {
        std::vector<uint8_t> v(1600);
        for (size_t i = 0; i < v.size(); ++i)
            v[i] = static_cast<uint8_t>(i & 0xFF);

        auto exp = v;
        std::ranges::rotate(exp, exp.begin() + 525);

        auto first = std::counted_iterator(v.begin(), static_cast<std::ptrdiff_t>(v.size()));
        auto mid = std::counted_iterator(v.begin() + 525, static_cast<std::ptrdiff_t>(v.size() - 525));
        auto res = raze::algorithm::rotate(first, mid, std::default_sentinel);

        RTTS_EXPECT(res.begin().base() == v.begin() + (v.size() - 525));
        RTTS_EXPECT(res.end() == std::default_sentinel);
        RTTS_EXPECT(v == exp);
    }
};