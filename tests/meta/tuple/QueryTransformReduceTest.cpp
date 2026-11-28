#include <tests/rts/rts.h>
#include <raze/meta/Tuple.h>
#include <raze/vx/Simd.h>

#include <memory>
#include <string>
#include <vector>

RTTS_CASE("raze::meta::tuple.query") {
    using namespace raze::meta;

    constexpr auto t = make_tuple(2, 4, 6, 8);
    static_assert(all_of(t, [] (int x) { return x % 2 == 0; }));
    static_assert(all_of_reversed(t, [] (int x) { return x % 2 == 0; }));
    static_assert(!any_of(t, [] (int x) { return x > 10; }));
    static_assert(none_of(t, [] (int x) { return x % 2 == 1; }));
    static_assert(none_of_reversed(t, [] (int x) { return x % 2 == 1; }));

    static_assert(all_of(make_tuple(1, true, 3.0)));
    static_assert(!all_of(make_tuple(1, false, 3.0)));
    static_assert(any_of(make_tuple(0, false, 1)));
    static_assert(any_of_reversed(make_tuple(0, false, 1)));
    static_assert(none_of(make_tuple(0, false, 0.0)));
    static_assert(none_of_reversed(make_tuple(0, false, 0.0)));
    static_assert(all_of(tuple<>{}));
    static_assert(all_of_reversed(tuple<>{}));
    static_assert(!any_of(tuple<>{}));
    static_assert(!any_of_reversed(tuple<>{}));

    int calls = 0;
    bool r1 = all_of(make_tuple(1, 0, 1, 1), [&] (int x) { ++calls; return x != 0; });
    RTTS_EXPECT(!r1 && calls == 2);

    calls = 0;
    bool r2 = all_of_reversed(make_tuple(1, 1, 0, 1), [&] (int x) { ++calls; return x != 0; });
    RTTS_EXPECT(!r2 && calls == 2);

    calls = 0;
    bool r3 = any_of(make_tuple(0, 1, 0, 0), [&] (int x) { ++calls; return x != 0; });
    RTTS_EXPECT(r3 && calls == 2);

    calls = 0;
    bool r4 = any_of_reversed(make_tuple(0, 0, 1, 0), [&] (int x) { ++calls; return x != 0; });
    RTTS_EXPECT(r4 && calls == 2);

    static_assert(count(make_tuple(true, false, 1, 0, 2.0)) == 3);
    static_assert(count_if(make_tuple(1, 2, 3, 4, 5), [] (int x) { return x > 2; }) == 3);
    static_assert(count(make_tuple(1, 2, 1, 3, 1), 1) == 3);

    auto mixed_simd = make_tuple(raze::vx::simd<int>(1), 5, 5);
    RTTS_EXPECT(count(mixed_simd, 5) == 2);

    constexpr auto vals = make_tuple(10, 20, 30, 20, 40);
    static_assert(find_index_if(vals, [] (int x) { return x == 20; }) == 1);
    static_assert(locate(vals, [] (int x) { return x == 20; }) == 1);
    static_assert(find_last_index_if(vals, [] (int x) { return x == 20; }) == 3);
    static_assert(rlocate(vals, [] (int x) { return x == 20; }) == 3);
    static_assert(find_index_if(vals, [] (int x) { return x == 99; }) == 5);
    static_assert(find_last_index_if(vals, [] (int x) { return x == 99; }) == 5);
    static_assert(find_last_index_if(tuple<>{}, [] (auto) { return true; }) == 0);
};

RTTS_CASE("raze::meta::tuple.transform") {
    using namespace raze::meta;

    constexpr auto a = make_tuple(1, 2, 3);
    constexpr auto b = make_tuple(10, 20, 30);

    constexpr auto m = map([] (int x, int y) { return x + y; }, a, b);
    static_assert(m == make_tuple(11, 22, 33));
    static_assert(transform([] (int x) { return x * 2; }, a) == make_tuple(2, 4, 6));

    constexpr auto mr = map_reversed([] (int x) { return x * 10; }, a);
    static_assert(mr == make_tuple(30, 20, 10));
    static_assert(transform_reversed([] (int x) { return x + 1; }, a) == make_tuple(4, 3, 2));

    constexpr auto mi = map_index([] (auto i, int x) { return int(i) * 100 + x; }, a);
    static_assert(mi == make_tuple(1, 102, 203));

    constexpr auto mir = map_index_reversed([] (auto i, int x) { return int(i) * 100 + x; }, a);
    static_assert(mir == make_tuple(203, 102, 1));

    std::vector<int> eval_order;
    auto seq = map([&] (int x) { eval_order.push_back(x); return x; }, a);
    RTTS_EXPECT(seq == a);
    RTTS_EXPECT((eval_order == std::vector<int>{ 1, 2, 3 }));

    auto ptrs = map([] (int x) { return std::make_unique<int>(x); }, a);
    RTTS_EXPECT(*ptrs[0_c] == 1 && *ptrs[1_c] == 2 && *ptrs[2_c] == 3);

    constexpr auto adj = adjacent_map([] (int x, int y) { return y - x; }, make_tuple(1, 4, 9, 16));
    static_assert(adj == make_tuple(3, 5, 7));
    static_assert(adjacent_map([] (int x, int y) { return x + y; }, make_tuple(1)) == tuple<>{});

    constexpr auto cast_t = member_cast<double>(make_tuple(1, 2L, 3.5f));
    static_assert(std::same_as<decltype(cast_t), const tuple<double, double, double>>);
    static_assert(cast_t == make_tuple(1.0, 2.0, 3.5));

    constexpr auto sel = select(make_tuple(true, false, true), a, b);
    static_assert(sel == make_tuple(1, 20, 3));

    constexpr auto blend = select(make_tuple(1, 0, 1), a, b,
        [] (int mask, int x, int y) { return mask ? x * 10 : y * 10; });
    static_assert(blend == make_tuple(10, 200, 30));
    RTTS_EXPECT(blend == make_tuple(10, 200, 30));
};

