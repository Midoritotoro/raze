#include <raze/algorithm/find/FindEnd.h>
#include <benchmarks/tools/BenchmarkHelper.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <random>

template <
    class _FirstUnwrappedBidirectionalIterator_,
    class _SecondUnwrappedBidirectionalIterator_,
    class _Predicate_>
__simd_nodiscard_inline_constexpr _FirstUnwrappedBidirectionalIterator_ scalar_find_end_unchecked(
    _FirstUnwrappedBidirectionalIterator_	__first1_unwrapped,
    _FirstUnwrappedBidirectionalIterator_	__last1_unwrapped,
    _SecondUnwrappedBidirectionalIterator_	__first2_unwrapped,
    _SecondUnwrappedBidirectionalIterator_	__last2_unwrapped,
    _Predicate_								__predicate)
{
    for (auto __candidate_unwrapped = __last1_unwrapped;; --__candidate_unwrapped) {
        auto __next1_unwrapped = __candidate_unwrapped;
        auto __next2_unwrapped = __last2_unwrapped;

        for (;;) {
            if (__first2_unwrapped == __next2_unwrapped) {
                raze::algorithm::__seek_possibly_wrapped_iterator(__first1_unwrapped, __next1_unwrapped);
                return __first1_unwrapped;
            }

            if (__first1_unwrapped == __next1_unwrapped)
                return __last1_unwrapped;

            --__next1_unwrapped;
            --__next2_unwrapped;

            if (__predicate(*__next1_unwrapped, *__next2_unwrapped) == false)
                break;
        }
    }
}

template <
    class _FirstBidirectionalIterator_,
    class _SecondBidirectionalIterator_,
    class _Predicate_ = std::equal_to<>>
__simd_nodiscard_inline_constexpr _FirstBidirectionalIterator_ scalar_find_end(
    _FirstBidirectionalIterator_	__first1,
    _FirstBidirectionalIterator_    __last1,
    _SecondBidirectionalIterator_	__first2,
    _SecondBidirectionalIterator_	__last2,
    _Predicate_						__predicate = _Predicate_{})
{
    raze::algorithm::__seek_possibly_wrapped_iterator(__first1, scalar_find_end_unchecked(raze::algorithm::__unwrap_iterator(__first1),
        raze::algorithm::__unwrap_iterator(__last1), raze::algorithm::__unwrap_iterator(__first2), raze::algorithm::__unwrap_iterator(__last2),
        raze::type_traits::__pass_function(__predicate)));

    return __first1;
}

template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmarkMain,
    SizeForBenchmark sizeForBenchmarkSub>
