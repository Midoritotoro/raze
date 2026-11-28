#pragma once

#include <src/raze/meta/tuple/Traits.h>


__RAZE_META_NAMESPACE_BEGIN

template <class F, class Acc>
struct __lfold_box {
    F& _f;
    Acc _acc;

    template <class Elem>
    raze_always_inline constexpr auto operator<<(Elem&& e) && {
        using res_t = decltype(__invoke(_f, static_cast<Acc&&>(_acc), std::forward<Elem>(e)));
        return __lfold_box<F, res_t>{ _f, __invoke(_f, static_cast<Acc&&>(_acc), std::forward<Elem>(e)) };
    }
};

template <sizetype I, class Elem>
struct __indexed_elem {
    Elem&& _e;
};

template <class F, class Acc>
struct __lfold_idx_box {
    F& _f;
    Acc _acc;

    template <sizetype I, class Elem>
    raze_always_inline constexpr auto operator<<(__indexed_elem<I, Elem>&& item) && {
        using res_t = decltype(__invoke(_f, static_cast<Acc&&>(_acc), index<I>, std::forward<Elem>(item._e)));
        return __lfold_idx_box<F, res_t>{ _f, __invoke(_f, static_cast<Acc&&>(_acc), index<I>, std::forward<Elem>(item._e)) };
    }
};

template <class F, class Acc>
struct __rfold_box {
    F& _f;
    Acc _acc;
};

template <class Elem>
struct __rfold_elem {
    Elem&& _e;

    template <class F, class Acc>
    friend raze_always_inline constexpr auto operator>>(__rfold_elem&& lhs, __rfold_box<F, Acc>&& rhs) {
        using res_t = decltype(__invoke(rhs._f, std::forward<Elem>(lhs._e), static_cast<Acc&&>(rhs._acc)));
        return __rfold_box<F, res_t>{ rhs._f, __invoke(rhs._f, std::forward<Elem>(lhs._e), static_cast<Acc&&>(rhs._acc)) };
    }
};

template <sizetype I, class Elem>
struct __rfold_idx_elem {
    Elem&& _e;

    template <class F, class Acc>
    friend raze_always_inline constexpr auto operator>>(__rfold_idx_elem&& lhs, __rfold_box<F, Acc>&& rhs) {
        using res_t = decltype(__invoke(rhs._f, index<I>, std::forward<Elem>(lhs._e), static_cast<Acc&&>(rhs._acc)));
        return __rfold_box<F, res_t>{ rhs._f, __invoke(rhs._f, index<I>, std::forward<Elem>(lhs._e), static_cast<Acc&&>(rhs._acc)) };
    }
};

template <sizetype B, sizetype E, class Op, class T>
constexpr decltype(auto) __tree_reduce(Op& op, T&& t) {
    constexpr sizetype len = E - B;

    if constexpr (len == 1) {
        return __adl_get<B>(std::forward<T>(t));
    }
    else if constexpr (len == 2) {
        return __invoke(op, __adl_get<B>(std::forward<T>(t)), __adl_get<B + 1>(std::forward<T>(t)));
    }
    else {
        constexpr sizetype mid = B + len / 2;
        return __invoke(op,
            __tree_reduce<B, mid>(op, std::forward<T>(t)),
            __tree_reduce<mid, E>(op, std::forward<T>(t)));
    }
}

template <sizetype B, sizetype E, class F, class Op, class ... Ts>
constexpr decltype(auto) __tree_map_reduce(F& f, Op& op, Ts&& ... ts) {
    constexpr sizetype len = E - B;

    if constexpr (len == 1) {
        return __invoke(f, __adl_get<B>(std::forward<Ts>(ts))...);
    }
    else if constexpr (len == 2) {
        return __invoke(op,
            __invoke(f, __adl_get<B>(std::forward<Ts>(ts))...),
            __invoke(f, __adl_get<B + 1>(std::forward<Ts>(ts))...));
    }
    else {
        constexpr sizetype mid = B + len / 2;
        return __invoke(op,
            __tree_map_reduce<B, mid>(f, op, std::forward<Ts>(ts)...),
            __tree_map_reduce<mid, E>(f, op, std::forward<Ts>(ts)...));
    }
}

