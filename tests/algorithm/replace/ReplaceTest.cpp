#include <tests/rts/rts.h>
#include <raze/algorithm/replace/Replace.h>

#include <algorithm>

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};

RTTS_CASE_TPL("raze::algorithm::replace", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto check = [] <class F, class G> (std::vector<T> value, F raze_op, G std_op) {
        auto expected = value;

        raze_op(value);
        std_op(expected);

        RTTS_ALL_EQUAL(value, expected);
    };

    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> size_dist(0, 1000);

    for (int i = 0; i < 1000; ++i) {
        auto value = rtts::random::vector<T>(size_dist(rng), 42 + i);
        const T old_value = rtts::random::generator<T>(1000 + i)();
        const T new_value = rtts::random::generator<T>(2000 + i)();

        check(value,
            [=](auto& v) { raze::algorithm::replace(v.begin(), v.end(), old_value, new_value); },
            [=](auto& v) { std::ranges::replace(v.begin(), v.end(), old_value, new_value); });
    }

    for (int i = 0; i < 500; ++i) {
        auto value = rtts::random::vector<T>(size_dist(rng), 30000 + i);
        const T old_value = value.empty() ? T(0) : value[value.size() / 2];
        const T new_value = old_value == T(0) ? T(1) : T(0);

        check(std::move(value),
            [=](auto& v) { raze::algorithm::replace(v.begin(), v.end(), old_value, new_value); },
            [=](auto& v) { std::ranges::replace(v.begin(), v.end(), old_value, new_value); });
    }

    for (int i = 0; i < 500; ++i) {
        auto value = rtts::random::vector<T>(size_dist(rng), 40000 + i);
        const T old_value = rtts::random::generator<T>(50000 + i)();

        check(std::move(value),
            [=](auto& v) { raze::algorithm::replace(v.begin(), v.end(), old_value, old_value); },
            [=](auto& v) { std::ranges::replace(v.begin(), v.end(), old_value, old_value); });
    }

    for (int i = 0; i < 1000; ++i) {
        auto value = rtts::random::vector<T>(size_dist(rng), 60000 + i);
        const T old_value = rtts::random::generator<T>(70000 + i)();
        const T new_value = rtts::random::generator<T>(80000 + i)();

        check(value,
            [=](auto& v) { raze::algorithm::replace(v, old_value, new_value); },
            [=](auto& v) { std::ranges::replace(v, old_value, new_value); });
    }

    for (size_t size : {0, 1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        check(std::vector<T>(size, T(42)),
            [](auto& v) { raze::algorithm::replace(v.begin(), v.end(), T(42), T(99)); },
            [](auto& v) { std::ranges::replace(v.begin(), v.end(), T(42), T(99)); });
    }

    for (size_t size : {10000, 50000, 100000}) {
        auto value = rtts::random::vector<T>(size, 90000 + static_cast<unsigned>(size));
        const T old_value = value[size / 3];
        const T new_value = old_value == T(0) ? T(1) : T(0);

        check(std::move(value),
            [=](auto& v) { raze::algorithm::replace(v.begin(), v.end(), old_value, new_value); },
            [=](auto& v) { std::ranges::replace(v.begin(), v.end(), old_value, new_value); });
    }

    for (size_t size : {1, 10, 100, 1000}) {
        check(std::vector<T>(size, T(42)),
            [](auto& v) { raze::algorithm::replace(v.begin(), v.end(), T(42), T(99)); },
            [](auto& v) { std::ranges::replace(v.begin(), v.end(), T(42), T(99)); });

        check(std::vector<T>(size, T(42)),
            [](auto& v) { raze::algorithm::replace(v.begin(), v.end(), T(99), T(0)); },
            [](auto& v) { std::ranges::replace(v.begin(), v.end(), T(99), T(0)); });
    }

    for (int i = 0; i < 500; ++i) {
        auto value = rtts::random::vector<T>(10 + (rng() % 491), 100000 + i);
        const T old_value = value[value.size() / 4];
        const T new_value = rtts::random::generator<T>(110000 + i)();

        check(std::move(value),
            [=](auto& v) { raze::algorithm::replace(v.begin(), v.end(), old_value, new_value); },
            [=](auto& v) { std::ranges::replace(v.begin(), v.end(), old_value, new_value); });
    }
};

RTTS_CASE("raze::algorithm::replace.projection") {
    for (int i = 0; i < 100; ++i) {
        std::vector<Point> value(100);
        auto gen = rtts::random::generator<int>(120000 + i);

        for (auto& p : value)
            p = {gen(), gen()};

        const int target = gen();
        const Point replacement{999, 888};
        const auto proj = [](const Point& p) { return p.x; };

        auto expected = value;

        raze::algorithm::replace(value.begin(), value.end(), target, replacement, proj);
        std::ranges::replace(expected.begin(), expected.end(), target, replacement, proj);

        RTTS_ALL_EQUAL(value, expected);
    }
};
