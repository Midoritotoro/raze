#include <tests/rts/rts.h>
#include <raze/algorithm/fill/GenerateN.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <list>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::generate_n", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    using namespace rtts::algorithm;
    namespace opt = raze::options;
    auto cfg = config::compact();

    auto make_gen = []() {
        int counter = 0;
        return [counter]() mutable -> T { return static_cast<T>((++counter) % 97); };
    };

    run<n, T>(cfg,
        [make_gen](auto c) {
            return raze::algorithm::generate_n(c.first(), static_cast<std::ptrdiff_t>(c.count()), make_gen());
        },
        [make_gen](auto c) {
            return std::ranges::generate_n(c.first(), static_cast<std::ptrdiff_t>(c.count()), make_gen());
        });

    run<n, T>(cfg,
        [make_gen](auto c) {
            return raze::algorithm::generate_n[opt::fscalar](c.first(), static_cast<std::ptrdiff_t>(c.count()), make_gen());
        },
        [make_gen](auto c) {
            return std::ranges::generate_n(c.first(), static_cast<std::ptrdiff_t>(c.count()), make_gen());
        });

    static_assert([]() constexpr {
        std::array<T, 5> a{};
        int n = 10;
        auto end = raze::algorithm::generate_n(a.begin(), 3, [&n]() constexpr -> T { return static_cast<T>(n++); });
        auto z = raze::algorithm::generate_n(a.begin(), 0, [&n]() constexpr -> T { return static_cast<T>(n++); });
        auto neg = raze::algorithm::generate_n(a.begin(), -5, [&n]() constexpr -> T { return static_cast<T>(n++); });
        return end == a.begin() + 3 && z == a.begin() && neg == a.begin()
            && a[0] == T(10) && a[1] == T(11) && a[2] == T(12) && a[3] == T(0);
    }());
};

RTTS_CASE("raze::algorithm::generate_n output iterators and negative counts") {
    std::vector<int> out;
    int x = 1;
    raze::algorithm::generate_n(std::back_inserter(out), 5, [&x]() {
        const int cur = x++;
        return cur * cur;
    });
    RTTS_EXPECT((out == std::vector<int>{ 1, 4, 9, 16, 25 }));

    int calls = 0;
    raze::algorithm::generate_n(std::back_inserter(out), -3, [&calls]() { return ++calls; });
    RTTS_EQUAL(calls, 0);
    RTTS_EQUAL(out.size(), size_t(5));
};