template <sizetype I, class Op, class T, class Cur, class ... Acc>
constexpr auto __inc_scan_l(Op& op, T&& t, Cur&& cur, Acc&& ... acc) {
    if constexpr (I == size_v<T>) {
        return meta::make_tuple(std::forward<Acc>(acc)..., std::forward<Cur>(cur));
    }
    else {
        auto next = __invoke(op, cur, __adl_get<I>(std::forward<T>(t)));
        return __inc_scan_l<I + 1>(op, std::forward<T>(t), std::move(next),
            std::forward<Acc>(acc)..., std::forward<Cur>(cur));
    }
}

template <sizetype I, class Op, class T, class Cur, class ... Acc>
constexpr auto __exc_scan_l(Op& op, T&& t, Cur&& cur, Acc&& ... acc) {
    if constexpr (I + 1 == size_v<T>) {
        return meta::make_tuple(std::forward<Acc>(acc)..., std::forward<Cur>(cur));
    }
    else {
        auto next = __invoke(op, cur, __adl_get<I>(std::forward<T>(t)));
        return __exc_scan_l<I + 1>(op, std::forward<T>(t), std::move(next),
            std::forward<Acc>(acc)..., std::forward<Cur>(cur));
    }
}

template <sizetype I, class Op, class T, class Cur, class ... Acc>
constexpr auto __inc_scan_r(Op& op, T&& t, Cur&& cur, Acc&& ... acc) {
    if constexpr (I == 0) {
        return meta::make_tuple(std::forward<Cur>(cur), std::forward<Acc>(acc)...);
    }
    else {
        auto next = __invoke(op, __adl_get<I - 1>(std::forward<T>(t)), cur);
        return __inc_scan_r<I - 1>(op, std::forward<T>(t), std::move(next),
            std::forward<Cur>(cur), std::forward<Acc>(acc)...);
    }
}

