#include <raze/algorithm/replace/ReplaceCopy.h>
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
        result.array[i] = 42;

    return result;
}

template <
    typename _Type_,
    SizeForBenchmark sizeForBenchmark = SizeForBenchmark::Large>
class StdReplaceCopyBenchmark {
public:
    static inline auto array = _GenerateArrayForReplaceBenchmark<_Type_, sizeForBenchmark>();
    static inline auto destination = _GenerateArrayForReplaceBenchmark<_Type_, sizeForBenchmark>();


    static void ReplaceCopy(benchmark::State& state) noexcept {
        while (state.KeepRunning()) {
            std::replace_copy(array.array, array.array + sizeForBenchmark, destination.array,
                static_cast<_Type_>(42), static_cast<_Type_>(43));

            benchmark::DoNotOptimize(array.array);
        }
    }
};

template <
    typename _Type_,
    SizeForBenchmark sizeForBenchmark = SizeForBenchmark::Large>
class RazeReplaceCopyBenchmark {
public:
    static inline auto array = _GenerateArrayForReplaceBenchmark<_Type_, sizeForBenchmark>();
    static inline auto destination = _GenerateArrayForReplaceBenchmark<_Type_, sizeForBenchmark>();

    static void ReplaceCopy(benchmark::State& state) noexcept {
        while (state.KeepRunning()) {
            raze::algorithm::replace_copy(array.array, array.array + sizeForBenchmark, destination.array,
                static_cast<_Type_>(42), static_cast<_Type_>(43));
            benchmark::DoNotOptimize(array.array);
        }
    }
};

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReplaceCopyBenchmark, StdReplaceCopyBenchmark, raze::int8, ReplaceCopy);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReplaceCopyBenchmark, StdReplaceCopyBenchmark, raze::int16, ReplaceCopy);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReplaceCopyBenchmark, StdReplaceCopyBenchmark, raze::int32, ReplaceCopy);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReplaceCopyBenchmark, StdReplaceCopyBenchmark, raze::int64, ReplaceCopy);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReplaceCopyBenchmark, StdReplaceCopyBenchmark, float, ReplaceCopy);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeReplaceCopyBenchmark, StdReplaceCopyBenchmark, double, ReplaceCopy);

RAZE_BENCHMARK_MAIN();