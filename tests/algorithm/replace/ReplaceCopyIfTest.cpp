#include <tests/rts/rts.h>
#include <raze/algorithm/replace/ReplaceCopyIf.h>

#include <algorithm>

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};

RTTS_CASE_TPL("raze::algorithm::replace_copy_if", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto check = [] <class F, class G> (const std::vector<T>&value, F raze_op, G std_op) {
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

            RTTS_EQUAL(result.size(), expected.size());
        }

        RTTS_ALL_EQUAL(result, expected);
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
            [=](auto const& v, auto out) { return raze::algorithm::replace_copy_if(v.begin(), v.end(), out, pred_scalar, new_value); },
            [=](auto const& v, auto out) { return std::ranges::replace_copy_if(v.begin(), v.end(), out, pred_scalar, new_value); });

        check(value,
            [=](auto const& v, auto out) { return raze::algorithm::replace_copy_if(v.begin(), v.end(), out, pred_vector, new_value); },
            [=](auto const& v, auto out) { return std::ranges::replace_copy_if(v.begin(), v.end(), out, pred_vector, new_value); });
    }

    for (int i = 0; i < 500; ++i) {
        auto value = rtts::random::vector<T>(size_dist(rng), 30000 + i);
        const T target = rtts::random::generator<T>(40000 + i)();
        const T new_value = rtts::random::generator<T>(50000 + i)();

        const auto pred_scalar = [target](const T& x) { return x == target; };
        const auto pred_vector = [target](auto x) { return x == target; };

        check(value,
            [=](auto const& v, auto out) { return raze::algorithm::replace_copy_if(v.begin(), v.end(), out, pred_scalar, new_value); },
            [=](auto const& v, auto out) { return std::ranges::replace_copy_if(v.begin(), v.end(), out, pred_scalar, new_value); });

        check(value,
            [=](auto const& v, auto out) { return raze::algorithm::replace_copy_if(v.begin(), v.end(), out, pred_vector, new_value); },
            [=](auto const& v, auto out) { return std::ranges::replace_copy_if(v.begin(), v.end(), out, pred_vector, new_value); });
    }

    for (int i = 0; i < 100; ++i) {
        auto value = rtts::random::vector<T>(size_dist(rng), 60000 + i);

        check(value,
            [](auto const& v, auto out) { return raze::algorithm::replace_copy_if(v.begin(), v.end(), out, [](const T&) { return false; }, T(42)); },
            [](auto const& v, auto out) { return std::ranges::replace_copy_if(v.begin(), v.end(), out, [](const T&) { return false; }, T(42)); });

        check(value,
            [](auto const& v, auto out) { return raze::algorithm::replace_copy_if(v.begin(), v.end(), out, [](auto) { return false; }, T(42)); },
            [](auto const& v, auto out) { return std::ranges::replace_copy_if(v.begin(), v.end(), out, [](auto) { return false; }, T(42)); });
    }

    for (int i = 0; i < 100; ++i) {
        auto value = rtts::random::vector<T>(size_dist(rng), 70000 + i);

        check(value,
            [](auto const& v, auto out) { return raze::algorithm::replace_copy_if(v.begin(), v.end(), out, [](const T&) { return true; }, T(42)); },
            [](auto const& v, auto out) { return std::ranges::replace_copy_if(v.begin(), v.end(), out, [](const T&) { return true; }, T(42)); });

        check(value,
            [](auto const& v, auto out) { return raze::algorithm::replace_copy_if(v.begin(), v.end(), out, [](auto) { return true; }, T(42)); },
            [](auto const& v, auto out) { return std::ranges::replace_copy_if(v.begin(), v.end(), out, [](auto) { return true; }, T(42)); });
    }

    for (int i = 0; i < 1000; ++i) {
        auto value = rtts::random::vector<T>(size_dist(rng), 80000 + i);
        const T threshold = rtts::random::generator<T>(90000 + i)();
        const T new_value = rtts::random::generator<T>(100000 + i)();

        const auto pred_scalar = [threshold](const T& x) { return x > threshold; };
        const auto pred_vector = [threshold](auto x) { return x > threshold; };

        check(value,
            [=](auto const& v, auto out) { return raze::algorithm::replace_copy_if(v, out, pred_scalar, new_value); },
            [=](auto const& v, auto out) { return std::ranges::replace_copy_if(v, out, pred_scalar, new_value); });

        check(value,
            [=](auto const& v, auto out) { return raze::algorithm::replace_copy_if(v, out, pred_vector, new_value); },
            [=](auto const& v, auto out) { return std::ranges::replace_copy_if(v, out, pred_vector, new_value); });
    }

    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        auto value = rtts::random::vector<T>(size, 110000 + static_cast<unsigned>(size));

        check(value,
            [](auto const& v, auto out) { return raze::algorithm::replace_copy_if(v.begin(), v.end(), out, [](const T& x) { return x > T(0); }, T(0)); },
            [](auto const& v, auto out) { return std::ranges::replace_copy_if(v.begin(), v.end(), out, [](const T& x) { return x > T(0); }, T(0)); });

        check(value,
            [](auto const& v, auto out) { return raze::algorithm::replace_copy_if(v.begin(), v.end(), out, [](auto x) { return x > T(0); }, T(0)); },
            [](auto const& v, auto out) { return std::ranges::replace_copy_if(v.begin(), v.end(), out, [](auto x) { return x > T(0); }, T(0)); });
    }

    for (size_t size : {10000, 50000, 100000}) {
        auto value = rtts::random::vector<T>(size, 120000 + static_cast<unsigned>(size));

        check(value,
            [](auto const& v, auto out) { return raze::algorithm::replace_copy_if(v.begin(), v.end(), out, [](const T& x) { return x > T(0); }, T(0)); },
            [](auto const& v, auto out) { return std::ranges::replace_copy_if(v.begin(), v.end(), out, [](const T& x) { return x > T(0); }, T(0)); });

        check(value,
            [](auto const& v, auto out) { return raze::algorithm::replace_copy_if(v.begin(), v.end(), out, [](auto x) { return x > T(0); }, T(0)); },
            [](auto const& v, auto out) { return std::ranges::replace_copy_if(v.begin(), v.end(), out, [](auto x) { return x > T(0); }, T(0)); });
    }
};

