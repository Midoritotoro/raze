#include <raze/algorithm/find/SearchN.h>
#include <benchmarks/tools/BenchmarkHelper.h>

namespace rb = raze::bench;

template <class T, class Fn>
static void run_search_n(benchmark::State& state, Fn&& fn) {
    const auto size = rb::arg_size(state, 0);
    const auto pos = rb::arg_size(state, 1);
    const auto count = static_cast<std::ptrdiff_t>(state.range(2));
    const auto data = rb::make_search_n_data<T>(size, pos, static_cast<raze::sizetype>(count));
    rb::bench_read<T>(state, data, [&](const auto& d) { return fn(d, count); });
}

template <class T>
static void register_type() {
    rb::register_triplet<T>("SearchN",
        [](benchmark::State& s) { run_search_n<T>(s, [](const auto& d, std::ptrdiff_t n) { return raze::algorithm::search_n(d, n, rb::needle_value<T>()); }); },
        [](benchmark::State& s) { run_search_n<T>(s, [](const auto& d, std::ptrdiff_t n) { return std::ranges::search_n(d, n, rb::needle_value<T>()); }); },
        [](benchmark::State& s) { run_search_n<T>(s, [](const auto& d, std::ptrdiff_t n) { return raze::algorithm::search_n[raze::options::fscalar](d, n, rb::needle_value<T>()); }); },
        rb::search_end_cases);

    rb::register_triplet<T>("SearchNPred",
        [](benchmark::State& s) { run_search_n<T>(s, [](const auto& d, std::ptrdiff_t n) { return raze::algorithm::search_n(d, n, rb::needle_value<T>(), rb::binary_eq_pred<T>()); }); },
        [](benchmark::State& s) { run_search_n<T>(s, [](const auto& d, std::ptrdiff_t n) { return std::ranges::search_n(d, n, rb::needle_value<T>(), rb::binary_eq_pred<T>()); }); },
        [](benchmark::State& s) { run_search_n<T>(s, [](const auto& d, std::ptrdiff_t n) { return raze::algorithm::search_n[raze::options::fscalar](d, n, rb::needle_value<T>(), rb::binary_eq_pred<T>()); }); },
        rb::search_end_cases);
}

void RegisterAll() {
    rb::register_all_types([]<class T>() { register_type<T>(); });
}

RAZE_BENCHMARK_MAIN();