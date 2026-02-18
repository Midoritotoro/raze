#include <raze/algorithm/replace/Replace.h>
#include <algorithm>

#include <benchmarks/tools/BenchmarkHelper.h>

template <
    typename            _Type_,
    raze::sizetype  _Size_>
struct _ReplaceBenchmarkArray {
    _Type_* array;

    _ReplaceBenchmarkArray() {
        array = new _Type_[_Size_];
    }

    ~_ReplaceBenchmarkArray() {
        delete[] array;
    }
};

template <
    typename            _Type_,
    raze::sizetype  _Size_>
_ReplaceBenchmarkArray<_Type_, _Size_> _GenerateArrayForReplaceBenchmark() noexcept {
    _ReplaceBenchmarkArray<_Type_, _Size_> result;

    for (raze::sizetype i = 0; i < _Size_; ++i)
        result.array[i] = i;

    for (raze::sizetype i = 0; i < _Size_; i += 2)
        result.array[i] = raze::math::__maximum_integral_limit<_Type_>() >> 1;

    return result;
}

template <
    typename _Type_,
    SizeForBenchmark sizeForBenchmark>
class StdReplaceBenchmark {
public:
    static void Replace(benchmark::State& state) noexcept {
        _Type_ array[sizeForBenchmark];

        for (raze::sizetype i = 0; i < sizeForBenchmark; ++i)
            array[i] = i;

        for (raze::sizetype i = 0; i < sizeForBenchmark; i += 2)
            array[i] = 42;

        while (state.KeepRunning()) {
            std::replace(array, array + sizeForBenchmark, 42, 43);
            benchmark::DoNotOptimize(array);
            benchmark::ClobberMemory();
        }
    }
};

template <
    typename _Type_,
    SizeForBenchmark sizeForBenchmark>
class RazeReplaceBenchmark {
public:
    static void Replace(benchmark::State& state) noexcept {
        _Type_ array[sizeForBenchmark];

        for (raze::sizetype i = 0; i < sizeForBenchmark; ++i)
            array[i] = i;

        for (raze::sizetype i = 0; i < sizeForBenchmark; i += 2)
            array[i] = 42;

        while (state.KeepRunning()) {
            raze::algorithm::replace(array, array + sizeForBenchmark, 42, 43);
            benchmark::DoNotOptimize(array);
            benchmark::ClobberMemory();
        }
    }
};

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReplaceBenchmark, StdReplaceBenchmark, raze::int8, Replace);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReplaceBenchmark, StdReplaceBenchmark, raze::int16, Replace);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReplaceBenchmark, StdReplaceBenchmark, raze::int32, Replace);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReplaceBenchmark, StdReplaceBenchmark, raze::int64, Replace);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReplaceBenchmark, StdReplaceBenchmark, float, Replace);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReplaceBenchmark, StdReplaceBenchmark, double, Replace);

RAZE_BENCHMARK_MAIN();