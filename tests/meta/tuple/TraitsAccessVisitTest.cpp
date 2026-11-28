#include <tests/rts/rts.h>
#include <raze/meta/Tuple.h>

#include <array>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace {

struct aggregate_point {
    int x;
    double y;
    char z;
};

struct ctor_point {
    int x;
    double y;
    constexpr ctor_point(int a, double b) noexcept : x(a), y(b) {}
};

struct adder_obj {
    int base = 100;
    constexpr int add(int a, int b) const noexcept { return base + a + b; }
};

}

RTTS_CASE("raze::meta::tuple.traits_and_concepts") {
    using namespace raze::meta;

    static_assert(tuple_like<tuple<>>);
    static_assert(tuple_like<tuple<int, double>>);
    static_assert(tuple_like<std::tuple<int, float>>);
    static_assert(tuple_like<std::pair<int, char>>);
    static_assert(tuple_like<std::array<int, 4>>);
    static_assert(!tuple_like<int>);

    static_assert(is_tuple_v<tuple<int>>);
    static_assert(!is_tuple_v<std::tuple<int>>);
    static_assert(!is_tuple_v<std::pair<int, int>>);

    static_assert(sized_tuple_like<tuple<int, float>, 2>);
    static_assert(non_empty_tuple_like<tuple<int>>);
    static_assert(!non_empty_tuple_like<tuple<>>);
    static_assert(same_size_tuples<tuple<int, float>, std::pair<char, double>, std::array<int, 2>>);
    static_assert(!same_size_tuples<tuple<int, float>, tuple<int>>);

    using mixed = tuple<int, double, int, char>;
    static_assert(count_type_v<int, mixed> == 2);
    static_assert(count_type_v<double, mixed> == 1);
    static_assert(count_type_v<float, mixed> == 0);
    static_assert(contains_type_v<char, mixed>);
    static_assert(!contains_type_v<float, mixed>);
    static_assert(type_index_v<double, mixed> == 1);
    static_assert(type_index_v<char, mixed> == 3);
    static_assert(!all_unique_types_v<mixed>);
    static_assert(all_unique_types_v<tuple<int, double, char>>);
    static_assert(all_unique_types_v<tuple<>>);
    static_assert(is_homogeneous_v<std::array<int, 3>>);
    static_assert(!is_homogeneous_v<mixed>);
    static_assert(!is_homogeneous_v<tuple<>>);

    static_assert(std::same_as<apply_traits_t<std::common_type, tuple<int, long long, double>>, double>);
    static_assert(std::same_as<map_traits_t<std::add_const, tuple<int, double>>, tuple<const int, const double>>);
    static_assert(std::same_as<as_tuple_t<int>, tuple<int>>);
    static_assert(std::same_as<as_tuple_t<std::pair<int, char>>, tuple<int, char>>);
    static_assert(std::same_as<as_tuple_t<tuple<int, double>>, tuple<int, double>>);

    RTTS_EXPECT(true);
};

