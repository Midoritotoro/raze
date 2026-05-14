#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>
#include <array>
#include <numeric>
#include <tuple>
#include <utility>

template <
    class _Type_,
    raze::arch::ISA _ISA_,
    raze::u32    _Width_>
struct concat_tests {
    template <raze::sizetype N, raze::sizetype K>
    void test_k_args() {
        static_assert(K >= 2, "concat requires at least 2 arguments");
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, N>>;

        struct alignas(64) Buffer { _Type_ data[N]; };
        std::array<Buffer, K> bufs;
        std::array<Simd, K> vecs;

        for (raze::sizetype k = 0; k < K; ++k) {
            std::iota(bufs[k].data, bufs[k].data + N, _Type_(k * 1000 + 1));
            vecs[k] = raze::vx::load<Simd>(bufs[k].data);
        }

        auto concatenated = std::apply([](const auto&... args) {
            return raze::vx::concat(args...);
        }, vecs);

        constexpr raze::sizetype total_len = N * K;
        for (raze::sizetype idx = 0; idx < total_len; ++idx) {
            raze::sizetype src_vec = idx / N;
            raze::sizetype src_idx = idx % N;
            raze_assert(concatenated[idx] == bufs[src_vec].data[src_idx]);
        }
    }

    template <raze::u64 N>
    void run_for_n() {
        test_k_args<N, 2>();
        //test_k_args<N, 3>();
        //test_k_args<N, 4>();
    }

    void operator()() {
        run_for_n<_Width_ / (sizeof(_Type_) * 8)>();
        //run_for_n<_Width_ / (sizeof(_Type_) * 8) + 1>();
        //run_for_n<1>();
    }
};

int main() {
    test_all<concat_tests>();
    return 0;
}