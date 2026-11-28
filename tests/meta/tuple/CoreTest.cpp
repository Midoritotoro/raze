#include <tests/rts/rts.h>
#include <raze/meta/Meta.h>
#include <raze/vx/Simd.h>

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace {

struct empty_tag {};
struct final_empty_tag final {};

struct less_only {
    int v;
    friend constexpr bool operator==(less_only a, less_only b) noexcept { return a.v == b.v; }
    friend constexpr bool operator<(less_only a, less_only b) noexcept { return a.v < b.v; }
};

struct explicit_target {
    int v;
    constexpr explicit explicit_target(int x) noexcept : v(x) {}
    friend constexpr bool operator==(explicit_target, explicit_target) noexcept = default;
};

template <class U>
concept can_eq = requires (U x, U y) { x == y; };

template <class U>
concept can_cmp = requires (U x, U y) { x <=> y; };

}

RTTS_CASE("raze::meta::tuple.core_and_layout") {
    using namespace raze::meta;

    static_assert(std::is_empty_v<tuple<>>);
    static_assert(std::is_trivially_copyable_v<tuple<>>);
    static_assert(std::is_aggregate_v<tuple<>>);
    static_assert(tuple<>::size() == 0);
    static_assert(tuple<>::empty());

    using t3 = tuple<int, double, char>;
    static_assert(std::is_trivially_copyable_v<t3>);
    static_assert(std::is_trivially_copy_assignable_v<t3>);
    static_assert(std::is_trivially_move_assignable_v<t3>);
    static_assert(std::is_aggregate_v<t3>);
    static_assert(t3::size() == 3);
    static_assert(!t3::empty());
    static_assert(!t3::has_references);
    static_assert(!t3::is_homogeneous);

    using homo4 = tuple<int, int, int, int>;
    static_assert(homo4::is_homogeneous);
    static_assert(std::same_as<repeat_tuple_t<int, 4>, homo4>);

    static_assert(sizeof(tuple<int, empty_tag>) == sizeof(int));
    static_assert(sizeof(tuple<int, final_empty_tag>) == sizeof(int));
    static_assert(sizeof(tuple<empty_tag, int, final_empty_tag>) == sizeof(int));

    using simd_f = raze::vx::simd<float>;
    using simd_pair = tuple<simd_f, simd_f>;
    static_assert(std::is_trivially_copyable_v<simd_pair>);
    static_assert(std::is_aggregate_v<simd_pair>);
    static_assert(alignof(simd_pair) == alignof(simd_f));
    static_assert(sizeof(simd_pair) == 2 * sizeof(simd_f));

    static_assert(std::tuple_size_v<t3> == 3);
    static_assert(std::same_as<std::tuple_element_t<0, t3>, int>);
    static_assert(std::same_as<std::tuple_element_t<1, t3>, double>);
    static_assert(std::same_as<std::tuple_element_t<2, t3>, char>);
    static_assert(std::same_as<std::common_type_t<tuple<int, float>, tuple<long long, double>>,
        tuple<long long, double>>);
    static_assert(std::same_as<std::common_reference_t<tuple<int&, float&>, tuple<const int&, const float&>>,
        tuple<const int&, const float&>>);

    constexpr tuple ct{ 10, 2.5, 'z' };
    static_assert(ct.get<0>() == 10);
    static_assert(ct.get<1>() == 2.5);
    static_assert(ct.get<2>() == 'z');
    static_assert(ct.get<int>() == 10);
    static_assert(ct.get<double>() == 2.5);
    static_assert(ct.get<char>() == 'z');
    static_assert(ct[0_c] == 10);
    static_assert(ct[index<1>] == 2.5);
    static_assert(ct[raze::ic<raze::sizetype, 2>{}] == 'z');
    static_assert(ct[type_v<double>] == 2.5);
    static_assert(get<0>(ct) == 10);
    static_assert(get<char>(ct) == 'z');

    auto [a, b, c] = ct;
    RTTS_EXPECT(a == 10);
    RTTS_EXPECT(b == 2.5);
    RTTS_EXPECT(c == 'z');

    constexpr auto t16 = make_tuple(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    static_assert(t16.size() == 16);
    static_assert(t16[0_c] == 0);
    static_assert(t16[15_c] == 15);

    constexpr auto t20 = make_tuple(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        10, 11, 12, 13, 14, 15, 16, 17, 18, 19);
    static_assert(t20.size() == 20);
    static_assert(t20[0_c] == 0);
    static_assert(t20[16_c] == 16);
    static_assert(t20[19_c] == 19);

    int ref_x = 7;
    auto ref_empty = make_tuple_exact<int&, empty_tag>(ref_x, empty_tag{});
    ref_empty[0_c] = 99;
    RTTS_EXPECT(ref_x == 99);
};

RTTS_CASE("raze::meta::tuple.references_and_forwarding") {
    using namespace raze::meta;

    int x = 1;
    int y = 2;

    using tv = tuple<int, int&, int&&>;
    static_assert(std::same_as<decltype(std::declval<tv&>().get<0>()), int&>);
    static_assert(std::same_as<decltype(std::declval<const tv&>().get<0>()), const int&>);
    static_assert(std::same_as<decltype(std::declval<tv&&>().get<0>()), int&&>);
    static_assert(std::same_as<decltype(std::declval<const tv&&>().get<0>()), const int&&>);

    static_assert(std::same_as<decltype(std::declval<tv&>().get<1>()), int&>);
    static_assert(std::same_as<decltype(std::declval<const tv&>().get<1>()), int&>);
    static_assert(std::same_as<decltype(std::declval<tv&&>().get<1>()), int&>);
    static_assert(std::same_as<decltype(std::declval<const tv&&>().get<1>()), int&>);

    static_assert(std::same_as<decltype(std::declval<tv&>().get<2>()), int&>);
    static_assert(std::same_as<decltype(std::declval<const tv&>().get<2>()), int&>);
    static_assert(std::same_as<decltype(std::declval<tv&&>().get<2>()), int&&>);
    static_assert(std::same_as<decltype(std::declval<const tv&&>().get<2>()), int&&>);

    auto tied = tie(x, y);
    tied = make_tuple(10, 20);
    RTTS_EXPECT(x == 10);
    RTTS_EXPECT(y == 20);

    int sx = 101;
    int sy = 202;
    tied = tie(sx, sy);
    RTTS_EXPECT(x == 101);
    RTTS_EXPECT(y == 202);

    const auto const_tied = tie(x, y);
    const_tied = make_tuple(30, 40);
    RTTS_EXPECT(x == 30);
    RTTS_EXPECT(y == 40);

    int cx = 303;
    int cy = 404;
    const_tied = tie(cx, cy);
    RTTS_EXPECT(x == 303);
    RTTS_EXPECT(y == 404);

    const_tied = make_tuple(short(50), short(60));
    RTTS_EXPECT(x == 50);
    RTTS_EXPECT(y == 60);

    int tmp_rv = 77;
    auto fw = forward_as_tuple(x, std::move(tmp_rv));
    static_assert(std::same_as<decltype(fw), tuple<int&, int&&>>);
    RTTS_EXPECT(fw[0_c] == 50);
    RTTS_EXPECT(fw[1_c] == 77);
    auto check_fw = [] (auto&& ft) { return ft[0_c] == 50 && ft[1_c] == 88; };
    RTTS_EXPECT(check_fw(forward_as_tuple(x, 88)));

    auto up = std::make_unique<int>(123);
    auto mvt = raze::meta::make_tuple(std::move(up), std::string("raze"));
    auto moved_ptr = get<0>(std::move(mvt));
    RTTS_EXPECT(moved_ptr && *moved_ptr == 123);
};

RTTS_CASE("raze::meta::tuple.conversion_swap_compare") {
    using namespace raze::meta;

    tuple<int, float> a{ 1, 2.0f };
    tuple<long long, double> b = a;
    RTTS_EXPECT(b[0_c] == 1LL);
    RTTS_EXPECT(b[1_c] == 2.0);

    b = tuple<short, float>{ short(5), 3.5f };
    RTTS_EXPECT(b[0_c] == 5LL);
    RTTS_EXPECT(b[1_c] == 3.5);

    static_assert(!std::is_convertible_v<tuple<int>, tuple<explicit_target>>);
    static_assert(std::is_constructible_v<tuple<explicit_target>, tuple<int>>);
    auto exp_t = static_cast<tuple<explicit_target>>(tuple<int>{ 42 });
    RTTS_EXPECT((exp_t == tuple<explicit_target>{ explicit_target{ 42 } }));

    tuple<int, std::string> s1{ 1, "one" };
    tuple<int, std::string> s2{ 2, "two" };
    swap(s1, s2);
    RTTS_EXPECT(s1[0_c] == 2 && s1[1_c] == "two");
    RTTS_EXPECT(s2[0_c] == 1 && s2[1_c] == "one");

    int p1 = 11, p2 = 22, q1 = 33, q2 = 44;
    const auto tp = tie(p1, p2);
    const auto tq = tie(q1, q2);
    swap(tp, tq);
    RTTS_EXPECT(p1 == 33 && p2 == 44 && q1 == 11 && q2 == 22);

    static_assert(tuple<>{} == tuple<>{});
    static_assert((tuple<>{} <=> tuple<>{}) == std::strong_ordering::equal);
    static_assert(tuple{ 1, 2.0 } == tuple{ 1LL, 2.0f });
    static_assert(tuple{ 1, 2.0 } != tuple{ 1LL, 3.0f });
    static_assert(tuple{ 1, 2 } < tuple{ 1, 3 });
    static_assert(tuple{ 2, 0 } > tuple{ 1, 99 });
    static_assert(tuple{ less_only{ 1 }, less_only{ 2 } } == tuple{ less_only{ 1 }, less_only{ 2 } });
    static_assert(tuple{ less_only{ 1 }, less_only{ 2 } } < tuple{ less_only{ 1 }, less_only{ 3 } });
    static_assert(std::same_as<decltype(tuple{ less_only{ 1 } } <=> tuple{ less_only{ 2 } }), std::weak_ordering>);

    using sv = tuple<raze::vx::simd<int>>;
    static_assert(!can_eq<sv>);
    static_assert(!can_cmp<sv>);

    RTTS_EXPECT((tuple{ 1, 2 } < tuple{ 1, 3 }));
};

