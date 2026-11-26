#include <raze/algorithm/numeric/TransformReduce.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_transform_reduce_unary(benchmark::State& state, Fn&& fn) {
    const auto data = rb::make_bounded_sum_data<T>(rb::arg_size(state));
    rb::bench_read<T>(state, data, std::forward<Fn>(fn));
}

template <class T, class Fn>
static void run_transform_reduce_binary(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state);
    const auto a = rb::make_bounded_sum_data<T>(size);
    const std::vector<T> b(size, T(1));
    rb::bench_binary_read<T>(state, a, b, std::forward<Fn>(fn));
}

template <class T>
static void register_type() {
    auto uop = [](T x) { return static_cast<T>(x * T(2)); };

    rb::register_triplet<T>("TransformReduceUnary",
        [uop](benchmark::State& s) { run_transform_reduce_unary<T>(s, [uop](const auto& d) { return raze::algorithm::transform_reduce(d, T(0), std::plus<>{}, uop); }); },
        [uop](benchmark::State& s) { run_transform_reduce_unary<T>(s, [uop](const auto& d) { return std::transform_reduce(d.begin(), d.end(), T(0), std::plus<>{}, uop); }); },
        [uop](benchmark::State& s) { run_transform_reduce_unary<T>(s, [uop](const auto& d) { return raze::algorithm::transform_reduce[raze::options::fscalar](d, T(0), std::plus<>{}, uop); }); });

    rb::register_triplet<T>("TransformReduceBinary",
        [](benchmark::State& s) { run_transform_reduce_binary<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::transform_reduce(a, b, T(0)); }); },
        [](benchmark::State& s) { run_transform_reduce_binary<T>(s, [](const auto& a, const auto& b) { return std::transform_reduce(a.begin(), a.end(), b.begin(), T(0)); }); },
        [](benchmark::State& s) { run_transform_reduce_binary<T>(s, [](const auto& a, const auto& b) { return raze::algorithm::transform_reduce[raze::options::fscalar](a, b, T(0)); }); });
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();

