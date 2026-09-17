#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>

RAZE_TEST_NAMESPACE_BEGIN

auto swap_adjacent_index(raze::sizetype group_size, raze::sizetype i, raze::sizetype size) {
    auto block = 2 * group_size;

    const auto block_begin = (i / block) * block;
    const auto offset = i % block;

    auto index = 0;

    if (offset < group_size) index = block_begin + offset + group_size;
    else index = block_begin + offset - group_size;

    return index % size;
}

RTTS_CASE_TPL("raze::vx::swap_adjacent", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;

    if constexpr (V::size() > 1) {
        constexpr size_t N = V::size();
        T arr[N];

        for (size_t i = 0; i < N; ++i)
            arr[i] = T(i + 1);

        V v = raze::vx::load<V>(arr);

        [&] <raze::sizetype... GroupSize> (std::integer_sequence<raze::sizetype, GroupSize...>) {
            ([&] {
                if constexpr (GroupSize > 0) {
                    auto r = raze::vx::swap_adjacent(v, std::integral_constant<raze::sizetype, GroupSize>{});

                    T swapped_arr[N];

                    for (auto i = 0; i < V::size(); ++i)
                        swapped_arr[i] = arr[swap_adjacent_index(GroupSize, i, V::size())];

                    for (auto i = 0; i < V::size(); ++i)
                        RTTS_EXPECT(r[i] == swapped_arr[i]);
                }
            }(), ...);
        }(std::make_integer_sequence<raze::sizetype, N + 1>{});
    }
};

RAZE_TEST_NAMESPACE_END