RTTS_CASE("raze::meta::tuple.reduce_and_scans") {
    using namespace raze::meta;

    constexpr auto t = make_tuple(1, 2, 3, 4);
    auto sub = [] (int x, int y) { return x - y; };

    static_assert(fold_left(sub, t, 100) == 90);
    static_assert(fold_left(sub, t) == -8);
    static_assert(fold_left(sub, tuple<>{}, 77) == 77);
    static_assert(fold_left(sub, make_tuple(42)) == 42);

    static_assert(fold_right(sub, t, 10) == 8);
    static_assert(fold_right(sub, t) == -2);
    static_assert(fold_right(sub, tuple<>{}, 77) == 77);
    static_assert(fold_right(sub, make_tuple(42)) == 42);

    constexpr auto fli = fold_left_index([] (int acc, auto i, int x) { return acc * 10 + int(i) + x; },
        make_tuple(1, 2, 3), 0);
    static_assert(fli == 135);

    constexpr auto fri = fold_right_index([] (auto i, int x, int acc) { return acc * 10 + int(i) + x; },
        make_tuple(1, 2, 3), 0);
    static_assert(fri == 531);

    std::string acc_str;
    std::string& ref_res = fold_left([] (std::string& s, const char* piece) -> std::string& {
        s += piece;
        return s;
    }, make_tuple("a", "b", "c"), acc_str);
    RTTS_EXPECT(&ref_res == &acc_str && acc_str == "abc");

    static_assert(reduce([] (int a, int b) { return a + b; }, t) == 10);
    static_assert(reduce([] (int a, int b) { return a + b; }, t, 5) == 15);
    static_assert(reduce([] (int a, int b) { return a + b; }, tuple<>{}, 5) == 5);
    static_assert(map_reduce([] (int a, int b) { return a * b; }, [] (int a, int b) { return a + b; },
        make_tuple(1, 2, 3), make_tuple(4, 5, 6)) == 32);

    static_assert(sum(t) == 10);
    static_assert(sum(t, 5) == 15);
    static_assert(prod(t) == 24);
    static_assert(prod(t, 2) == 48);
    static_assert(bit_and(make_tuple(0b1110, 0b1011, 0b1010)) == 0b1010);
    static_assert(bit_or(make_tuple(0b0001, 0b0100, 0b1000)) == 0b1101);
    static_assert(bit_xor(make_tuple(0b1100, 0b1010, 0b0110)) == 0b0000);
    static_assert(min(make_tuple(5, 2, 9, 1, 4)) == 1);
    static_assert(max(make_tuple(5, 2, 9, 1, 4)) == 9);
    static_assert(min(make_tuple(-5, 2, -1), [] (int x) { return x * x; }) == 1);
    static_assert(max(make_tuple(-5, 2, -1), [] (int x) { return x * x; }) == 25);
    static_assert(inner_product(make_tuple(1, 2, 3), make_tuple(4, 5, 6), 10) == 42);
    static_assert(inner_product(tuple<>{}, tuple<>{}, 9) == 9);

    auto plus = [] (int a, int b) { return a + b; };
    static_assert(inclusive_scan_left(plus, t) == make_tuple(1, 3, 6, 10));
    static_assert(inclusive_scan_left(plus, t, 10) == make_tuple(11, 13, 16, 20));
    static_assert(exclusive_scan_left(plus, t, 10) == make_tuple(10, 11, 13, 16));
    static_assert(inclusive_scan_right(plus, t) == make_tuple(10, 9, 7, 4));
    static_assert(inclusive_scan_right(plus, t, 10) == make_tuple(20, 19, 17, 14));
    static_assert(exclusive_scan_right(plus, t, 10) == make_tuple(19, 17, 14, 10));

    static_assert(inclusive_scan_left(plus, tuple<>{}) == tuple<>{});
    static_assert(exclusive_scan_left(plus, tuple<>{}, 0) == tuple<>{});
    static_assert(inclusive_scan_right(plus, tuple<>{}) == tuple<>{});
    static_assert(exclusive_scan_right(plus, tuple<>{}, 0) == tuple<>{});

    RTTS_EXPECT(true);
};

