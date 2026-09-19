#include <tests/rts/rts.h>
#include <raze/algorithm/batch/ForEachN.h>

#include <algorithm>

RTTS_CASE_TPL("raze::algorithm::for_each_n", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto check = [] <class F, class G> (std::vector<T> value, size_t n, F raze_op, G std_op) {
        auto expected = value;

        const auto raze_ret = raze_op(value, n);
        const auto std_ret = std_op(expected, n);

        RTTS_EQUAL(
            std::distance(value.begin(), raze_ret.in),
            std::distance(expected.begin(), std_ret.in));

        RTTS_ALL_EQUAL(value, expected);
    };

    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> size_dist(0, 1000);

    for (int i = 0; i < 1000; ++i) {
        auto value = rtts::random::vector<T>(size_dist(rng), 42 + i);
        const size_t n = value.empty() ? 0 : rng() % (value.size() + 1);
        const T add = rtts::random::generator<T>(1000 + i)();

        const auto f_scalar = [add](T& x) { x += add; };
        const auto f_vector = [add](auto& x) { x += add; };

        check(value, n,
            [=](auto& v, size_t count) { return raze::algorithm::for_each_n(v.begin(), count, f_scalar); },
            [=](auto& v, size_t count) { return std::ranges::for_each_n(v.begin(), count, f_scalar); });

        check(std::move(value), n,
            [=](auto& v, size_t count) { return raze::algorithm::for_each_n(v.begin(), count, f_vector); },
            [=](auto& v, size_t count) { return std::ranges::for_each_n(v.begin(), count, f_vector); });
    }

    for (int i = 0; i < 500; ++i) {
        auto value = rtts::random::vector<T>(size_dist(rng), 30000 + i);
        const size_t n = value.empty() ? 0 : value.size() / 2;
        const T mul = rtts::random::generator<T>(40000 + i)();

        const auto f_scalar = [mul](T& x) { x *= mul; };
        const auto f_vector = [mul](auto& x) { x *= mul; };

        check(value, n,
            [=](auto& v, size_t count) { return raze::algorithm::for_each_n(v.begin(), count, f_scalar); },
            [=](auto& v, size_t count) { return std::ranges::for_each_n(v.begin(), count, f_scalar); });

        check(std::move(value), n,
            [=](auto& v, size_t count) { return raze::algorithm::for_each_n(v.begin(), count, f_vector); },
            [=](auto& v, size_t count) { return std::ranges::for_each_n(v.begin(), count, f_vector); });
    }

    for (int i = 0; i < 100; ++i) {
        auto value = rtts::random::vector<T>(size_dist(rng), 50000 + i);

        check(value, 0,
            [](auto& v, size_t n) { return raze::algorithm::for_each_n(v.begin(), n, [](T&) {}); },
            [](auto& v, size_t n) { return std::ranges::for_each_n(v.begin(), n, [](T&) {}); });

        check(std::move(value), 0,
            [](auto& v, size_t n) { return raze::algorithm::for_each_n(v.begin(), n, [](auto&) {}); },
            [](auto& v, size_t n) { return std::ranges::for_each_n(v.begin(), n, [](auto&) {}); });
    }

    for (size_t size : {0, 1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        auto value = rtts::random::vector<T>(size, 60000 + static_cast<unsigned>(size));

        for (size_t n : {size_t(0), size_t(size / 2), size_t(size)}) {
            check(value, n,
                [](auto& v, size_t count) { return raze::algorithm::for_each_n(v.begin(), count, [](T& x) { x = T(42); }); },
                [](auto& v, size_t count) { return std::ranges::for_each_n(v.begin(), count, [](T& x) { x = T(42); }); });

            check(value, n,
                [](auto& v, size_t count) { return raze::algorithm::for_each_n(v.begin(), count, [](auto& x) { x = T(42); }); },
                [](auto& v, size_t count) { return std::ranges::for_each_n(v.begin(), count, [](auto& x) { x = T(42); }); });
        }
    }

    for (size_t size : {10000, 50000, 100000}) {
        auto value = rtts::random::vector<T>(size, 70000 + static_cast<unsigned>(size));

        for (size_t n : {size_t(1), size_t(size / 2), size_t(size)}) {
            check(value, n,
                [](auto& v, size_t count) { return raze::algorithm::for_each_n(v.begin(), count, [](T& x) { x += T(1); }); },
                [](auto& v, size_t count) { return std::ranges::for_each_n(v.begin(), count, [](T& x) { x += T(1); }); });

            check(value, n,
                [](auto& v, size_t count) { return raze::algorithm::for_each_n(v.begin(), count, [](auto& x) { x += T(1); }); },
                [](auto& v, size_t count) { return std::ranges::for_each_n(v.begin(), count, [](auto& x) { x += T(1); }); });
        }
    }
};

RTTS_CASE_TPL("raze::algorithm::for_each_n.projection", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto check = [] <class F, class G> (std::vector<T> value, size_t n, F raze_op, G std_op) {
        auto expected = value;

        const auto raze_ret = raze_op(value, n);
        const auto std_ret = std_op(expected, n);

        RTTS_EQUAL(
            std::distance(value.begin(), raze_ret.in),
            std::distance(expected.begin(), std_ret.in));

        RTTS_ALL_EQUAL(value, expected);
    };

    for (int i = 0; i < 500; ++i) {
        auto value = rtts::random::vector<T>(100, 80000 + i);
        const size_t n = i % 101;
        const T add = rtts::random::generator<T>(90000 + i)();

        const auto proj_scalar = [](T& x) -> T& { return x; };
        const auto proj_vector = [](auto& x) -> auto& { return x; };

        const auto f_scalar = [add](T& x) { x += add; };
        const auto f_vector = [add](auto& x) { x += add; };

        check(value, n,
            [=](auto& v, size_t count) { return raze::algorithm::for_each_n(v.begin(), count, f_scalar, proj_scalar); },
            [=](auto& v, size_t count) { return std::ranges::for_each_n(v.begin(), count, f_scalar, proj_scalar); });

        check(std::move(value), n,
            [=](auto& v, size_t count) { return raze::algorithm::for_each_n(v.begin(), count, f_vector, proj_vector); },
            [=](auto& v, size_t count) { return std::ranges::for_each_n(v.begin(), count, f_vector, proj_vector); });
    }
};

RTTS_CASE_TPL("raze::algorithm::for_each_n.constexpr", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    constexpr auto test = [] {
        std::array<T, 8> value{T(1), T(2), T(3), T(4), T(5), T(6), T(7), T(8)};
        std::array<T, 8> expected = value;

        const auto raze_ret = raze::algorithm::for_each_n(
            value.begin(), 5,
            [](auto& x) constexpr { x += T(10); });

        const auto std_ret = std::ranges::for_each_n(
            expected.begin(), 5,
            [](auto& x) constexpr { x += T(10); });

        if (std::distance(value.begin(), raze_ret.in) !=
            std::distance(expected.begin(), std_ret.in))
            return false;

        return std::equal(value.begin(), value.end(), expected.begin());
    };

    constexpr bool result = test();
    RTTS_EXPECT(result);
};
