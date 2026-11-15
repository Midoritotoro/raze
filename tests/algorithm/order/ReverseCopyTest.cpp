#include <tests/rts/rts.h>
#include <raze/algorithm/order/ReverseCopy.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <list>
#include <random>
#include <ranges>
#include <vector>

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    struct reverse_copy_item {
        int32_t key;
        int32_t value;
        bool operator==(const reverse_copy_item&) const = default;
    };

    template <class T>
    void reverse_copy_boundary_sweep() {
        constexpr size_t MaxN = 140;
        for (size_t n = 0; n <= MaxN; ++n) {
            std::vector<T> input(n);
            for (size_t i = 0; i < n; ++i)
                input[i] = static_cast<T>(i);

            std::vector<T> exp(n);
            auto std_result = std::ranges::reverse_copy(input, exp.begin());
            RTTS_EXPECT(std_result.in == input.end());
            RTTS_EXPECT(std_result.out == exp.end());

            std::vector<T> out(n);
            auto result = raze::algorithm::reverse_copy(input.begin(), input.end(), out.begin());
            RTTS_EXPECT(result.in == input.end());
            RTTS_EXPECT(result.out == out.end());
            RTTS_EXPECT(out == exp);

            out.assign(n, T{});
            result = raze::algorithm::reverse_copy(input, out.begin());
            RTTS_EXPECT(result.in == input.end());
            RTTS_EXPECT(result.out == out.end());
            RTTS_EXPECT(out == exp);

            out.assign(n, T{});
            result = raze::algorithm::reverse_copy[raze::options::fscalar](input, out.begin());
            RTTS_EXPECT(result.in == input.end());
            RTTS_EXPECT(result.out == out.end());
            RTTS_EXPECT(out == exp);

            out.assign(n, T{});
            result = raze::algorithm::reverse_copy[raze::options::unroll<2>](input, out.begin());
            RTTS_EXPECT(result.in == input.end());
            RTTS_EXPECT(result.out == out.end());
            RTTS_EXPECT(out == exp);

            out.assign(n, T{});
            result = raze::algorithm::reverse_copy[raze::options::fstatic](input, out.begin());
            RTTS_EXPECT(result.in == input.end());
            RTTS_EXPECT(result.out == out.end());
            RTTS_EXPECT(out == exp);
        }
    }

    template <class T>
    void reverse_copy_fuzz(unsigned seed, size_t iters) {
        std::mt19937 rng(seed);
        std::uniform_int_distribution<size_t> sz(0, 512);
        for (size_t i = 0; i < iters; ++i) {
            const size_t n = sz(rng);
            auto input = rtts::random::vector<T>(n, seed + static_cast<unsigned>(i));
            const auto original = input;

            const size_t first = n == 0 ? 0 : rng() % (n + 1);
            const size_t last = first + (n == first ? 0 : rng() % (n - first + 1));
            const size_t count = last - first;

            std::vector<T> exp(count + 2, T{});
            std::vector<T> out(count + 2, T{});

            auto std_result = std::ranges::reverse_copy(input.begin() + first, input.begin() + last, exp.begin() + 1);
            auto result = raze::algorithm::reverse_copy(input.begin() + first, input.begin() + last, out.begin() + 1);

            RTTS_EXPECT(result.in == std_result.in);
            RTTS_EXPECT(result.out - out.begin() == std_result.out - exp.begin());
            RTTS_EXPECT(out == exp);
            RTTS_EXPECT(input == original);

            out.assign(count + 2, T{});
            result = raze::algorithm::reverse_copy[raze::options::fscalar](input.begin() + first, input.begin() + last, out.begin() + 1);

            RTTS_EXPECT(result.in == input.begin() + last);
            RTTS_EXPECT(result.out == out.begin() + 1 + count);
            RTTS_EXPECT(out == exp);
            RTTS_EXPECT(input == original);
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::reverse_copy", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    RTTS_EXPECT(true);

    static_assert([]() constexpr {
        std::array<T, 0> input{};
        std::array<T, 0> out{};
        auto result = raze::algorithm::reverse_copy(input, out.begin());
        return result.in == input.end() && result.out == out.end();
    }());

    static_assert([]() constexpr {
        std::array<T, 1> input{ T(1) };
        std::array<T, 1> out{};
        auto result = raze::algorithm::reverse_copy(input, out.begin());
        return result.in == input.end() && result.out == out.end() && out == std::array<T, 1>{ T(1) };
    }());

    static_assert([]() constexpr {
        std::array<T, 4> input{ T(1), T(2), T(3), T(4) };
        std::array<T, 4> out{};
        auto result = raze::algorithm::reverse_copy(input.begin(), input.end(), out.begin());
        return result.in == input.end() && result.out == out.end() &&
               out == std::array<T, 4>{ T(4), T(3), T(2), T(1) };
    }());

    static_assert([]() constexpr {
        std::array<T, 5> input{ T(1), T(2), T(3), T(4), T(5) };
        std::array<T, 5> out{};
        auto result = raze::algorithm::reverse_copy[raze::options::fscalar](input, out.begin());
        return result.in == input.end() && result.out == out.end() &&
               out == std::array<T, 5>{ T(5), T(4), T(3), T(2), T(1) };
    }());

    static_assert([]() constexpr {
        std::array<T, 4> input{ T(1), T(2), T(3), T(4) };
        std::array<T, 4> out{};
        auto result = raze::algorithm::reverse_copy(input.begin() + 1, input.begin() + 3, out.begin() + 1);
        return result.in == input.begin() + 3 && result.out == out.begin() + 3 &&
               out == std::array<T, 4>{ T(0), T(3), T(2), T(0) };
    }());
};

RTTS_CASE_TPL("raze::algorithm::reverse_copy boundaries", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    reverse_copy_boundary_sweep<T>();
};

RTTS_CASE_TPL("raze::algorithm::reverse_copy fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        reverse_copy_fuzz<T>(seed, 200);
};

RTTS_CASE("raze::algorithm::reverse_copy structs") {
    std::vector<reverse_copy_item> input{
        { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 }, { 5, 50 }
    };
    const auto original = input;
    std::vector<reverse_copy_item> exp(input.size());
    std::vector<reverse_copy_item> out(input.size());

    std::ranges::reverse_copy(input, exp.begin());

    auto result = raze::algorithm::reverse_copy(input, out.begin());
    RTTS_EXPECT(result.in == input.end());
    RTTS_EXPECT(result.out == out.end());
    RTTS_EXPECT(out == exp);
    RTTS_EXPECT(input == original);

    out.assign(input.size(), {});
    result = raze::algorithm::reverse_copy[raze::options::fscalar](input, out.begin());
    RTTS_EXPECT(result.in == input.end());
    RTTS_EXPECT(result.out == out.end());
    RTTS_EXPECT(out == exp);
};

RTTS_CASE("raze::algorithm::reverse_copy bidirectional") {
    std::list<int> input{ 1, 2, 3, 4, 5 };
    std::vector<int> out;

    auto result = raze::algorithm::reverse_copy(input, std::back_inserter(out));
    RTTS_EXPECT(result.in == input.end());
    RTTS_EXPECT((out == std::vector<int>{ 5, 4, 3, 2, 1 }));

    out.clear();
    auto scalar_result = raze::algorithm::reverse_copy[raze::options::fscalar](
        input.begin(), input.end(), std::back_inserter(out));
    RTTS_EXPECT(scalar_result.in == input.end());
    RTTS_EXPECT((out == std::vector<int>{ 5, 4, 3, 2, 1 }));
};

RTTS_CASE("raze::algorithm::reverse_copy sentinel") {
    std::array<int, 5> input{ 1, 2, 3, 4, 5 };
    std::array<int, 3> out{};

    auto first = std::counted_iterator(input.begin() + 1, 3);
    auto result = raze::algorithm::reverse_copy(first, std::default_sentinel, out.begin());

    RTTS_EXPECT(result.in == std::default_sentinel);
    RTTS_EXPECT(result.out == out.end());
    RTTS_EXPECT((out == std::array<int, 3>{ 4, 3, 2 }));
};