class StdFindEndBenchmark {
public:
    static void SubAtBegin(benchmark::State& state) noexcept {
        std::vector<_Char_> haystack(sizeForBenchmarkMain);
        std::vector<_Char_> needle(sizeForBenchmarkSub);

        std::fill(haystack.begin(), haystack.end(), static_cast<_Char_>(1));
        std::fill(needle.begin(), needle.end(), static_cast<_Char_>(2));

        std::copy(needle.begin(), needle.end(), haystack.begin());

        while (state.KeepRunning()) {
            auto result = std::find_end(haystack.begin(), haystack.end(), needle.begin(), needle.end());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }


    static void SubInMiddle(benchmark::State& state) noexcept {
        std::vector<_Char_> haystack(sizeForBenchmarkMain);
        std::vector<_Char_> needle(sizeForBenchmarkSub);

        std::fill(haystack.begin(), haystack.end(), static_cast<_Char_>(1));
        std::fill(needle.begin(), needle.end(), static_cast<_Char_>(2));

        if (sizeForBenchmarkSub <= sizeForBenchmarkMain)
            std::copy(needle.begin(), needle.end(), haystack.begin() + (sizeForBenchmarkMain - sizeForBenchmarkSub) / 2);

        while (state.KeepRunning()) {
            auto result = std::find_end(haystack.begin(), haystack.end(), needle.begin(), needle.end());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }


    static void SubAtEnd(benchmark::State& state) noexcept {
        std::vector<_Char_> haystack(sizeForBenchmarkMain);
        std::vector<_Char_> needle(sizeForBenchmarkSub);

        std::fill(haystack.begin(), haystack.end(), static_cast<_Char_>(1));
        std::fill(needle.begin(), needle.end(), static_cast<_Char_>(2));

        if (sizeForBenchmarkSub <= sizeForBenchmarkMain)
            std::copy(needle.begin(), needle.end(), haystack.begin() + (sizeForBenchmarkMain - sizeForBenchmarkSub));

        while (state.KeepRunning()) {
            auto result = std::find_end(haystack.begin(), haystack.end(), needle.begin(), needle.end());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};

template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmarkMain,
    SizeForBenchmark sizeForBenchmarkSub>
class RazeFindEndBenchmark {
public:
    static void SubAtBegin(benchmark::State& state) noexcept {
        std::vector<_Char_> haystack(sizeForBenchmarkMain);
        std::vector<_Char_> needle(sizeForBenchmarkSub);

        std::fill(haystack.begin(), haystack.end(), static_cast<_Char_>(1));
        std::fill(needle.begin(), needle.end(), static_cast<_Char_>(2));

        std::copy(needle.begin(), needle.end(), haystack.begin());

        while (state.KeepRunning()) {
            auto result = raze::algorithm::find_end(haystack.begin(), haystack.end(), needle.begin(), needle.end());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }


    static void SubInMiddle(benchmark::State& state) noexcept {
        std::vector<_Char_> haystack(sizeForBenchmarkMain);
        std::vector<_Char_> needle(sizeForBenchmarkSub);

        std::fill(haystack.begin(), haystack.end(), static_cast<_Char_>(1));
        std::fill(needle.begin(), needle.end(), static_cast<_Char_>(2));

        if (sizeForBenchmarkSub <= sizeForBenchmarkMain)
            std::copy(needle.begin(), needle.end(), haystack.begin() + (sizeForBenchmarkMain - sizeForBenchmarkSub) / 2);

        while (state.KeepRunning()) {
            auto result = raze::algorithm::find_end(haystack.begin(), haystack.end(), needle.begin(), needle.end());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }


    static void SubAtEnd(benchmark::State& state) noexcept {
        std::vector<_Char_> haystack(sizeForBenchmarkMain);
        std::vector<_Char_> needle(sizeForBenchmarkSub);

        std::fill(haystack.begin(), haystack.end(), static_cast<_Char_>(1));
        std::fill(needle.begin(), needle.end(), static_cast<_Char_>(2));

        if (sizeForBenchmarkSub <= sizeForBenchmarkMain)
            std::copy(needle.begin(), needle.end(), haystack.begin() + (sizeForBenchmarkMain - sizeForBenchmarkSub));

        while (state.KeepRunning()) {
            auto result = raze::algorithm::find_end(haystack.begin(), haystack.end(), needle.begin(), needle.end());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};

template <
    typename _Char_,
    SizeForBenchmark sizeForBenchmarkMain,
    SizeForBenchmark sizeForBenchmarkSub>
class ScalarFindEndBenchmark {
public:
    static void SubAtBegin(benchmark::State& state) noexcept {
        std::vector<_Char_> haystack(sizeForBenchmarkMain);
        std::vector<_Char_> needle(sizeForBenchmarkSub);

        std::fill(haystack.begin(), haystack.end(), static_cast<_Char_>(1));
        std::fill(needle.begin(), needle.end(), static_cast<_Char_>(2));

        std::copy(needle.begin(), needle.end(), haystack.begin());

        while (state.KeepRunning()) {
            auto result = scalar_find_end(haystack.begin(), haystack.end(), needle.begin(), needle.end());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }


    static void SubInMiddle(benchmark::State& state) noexcept {
        std::vector<_Char_> haystack(sizeForBenchmarkMain);
        std::vector<_Char_> needle(sizeForBenchmarkSub);

        std::fill(haystack.begin(), haystack.end(), static_cast<_Char_>(1));
        std::fill(needle.begin(), needle.end(), static_cast<_Char_>(2));

        if (sizeForBenchmarkSub <= sizeForBenchmarkMain)
            std::copy(needle.begin(), needle.end(), haystack.begin() + (sizeForBenchmarkMain - sizeForBenchmarkSub) / 2);

        while (state.KeepRunning()) {
            auto result = scalar_find_end(haystack.begin(), haystack.end(), needle.begin(), needle.end());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }


    static void SubAtEnd(benchmark::State& state) noexcept {
        std::vector<_Char_> haystack(sizeForBenchmarkMain);
        std::vector<_Char_> needle(sizeForBenchmarkSub);

        std::fill(haystack.begin(), haystack.end(), static_cast<_Char_>(1));
        std::fill(needle.begin(), needle.end(), static_cast<_Char_>(2));

        if (sizeForBenchmarkSub <= sizeForBenchmarkMain)
            std::copy(needle.begin(), needle.end(), haystack.begin() + (sizeForBenchmarkMain - sizeForBenchmarkSub));

        while (state.KeepRunning()) {
            auto result = scalar_find_end(haystack.begin(), haystack.end(), needle.begin(), needle.end());
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    }
};

#define REGISTER_ALL(TEST) \
    RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeFindEndBenchmark, ScalarFindEndBenchmark, raze::int8, TEST); \
    RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeFindEndBenchmark, ScalarFindEndBenchmark, raze::int16, TEST); \
    RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeFindEndBenchmark, ScalarFindEndBenchmark, raze::int32, TEST); \
    RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeFindEndBenchmark, ScalarFindEndBenchmark, raze::int64, TEST); \
    RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeFindEndBenchmark, StdFindEndBenchmark, raze::int8, TEST); \
    RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeFindEndBenchmark, StdFindEndBenchmark, raze::int16, TEST); \
    RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeFindEndBenchmark, StdFindEndBenchmark, raze::int32, TEST); \
    RAZE_ADD_SEARCH_BENCHMARKS_FOR_EACH_SIZE(RazeFindEndBenchmark, StdFindEndBenchmark, raze::int64, TEST);

REGISTER_ALL(SubAtBegin)
REGISTER_ALL(SubInMiddle)
REGISTER_ALL(SubAtEnd)


RAZE_BENCHMARK_MAIN();
