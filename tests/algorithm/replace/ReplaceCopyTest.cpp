#include <tests/rts/rts.h>
#include <raze/algorithm/replace/ReplaceCopy.h>

#include <algorithm>

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};

RTTS_CASE_TPL("raze::algorithm::replace_copy", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto check = [] <class F, class G> (const std::vector<T>& value, F raze_op, G std_op) {
        std::vector<T> result(value.size());
        std::vector<T> expected(value.size());

        if (value.size() % 2 == 0) {
            const auto raze_ret = raze_op(value, result.begin());
            const auto std_ret = std_op(value, expected.begin());

            RTTS_EQUAL(
                std::distance(value.begin(), raze_ret.in),
                std::distance(value.begin(), std_ret.in));

            RTTS_EQUAL(
                std::distance(result.begin(), raze_ret.out),
                std::distance(expected.begin(), std_ret.out));
        }
        else {
            result.clear();
            expected.clear();

            const auto raze_ret = raze_op(value, std::back_inserter(result));
            const auto std_ret = std_op(value, std::back_inserter(expected));

            RTTS_EQUAL(
                std::distance(value.begin(), raze_ret.in),
                std::distance(value.begin(), std_ret.in));
        }

        RTTS_ALL_EQUAL(result, expected);
    };

    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> size_dist(0, 1000);

    for (int i = 0; i < 1000; ++i) {
        auto value = rtts::random::vector<T>(size_dist(rng), 42 + i);
        const T old_value = rtts::random::generator<T>(1000 + i)();
        const T new_value = rtts::random::generator<T>(2000 + i)();

        check(value,
            [=](auto const& v, auto out) { return raze::algorithm::replace_copy(v.begin(), v.end(), out, old_value, new_value); },
            [=](auto const& v, auto out) { return std::ranges::replace_copy(v.begin(), v.end(), out, old_value, new_value); });
    }

    for (int i = 0; i < 500; ++i) {
        auto value = rtts::random::vector<T>(size_dist(rng), 30000 + i);
        const T old_value = value.empty() ? T(0) : value[value.size() / 2];
        const T new_value = old_value == T(0) ? T(1) : T(0);

        check(value,
            [=](auto const& v, auto out) { return raze::algorithm::replace_copy(v.begin(), v.end(), out, old_value, new_value); },
            [=](auto const& v, auto out) { return std::ranges::replace_copy(v.begin(), v.end(), out, old_value, new_value); });
    }

    for (int i = 0; i < 500; ++i) {
        auto value = rtts::random::vector<T>(size_dist(rng), 40000 + i);
        const T old_value = rtts::random::generator<T>(50000 + i)();

        check(value,
            [=](auto const& v, auto out) { return raze::algorithm::replace_copy(v.begin(), v.end(), out, old_value, old_value); },
            [=](auto const& v, auto out) { return std::ranges::replace_copy(v.begin(), v.end(), out, old_value, old_value); });
    }

    for (int i = 0; i < 1000; ++i) {
        auto value = rtts::random::vector<T>(size_dist(rng), 60000 + i);
        const T old_value = rtts::random::generator<T>(70000 + i)();
        const T new_value = rtts::random::generator<T>(80000 + i)();

        check(value,
            [=](auto const& v, auto out) { return raze::algorithm::replace_copy(v, out, old_value, new_value); },
            [=](auto const& v, auto out) { return std::ranges::replace_copy(v, out, old_value, new_value); });
    }

    for (size_t size : {0, 1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        check(std::vector<T>(size, T(42)),
            [](auto const& v, auto out) { return raze::algorithm::replace_copy(v.begin(), v.end(), out, T(42), T(99)); },
            [](auto const& v, auto out) { return std::ranges::replace_copy(v.begin(), v.end(), out, T(42), T(99)); });
    }

    for (size_t size : {10000, 50000, 100000}) {
        auto value = rtts::random::vector<T>(size, 90000 + static_cast<unsigned>(size));
        const T old_value = value[size / 3];
        const T new_value = old_value == T(0) ? T(1) : T(0);

        check(value,
            [=](auto const& v, auto out) { return raze::algorithm::replace_copy(v.begin(), v.end(), out, old_value, new_value); },
            [=](auto const& v, auto out) { return std::ranges::replace_copy(v.begin(), v.end(), out, old_value, new_value); });
    }

    for (size_t size : {1, 10, 100, 1000}) {
        check(std::vector<T>(size, T(42)),
            [](auto const& v, auto out) { return raze::algorithm::replace_copy(v.begin(), v.end(), out, T(42), T(99)); },
            [](auto const& v, auto out) { return std::ranges::replace_copy(v.begin(), v.end(), out, T(42), T(99)); });

        check(std::vector<T>(size, T(42)),
            [](auto const& v, auto out) { return raze::algorithm::replace_copy(v.begin(), v.end(), out, T(99), T(0)); },
            [](auto const& v, auto out) { return std::ranges::replace_copy(v.begin(), v.end(), out, T(99), T(0)); });
    }

    for (int i = 0; i < 500; ++i) {
        auto value = rtts::random::vector<T>(10 + (rng() % 491), 100000 + i);
        const T old_value = value[value.size() / 4];
        const T new_value = rtts::random::generator<T>(110000 + i)();

        check(value,
            [=](auto const& v, auto out) { return raze::algorithm::replace_copy(v.begin(), v.end(), out, old_value, new_value); },
            [=](auto const& v, auto out) { return std::ranges::replace_copy(v.begin(), v.end(), out, old_value, new_value); });
    }
};

RTTS_CASE("raze::algorithm::replace_copy.projection") {
    for (int i = 0; i < 100; ++i) {
        std::vector<Point> value(100), result(100), expected(100);
        auto gen = rtts::random::generator<int>(120000 + i);

        for (auto& p : value)
            p = {gen(), gen()};

        const int target = gen();
        const Point replacement{999, 888};
        const auto proj = [](const Point& p) { return p.x; };

        const auto raze_ret = raze::algorithm::replace_copy(
            value.begin(), value.end(), result.begin(), target, replacement, proj);

        const auto std_ret = std::ranges::replace_copy(
            value.begin(), value.end(), expected.begin(), target, replacement, proj);

        RTTS_EQUAL(
            std::distance(value.begin(), raze_ret.in),
            std::distance(value.begin(), std_ret.in));

        RTTS_EQUAL(
            std::distance(result.begin(), raze_ret.out),
            std::distance(expected.begin(), std_ret.out));

        RTTS_ALL_EQUAL(result, expected);
    }
};

RTTS_CASE_TPL("raze::algorithm::replace_copy.constexpr", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    constexpr auto test = [] {
        std::array<T, 8> input{
            T(1), T(2), T(3), T(2),
            T(4), T(2), T(5), T(1)
        };

        std::array<T, 8> result{};
        std::array<T, 8> expected{};

        const auto raze_ret = raze::algorithm::replace_copy(
            input.begin(), input.end(), result.begin(), T(2), T(99));

        const auto std_ret = std::ranges::replace_copy(
            input.begin(), input.end(), expected.begin(), T(2), T(99));

        if (std::distance(input.begin(), raze_ret.in) !=
            std::distance(input.begin(), std_ret.in))
            return false;

        if (std::distance(result.begin(), raze_ret.out) !=
            std::distance(expected.begin(), std_ret.out))
            return false;

        return result == expected;
    };

    constexpr bool result = test();
    RTTS_EXPECT(result);
};
