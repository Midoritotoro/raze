#include <tests/rts/rts.h>
#include <raze/algorithm/binary_search/BinarySearch.h>

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

RTTS_CASE_TPL("raze::algorithm::binary_search", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(7), size_t(16), size_t(65), size_t(256) }) {
        auto v = rtts::random::vector<T>(sz, 530u + static_cast<unsigned>(sz));
        std::ranges::sort(v);

        std::vector<T> queries{ T(0), T(1), T(50), T(120) };
        if (!v.empty()) {
            queries.push_back(v.front());
            queries.push_back(v[sz / 2]);
            queries.push_back(v.back());
        }

        for (T q : queries) {
            const bool exp = std::ranges::binary_search(v, q);
            RTTS_EQUAL(raze::algorithm::binary_search(v, q), exp);
            RTTS_EQUAL(raze::algorithm::binary_search(v.begin(), v.end(), q), exp);
            RTTS_EQUAL(raze::algorithm::binary_search[opt::fscalar](v, q), exp);
            RTTS_EQUAL(raze::algorithm::binary_search[opt::fstatic](v, q), exp);
        }
    }

    static_assert([]() constexpr {
        std::array<T, 5> a{ T(1), T(3), T(5), T(7), T(9) };
        return raze::algorithm::binary_search(a, T(5))
            && !raze::algorithm::binary_search(a, T(4))
            && !raze::algorithm::binary_search(a, T(0))
            && !raze::algorithm::binary_search(a, T(10));
    }());
};

RTTS_CASE("raze::algorithm::binary_search P2248 braced-init, projections, forward_list, and non-common") {
    const std::vector<boxed_item> v{ { { 10 }, 1 }, { { 20 }, 2 }, { { 30 }, 3 } };
    RTTS_EXPECT(raze::algorithm::binary_search(v, { 20 }, std::less<>{}, &boxed_item::box));
    RTTS_EXPECT(!raze::algorithm::binary_search(v, { 25 }, std::less<>{}, &boxed_item::box));

    const std::forward_list<int> fl{ 1, 3, 5, 7, 9 };
    RTTS_EXPECT(raze::algorithm::binary_search(fl, 7));
    RTTS_EXPECT(!raze::algorithm::binary_search(fl, 6));

    const std::vector<int> desc{ 9, 7, 5, 3, 1 };
    RTTS_EXPECT(raze::algorithm::binary_search(rtts::make_non_common(desc), 5, std::greater<>{}));
};

namespace {
    inline constexpr unsigned fuzz_seeds[] = { 42u, 0x9E3779B9u, 0xDEADBEEFu };

    template <class T>
    void binary_search_fuzz(unsigned seed, size_t iters) {
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
                    const bool exp = std::ranges::binary_search(sub, q, comp);
                    RTTS_EQUAL(raze::algorithm::binary_search(sub, q, comp), exp);
                    RTTS_EQUAL(raze::algorithm::binary_search(sub.begin(), sub.end(), q, comp), exp);
                    RTTS_EQUAL(raze::algorithm::binary_search[opt::fscalar](sub, q, comp), exp);
                    RTTS_EQUAL(raze::algorithm::binary_search[opt::fstatic](sub, q, comp), exp);
                }
                if (it % 8 == 0 && (hi - lo) <= 64) {
                    std::forward_list<T> fl(sub.begin(), sub.end());
                    std::deque<T> dq(sub.begin(), sub.end());
                    for (T q : queries) {
                        const bool exp = std::ranges::binary_search(sub, q, comp);
                        RTTS_EQUAL(raze::algorithm::binary_search(fl, q, comp), exp);
                        RTTS_EQUAL(raze::algorithm::binary_search(dq, q, comp), exp);
                    }
                }
            };

            if (desc) check(std::greater<>{});
            else      check(std::less<>{});
        }
    }
}

RTTS_CASE_TPL("raze::algorithm::binary_search fuzz", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (unsigned seed : fuzz_seeds)
        binary_search_fuzz<T>(seed, 40);
};