RTTS_CASE("raze::algorithm::replace_copy_if.projection") {
    for (int i = 0; i < 100; ++i) {
        std::vector<Point> value(100), result(100), expected(100);
        auto gen = rtts::random::generator<int>(130000 + i);

        for (auto& p : value)
            p = { gen(), gen() };

        const int threshold = gen();
        const Point replacement{ 777, 666 };

        const auto pred = [threshold](int x) { return x > threshold; };
        const auto proj = [](const Point& p) { return p.x; };

        const auto raze_ret = raze::algorithm::replace_copy_if(
            value.begin(), value.end(), result.begin(), pred, replacement, proj);

        const auto std_ret = std::ranges::replace_copy_if(
            value.begin(), value.end(), expected.begin(), pred, replacement, proj);

        RTTS_EQUAL(
            std::distance(value.begin(), raze_ret.in),
            std::distance(value.begin(), std_ret.in));

        RTTS_EQUAL(
            std::distance(result.begin(), raze_ret.out),
            std::distance(expected.begin(), std_ret.out));

        RTTS_ALL_EQUAL(result, expected);
    }
};

RTTS_CASE_TPL("raze::algorithm::replace_copy_if.constexpr", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    constexpr auto test = [] {
        std::array<T, 8> input{
            T(1), T(2), T(3), T(2),
            T(4), T(2), T(5), T(1)
        };

        std::array<T, 8> result{};
        std::array<T, 8> expected{};

        const auto pred_scalar = [](const T& x) constexpr { return x == T(2); };
        const auto pred_vector = [](auto x) constexpr { return x == T(2); };

        const auto raze_scalar = raze::algorithm::replace_copy_if(input.begin(), input.end(), result.begin(), pred_scalar, T(99));
        const auto std_scalar = std::ranges::replace_copy_if(input.begin(), input.end(), expected.begin(), pred_scalar, T(99));

        if (std::distance(input.begin(), raze_scalar.in) !=  std::distance(input.begin(), std_scalar.in)) return false;
        if (std::distance(result.begin(), raze_scalar.out) != std::distance(expected.begin(), std_scalar.out)) return false;

        if (result != expected) return false;

        result.fill(T(0));
        expected.fill(T(0));

        const auto raze_vector = raze::algorithm::replace_copy_if(input, result.begin(), pred_vector, T(77));
        const auto std_vector = std::ranges::replace_copy_if(input, expected.begin(), pred_vector, T(77));

        if (std::distance(input.begin(), raze_vector.in) != std::distance(input.begin(), std_vector.in)) return false;
        if (std::distance(result.begin(), raze_vector.out) != std::distance(expected.begin(), std_vector.out)) return false;

        return result == expected;
    };

	constexpr bool result = test();
    RTTS_EXPECT(result);
};
