#include <tests/vx/SimdTestTools.h>
#include <raze/vx/SimdDataparAlgorithms.h>
#include <raze/math/Math.h>


template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::uint32    _Width_>
struct fused_operations_tests {
    using Simd = raze::vx::simd<_Type_, raze::vx::x86_runtime_abi<_ISA_, _Width_>>;
    using Mask = typename Simd::mask_type;
    using U = typename raze::IntegerForSizeof<_Type_>::Unsigned;
    static constexpr size_t N = Simd::size();

    void operator()() {
        alignas(64) _Type_ arrA[N], arrB[N], arrC[N], arrSrc[N];

        for (size_t i = 0; i < N; ++i) {
            arrA[i] = _Type_(i + 1);
            arrB[i] = _Type_((i + 1) * 3);
            arrC[i] = _Type_((i + 1) * 7);
            arrSrc[i] = _Type_(100 + i);
        }

        Simd a = raze::vx::load<Simd>(arrA);
        Simd b = raze::vx::load<Simd>(arrB);
        Simd c = raze::vx::load<Simd>(arrC);
        Simd src = raze::vx::load<Simd>(arrSrc);

        Mask m;
        for (size_t i = 0; i < N; ++i)
            m[i] = (i % 2 == 0);

        auto w = raze::vx::where(a, src, m);
        auto wz = raze::vx::where(a, m);

        auto fma_ref = [](_Type_ x, _Type_ y, _Type_ z) { return _Type_(x * y + z); };
        auto fms_ref = [](_Type_ x, _Type_ y, _Type_ z) { return _Type_(x * y - z); };
        auto fnma_ref = [](_Type_ x, _Type_ y, _Type_ z) { return _Type_(-(x * y) + z); };
        auto fnms_ref = [](_Type_ x, _Type_ y, _Type_ z) { return _Type_(-(x * y) - z); };


        auto simd_fma = [&](const auto& x, const auto& y, const auto& z) { return raze::math::fma(x, y, z); };
        auto simd_fms = [&](const auto& x, const auto& y, const auto& z) { return raze::math::fms(x, y, z); };
        auto simd_fnma = [&](const auto& x, const auto& y, const auto& z) { return raze::math::fnma(x, y, z); };
        auto simd_fnms = [&](const auto& x, const auto& y, const auto& z) { return raze::math::fnms(x, y, z); };

        test_where_ternary<_Type_, N>(arrA, arrB, arrC, m, a, b, c, src, w, wz, simd_fma, [=](_Type_ A, _Type_ B, _Type_ C, _Type_ Src, bool cond, int pos) {
            if (!cond) 
                return Src;

            return fma_ref(A, B, C);
        });
        test_where_ternary<_Type_, N>(arrA, arrB, arrC, m, a, b, c, src, w, wz, simd_fms,[=](_Type_ A, _Type_ B, _Type_ C, _Type_ Src, bool cond, int pos) {
            if (!cond) 
                return Src;

            return fms_ref(A, B, C);
        });
        test_where_ternary<_Type_, N>(arrA, arrB, arrC, m, a, b, c, src, w, wz, simd_fnma,[=](_Type_ A, _Type_ B, _Type_ C, _Type_ Src, bool cond, int pos) {
            if (!cond) 
                return Src;

            return fnma_ref(A, B, C);
        });
        test_where_ternary<_Type_, N>(arrA, arrB, arrC, m, a, b, c, src, w, wz, simd_fnms,[=](_Type_ A, _Type_ B, _Type_ C, _Type_ Src, bool cond, int pos) {
            if (!cond) 
                return Src;

            return fnms_ref(A, B, C);
        });
    }
};

int main() {
    test_all<fused_operations_tests>();
    return 0;
}
