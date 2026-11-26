#include <tests/rts/rts.h>
#include <raze/algorithm/order/ShiftRight.h>

#include <algorithm>
#include <array>
#include <forward_list>
#include <list>
#include <memory>
#include <ranges>
#include <vector>

RTTS_CASE_TPL("raze::algorithm::shift_right", rtts::algorithm::all_types)
<class T>(rtts::type<T>) {
    namespace opt = raze::options;

    for (size_t sz : { size_t(0), size_t(1), size_t(2), size_t(5), size_t(16), size_t(65) }) {
        std::vector<T> base(sz);
        for (size_t i = 0; i < sz; ++i)
            base[i] = static_cast<T>((i % 97) + 1);

        for (std::ptrdiff_t sh : { std::ptrdiff_t(0), std::ptrdiff_t(1), std::ptrdiff_t(2),
            static_cast<std::ptrdiff_t>(sz / 2), static_cast<std::ptrdiff_t>(sz), static_cast<std::ptrdiff_t>(sz + 3) })
        {
            auto exp = base;
            auto exp_beg = std::shift_right(exp.begin(), exp.end(), sh);
            const auto exp_off = exp_beg - exp.begin();

            auto check = [&](auto fn) {
                auto v = base;
                auto r = fn(v, sh);
                RTTS_EXPECT(r.begin() - v.begin() == exp_off);
                RTTS_EXPECT(r.end() == v.end());
                RTTS_EXPECT(std::ranges::equal(r, std::ranges::subrange(exp_beg, exp.end())));
            };

            check([](auto& v, auto n) { return raze::algorithm::shift_right(v, n); });
            check([](auto& v, auto n) { return raze::algorithm::shift_right(v.begin(), v.end(), n); });
            check([](auto& v, auto n) { return raze::algorithm::shift_right[opt::fscalar](v, n); });
            check([](auto& v, auto n) { return raze::algorithm::shift_right[opt::fstatic](v, n); });
        }
    }

    static_assert([]() constexpr {
        std::array<T, 5> a{ T(1), T(2), T(3), T(4), T(5) };
        auto r = raze::algorithm::shift_right(a, 2);
        return r.begin() == a.begin() + 2 && r.end() == a.end()
            && a[2] == T(1) && a[3] == T(2) && a[4] == T(3);
    }());
};

RTTS_CASE("raze::algorithm::shift_right forward-only, bidirectional, move-only, and dangling") {
    // Forward-only container (tests cyclic forward shift_right algorithm!)
    for (std::ptrdiff_t sh : { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 }) {
        std::forward_list<int> fl{ 1, 2, 3, 4, 5, 6, 7 };
        std::forward_list<int> exp = fl;
        auto eb = std::shift_right(exp.begin(), exp.end(), sh);
        auto r = raze::algorithm::shift_right(fl, sh);
        RTTS_EXPECT(r.end() == fl.end());
        RTTS_EXPECT(std::ranges::equal(r, std::ranges::subrange(eb, exp.end())));
    }

    // Bidirectional std::list
    {
        std::list<int> lst{ 10, 20, 30, 40, 50 };
        auto r = raze::algorithm::shift_right(lst, 2);
        RTTS_EXPECT((std::vector<int>(r.begin(), r.end()) == std::vector<int>{ 10, 20, 30 }));
    }

    // Move-only unique_ptr
    {
        std::vector<std::unique_ptr<int>> v;
        for (int i = 1; i <= 5; ++i)
            v.push_back(std::make_unique<int>(i));
        auto r = raze::algorithm::shift_right(v, 2);
        RTTS_EQUAL(r.size(), size_t(3));
        RTTS_EXPECT(*v[2] == 1 && *v[3] == 2 && *v[4] == 3);
    }

    // Non-common sentinel & rvalue dangling
    {
        std::vector<int> v{ 1, 2, 3, 4 };
        auto r = raze::algorithm::shift_right(rtts::make_non_common(v), 1);
        RTTS_EXPECT((std::vector<int>(r.begin(), r.end()) == std::vector<int>{ 1, 2, 3 }));

        auto dr = raze::algorithm::shift_right(std::vector<int>{ 1, 2, 3 }, 1);
        static_assert(std::same_as<decltype(dr), std::ranges::dangling>);
    }
};

