#include <tests/rts/rts.h>
#include <raze/meta/Tuple.h>

#include <memory>
#include <string>
#include <utility>

using namespace raze::meta;

RTTS_CASE("raze::meta::tuple.structural") {
    constexpr auto a = make_tuple(1, 2.0);
    constexpr auto b = make_tuple('x', 4LL);
    static_assert(cat(a, b) == make_tuple(1, 2.0, 'x', 4LL));
    static_assert(cat(tuple<>{}, a, tuple<>{}) == a);
    static_assert(cat() == tuple<>{});

    auto ma = make_tuple(10, 20);
    auto mb = make_tuple(30, 40);
    auto cref = cat_ref(ma, mb);
    cref[0_c] = 100;
    cref[3_c] = 400;
    RTTS_EXPECT(ma[0_c] == 100 && mb[1_c] == 400);

    constexpr auto seq = make_tuple(10, 20, 30, 40, 50);
    static_assert(slice<1, 4>(seq) == make_tuple(20, 30, 40));
    static_assert(extract(seq, 1_c, 4_c) == make_tuple(20, 30, 40));
    static_assert(extract(seq, 2_c) == make_tuple(30, 40, 50));
    static_assert(take<2>(seq) == make_tuple(10, 20));
    static_assert(take(seq, 3_c) == make_tuple(10, 20, 30));
    static_assert(drop<2>(seq) == make_tuple(30, 40, 50));
    static_assert(drop(seq, 3_c) == make_tuple(40, 50));
    static_assert(split<2>(seq) == make_tuple(make_tuple(10, 20), make_tuple(30, 40, 50)));
    static_assert(split(seq, 3_c) == make_tuple(make_tuple(10, 20, 30), make_tuple(40, 50)));
    static_assert(split_half(make_tuple(1, 2, 3, 4)) == make_tuple(make_tuple(1, 2), make_tuple(3, 4)));

    auto ms = make_tuple(1, 2, 3, 4);
    auto [lo, hi] = split_half_ref(ms);
    lo[1_c] = 22;
    hi[0_c] = 33;
    RTTS_EXPECT(ms == make_tuple(1, 22, 33, 4));

    static_assert(chunks<2>(seq) == make_tuple(make_tuple(10, 20), make_tuple(30, 40), make_tuple(50)));
    static_assert(chunks(seq, 3_c) == make_tuple(make_tuple(10, 20, 30), make_tuple(40, 50)));
    static_assert(windows<3>(seq) == make_tuple(make_tuple(10, 20, 30), make_tuple(20, 30, 40), make_tuple(30, 40, 50)));
    static_assert(windows(seq, 2_c) == make_tuple(make_tuple(10, 20), make_tuple(20, 30), make_tuple(30, 40), make_tuple(40, 50)));

    auto win_r = windows<2>(raze::meta::make_tuple(std::string("a"), std::string("b"), std::string("c")));
    RTTS_EXPECT(win_r[0_c][1_c] == "b" && win_r[1_c][0_c] == "b");

    static_assert(stride<2, 0>(seq) == make_tuple(10, 30, 50));
    static_assert(stride<2, 1>(seq) == make_tuple(20, 40));
    static_assert(stride<2, 10>(seq) == tuple<>{});

    constexpr auto il = interleave(make_tuple(1, 2, 3), make_tuple('a', 'b', 'c'));
    static_assert(il == make_tuple(1, 'a', 2, 'b', 3, 'c'));
    static_assert(deinterleave<2>(il) == make_tuple(make_tuple(1, 2, 3), make_tuple('a', 'b', 'c')));
    static_assert(deinterleave(il, 2_c) == make_tuple(make_tuple(1, 2, 3), make_tuple('a', 'b', 'c')));

    auto il_m = make_tuple(1, 10, 2, 20);
    auto de_ref = deinterleave_ref<2>(il_m);
    de_ref[0_c][1_c] = 200;
    de_ref[1_c][0_c] = 100;
    RTTS_EXPECT(il_m == make_tuple(1, 100, 200, 20));

    static_assert(push_front(make_tuple(2, 3), 1) == make_tuple(1, 2, 3));
    static_assert(push_back(make_tuple(1, 2), 3) == make_tuple(1, 2, 3));
    static_assert(pop_front(make_tuple(1, 2, 3)) == make_tuple(2, 3));
    static_assert(pop_back(make_tuple(1, 2, 3)) == make_tuple(1, 2));
    static_assert(pop_front(tuple<>{}) == tuple<>{});
    static_assert(pop_back(tuple<>{}) == tuple<>{});
    static_assert(insert_at<1>(make_tuple(1, 3), 2) == make_tuple(1, 2, 3));
    static_assert(insert_at(make_tuple(1, 2), 2_c, 3) == make_tuple(1, 2, 3));
    static_assert(erase_at<1>(make_tuple(1, 2, 3)) == make_tuple(1, 3));
    static_assert(erase_at(make_tuple(1, 2, 3), 0_c) == make_tuple(2, 3));
    static_assert(replace_at<1>(make_tuple(1, 2, 3), 9.5) == make_tuple(1, 9.5, 3));
    static_assert(replace_at(make_tuple(1, 2, 3), 2_c, 'z') == make_tuple(1, 2, 'z'));
    static_assert(swap_at<0, 2>(make_tuple(1, 2, 3)) == make_tuple(3, 2, 1));
    static_assert(swap_at(make_tuple(1, 2, 3), 0_c, 1_c) == make_tuple(2, 1, 3));

    auto mvp = raze::meta::make_tuple(std::make_unique<int>(1), std::make_unique<int>(3));
    auto ins = insert_at<1>(std::move(mvp), std::make_unique<int>(2));
    RTTS_EXPECT(*ins[0_c] == 1 && *ins[1_c] == 2 && *ins[2_c] == 3);
    auto era = erase_at<1>(std::move(ins));
    RTTS_EXPECT(*era[0_c] == 1 && *era[1_c] == 3);
};

