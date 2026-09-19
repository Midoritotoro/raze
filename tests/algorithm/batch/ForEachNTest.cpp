#include <tests/rts/rts.h>
#include <raze/algorithm/batch/ForEachN.h>

#include <algorithm>

RTTS_CASE_TPL("raze::algorithm::for_each_n", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto check = [] <class Container, class F, class G> (Container value, size_t n, F raze_op, G std_op) {
        auto expected = value;

        const auto std_ret = std_op(expected, n);
        const auto raze_ret = raze_op(value, n);

        RTTS_EQUAL(
            std::distance(value.begin(), raze_ret.in),
            std::distance(expected.begin(), std_ret.in));

        RTTS_EXPECT(std::ranges::equal(value, expected));
    };

    auto check_counted = [] <class Container, class F, class G> (Container value, size_t n, F raze_op, G std_op) {
        auto expected = value;
        const auto size = std::ranges::distance(value);

        const auto raze_first = std::counted_iterator(value.begin(), size);
        const auto std_first = std::counted_iterator(expected.begin(), size);

        const auto std_ret = std_op(std_first, n);
        const auto raze_ret = raze_op(raze_first, n);

        RTTS_EQUAL(
            std::distance(value.begin(), raze_ret.in.base()),
            std::distance(expected.begin(), std_ret.in.base()));

        RTTS_EQUAL(raze_ret.in.count(), size - n);
        RTTS_EQUAL(std_ret.in.count(), size - n);

        RTTS_EXPECT(std::ranges::equal(value, expected));
    };

    auto run_all = [&] <class F> (size_t size, F f) {
        f(rtts::random::vector<T>(size, 42));
        f(rtts::random::deque<T>(size, 42));
        f(rtts::random::list<T>(size, 42));
        f(rtts::random::forward_list<T>(size, 42));
        f(rtts::random::array<T, 256>(42));
    };

    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> size_dist(0, 1000);

    for (int i = 0; i < 1000; ++i) {
        const size_t size = size_dist(rng);
        const size_t n = size == 0 ? 0 : rng() % (size + 1);
        const T add = rtts::random::generator<T>(1000 + i)();

        const auto f_scalar = [add](T& x) { x += add; };
        const auto f_vector = [add](auto& x) { x += add; };

        auto run = [&] <class Container> (Container value) {
            const size_t container_size = std::ranges::distance(value);
            if (container_size == 0) return;
            const size_t count = std::min(n, container_size - 1);

            check(value, count,
                [=](auto& v, size_t count) { return raze::algorithm::for_each_n(v.begin(), count, f_scalar); },
                [=](auto& v, size_t count) { return std::ranges::for_each_n(v.begin(), count, f_scalar); });

            check(std::move(value), count,
                [=](auto& v, size_t count) { return raze::algorithm::for_each_n(v.begin(), count, f_vector); },
                [=](auto& v, size_t count) { return std::ranges::for_each_n(v.begin(), count, f_vector); });

            auto counted_value = rtts::random::vector<T>(container_size, 2000 + i);
            if constexpr (std::same_as<Container, std::vector<T>>) {
                check_counted(std::move(counted_value), count,
                    [=](auto first, size_t count) { return raze::algorithm::for_each_n(first, count, f_scalar); },
                    [=](auto first, size_t count) { return std::ranges::for_each_n(first, count, f_scalar); });
            }
        };

        run_all(size, [&](auto value) { run(std::move(value)); });
    }

    for (int i = 0; i < 500; ++i) {
        const size_t size = size_dist(rng);
        const size_t n = size == 0 ? 0 : size / 2;
        const T mul = rtts::random::generator<T>(40000 + i)();

        const auto f_scalar = [mul](T& x) { x *= mul; };
        const auto f_vector = [mul](auto& x) { x *= mul; };

        auto run = [&] <class Container> (Container value) {
            const size_t container_size = std::ranges::distance(value);
            if (container_size == 0) return;
            const size_t count = std::min(n, container_size - 1);

            check(value, count,
                [=](auto& v, size_t count) { return raze::algorithm::for_each_n(v.begin(), count, f_scalar); },
                [=](auto& v, size_t count) { return std::ranges::for_each_n(v.begin(), count, f_scalar); });

            check(std::move(value), count,
                [=](auto& v, size_t count) { return raze::algorithm::for_each_n(v.begin(), count, f_vector); },
                [=](auto& v, size_t count) { return std::ranges::for_each_n(v.begin(), count, f_vector); });
        };

        run_all(size, [&](auto value) { run(std::move(value)); });
    }

    for (int i = 0; i < 100; ++i) {
        const size_t size = size_dist(rng);

        auto run = [&] <class Container> (Container value) {
            const size_t container_size = std::ranges::distance(value);
            if (container_size == 0) return;

            check(value, 0,
                [](auto& v, size_t n) { return raze::algorithm::for_each_n(v.begin(), n, [](T&) {}); },
                [](auto& v, size_t n) { return std::ranges::for_each_n(v.begin(), n, [](T&) {}); });

            check(std::move(value), 0,
                [](auto& v, size_t n) { return raze::algorithm::for_each_n(v.begin(), n, [](auto&) {}); },
                [](auto& v, size_t n) { return std::ranges::for_each_n(v.begin(), n, [](auto&) {}); });

            check_counted(value, 0,
                [](auto first, size_t n) { return raze::algorithm::for_each_n(first, n, [](T&) {}); },
                [](auto first, size_t n) { return std::ranges::for_each_n(first, n, [](T&) {}); });

            check_counted(std::move(value), 0,
                [](auto first, size_t n) { return raze::algorithm::for_each_n(first, n, [](auto&) {}); },
                [](auto first, size_t n) { return std::ranges::for_each_n(first, n, [](auto&) {}); });
        };

        run_all(size, [&](auto value) { run(std::move(value)); });
    }

   /* for (size_t size : {0, 1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        for (size_t n : {size_t(0), size_t(size / 2), size_t(size)}) {
            auto run = [&] <class Container> (Container value) {
                const size_t container_size = std::ranges::distance(value);
                if (container_size == 0) return;
                const size_t count = std::min(n, container_size - 1);

                check(value, count,
                    [](auto& v, size_t count) { return raze::algorithm::for_each_n(v.begin(), count, [](T& x) { x = T(42); }); },
                    [](auto& v, size_t count) { return std::ranges::for_each_n(v.begin(), count, [](T& x) { x = T(42); }); });

                check(std::move(value), count,
                    [](auto& v, size_t count) { return raze::algorithm::for_each_n(v.begin(), count, [](auto& x) { x = T(42); }); },
                    [](auto& v, size_t count) { return std::ranges::for_each_n(v.begin(), count, [](auto& x) { x = T(42); }); });

                check_counted(value, count,
                    [](auto first, size_t count) { return raze::algorithm::for_each_n(first, count, [](T& x) { x = T(42); }); },
                    [](auto first, size_t count) { return std::ranges::for_each_n(first, count, [](T& x) { x = T(42); }); });

                check_counted(std::move(value), count,
                    [](auto first, size_t count) { return raze::algorithm::for_each_n(first, count, [](auto& x) { x = T(42); }); },
                    [](auto first, size_t count) { return std::ranges::for_each_n(first, count, [](auto& x) { x = T(42); }); });
            };

            run_all(size, [&](auto value) { run(std::move(value)); });
        }
    }*/

    /*for (size_t size : {10000, 50000, 100000}) {
        for (size_t n : {size_t(1), size_t(size / 2), size_t(size)}) {
            auto run = [&] <class Container> (Container value) {
                const size_t container_size = std::ranges::distance(value);
                if (container_size == 0) return;
                const size_t count = std::min(n, container_size - 1);

                check(value, count,
                    [](auto& v, size_t count) { return raze::algorithm::for_each_n(v.begin(), count, [](T& x) { x += T(1); }); },
                    [](auto& v, size_t count) { return std::ranges::for_each_n(v.begin(), count, [](T& x) { x += T(1); }); });

                check(std::move(value), count,
                    [](auto& v, size_t count) { return raze::algorithm::for_each_n(v.begin(), count, [](T& x) { x += T(1); }); },
                    [](auto& v, size_t count) { return std::ranges::for_each_n(v.begin(), count, [](T& x) { x += T(1); }); });

                check_counted(value, count,
                    [](auto first, size_t count) { return raze::algorithm::for_each_n(first, count, [](T& x) { x += T(1); }); },
                    [](auto first, size_t count) { return std::ranges::for_each_n(first, count, [](T& x) { x += T(1); }); });

                check_counted(std::move(value), count,
                    [](auto first, size_t count) { return raze::algorithm::for_each_n(first, count, [](auto& x) { x += T(1); }); },
                    [](auto first, size_t count) { return std::ranges::for_each_n(first, count, [](auto& x) { x += T(1); }); });
            };

            run_all(size, [&](auto value) { run(std::move(value)); });
        }
    }*/
};

