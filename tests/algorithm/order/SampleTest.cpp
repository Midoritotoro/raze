#include <tests/rts/rts.h>
#include <raze/algorithm/order/Sample.h>

#include <algorithm>
#include <array>
#include <forward_list>
#include <iterator>
#include <list>
#include <random>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::sample", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(5), size_t(32), size_t(100) }) {
        std::vector<T> pop(sz);
        for (size_t i = 0; i < sz; ++i)
            pop[i] = static_cast<T>(i % 100);

        for (std::ptrdiff_t k : { std::ptrdiff_t(0), std::ptrdiff_t(1), static_cast<std::ptrdiff_t>(sz / 2),
            static_cast<std::ptrdiff_t>(sz), static_cast<std::ptrdiff_t>(sz + 5) })
        {
            const size_t expected_n = std::min(sz, static_cast<size_t>(k));
            std::vector<T> out1(expected_n), out2(expected_n);
            std::mt19937 g1(42), g2(42);

            auto r1 = raze::algorithm::sample(pop, out1.begin(), k, g1);
            auto r2 = raze::algorithm::sample[opt::fscalar](pop.begin(), pop.end(), out2.begin(), k, g2);

            RTTS_EXPECT(r1 == out1.end() && r2 == out2.end());
            RTTS_ALL_EQUAL(out1, out2);

            // Selection sampling on forward range preserves relative order when values are distinct
            if (sz <= 100)
                RTTS_EXPECT(std::ranges::is_sorted(out1));
        }
    }
};

RTTS_CASE("raze::algorithm::sample reservoir input-only, forward_list, and back_inserter") {
    // Reservoir sampling (single-pass input range -> random-access output)
    {
        std::vector<int> pop(50);
        std::iota(pop.begin(), pop.end(), 1);
        rtts::single_pass_input_range<int> sp(pop);
        std::vector<int> out(10, 0);
        std::mt19937 rng(99);

        auto end = raze::algorithm::sample(sp, out.begin(), 10, rng);
        RTTS_EXPECT(end == out.end());
        RTTS_EQUAL(sp.reads(), pop.size());
        std::ranges::sort(out);
        RTTS_EXPECT(std::ranges::adjacent_find(out) == out.end());
        RTTS_EXPECT(out.front() >= 1 && out.back() <= 50);
    }

    // Forward list -> back_inserter (selection sampling)
    {
        std::forward_list<int> fl{ 10, 20, 30, 40, 50, 60, 70 };
        std::vector<int> out;
        std::mt19937 rng(7);
        raze::algorithm::sample(fl, std::back_inserter(out), 4, rng);
        RTTS_EQUAL(out.size(), size_t(4));
        RTTS_EXPECT(std::ranges::is_sorted(out));
    }

    // Negative / zero sample count
    {
        std::vector<int> pop{ 1, 2, 3 };
        std::vector<int> out;
        std::mt19937 rng(1);
        raze::algorithm::sample(pop, std::back_inserter(out), -2, rng);
        RTTS_EXPECT(out.empty());
    }
};

