#include <tests/rts/rts.h>
#include <raze/algorithm/binary_search/UpperBound.h>

#include <algorithm>
#include <array>
#include <forward_list>
#include <ranges>
#include <vector>
#include <deque>
#include <random>

namespace {
    struct key_box {
        int val;
        constexpr bool operator<(const key_box& o) const noexcept { return val < o.val; }
    };
    struct boxed_item {
        key_box box;
        int id;
    };
}

RTTS_CASE_TPL("raze::algorithm::upper_bound", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(7), size_t(16), size_t(65), size_t(256) }) {
        auto v = rtts::random::vector<T>(sz, 510u + static_cast<unsigned>(sz));
        std::ranges::sort(v);

        std::vector<T> queries{ T(0), T(1), T(50), T(120) };
        if (!v.empty()) {
            queries.push_back(v.front());
            queries.push_back(v[sz / 2]);
            queries.push_back(v.back());
        }

        for (T q : queries) {
            const auto exp = std::ranges::upper_bound(v, q);
            RTTS_EXPECT(raze::algorithm::upper_bound(v, q) == exp);
            RTTS_EXPECT(raze::algorithm::upper_bound(v.begin(), v.end(), q) == exp);
            RTTS_EXPECT(raze::algorithm::upper_bound[opt::fscalar](v, q) == exp);
            RTTS_EXPECT(raze::algorithm::upper_bound[opt::fstatic](v, q) == exp);
        }
    }

    static_assert([]() constexpr {
        std::array<T, 6> a{ T(1), T(2), T(2), T(2), T(5), T(8) };
        return raze::algorithm::upper_bound(a, T(2)) == a.begin() + 4
            && raze::algorithm::upper_bound(a, T(3)) == a.begin() + 4
            && raze::algorithm::upper_bound(a, T(0)) == a.begin()
            && raze::algorithm::upper_bound(a, T(8)) == a.end();
    }());
};

RTTS_CASE("raze::algorithm::upper_bound P2248 braced-init, projections, forward_list, and dangling") {
    {
        const std::vector<boxed_item> v{ { { 10 }, 1 }, { { 20 }, 2 }, { { 20 }, 3 }, { { 30 }, 4 } };
        auto it = raze::algorithm::upper_bound(v, { 20 }, std::less<>{}, &boxed_item::box);
        RTTS_EXPECT(it == v.begin() + 3 && it->id == 4);
    }

    {
        const std::forward_list<int> fl{ 1, 3, 3, 5, 7 };
        auto it = raze::algorithm::upper_bound(fl, 3);
        RTTS_EQUAL(*it, 5);

        const std::vector<int> desc{ 9, 7, 5, 5, 2, 1 };
        auto dit = raze::algorithm::upper_bound(rtts::make_non_common(desc), 5, std::greater<>{});
        RTTS_EXPECT(dit == desc.begin() + 4);

        auto dr = raze::algorithm::upper_bound(std::vector<int>{ 1, 2, 3 }, 2);
        static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
    }
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void upper_bound_fuzz(unsigned seed, size_t iters) {
        namespace opt = raze::options;
        std::mt19937 rng(seed);
        rtts::random::generator<T> val_gen(seed ^ 0x5A5A5A5Au);

        for (size_t it = 0; it < iters; ++it) {
            const size_t n = static_cast<size_t>(rng() % 257);
            std::vector<T> v(n);
            if ((it & 1u) == 0u) {
                v = rtts::random::vector<T>(n, rng());
            }
            else {
                const unsigned alpha = (it % 4 == 1) ? 4u : 16u;
                for (size_t i = 0; i < n; ++i)
                    v[i] = static_cast<T>(rng() % alpha);
            }

            const bool desc = (it % 3 == 2);
            if (desc)
                std::ranges::sort(v, std::greater<>{});
            else
                std::ranges::sort(v);

            const size_t lo = n ? static_cast<size_t>(rng() % (n + 1)) : 0;
            const size_t hi = n ? (lo + static_cast<size_t>(rng() % (n - lo + 1))) : 0;
            auto sub = std::ranges::subrange(v.begin() + static_cast<std::ptrdiff_t>(lo),
                                             v.begin() + static_cast<std::ptrdiff_t>(hi));

            std::vector<T> queries{ T(0), T(1), T(12), val_gen() };
            if (lo < hi) {
                queries.push_back(v[lo]);
                queries.push_back(v[lo + (hi - lo) / 2]);
                queries.push_back(v[hi - 1]);
                queries.push_back(v[lo + static_cast<size_t>(rng() % (hi - lo))]);
            }

            auto check = [&](auto comp) {
                for (T q : queries) {
                    const auto exp = std::ranges::upper_bound(sub, q, comp);
                    RTTS_EXPECT(raze::algorithm::upper_bound(sub, q, comp) == exp);
                    RTTS_EXPECT(raze::algorithm::upper_bound(sub.begin(), sub.end(), q, comp) == exp);
                    RTTS_EXPECT(raze::algorithm::upper_bound[opt::fscalar](sub, q, comp) == exp);
                    RTTS_EXPECT(raze::algorithm::upper_bound[opt::fstatic](sub, q, comp) == exp);
                }
                if (it % 8 == 0 && (hi - lo) <= 64) {
                    std::forward_list<T> fl(sub.begin(), sub.end());
                    std::deque<T> dq(sub.begin(), sub.end());
                    for (T q : queries) {
                        RTTS_EXPECT(raze::algorithm::upper_bound(fl, q, comp) == std::ranges::upper_bound(fl, q, comp));
                        RTTS_EXPECT(raze::algorithm::upper_bound(dq, q, comp) == std::ranges::upper_bound(dq, q, comp));
                    }
                }
            };

            if (desc) check(std::greater<>{});
            else      check(std::less<>{});
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::upper_bound fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        upper_bound_fuzz<T>(seed, 40);
};
