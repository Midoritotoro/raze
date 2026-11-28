#include <tests/rts/rts.h>
#include <raze/meta/Tuple.h>

#include <memory>
#include <type_traits>
#include <vector>

RTTS_CASE("raze::meta::tuple.filter_and_dedup") {
    using namespace raze::meta;

    constexpr auto mixed = make_tuple(1, 2.5, 'a', 3.5f, 4LL);
    constexpr auto ints = filter<std::is_integral>(mixed);
    constexpr auto non_ints = filter_not<std::is_integral>(mixed);
    constexpr auto part = partition<std::is_integral>(mixed);

    static_assert(ints == make_tuple(1, 'a', 4LL));
    static_assert(non_ints == make_tuple(2.5, 3.5f));
    static_assert(part[0_c] == ints && part[1_c] == non_ints);

    auto mut = make_tuple(1, 2.5, 3);
    auto fref = filter_ref<std::is_integral>(mut);
    fref[0_c] = 10;
    fref[1_c] = 30;
    auto fnref = filter_not_ref<std::is_integral>(mut);
    fnref[0_c] = 9.5;
    RTTS_EXPECT(mut == make_tuple(10, 9.5, 30));

    auto mp = raze::meta::make_tuple(std::make_unique<int>(7), 2.5, std::make_unique<int>(9));
    auto only_ptrs = filter_not<std::is_floating_point>(std::move(mp));
    RTTS_EXPECT(*only_ptrs[0_c] == 7 && *only_ptrs[1_c] == 9);

    constexpr auto dup = make_tuple(1, 2, 3.0, 4.0, 5, 'x', 'y', 6.0);
    static_assert(unique_types(dup) == make_tuple(1, 3.0, 5, 'x', 6.0));
    static_assert(dedup_types(dup) == make_tuple(1, 3.0, 'x'));
    static_assert(unique_types(tuple<>{}) == tuple<>{});
    static_assert(dedup_types(tuple<>{}) == tuple<>{});
};

RTTS_CASE("raze::meta::tuple.generate") {
    using namespace raze::meta;

    static_assert(fill<4>(7) == make_tuple(7, 7, 7, 7));
    static_assert(fill(3_c, 9) == make_tuple(9, 9, 9));
    static_assert(splat<3>(5.5) == make_tuple(5.5, 5.5, 5.5));
    static_assert(splat(2_c, 'k') == make_tuple('k', 'k'));
    static_assert(fill<0>(42) == tuple<>{});

    static_assert(generate<4>([] (auto i) { return int(i) * int(i); }) == make_tuple(0, 1, 4, 9));
    static_assert(generate(3_c, [] (auto i) { return int(i) + 10; }) == make_tuple(10, 11, 12));
    static_assert(generate_reversed<4>([] (auto i) { return int(i); }) == make_tuple(3, 2, 1, 0));
    static_assert(generate_reversed(3_c, [] (auto i) { return int(i) * 2; }) == make_tuple(4, 2, 0));

    std::vector<int> fwd_order;
    auto g1 = generate<4>([&] (auto i) {
        fwd_order.push_back(int(i));
        return int(i);
    });
    RTTS_EXPECT(g1 == make_tuple(0, 1, 2, 3));
    RTTS_EXPECT((fwd_order == std::vector<int>{ 0, 1, 2, 3 }));

    std::vector<int> rev_order;
    auto g2 = generate_reversed<4>([&] (auto i) {
        rev_order.push_back(int(i));
        return int(i);
    });
    RTTS_EXPECT(g2 == make_tuple(3, 2, 1, 0));
    RTTS_EXPECT((rev_order == std::vector<int>{ 3, 2, 1, 0 }));

    static_assert(iota<5>() == make_tuple(raze::sizetype(0), raze::sizetype(1), raze::sizetype(2), raze::sizetype(3), raze::sizetype(4)));
    static_assert(iota<4, int>(10) == make_tuple(10, 11, 12, 13));
    static_assert(iota(4_c, 5) == make_tuple(5, 6, 7, 8));
    static_assert(iota<0, int>() == tuple<>{});
};