RTTS_CASE("raze::meta::tuple.permute_and_patterns") {
    constexpr auto t = make_tuple(10, 20, 30, 40);
    static_assert(reverse(t) == make_tuple(40, 30, 20, 10));
    static_assert(rotate_left<1>(t) == make_tuple(20, 30, 40, 10));
    static_assert(rotate_left(t, 5_c) == make_tuple(20, 30, 40, 10));
    static_assert(rotate_right<1>(t) == make_tuple(40, 10, 20, 30));
    static_assert(rotate_right(t, 5_c) == make_tuple(40, 10, 20, 30));
    static_assert(rotate_left<3>(tuple<>{}) == tuple<>{});
    static_assert(rotate_right<3>(tuple<>{}) == tuple<>{});

    static_assert(swap_adjacent<1>(t) == make_tuple(20, 10, 40, 30));
    static_assert(swap_adjacent<2>(t) == make_tuple(30, 40, 10, 20));
    static_assert(swap_adjacent(t, 2_c) == make_tuple(30, 40, 10, 20));

    static_assert(reorder<3, 1, 2, 0>(t) == make_tuple(40, 20, 30, 10));
    static_assert(reorder(t, std::index_sequence<2, 0>{}) == make_tuple(30, 10));
    static_assert(reorder(t, 3_c, 0_c, 3_c) == make_tuple(40, 10, 40));

    auto dup_move = reorder<0, 0, 1, 0>(raze::meta::make_tuple(std::string("alpha"), std::string("beta")));
    RTTS_EXPECT(dup_move[0_c] == "alpha");
    RTTS_EXPECT(dup_move[1_c] == "alpha");
    RTTS_EXPECT(dup_move[2_c] == "beta");
    RTTS_EXPECT(dup_move[3_c] == "alpha");

    constexpr auto rev_p = permute(t, [] (auto i, auto n) { return n - 1 - i; });
    static_assert(rev_p == make_tuple(40, 30, 20, 10));

    constexpr auto bfly = permute(t, [] (auto i) { return i ^ 1; });
    static_assert(bfly == make_tuple(20, 10, 40, 30));

    constexpr auto gather = permute<6>(t, [] (auto i, auto n) { return i % n; });
    static_assert(gather == make_tuple(10, 20, 30, 40, 10, 20));

    constexpr auto gather_c = permute(t, 2_c, [] (auto i, auto) { return i * 2; });
    static_assert(gather_c == make_tuple(10, 30));

    auto mut = make_tuple(1, 2, 3, 4);
    auto pr = permute_ref(mut, [] (auto i, auto n) { return n - 1 - i; });
    pr[0_c] = 44;
    pr[3_c] = 11;
    RTTS_EXPECT(mut == make_tuple(11, 2, 3, 44));

    auto rr = rotate_left_ref<1>(mut);
    rr[0_c] = 22;
    RTTS_EXPECT(mut[1_c] == 22);

    auto sar = swap_adjacent_ref<2>(mut);
    sar[0_c] = 33;
    RTTS_EXPECT(mut[2_c] == 33);

    constexpr auto z = zip(make_tuple(1, 2, 3), make_tuple('a', 'b', 'c'));
    static_assert(z == make_tuple(make_tuple(1, 'a'), make_tuple(2, 'b'), make_tuple(3, 'c')));
    static_assert(unzip(z) == make_tuple(make_tuple(1, 2, 3), make_tuple('a', 'b', 'c')));
    static_assert(transpose(z) == make_tuple(make_tuple(1, 2, 3), make_tuple('a', 'b', 'c')));
    static_assert(zip_min(make_tuple(1, 2, 3), make_tuple('a', 'b')) == make_tuple(make_tuple(1, 'a'), make_tuple(2, 'b')));

    auto za = make_tuple(1, 2);
    auto zb = make_tuple(10, 20);
    auto zr = zip_ref(za, zb);
    zr[1_c][0_c] = 99;
    zr[0_c][1_c] = 88;
    RTTS_EXPECT(za[1_c] == 99 && zb[0_c] == 88);

    auto mat = make_tuple(make_tuple(1, 2), make_tuple(3, 4));
    auto tr = transpose_ref(mat);
    tr[0_c][1_c] = 30;
    RTTS_EXPECT(mat[1_c][0_c] == 30);

    constexpr auto nested = make_tuple(1, make_tuple(2, make_tuple(3, 4)), 5);
    static_assert(flatten(nested) == make_tuple(1, 2, make_tuple(3, 4), 5));
    static_assert(flatten_all(nested) == make_tuple(1, 2, 3, 4, 5));
    static_assert(flatten(tuple<>{}) == tuple<>{});
    static_assert(flatten_all(tuple<>{}) == tuple<>{});

    constexpr auto cp = cartesian_product(make_tuple(1, 2), make_tuple('a', 'b'));
    static_assert(cp == make_tuple(
        make_tuple(1, 'a'), make_tuple(1, 'b'),
        make_tuple(2, 'a'), make_tuple(2, 'b')));
    static_assert(cartesian_product(make_tuple(1, 2), tuple<>{}) == tuple<>{});
    static_assert(cartesian_product() == tuple<>{});
};

