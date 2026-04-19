#include <tests/vx/SimdTestTools.h>
#include <raze/vx/SimdDataparAlgorithms.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::uint32    _Width_>
struct horizontal_minmax_tests {
    using Simd = raze::vx::simd<_Type_, raze::vx::x86_runtime_abi<_ISA_, _Width_>>;
    using Mask = typename Simd::mask_type;
    static constexpr size_t N = Simd::size();

    void operator()() {
        alignas(64) _Type_ arr[N], fallback[N];

        std::iota(fallback, fallback + N, 1);

        for (size_t i = 0; i < N; ++i)
            arr[i] = _Type_((i % 3 == 0) ? -(i + 5) : (i + 1));

        Simd v; v.copy_from(arr);
        Simd fbk; fbk.copy_from(fallback);

        raze_assert(raze::vx::horizontal_min(v) == (*std::min_element(arr, arr + N)));
        raze_assert(raze::vx::horizontal_max(v) == (*std::max_element(arr, arr + N)));

        for (auto i = 0; i < std::min(int(std::pow(2, N)), 10000); ++i) {
            auto m = make_random_mask<Mask>();

            {
                auto rminw = raze::vx::horizontal_min[fbk, m](v);
                auto rmaxw = raze::vx::horizontal_max[fbk, m](v);

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
                auto rminwz = raze::vx::horizontal_min[m](v);
                auto rmaxwz = raze::vx::horizontal_max[m](v);

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
        }
    }
};

int main() {
    test_all<horizontal_minmax_tests>();
    return 0;
}
