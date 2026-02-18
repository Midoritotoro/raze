#include <raze/algorithm/fill/Fill.h>
#include <algorithm>

#include <benchmarks/tools/BenchmarkHelper.h>


template <
    typename            _Type_,
    raze::sizetype  _Size_>
struct _FillBenchmarkArray {
    _Type_* array;

    _FillBenchmarkArray() {
        array = new _Type_[_Size_];
    }

    ~_FillBenchmarkArray() {
        delete[] array;
    }
};

template <
    typename            _Type_,
    raze::sizetype  _Size_>
_FillBenchmarkArray<_Type_, _Size_> _GenerateArrayForBenchmark() noexcept {
    _FillBenchmarkArray<_Type_, _Size_> result;

    for (raze::sizetype i = 0; i < _Size_; ++i)
        result.array[i] = i;

    return result;
}

template <
    typename _Type_,
    SizeForBenchmark sizeForBenchmark>
class StdFillBenchmark {
public:
    static void Fill(benchmark::State& state) noexcept {
        auto array = _GenerateArrayForBenchmark<_Type_, sizeForBenchmark>();

        while (state.KeepRunning()) {
            std::fill(array.array, array.array + sizeForBenchmark, std::numeric_limits<_Type_>::max());
            benchmark::DoNotOptimize(array.array);
            benchmark::ClobberMemory();
        }
    }
};

template <
    typename _Type_,
    SizeForBenchmark sizeForBenchmark>
class RazeFillBenchmark {
public:
    static void Fill(benchmark::State& state) noexcept {
        auto array = _GenerateArrayForBenchmark<_Type_, sizeForBenchmark>();

        while (state.KeepRunning()) {
            raze::algorithm::fill(array.array, array.array + sizeForBenchmark, std::numeric_limits<_Type_>::max());
            benchmark::DoNotOptimize(array.array);
            benchmark::ClobberMemory();
        }
    }
};

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFillBenchmark, StdFillBenchmark, raze::int8, Fill);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFillBenchmark, StdFillBenchmark, raze::int16, Fill);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFillBenchmark, StdFillBenchmark, raze::int32, Fill);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFillBenchmark, StdFillBenchmark, raze::int64, Fill);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFillBenchmark, StdFillBenchmark, float, Fill);
RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeFillBenchmark, StdFillBenchmark, double, Fill);

RAZE_BENCHMARK_MAIN();
