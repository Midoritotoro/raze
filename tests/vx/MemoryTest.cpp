#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::u32    _Width_>
struct variable_length_memory_tests {
    using U = typename raze::IntegerForSizeof<_Type_>::Unsigned;

    template <raze::u32 _N_>
    void test_size() const {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, _N_>>;
        constexpr size_t N = _N_;
        constexpr size_t PAD = 4;

        {
            alignas(64) _Type_ src[N];
            alignas(64) _Type_ dst[N];
            for (size_t i = 0; i < N; ++i) src[i] = static_cast<_Type_>(i * 3 + 1);

            Simd v;
            v = raze::vx::load[raze::vx::aligned](raze::options::as<Simd>(), src);
            //v.copy_from(src, raze::vx::aligned);
            //v.copy_to(dst, raze::vx::aligned);
            //raze_assert(std::equal(src, src + N, dst));
        }

        //{
        //    alignas(64) _Type_ buf[N * 2 + PAD];
        //    _Type_* src = buf + 1;
        //    _Type_* dst = buf + N + 1;
        //    for (size_t i = 0; i < N; ++i) src[i] = static_cast<_Type_>(i * 7 + 2);

        //    Simd v;
        //    v.copy_from(src);
        //    v.copy_to(dst);
        //    raze_assert(std::equal(src, src + N, dst));
        //}

        //{
        //    alignas(64) _Type_ src[N];
        //    alignas(64) _Type_ buf[N + PAD];
        //    _Type_* dst = buf + 1;
        //    for (size_t i = 0; i < N; ++i) src[i] = static_cast<_Type_>(~i);

        //    Simd v;
        //    v.copy_from(src, raze::vx::aligned);
        //    v.copy_to(dst);
        //    raze_assert(std::equal(src, src + N, dst));
        //}

        //{
        //    alignas(64) _Type_ buf[N + PAD];
        //    alignas(64) _Type_ dst[N];
        //    _Type_* src = buf + 2;
        //    for (size_t i = 0; i < N; ++i) src[i] = static_cast<_Type_>(i * 13);

        //    raze::vx::load<Simd>[raze::vx::unsafe]();

        //    Simd v;
        //    v.copy_from(src);
        //    v.copy_to(dst, raze::vx::aligned);
        //    raze_assert(std::equal(src, src + N, dst));
        //}

        //{
        //    alignas(64) _Type_ src[N];
        //    alignas(64) _Type_ dst[N];
        //    for (size_t i = 0; i < N; ++i) src[i] = static_cast<_Type_>(i * 257 + 42);

        //    Simd v;
        //    v.copy_from(src);
        //    v.copy_to(dst);
        //    raze_assert(std::equal(src, src + N, dst));
        //}
    }

    void operator()() const {
        test_size<_Width_ / (sizeof(_Type_) * 8)>();
        test_size<1>();
        test_size<7>();
    }
};

int main() {
    test_all<variable_length_memory_tests>();
    return 0;
}
