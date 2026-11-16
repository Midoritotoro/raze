#include <tests/rts/rts.h>
#include <raze/algorithm/find/SearchN.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <ranges>
#include <string>
#include <vector>

namespace {
    struct search_n_item {
        int32_t key;
        int32_t value;
        bool operator==(const search_n_item&) const = default;
    };

    template <class T>
    void search_n_suite() {
        using namespace rtts::algorithm;
        auto cfg = config::thorough();

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::search_n(c.first(), c.last(), 1, target);
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::search_n(c.first(), c.last(), 1, target);
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::search_n(c.range(), 2, target);
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::search_n(c.range(), 2, target);
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::search_n[raze::options::fscalar](c.first(), c.last(), 2, target);
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::search_n(c.first(), c.last(), 2, target);
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::search_n[raze::options::unroll<2>](c.range(), 2, target);
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::search_n(c.range(), 2, target);
            });

        run<n, T>(cfg,
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::search_n[raze::options::fstatic](c.range(), 2, target);
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::search_n(c.range(), 2, target);
            });

        run<range, T>(cfg,
            [](auto c) { return raze::algorithm::search_n(c.first(), c.last(), 0, T(1)); },
            [](auto c) { return std::ranges::search_n(c.first(), c.last(), 0, T(1)); });

        static_assert(constexpr_run<T, range, 0>(
            [](auto c) { return raze::algorithm::search_n(c.first(), c.last(), 0, T(1)); },
            [](auto c) { return std::ranges::search_n(c.first(), c.last(), 0, T(1)); }));

        static_assert(constexpr_run<T, n, 5, 3>(
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return raze::algorithm::search_n(c.range(), 1, target);
            },
            [](auto c) {
                T target = *std::ranges::next(c.first(), static_cast<std::ptrdiff_t>(c.count()));
                return std::ranges::search_n(c.range(), 1, target);
            }));

        static_assert([]() constexpr {
            std::array<T, 0> a{};
            auto r0 = raze::algorithm::search_n(a, 0, T(1));
            auto r1 = raze::algorithm::search_n(a, 2, T(1));
            return r0.begin() == a.begin() && r0.end() == a.begin() &&
                   r1.begin() == a.end() && r1.end() == a.end();
        }());

        static_assert([]() constexpr {
            std::array<T, 6> a{ T(1), T(1), T(2), T(1), T(1), T(1) };
            auto hit = raze::algorithm::search_n(a, 3, T(1));
            auto zero = raze::algorithm::search_n(a, 0, T(9));
            auto neg = raze::algorithm::search_n(a, -2, T(1));
            auto miss = raze::algorithm::search_n(a, 4, T(1));
            return hit.begin() == a.begin() + 3 && hit.end() == a.begin() + 6 &&
                   zero.begin() == a.begin() && zero.end() == a.begin() &&
                   neg.begin() == a.begin() && neg.end() == a.begin() &&
                   miss.begin() == a.end() && miss.end() == a.end();
        }());
    }
}

RTTS_CASE_TPL("raze::algorithm::search_n", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    search_n_suite<T>();
};

RTTS_CASE_TPL("raze::algorithm::search_n streaks", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    for (size_t n : { size_t(1), size_t(17), size_t(65), size_t(131), size_t(260) }) {
        for (size_t cnt : { size_t(1), size_t(4), size_t(17), size_t(65) }) {
            if (cnt > n)
                continue;

            for (size_t s : { size_t(0), size_t(1), n / 2, n - cnt }) {
                if (s + cnt > n)
                    continue;

                std::vector<T> v(n, T(1));
                for (size_t k = 0; k < cnt; ++k)
                    v[s + k] = T(0);

                if (cnt > 1 && s >= cnt) {
                    for (size_t k = 0; k < cnt - 1; ++k)
                        v[s - cnt + k] = T(0);
                    v[s - 1] = T(1);
                }

                auto exp = std::ranges::search_n(v, static_cast<std::ptrdiff_t>(cnt), T(0));
                auto check = [&](auto sub) {
                    RTTS_EXPECT(sub.begin() == exp.begin() && sub.end() == exp.end());
                };

                check(raze::algorithm::search_n(v, static_cast<std::ptrdiff_t>(cnt), T(0)));
                check(raze::algorithm::search_n[raze::options::fscalar](v, static_cast<std::ptrdiff_t>(cnt), T(0)));
                check(raze::algorithm::search_n[raze::options::unroll<2>](v, static_cast<std::ptrdiff_t>(cnt), T(0)));
                check(raze::algorithm::search_n[raze::options::fstatic](v, static_cast<std::ptrdiff_t>(cnt), T(0)));

                auto miss = raze::algorithm::search_n(v, static_cast<std::ptrdiff_t>(cnt + 1), T(0));
                RTTS_EXPECT(miss.begin() == v.end() && miss.end() == v.end());
            }
        }
    }
};

RTTS_CASE("raze::algorithm::search_n structs, projections and predicates") {
    const std::vector<uint8_t> bytes(128, uint8_t(44));
    RTTS_EXPECT(raze::algorithm::search_n(bytes, 4, 300).begin() == bytes.end());
    RTTS_EXPECT(raze::algorithm::search_n(bytes, 4, 44).begin() == bytes.begin());

    const std::vector<int32_t> ints(64, 3);
    RTTS_EXPECT(raze::algorithm::search_n(ints, 1, 3.5).begin() == ints.end());

    const std::vector<int32_t> seq{ 10, 2, 1, 3, 4 };
    auto p1 = raze::algorithm::search_n(seq, 1, 5, std::less<>{});
    RTTS_EXPECT(p1.begin() == seq.begin() + 1 && p1.end() == seq.begin() + 2);
    auto p3 = raze::algorithm::search_n(seq, 3, 5, std::less<>{});
    RTTS_EXPECT(p3.begin() == seq.begin() + 1 && p3.end() == seq.begin() + 4);

    const std::vector<search_n_item> items{
        { 1, 10 }, { 3, 20 }, { 3, 30 }, { 3, 40 }, { 5, 50 }
    };
    auto res = raze::algorithm::search_n(items, 3, 3, std::equal_to<>{},
        [](const auto& item) { return item.key; });
    RTTS_EXPECT(res.begin() == items.begin() + 1 && res.end() == items.begin() + 4);

    const std::vector<std::string> words{ "a", "go", "go", "go", "end" };
    auto wres = raze::algorithm::search_n(words, 3, std::string("go"));
    RTTS_EXPECT(wres.begin() == words.begin() + 1 && wres.end() == words.begin() + 4);
};