RTTS_CASE_TPL("raze::algorithm::for_each_n.projection", rtts::algorithm::all_types)
<class T> (rtts::type<T>) {
    auto check = [] <class Container, class F, class G> (Container value, size_t n, F raze_op, G std_op) {
        auto expected = value;

        const auto std_ret = std_op(expected, n);
        const auto raze_ret = raze_op(value, n);

        RTTS_EQUAL(
            std::distance(value.begin(), raze_ret.in),
            std::distance(expected.begin(), std_ret.in));

        RTTS_EXPECT(std::ranges::equal(value, expected));
    };

    auto check_counted = [] <class Container, class F, class G> (Container value, size_t n, F raze_op, G std_op) {
        auto expected = value;
        const auto size = std::ranges::distance(value);

        const auto raze_first = std::counted_iterator(value.begin(), size);
        const auto std_first = std::counted_iterator(expected.begin(), size);

        const auto std_ret = std_op(std_first, n);
        const auto raze_ret = raze_op(raze_first, n);

        RTTS_EQUAL(
            std::distance(value.begin(), raze_ret.in.base()),
            std::distance(expected.begin(), std_ret.in.base()));

        RTTS_EQUAL(raze_ret.in.count(), size - n);
        RTTS_EQUAL(std_ret.in.count(), size - n);

        RTTS_EXPECT(std::ranges::equal(value, expected));
    };

    auto run_all = [&] <class F> (size_t size, F f) {
        f(rtts::random::vector<T>(size, 90000));
        f(rtts::random::deque<T>(size, 90000));
        f(rtts::random::list<T>(size, 90000));
        f(rtts::random::forward_list<T>(size, 90000));
        f(rtts::random::array<T, 256>(90000));
    };

    for (int i = 0; i < 500; ++i) {
        const size_t size = 100;
        const size_t n = i % 101;
        const T add = rtts::random::generator<T>(100000 + i)();

        const auto proj_scalar = [](T& x) -> T& { return x; };
        const auto proj_vector = [](auto& x) -> auto& { return x; };

        const auto f_scalar = [add](T& x) { x += add; };
        const auto f_vector = [add](auto& x) { x += add; };

        auto run = [&] <class Container> (Container value) {
            const size_t container_size = std::ranges::distance(value);
            if (container_size == 0) return;
            const size_t count = std::min(n, container_size - 1);

            check(value, count,
                [=](auto& v, size_t count) { return raze::algorithm::for_each_n(v.begin(), count, f_scalar, proj_scalar); },
                [=](auto& v, size_t count) { return std::ranges::for_each_n(v.begin(), count, f_scalar, proj_scalar); });

            check(std::move(value), count,
                [=](auto& v, size_t count) { return raze::algorithm::for_each_n(v.begin(), count, f_vector, proj_vector); },
                [=](auto& v, size_t count) { return std::ranges::for_each_n(v.begin(), count, f_vector, proj_vector); });
        };

        run_all(size, [&](auto value) { run(std::move(value)); });
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

        if (!std::ranges::equal(value, expected))
            return false;

        std::array<T, 8> counted_value{T(1), T(2), T(3), T(4), T(5), T(6), T(7), T(8)};
        std::array<T, 8> counted_expected = counted_value;

        const auto counted_raze_ret = raze::algorithm::for_each_n(
            std::counted_iterator(counted_value.begin(), counted_value.size()), 5,
            [](auto& x) constexpr { x += T(10); });

        const auto counted_std_ret = std::ranges::for_each_n(
            std::counted_iterator(counted_expected.begin(), counted_expected.size()), 5,
            [](auto& x) constexpr { x += T(10); });

        if (counted_raze_ret.in.count() != counted_value.size() - 5 ||
            counted_std_ret.in.count() != counted_expected.size() - 5)
            return false;

        if (std::distance(counted_value.begin(), counted_raze_ret.in.base()) !=
            std::distance(counted_expected.begin(), counted_std_ret.in.base()))
            return false;

        return std::ranges::equal(counted_value, counted_expected);
    };

    constexpr bool result = test();
    RTTS_EXPECT(result);
};