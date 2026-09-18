#include <tests/rts/rts.h>
#include <raze/algorithm/replace/Replace.h>

#include <algorithm>

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};

RTTS_CASE_TPL("raze::algorithm::replace_if", rtts::algorithm::all_types)
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
        const T threshold = rtts::random::generator<T>(1000 + i)();
        const T new_value = rtts::random::generator<T>(2000 + i)();

        const auto pred_scalar = [threshold](const T& x) { return x > threshold; };
        const auto pred_vector = [threshold](auto x) { return x > threshold; };

        check(value,
            [=](auto& v) { raze::algorithm::replace_if(v.begin(), v.end(), pred_scalar, new_value); },
            [=](auto& v) { std::ranges::replace_if(v.begin(), v.end(), pred_scalar, new_value); });

        check(std::move(value),
            [=](auto& v) { raze::algorithm::replace_if(v.begin(), v.end(), pred_vector, new_value); },
            [=](auto& v) { std::ranges::replace_if(v.begin(), v.end(), pred_vector, new_value); });
    }

    for (int i = 0; i < 500; ++i) {
        auto value = rtts::random::vector<T>(size_dist(rng), 30000 + i);
        const T target = rtts::random::generator<T>(40000 + i)();
        const T new_value = rtts::random::generator<T>(50000 + i)();

        const auto pred_scalar = [target](const T& x) { return x == target; };
        const auto pred_vector = [target](auto x) { return x == target; };

        check(value,
            [=](auto& v) { raze::algorithm::replace_if(v.begin(), v.end(), pred_scalar, new_value); },
            [=](auto& v) { std::ranges::replace_if(v.begin(), v.end(), pred_scalar, new_value); });

        check(std::move(value),
            [=](auto& v) { raze::algorithm::replace_if(v.begin(), v.end(), pred_vector, new_value); },
            [=](auto& v) { std::ranges::replace_if(v.begin(), v.end(), pred_vector, new_value); });
    }

    for (int i = 0; i < 100; ++i) {
        auto value = rtts::random::vector<T>(size_dist(rng), 60000 + i);
        check(value,
            [](auto& v) { raze::algorithm::replace_if(v.begin(), v.end(), [](const T&) { return false; }, T(42)); },
            [](auto& v) { std::ranges::replace_if(v.begin(), v.end(), [](const T&) { return false; }, T(42)); });

        check(std::move(value),
            [](auto& v) { raze::algorithm::replace_if(v.begin(), v.end(), [](auto) { return false; }, T(42)); },
            [](auto& v) { std::ranges::replace_if(v.begin(), v.end(), [](auto) { return false; }, T(42)); });
    }

    for (int i = 0; i < 100; ++i) {
        auto value = rtts::random::vector<T>(size_dist(rng), 70000 + i);
        check(value,
            [](auto& v) { raze::algorithm::replace_if(v.begin(), v.end(), [](const T&) { return true; }, T(42)); },
            [](auto& v) { std::ranges::replace_if(v.begin(), v.end(), [](const T&) { return true; }, T(42)); });

        check(std::move(value),
            [](auto& v) { raze::algorithm::replace_if(v.begin(), v.end(), [](auto) { return true; }, T(42)); },
            [](auto& v) { std::ranges::replace_if(v.begin(), v.end(), [](auto) { return true; }, T(42)); });
    }

    for (int i = 0; i < 1000; ++i) {
        auto value = rtts::random::vector<T>(size_dist(rng), 80000 + i);
        const T threshold = rtts::random::generator<T>(90000 + i)();
        const T new_value = rtts::random::generator<T>(100000 + i)();

        const auto pred_scalar = [threshold](const T& x) { return x > threshold; };
        const auto pred_vector = [threshold](auto x) { return x > threshold; };

        check(value,
            [=](auto& v) { raze::algorithm::replace_if(v, pred_scalar, new_value); },
            [=](auto& v) { std::ranges::replace_if(v, pred_scalar, new_value); });
        check(std::move(value),
            [=](auto& v) { raze::algorithm::replace_if(v, pred_vector, new_value); },
            [=](auto& v) { std::ranges::replace_if(v, pred_vector, new_value); });
    }

    for (size_t size : {  1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256 }) {
        auto value = rtts::random::vector<T>(size, 110000 + static_cast<unsigned>(size));
        check(value,
            [](auto& v) { raze::algorithm::replace_if(v.begin(), v.end(), [](const T& x) { return x > T(0); }, T(0)); },
            [](auto& v) { std::ranges::replace_if(v.begin(), v.end(), [](const T& x) { return x > T(0); }, T(0)); });
        check(std::move(value),
            [](auto& v) { raze::algorithm::replace_if(v.begin(), v.end(), [](auto x) { return x > T(0); }, T(0)); },
            [](auto& v) { std::ranges::replace_if(v.begin(), v.end(), [](auto x) { return x > T(0); }, T(0)); });
    }

    for (size_t size : {10000, 50000, 100000}) {
        auto value = rtts::random::vector<T>(size, 120000 + static_cast<unsigned>(size));
        check(value,
            [](auto& v) { raze::algorithm::replace_if(v.begin(), v.end(),  [](const T& x) { return x > T(0); },  T(0)); },
            [](auto& v) { std::ranges::replace_if(v.begin(), v.end(), [](const T& x) { return x > T(0); }, T(0)); });
        check(std::move(value),
            [](auto& v) { raze::algorithm::replace_if(v.begin(), v.end(), [](auto x) { return x > T(0); }, T(0)); },
            [](auto& v) { std::ranges::replace_if(v.begin(), v.end(), [](auto x) { return x > T(0); }, T(0)); });
    }
};

RTTS_CASE("raze::algorithm::replace_if.projection") {
    for (int i = 0; i < 100; ++i) {
        std::vector<Point> value(100);
        auto gen = rtts::random::generator<int>(130000 + i);

        for (auto& p : value)
            p = {gen(), gen()};

        const int threshold = gen();
        const Point replacement{777, 666};

        const auto pred = [threshold](int x) { return x > threshold; };
        const auto proj = [](const Point& p) { return p.x; };

        auto expected = value;

        raze::algorithm::replace_if(value.begin(), value.end(), pred, replacement, proj);
        std::ranges::replace_if(expected.begin(), expected.end(), pred, replacement, proj);

        RTTS_ALL_EQUAL(value, expected);
    }
};

