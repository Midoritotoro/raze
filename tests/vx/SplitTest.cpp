#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>

#include <array>
#include <numeric>
#include <tuple>
#include <utility>

template <
    class _Type_,
    raze::arch::ISA _ISA_,
    raze::u32 _Width_>
struct split_tests {
    template <raze::sizetype N>
    void test_split() {
        if constexpr (N > 1) {
            using Simd = raze::vx::simd<_Type_,  raze::vx::runtime_abi<_ISA_, N>>;

            alignas(64) _Type_ src[N];
            std::iota(src, src + N, _Type_(1));

            auto x = raze::vx::load<Simd>(src);

            auto [lo, hi] = raze::vx::split(x);

            for (raze::sizetype i = 0; i < lo.size(); ++i)
                raze_assert(lo[i] == src[i]);

            for (raze::sizetype i = 0; i < hi.size(); ++i)
                raze_assert(hi[i] == src[lo.size() + i]);

            auto merged = raze::vx::concat(lo, hi);

            for (raze::sizetype i = 0; i < N; ++i)
                raze_assert(merged[i] == src[i]);
        }
    }

    template <raze::sizetype N, raze::sizetype Chunk>
    void test_split_by() {
        using Simd =  raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, N>>;

        alignas(64) _Type_ src[N];
        std::iota(src, src + N, _Type_(1));

        auto x = raze::vx::load<Simd>(src);

        auto parts = raze::vx::split_by<Chunk>(x);

        raze::sizetype offset = 0;

        std::apply(
            [&](const auto&... vecs) {
                (
                    [&] {
                        for (raze::sizetype i = 0; i < vecs.size(); ++i)
                            raze_assert(vecs[i] == src[offset + i]);

                        offset += vecs.size();
                    }(),
                    ...
                );
            },
            parts
        );

        raze_assert(offset == N);
    }

    template <raze::u64 N>
    void run_for_n() {
        test_split<N>();
        test_split_by<N, 1>();

        if constexpr (N >= 2) test_split_by<N, 2>();
        if constexpr (N >= 3) test_split_by<N, 3>();

        test_split_by<N, N>();
    }

    void operator()() {
        run_for_n<_Width_ / (sizeof(_Type_) * 8)>();
        run_for_n<_Width_ / (sizeof(_Type_) * 8) + 1>();
        run_for_n<1>();
    }
};

int main() {
    test_all<split_tests>();
    return 0;
}