RTTS_CASE("raze::meta::tuple.access_and_apply") {
    using namespace raze::meta;

    auto t = make_tuple(1, 2.5, 'a');
    auto ref = to_ref(t);
    ref[0_c] = 42;
    RTTS_EXPECT(t[0_c] == 42);

    auto rref = reversed_ref(t);
    RTTS_EXPECT(rref[0_c] == 'a');
    RTTS_EXPECT(rref[2_c] == 42);
    rref[2_c] = 77;
    RTTS_EXPECT(t[0_c] == 77);

    std::pair<int, double> sp{ 3, 4.5 };
    auto converted = to_tuple(sp);
    static_assert(std::same_as<decltype(converted), tuple<int, double>>);
    RTTS_EXPECT(converted[0_c] == 3 && converted[1_c] == 4.5);

    auto as1 = as_tuple(99);
    static_assert(std::same_as<decltype(as1), tuple<int>>);
    RTTS_EXPECT(as1[0_c] == 99);

    auto as2 = as_tuple(sp);
    static_assert(std::same_as<decltype(as2), tuple<int, double>>);
    RTTS_EXPECT(as2[0_c] == 3 && as2[1_c] == 4.5);

    auto& as3 = as_tuple(t);
    as3[0_c] = 88;
    RTTS_EXPECT(t[0_c] == 88);

    constexpr auto ap = from_tuple<aggregate_point>(make_tuple(1, 2.5, 'x'));
    static_assert(ap.x == 1 && ap.y == 2.5 && ap.z == 'x');

    constexpr auto cp = raze::meta::make_from_tuple<ctor_point>(std::pair{ 7, 8.5 });
    static_assert(cp.x == 7 && cp.y == 8.5);

    RTTS_EXPECT(front(t) == 88);
    RTTS_EXPECT(back(t) == 'a');
    front(t) = 12;
    back(t) = 'z';
    RTTS_EXPECT(t[0_c] == 12 && t[2_c] == 'z');

    constexpr auto sum3 = apply([] (int a, int b, int c) noexcept { return a + b + c; }, make_tuple(1, 2, 3));
    static_assert(sum3 == 6);
    static_assert(noexcept(apply([] (int a) noexcept { return a; }, make_tuple(1))));

    constexpr adder_obj obj{ 10 };
    constexpr int mem_res = apply(&adder_obj::add, make_tuple(obj, 4, 5));
    static_assert(mem_res == 19);
    RTTS_EXPECT(mem_res == 19);
};

RTTS_CASE("raze::meta::tuple.visit") {
    using namespace raze::meta;

    auto t1 = make_tuple(1, 2, 3, 4);
    auto t2 = make_tuple(10, 20, 30, 40);

    std::vector<int> order;
    for_each([&] (int a, int b) { order.push_back(a + b); }, t1, t2);
    RTTS_EXPECT((order == std::vector<int>{ 11, 22, 33, 44 }));

    order.clear();
    for_each_reversed([&] (int a, int b) { order.push_back(a + b); }, t1, t2);
    RTTS_EXPECT((order == std::vector<int>{ 44, 33, 22, 11 }));

    std::vector<std::pair<int, int>> idx_order;
    for_each_index([&] (auto i, int a) { idx_order.emplace_back(int(i), a); }, t1);
    RTTS_EXPECT((idx_order == std::vector<std::pair<int, int>>{ { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 4 } }));

    idx_order.clear();
    for_each_index_reversed([&] (auto i, int a) { idx_order.emplace_back(int(i), a); }, t1);
    RTTS_EXPECT((idx_order == std::vector<std::pair<int, int>>{ { 3, 4 }, { 2, 3 }, { 1, 2 }, { 0, 1 } }));

    std::vector<std::pair<int, int>> adj;
    adjacent_for_each([&] (int a, int b) { adj.emplace_back(a, b); }, t1);
    RTTS_EXPECT((adj == std::vector<std::pair<int, int>>{ { 1, 2 }, { 2, 3 }, { 3, 4 } }));

    adj.clear();
    adjacent_for_each_reversed([&] (int a, int b) { adj.emplace_back(a, b); }, t1);
    RTTS_EXPECT((adj == std::vector<std::pair<int, int>>{ { 3, 4 }, { 2, 3 }, { 1, 2 } }));

    for_each([] (auto) {}, tuple<>{});
    adjacent_for_each([] (auto, auto) {}, make_tuple(1));

    auto het = make_tuple(10, 20.5, 30LL, 40.25f);
    for (raze::sizetype k = 0; k < 4; ++k) {
        double got = visit_at(k, [] (auto v) { return static_cast<double>(v); }, het);
        double expected = (k == 0) ? 10.0 : (k == 1) ? 20.5 : (k == 2) ? 30.0 : 40.25;
        RTTS_EXPECT(got == expected);

        raze::sizetype got_idx = visit_at_index(k, [] (auto i, auto) { return i.value; }, het);
        RTTS_EXPECT(got_idx == k);
    }

    int captured = -1;
    RTTS_EXPECT(try_visit_at(2, [&] (int v) { captured = v; }, t1));
    RTTS_EXPECT(captured == 3);
    RTTS_EXPECT(!try_visit_at(4, [&] (int v) { captured = v; }, t1));
    RTTS_EXPECT(!try_visit_at(0, [] (auto) {}, tuple<>{}));
};