template <sizetype I, class Op, class T, class Cur, class ... Acc>
constexpr auto __exc_scan_r(Op& op, T&& t, Cur&& cur, Acc&& ... acc) {
    if constexpr (I == 0) {
        return meta::make_tuple(std::forward<Cur>(cur), std::forward<Acc>(acc)...);
    }
    else {
        auto next = __invoke(op, __adl_get<I>(std::forward<T>(t)), cur);
        return __exc_scan_r<I - 1>(op, std::forward<T>(t), std::move(next),
            std::forward<Cur>(cur), std::forward<Acc>(acc)...);
    }
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Left-associative fold over `t` starting from initial accumulator `init`: `f(...f(f(init, t[0]), t[1])..., t[N-1])`.
 *
 * Supports heterogeneous accumulator type changes across steps.
 *
 * @tparam F Binary folding callable `(acc, elem)`.
 * @tparam T A `tuple_like` type.
 * @tparam Init Initial accumulator type.
 * @param f Folding callable.
 * @param t Source tuple-like object.
 * @param init Initial accumulator value (returned forwarded if `size_v<T> == 0`).
 * @return Final accumulated value.
 */
template <class F, tuple_like T, class Init>
raze_nodiscard constexpr decltype(auto) fold_left(F&& f, T&& t, Init&& init) {
    if constexpr (size_v<T> == 0) {
        return std::forward<Init>(init);
    }
    else {
        return [&] <sizetype ... Is> (std::index_sequence<Is...>) -> decltype(auto) {
            return (__lfold_box<std::remove_reference_t<F>, Init>{ f, std::forward<Init>(init) }
                << ... << __adl_get<Is>(std::forward<T>(t)))._acc;
        }(std::make_index_sequence<size_v<T>>{});
    }
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Left-associative fold over a non-empty `t` using `t[0]` as the initial accumulator.
 * @tparam F Binary folding callable `(acc, elem)`.
 * @tparam T A `non_empty_tuple_like` type.
 * @param f Folding callable.
 * @param t Source non-empty tuple-like object.
 * @return Final accumulated value (or `get<0>(t)` if `size_v<T> == 1`).
 */
template <class F, non_empty_tuple_like T>
raze_nodiscard constexpr decltype(auto) fold_left(F&& f, T&& t) {
    if constexpr (size_v<T> == 1) {
        return __adl_get<0>(std::forward<T>(t));
    }
    else {
        return [&] <sizetype ... Is> (std::index_sequence<Is...>) -> decltype(auto) {
            using first_t = member_t<0, T>;
            return (__lfold_box<std::remove_reference_t<F>, first_t>{ f, __adl_get<0>(std::forward<T>(t)) }
                << ... << __adl_get<Is + 1>(std::forward<T>(t)))._acc;
        }(std::make_index_sequence<size_v<T> - 1>{});
    }
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Indexed left-associative fold invoking `f(acc, index<I>, t[I])` for `I = 0 .. N-1`.
 * @tparam F Ternary folding callable `(acc, index_t<I>, elem)`.
 * @tparam T A `tuple_like` type.
 * @tparam Init Initial accumulator type.
 * @param f Folding callable.
 * @param t Source tuple-like object.
 * @param init Initial accumulator value.
 * @return Final accumulated value.
 */
template <class F, tuple_like T, class Init>
raze_nodiscard constexpr decltype(auto) fold_left_index(F&& f, T&& t, Init&& init) {
    if constexpr (size_v<T> == 0) {
        return std::forward<Init>(init);
    }
    else {
        return [&] <sizetype ... Is> (std::index_sequence<Is...>) -> decltype(auto) {
            return (__lfold_idx_box<std::remove_reference_t<F>, Init>{ f, std::forward<Init>(init) }
                << ... << __indexed_elem<Is, member_t<Is, T>>{ __adl_get<Is>(std::forward<T>(t)) })._acc;
        }(std::make_index_sequence<size_v<T>>{});
    }
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Right-associative fold over `t` starting from `init`: `f(t[0], f(t[1], ... f(t[N-1], init)))`.
 * @tparam F Binary folding callable `(elem, acc)`.
 * @tparam T A `tuple_like` type.
 * @tparam Init Initial accumulator type.
 * @param f Folding callable.
 * @param t Source tuple-like object.
 * @param init Initial accumulator value (returned forwarded if `size_v<T> == 0`).
 * @return Final accumulated value.
 */
template <class F, tuple_like T, class Init>
raze_nodiscard constexpr decltype(auto) fold_right(F&& f, T&& t, Init&& init) {
    if constexpr (size_v<T> == 0) {
        return std::forward<Init>(init);
    }
    else {
        return [&] <sizetype ... Is> (std::index_sequence<Is...>) -> decltype(auto) {
            return (__rfold_elem<member_t<Is, T>>{ __adl_get<Is>(std::forward<T>(t)) }
                >> ... >> __rfold_box<std::remove_reference_t<F>, Init>{ f, std::forward<Init>(init) })._acc;
        }(std::make_index_sequence<size_v<T>>{});
    }
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Right-associative fold over a non-empty `t` using `t[N-1]` as the initial accumulator.
 * @tparam F Binary folding callable `(elem, acc)`.
 * @tparam T A `non_empty_tuple_like` type.
 * @param f Folding callable.
 * @param t Source non-empty tuple-like object.
 * @return Final accumulated value (or `get<0>(t)` if `size_v<T> == 1`).
 */
template <class F, non_empty_tuple_like T>
raze_nodiscard constexpr decltype(auto) fold_right(F&& f, T&& t) {
    if constexpr (size_v<T> == 1) {
        return __adl_get<0>(std::forward<T>(t));
    }
    else {
        constexpr sizetype last = size_v<T> - 1;

        return [&] <sizetype ... Is> (std::index_sequence<Is...>) -> decltype(auto) {
            using last_t = member_t<last, T>;
            return (__rfold_elem<member_t<Is, T>>{ __adl_get<Is>(std::forward<T>(t)) }
                >> ... >> __rfold_box<std::remove_reference_t<F>, last_t>{
                    f, __adl_get<last>(std::forward<T>(t)) })._acc;
        }(std::make_index_sequence<last>{});
    }
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Indexed right-associative fold invoking `f(index<I>, t[I], acc)` from `I = N-1` down to `0`.
 * @tparam F Ternary folding callable `(index_t<I>, elem, acc)`.
 * @tparam T A `tuple_like` type.
 * @tparam Init Initial accumulator type.
 * @param f Folding callable.
 * @param t Source tuple-like object.
 * @param init Initial accumulator value.
 * @return Final accumulated value.
 */
template <class F, tuple_like T, class Init>
raze_nodiscard constexpr decltype(auto) fold_right_index(F&& f, T&& t, Init&& init) {
    if constexpr (size_v<T> == 0) {
        return std::forward<Init>(init);
    }
    else {
        return [&] <sizetype ... Is> (std::index_sequence<Is...>) -> decltype(auto) {
            return (__rfold_idx_elem<Is, member_t<Is, T>>{ __adl_get<Is>(std::forward<T>(t)) }
                >> ... >> __rfold_box<std::remove_reference_t<F>, Init>{ f, std::forward<Init>(init) })._acc;
        }(std::make_index_sequence<size_v<T>>{});
    }
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Reduces a non-empty `tuple_like` via a balanced binary tree to minimize dependency chain depth (ILP-friendly).
 * @tparam Op Associative binary reduction operator.
 * @tparam T A `non_empty_tuple_like` type.
 * @param op Binary reduction operator.
 * @param t Source tuple-like object.
 * @return Reduced value.
 */
template <class Op, non_empty_tuple_like T>
raze_nodiscard constexpr decltype(auto) reduce(Op&& op, T&& t) {
    return __tree_reduce<0, size_v<T>>(op, std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Reduces `t` via a balanced binary tree and combines the result with `init` via `op(init, tree_result)`.
 * @tparam Op Associative binary reduction operator.
 * @tparam T A `tuple_like` type.
 * @tparam Init Initial value type.
 * @param op Binary reduction operator.
 * @param t Source tuple-like object.
 * @param init Initial value (returned directly if `size_v<T> == 0`).
 * @return Reduced value.
 */
template <class Op, tuple_like T, class Init>
raze_nodiscard constexpr auto reduce(Op&& op, T&& t, Init&& init) {
    if constexpr (size_v<T> == 0)
        return std::forward<Init>(init);
    else
        return __invoke(op, std::forward<Init>(init), __tree_reduce<0, size_v<T>>(op, std::forward<T>(t)));
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Fused transform and balanced binary-tree reduction across one or more equal-size non-empty tuples without intermediate tuple materialization.
 * @tparam F Element-wise transform callable.
 * @tparam Op Associative binary reduction operator.
 * @tparam T First `non_empty_tuple_like` type.
 * @tparam Ts Additional `tuple_like` types of the same size as `T`.
 * @param f Transform callable invoked on corresponding elements.
 * @param op Binary reduction operator combining transformed results.
 * @param t First source tuple-like object.
 * @param ts Additional source tuple-like objects.
 * @return Reduced value.
 */
template <class F, class Op, non_empty_tuple_like T, tuple_like ... Ts>
    requires (same_size_tuples<T, Ts...>)
raze_nodiscard constexpr decltype(auto) map_reduce(F&& f, Op&& op, T&& t, Ts&& ... ts) {
    return __tree_map_reduce<0, size_v<T>>(f, op, std::forward<T>(t), std::forward<Ts>(ts)...);
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Computes the balanced tree sum (`+`) of all elements in a non-empty tuple-like object.
 * @tparam T A `non_empty_tuple_like` type.
 * @param t Source tuple-like object.
 * @return Sum of elements.
 */
template <non_empty_tuple_like T>
raze_nodiscard constexpr auto sum(T&& t) {
    return meta::reduce([] (auto&& a, auto&& b) {
        return std::forward<decltype(a)>(a) + std::forward<decltype(b)>(b);
    }, std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Computes the balanced tree sum (`+`) of `init` and all elements in `t`.
 * @tparam T A `tuple_like` type.
 * @tparam Init Initial value type.
 * @param t Source tuple-like object.
 * @param init Initial additive value.
 * @return Sum of `init` and all elements of `t`.
 */
template <tuple_like T, class Init>
raze_nodiscard constexpr auto sum(T&& t, Init&& init) {
    return meta::reduce([] (auto&& a, auto&& b) {
        return std::forward<decltype(a)>(a) + std::forward<decltype(b)>(b);
    }, std::forward<T>(t), std::forward<Init>(init));
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Computes the balanced tree product (`*`) of all elements in a non-empty tuple-like object.
 * @tparam T A `non_empty_tuple_like` type.
 * @param t Source tuple-like object.
 * @return Product of elements.
 */
template <non_empty_tuple_like T>
raze_nodiscard constexpr auto prod(T&& t) {
    return meta::reduce([] (auto&& a, auto&& b) {
        return std::forward<decltype(a)>(a) * std::forward<decltype(b)>(b);
    }, std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Computes the balanced tree product (`*`) of `init` and all elements in `t`.
 * @tparam T A `tuple_like` type.
 * @tparam Init Initial value type.
 * @param t Source tuple-like object.
 * @param init Initial multiplicative value.
 * @return Product of `init` and all elements of `t`.
 */
template <tuple_like T, class Init>
raze_nodiscard constexpr auto prod(T&& t, Init&& init) {
    return meta::reduce([] (auto&& a, auto&& b) {
        return std::forward<decltype(a)>(a) * std::forward<decltype(b)>(b);
    }, std::forward<T>(t), std::forward<Init>(init));
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Computes the balanced tree bitwise AND (`&`) of all elements in `t`.
 * @tparam T A `non_empty_tuple_like` type.
 * @param t Source tuple-like object.
 * @return Bitwise AND reduction of all elements.
 */
template <non_empty_tuple_like T>
raze_nodiscard constexpr auto bit_and(T&& t) {
    return meta::reduce([] (auto&& a, auto&& b) {
        return std::forward<decltype(a)>(a) & std::forward<decltype(b)>(b);
    }, std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Computes the balanced tree bitwise OR (`|`) of all elements in `t`.
 * @tparam T A `non_empty_tuple_like` type.
 * @param t Source tuple-like object.
 * @return Bitwise OR reduction of all elements.
 */
template <non_empty_tuple_like T>
raze_nodiscard constexpr auto bit_or(T&& t) {
    return meta::reduce([] (auto&& a, auto&& b) {
        return std::forward<decltype(a)>(a) | std::forward<decltype(b)>(b);
    }, std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Computes the balanced tree bitwise XOR (`^`) of all elements in `t`.
 * @tparam T A `non_empty_tuple_like` type.
 * @param t Source tuple-like object.
 * @return Bitwise XOR reduction of all elements.
 */
template <non_empty_tuple_like T>
raze_nodiscard constexpr auto bit_xor(T&& t) {
    return meta::reduce([] (auto&& a, auto&& b) {
        return std::forward<decltype(a)>(a) ^ std::forward<decltype(b)>(b);
    }, std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Returns the minimum element of a non-empty tuple-like object using `<`.
 * @tparam T A `non_empty_tuple_like` type.
 * @param t Source tuple-like object.
 * @return Minimum value across all elements.
 */
template <non_empty_tuple_like T>
raze_nodiscard constexpr auto min(T&& t) {
    return meta::reduce([] (const auto& a, const auto& b) {
        return b < a ? b : a;
    }, std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Returns the minimum projected value `proj(elem)` across all elements of `t`.
 * @tparam T A `non_empty_tuple_like` type.
 * @tparam Proj Unary projection callable.
 * @param t Source tuple-like object.
 * @param proj Projection applied to each element before comparing with `<`.
 * @return Minimum projected value.
 */
template <non_empty_tuple_like T, class Proj>
raze_nodiscard constexpr auto min(T&& t, Proj&& proj) {
    return meta::map_reduce(std::forward<Proj>(proj), [] (const auto& a, const auto& b) {
        return b < a ? b : a;
    }, std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Returns the maximum element of a non-empty tuple-like object using `<`.
 * @tparam T A `non_empty_tuple_like` type.
 * @param t Source tuple-like object.
 * @return Maximum value across all elements.
 */
template <non_empty_tuple_like T>
raze_nodiscard constexpr auto max(T&& t) {
    return meta::reduce([] (const auto& a, const auto& b) {
        return a < b ? b : a;
    }, std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Returns the maximum projected value `proj(elem)` across all elements of `t`.
 * @tparam T A `non_empty_tuple_like` type.
 * @tparam Proj Unary projection callable.
 * @param t Source tuple-like object.
 * @param proj Projection applied to each element before comparing with `<`.
 * @return Maximum projected value.
 */
template <non_empty_tuple_like T, class Proj>
raze_nodiscard constexpr auto max(T&& t, Proj&& proj) {
    return meta::map_reduce(std::forward<Proj>(proj), [] (const auto& a, const auto& b) {
        return a < b ? b : a;
    }, std::forward<T>(t));
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Generalized inner product combining `prod_op(t1[i], t2[i])` via a balanced tree using `sum_op` and initial value `init`.
 * @tparam T1 First `tuple_like` type.
 * @tparam T2 Second `tuple_like` type (`size_v<T1> == size_v<T2>`).
 * @tparam Init Initial accumulator type.
 * @tparam SumOp Binary reduction operator.
 * @tparam ProdOp Binary element-combining operator.
 * @param t1 First tuple-like operand.
 * @param t2 Second tuple-like operand.
 * @param init Initial value.
 * @param sum_op Reduction operator.
 * @param prod_op Pairwise combination operator.
 * @return Generalized inner product result.
 */
template <tuple_like T1, tuple_like T2, class Init, class SumOp, class ProdOp>
    requires (size_v<T1> == size_v<T2>)
raze_nodiscard constexpr auto inner_product(T1&& t1, T2&& t2, Init&& init, SumOp&& sum_op, ProdOp&& prod_op) {
    if constexpr (size_v<T1> == 0)
        return std::forward<Init>(init);
    else
        return __invoke(sum_op, std::forward<Init>(init),
            meta::map_reduce(prod_op, sum_op, std::forward<T1>(t1), std::forward<T2>(t2)));
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Computes the standard dot product `init + sum_i(t1[i] * t2[i])` using balanced tree reduction.
 * @tparam T1 First `tuple_like` type.
 * @tparam T2 Second `tuple_like` type (`size_v<T1> == size_v<T2>`).
 * @tparam Init Initial accumulator type.
 * @param t1 First tuple-like operand.
 * @param t2 Second tuple-like operand.
 * @param init Initial additive value.
 * @return Inner product result.
 */
template <tuple_like T1, tuple_like T2, class Init>
    requires (size_v<T1> == size_v<T2>)
raze_nodiscard constexpr auto inner_product(T1&& t1, T2&& t2, Init&& init) {
    return meta::inner_product(
        std::forward<T1>(t1), std::forward<T2>(t2), std::forward<Init>(init),
        [] (auto&& a, auto&& b) { return std::forward<decltype(a)>(a) + std::forward<decltype(b)>(b); },
        [] (auto&& a, auto&& b) { return std::forward<decltype(a)>(a) * std::forward<decltype(b)>(b); });
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Computes the left-to-right inclusive prefix scan of `t` under `op`.
 *
 * For `t = (a, b, c)`, produces `(a, op(a, b), op(op(a, b), c))`. Returns `tuple<>{}` for empty tuples.
 *
 * @tparam Op Binary scan operator.
 * @tparam T A `tuple_like` type.
 * @param op Binary scan operator.
 * @param t Source tuple-like object.
 * @return `tuple` of inclusive prefix accumulations.
 */
template <class Op, tuple_like T>
raze_nodiscard constexpr auto inclusive_scan_left(Op&& op, T&& t) {
    if constexpr (size_v<T> == 0) {
        return tuple<>{};
    }
    else {
        auto first = __adl_get<0>(std::forward<T>(t));
        return __inc_scan_l<1>(op, std::forward<T>(t), std::move(first));
    }
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Computes the left-to-right inclusive prefix scan of `t` seeded with `init`.
 *
 * For `t = (a, b)`, produces `(op(init, a), op(op(init, a), b))`. Returns `tuple<>{}` for empty tuples.
 *
 * @tparam Op Binary scan operator.
 * @tparam T A `tuple_like` type.
 * @tparam Init Initial seed type.
 * @param op Binary scan operator.
 * @param t Source tuple-like object.
 * @param init Initial seed value.
 * @return `tuple` of seeded inclusive prefix accumulations.
 */
template <class Op, tuple_like T, class Init>
raze_nodiscard constexpr auto inclusive_scan_left(Op&& op, T&& t, Init&& init) {
    if constexpr (size_v<T> == 0) {
        return tuple<>{};
    }
    else {
        auto first = __invoke(op, std::forward<Init>(init), __adl_get<0>(std::forward<T>(t)));
        return __inc_scan_l<1>(op, std::forward<T>(t), std::move(first));
    }
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Computes the left-to-right exclusive prefix scan of `t` seeded with `init`.
 *
 * For `t = (a, b, c)`, produces `(init, op(init, a), op(op(init, a), b))`. Returns `tuple<>{}` for empty tuples.
 *
 * @tparam Op Binary scan operator.
 * @tparam T A `tuple_like` type.
 * @tparam Init Initial seed type.
 * @param op Binary scan operator.
 * @param t Source tuple-like object.
 * @param init Initial seed value.
 * @return `tuple` of exclusive prefix accumulations.
 */
template <class Op, tuple_like T, class Init>
raze_nodiscard constexpr auto exclusive_scan_left(Op&& op, T&& t, Init&& init) {
    if constexpr (size_v<T> == 0)
        return tuple<>{};
    else
        return __exc_scan_l<0>(op, std::forward<T>(t), std::forward<Init>(init));
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Computes the right-to-left inclusive suffix scan of `t` under `op`.
 *
 * For `t = (a, b, c)`, produces `(op(a, op(b, c)), op(b, c), c)`. Returns `tuple<>{}` for empty tuples.
 *
 * @tparam Op Binary scan operator.
 * @tparam T A `tuple_like` type.
 * @param op Binary scan operator.
 * @param t Source tuple-like object.
 * @return `tuple` of inclusive suffix accumulations.
 */
template <class Op, tuple_like T>
raze_nodiscard constexpr auto inclusive_scan_right(Op&& op, T&& t) {
    constexpr sizetype n = size_v<T>;

    if constexpr (n == 0) {
        return tuple<>{};
    }
    else {
        auto last = __adl_get<n - 1>(std::forward<T>(t));
        return __inc_scan_r<n - 1>(op, std::forward<T>(t), std::move(last));
    }
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Computes the right-to-left inclusive suffix scan of `t` seeded with `init`.
 *
 * For `t = (a, b)`, produces `(op(a, op(b, init)), op(b, init))`. Returns `tuple<>{}` for empty tuples.
 *
 * @tparam Op Binary scan operator.
 * @tparam T A `tuple_like` type.
 * @tparam Init Initial seed type.
 * @param op Binary scan operator.
 * @param t Source tuple-like object.
 * @param init Initial seed value.
 * @return `tuple` of seeded inclusive suffix accumulations.
 */
template <class Op, tuple_like T, class Init>
raze_nodiscard constexpr auto inclusive_scan_right(Op&& op, T&& t, Init&& init) {
    constexpr sizetype n = size_v<T>;

    if constexpr (n == 0) {
        return tuple<>{};
    }
    else {
        auto last = __invoke(op, __adl_get<n - 1>(std::forward<T>(t)), std::forward<Init>(init));
        return __inc_scan_r<n - 1>(op, std::forward<T>(t), std::move(last));
    }
}

/**
 * @ingroup raze_meta_tuple_reduce
 * @brief Computes the right-to-left exclusive suffix scan of `t` seeded with `init`.
 *
 * For `t = (a, b, c)`, produces `(op(b, op(c, init)), op(c, init), init)`. Returns `tuple<>{}` for empty tuples.
 *
 * @tparam Op Binary scan operator.
 * @tparam T A `tuple_like` type.
 * @tparam Init Initial seed type.
 * @param op Binary scan operator.
 * @param t Source tuple-like object.
 * @param init Initial seed value.
 * @return `tuple` of exclusive suffix accumulations.
 */
template <class Op, tuple_like T, class Init>
raze_nodiscard constexpr auto exclusive_scan_right(Op&& op, T&& t, Init&& init) {
    constexpr sizetype n = size_v<T>;

    if constexpr (n == 0)
        return tuple<>{};
    else
        return __exc_scan_r<n - 1>(op, std::forward<T>(t), std::forward<Init>(init));
}

__RAZE_META_NAMESPACE_END
