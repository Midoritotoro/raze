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

            if (sz <= 100)
                RTTS_EXPECT(std::ranges::is_sorted(out1));
        }
    }
};

RTTS_CASE("raze::algorithm::sample reservoir input-only, forward_list, and back_inserter") {
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

    {
        std::forward_list<int> fl{ 10, 20, 30, 40, 50, 60, 70 };
        std::vector<int> out;
        std::mt19937 rng(7);
        raze::algorithm::sample(fl, std::back_inserter(out), 4, rng);
        RTTS_EQUAL(out.size(), size_t(4));
        RTTS_EXPECT(std::ranges::is_sorted(out));
    }

    {
        std::vector<int> pop{ 1, 2, 3 };
        std::vector<int> out;
        std::mt19937 rng(1);
        raze::algorithm::sample(pop, std::back_inserter(out), -2, rng);
        RTTS_EXPECT(out.empty());
    }
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void sample_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 257);
            const auto pop = rtts::random::vector<T>(n, rng());
            const std::ptrdiff_t k = (it % 9 == 0)
                ? -static_cast<std::ptrdiff_t>(rng() % 5)
                : static_cast<std::ptrdiff_t>(rng() % (n + 10));
            const size_t exp_n = (k > 0) ? std::min(n, static_cast<size_t>(k)) : 0;
            const unsigned gseed = rng();

            std::vector<T> o1(exp_n), o2(exp_n), o3(exp_n), o4(exp_n);
            std::mt19937 g1(gseed), g2(gseed), g3(gseed), g4(gseed);

            auto r1 = raze::algorithm::sample(pop, o1.begin(), k, g1);
            auto r2 = raze::algorithm::sample(pop.begin(), pop.end(), o2.begin(), k, g2);
            auto r3 = raze::algorithm::sample[opt::fscalar](pop, o3.begin(), k, g3);
            auto r4 = raze::algorithm::sample[opt::fstatic](pop, o4.begin(), k, g4);

            RTTS_EXPECT(r1 == o1.end() && r2 == o2.end() && r3 == o3.end() && r4 == o4.end());
            RTTS_ALL_EQUAL(o1, o2);
            RTTS_ALL_EQUAL(o1, o3);
            RTTS_ALL_EQUAL(o1, o4);
            const auto exp_next = g1();
            RTTS_EQUAL(g2(), exp_next);
            RTTS_EQUAL(g3(), exp_next);
            RTTS_EQUAL(g4(), exp_next);

            auto spop = pop;
            auto sout = o1;
            std::ranges::sort(spop);
            std::ranges::sort(sout);
            RTTS_EXPECT(std::ranges::includes(spop, sout));

            if (it % 6 == 0) {
                rtts::single_pass_input_range<T> sp(pop);
                std::vector<T> rout(exp_n);
                std::mt19937 gr(gseed);
                auto rr = raze::algorithm::sample(sp, rout.begin(), k, gr);
                RTTS_EXPECT(rr == rout.end());
                if (k > 0)
                    RTTS_EQUAL(sp.reads(), pop.size());
                std::ranges::sort(rout);
                RTTS_EXPECT(std::ranges::includes(spop, rout));
            }

            if (it % 8 == 0 && n <= 64) {
                std::forward_list<T> fl(spop.begin(), spop.end());
                std::vector<T> fout;
                std::mt19937 gf(gseed);
                raze::algorithm::sample(fl, std::back_inserter(fout), k, gf);
                RTTS_EQUAL(fout.size(), exp_n);
                RTTS_EXPECT(std::ranges::is_sorted(fout));
                RTTS_EXPECT(std::ranges::includes(spop, fout));
            }
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::sample fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        sample_fuzz<T>(seed, 35);
};
