#include <raze/math/BitMath.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <raze/compatibility/SimdCompatibility.h>

class BsfInstructionForCtzBenchmark {
public:
    static void Ctz(benchmark::State& state) noexcept {
        static constexpr unsigned int value = 0b10000000000000000000000000000000;
        unsigned long index = 0;

        while (state.KeepRunning()) {
            _BitScanForward(&index, value);
            benchmark::DoNotOptimize(index);
        }
    }
};

class TzcntInstructionForCtzBenchmark {
public:
    static void Ctz(benchmark::State& state) noexcept {
        static constexpr unsigned int value = 0b10000000000000000000000000000000;

        while (state.KeepRunning()) {
            benchmark::DoNotOptimize(_tzcnt_u32(value));
        }
    }
};


class SimdStlCtzBenchmark {
public:
    static void Ctz(benchmark::State& state) noexcept {
        static constexpr unsigned int value = 0b10000000000000000000000000000000;

        while (state.KeepRunning()) {
            benchmark::DoNotOptimize(raze::math::CountTrailingZeroBits(value));
        }
    }
};

class StdCtzBenchmark {
public:
    static void Ctz(benchmark::State& state) noexcept {
        static constexpr unsigned int value = 0b10000000000000000000000000000000;

        while (state.KeepRunning()) {
            benchmark::DoNotOptimize(std::countr_zero(value));
        }
    }
};

//RAZE_ADD_BENCHMARK_WITH_CUSTOM_REPITITIONS(SimdStlCtzBenchmark::Ctz, BsfInstructionForCtzBenchmark::Ctz, 1000000);
//RAZE_ADD_BENCHMARK_WITH_CUSTOM_REPITITIONS(SimdStlCtzBenchmark::Ctz, TzcntInstructionForCtzBenchmark::Ctz, 1000000);
RAZE_ADD_BENCHMARK_WITH_CUSTOM_REPITITIONS(SimdStlCtzBenchmark::Ctz, StdCtzBenchmark::Ctz, 1000000);

RAZE_BENCHMARK_MAIN();