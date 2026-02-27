#include <raze/math/BitMath.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <raze/compatibility/SimdCompatibility.h>

class BsrInstructionForClzBenchmark {
public:
    static void Clz(benchmark::State& state) noexcept {
        static constexpr unsigned int value = 0b00000000000000000000000000000001;
        unsigned long index = 0;

        while (state.KeepRunning()) {
            _BitScanReverse(&index, value);
            benchmark::DoNotOptimize(index);
        }
    }
};

class LzcntInstructionForClzBenchmark {
public:
    static void Clz(benchmark::State& state) noexcept {
        static constexpr unsigned int value = 0b00000000000000000000000000000001;

        while (state.KeepRunning()) {
            benchmark::DoNotOptimize(_lzcnt_u32(value));
        }
    }
};


class SimdStlClzBenchmark {
public:
    static void Clz(benchmark::State& state) noexcept {
        static constexpr unsigned int value = 0b00000000000000000000000000000001;

        while (state.KeepRunning())
            benchmark::DoNotOptimize(raze::math::count_leading_zero_bits(value));
    }
};

class StdClzBenchmark {
public:
    static void Clz(benchmark::State& state) noexcept {
        static constexpr unsigned int value = 0b00000000000000000000000000000001;

        while (state.KeepRunning())
            benchmark::DoNotOptimize(std::countl_zero(value));
    }
};

RAZE_ADD_BENCHMARK(SimdStlClzBenchmark::Clz, BsrInstructionForClzBenchmark::Clz);
RAZE_ADD_BENCHMARK(SimdStlClzBenchmark::Clz, LzcntInstructionForClzBenchmark::Clz);
RAZE_ADD_BENCHMARK(SimdStlClzBenchmark::Clz, StdClzBenchmark::Clz);

RAZE_BENCHMARK_MAIN();