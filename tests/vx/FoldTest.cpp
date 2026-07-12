#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::u32    _Width_>
struct horizontal_minmax_tests {
    template <raze::u64 _Size_>
    void test_size() {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, _Size_>>;
        using Mask = typename Simd::mask_type;
        static constexpr size_t N = Simd::size();

        alignas(64) _Type_ arr[N], fallback[N];

        std::iota(fallback, fallback + N, 1);

        for (size_t i = 0; i < N; ++i)
            arr[i] = _Type_((i % 3 == 0) ? -(i + 5) : (i + 1));

        Simd v = raze::vx::load<Simd>(arr);
        Simd fbk = raze::vx::load<Simd>(fallback);

        /*{
            auto hmin = raze::vx::fold(v, raze::vx::vertical_min);
            auto expected = (*std::min_element(arr, arr + N));
            raze_assert(hmin == expected);
        }
        {
            auto hmax = raze::vx::horizontal_max(v);
            auto expected = (*std::max_element(arr, arr + N));
            raze_assert(hmax == expected);
        }

        for (auto i = 0; i < std::min(int(std::pow(2, N)), 10000); ++i) {
            auto m = make_random_mask<Mask>();

            {
                auto rminw = raze::vx::fold[fbk, m](v, raze::vx::vertical_min);
                auto rmaxw = raze::vx::fold[fbk, m](v, raze::vx::vertical_max);

                _Type_ expected_min = m[0] ? arr[0] : fallback[0];
                _Type_ expected_max = m[0] ? arr[0] : fallback[0];

                for (size_t i = 1; i < N; ++i) {
                    expected_min = m[i] ? std::min(expected_min, arr[i]) : std::min(expected_min, fallback[i]);
                    expected_max = m[i] ? std::max(expected_max, arr[i]) : std::max(expected_max, fallback[i]);
                }

                raze_assert(rminw == expected_min);
                raze_assert(rmaxw == expected_max);
            }

            {
                auto rminwz = raze::vx::fold[m](v, raze::vx::vertical_min);
                auto rmaxwz = raze::vx::fold[m](v, raze::vx::vertical_max);

                _Type_ expected_min = m[0] ? arr[0] : _Type_(0);
                _Type_ expected_max = expected_min;

                for (size_t i = 1; i < N; ++i) {
                    _Type_ val = m[i] ? arr[i] : _Type_(0);
                    expected_min = std::min(expected_min, val);
                    expected_max = std::max(expected_max, val);
                }

                raze_assert(rminwz == expected_min);
                raze_assert(rmaxwz == expected_max);
            }
        }*/

        {
            auto result = raze::vx::fold(v, raze::vx::add);
            auto expected = std::accumulate(arr, arr + N, raze::vx::__reduce_type<_Type_>{0}, std::plus{});
            raze_assert(result == expected);
        }

        for (auto i = 0; i < std::min(int(std::pow(2, N)), 10000); ++i) {
            auto m = make_random_mask<Mask>();

            {
                auto r1 = raze::vx::fold[m, fbk](v, raze::vx::add);
                raze::vx::__reduce_type<_Type_> expected = 0;

                for (size_t i = 0; i < N; ++i)
                    if (m[i]) expected += arr[i];
                    else expected += fallback[i];

                raze_assert(expected == r1);
            }

            {
                auto r1 = raze::vx::fold[m](v, raze::vx::add);
                raze::vx::__reduce_type<_Type_> expected = 0;

                for (size_t i = 0; i < N; ++i)
                    if (m[i]) expected += arr[i];

                raze_assert(expected == r1);
            }
        }
    }

    void operator()() {
        test_size<_Width_ / (sizeof(_Type_) * 8)>();
        //test_size<_Width_ / (sizeof(_Type_) * 8) * 2>();
        //test_size<1>();
    }
};

int main() {
    test_all<horizontal_minmax_tests>();
    return 0;
}
