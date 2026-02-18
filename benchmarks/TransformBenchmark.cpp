#include <raze/algorithm/transform/Transform.h>
#include <algorithm>

#include <benchmarks/tools/BenchmarkHelper.h>

template <
    typename            _Type_,
    raze::sizetype  _Size_>
struct _TransformBenchmarkArray {
    _Type_* array;

    _TransformBenchmarkArray() {
        array = new _Type_[_Size_];
    }

    ~_TransformBenchmarkArray() {
        delete[] array;
    }
};

template <
    typename            _Type_,
    raze::sizetype  _Size_>
_TransformBenchmarkArray<_Type_, _Size_> _GenerateArrayForReplaceBenchmark() noexcept {
    _TransformBenchmarkArray<_Type_, _Size_> result;

    for (raze::sizetype i = 0; i < _Size_; ++i)
        result.array[i] = i;

    return result;
}

template <
    typename _Type_,
    SizeForBenchmark sizeForBenchmark = SizeForBenchmark::Large>
class StdTransformBenchmark {
public:
    static inline auto array = _GenerateArrayForReplaceBenchmark<_Type_, sizeForBenchmark>();
    static inline auto array2 = _GenerateArrayForReplaceBenchmark<_Type_, sizeForBenchmark>();

    static inline auto destination = _GenerateArrayForReplaceBenchmark<_Type_, sizeForBenchmark>();

    template <typename _Predicate_>
    static void Transform(benchmark::State& state) noexcept {
        constexpr auto pred = _Predicate_{};

        while (state.KeepRunning()) {
            std::transform(array.array, array.array + sizeForBenchmark, array2.array, destination.array, pred);
            benchmark::DoNotOptimize(array.array);
        }
    }
};

template <
    typename _Type_,
    SizeForBenchmark sizeForBenchmark = SizeForBenchmark::Large>
class RazeTransformBenchmark {
public:
    static inline auto array = _GenerateArrayForReplaceBenchmark<_Type_, sizeForBenchmark>();
    static inline auto array2  = _GenerateArrayForReplaceBenchmark<_Type_, sizeForBenchmark>();

    static inline auto destination = _GenerateArrayForReplaceBenchmark<_Type_, sizeForBenchmark>();

    template <typename _Predicate_>
    static void Transform(benchmark::State& state) noexcept {
        constexpr auto pred = _Predicate_{};

        while (state.KeepRunning()) {
            raze::algorithm::transform(array.array, array.array + sizeForBenchmark, array2.array, destination.array, pred);

            benchmark::DoNotOptimize(array.array);
        }
    }
};

RAZE_ADD_BENCHMARKS_FOR_EACH_SIZE(RazeTransformBenchmark, StdTransformBenchmark, raze::int32, Transform<raze::type_traits::multiplies<>>);


RAZE_BENCHMARK_MAIN();