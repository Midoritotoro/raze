#pragma once 

#include <src/raze/vx/storage/Size.h>


__RAZE_VX_NAMESPACE_BEGIN

template <std::size_t Idx, class H, class T>
constexpr auto& get(simd_tuple_node<H, T>& node) noexcept {
    if constexpr (Idx == 0) return node._head;
    else return get<Idx - 1>(node._tail);
}

template <sizetype Idx, class H, class T>
constexpr const auto& get(const simd_tuple_node<H, T>& node) noexcept {
    if constexpr (Idx == 0)  return node._head;
    else return get<Idx - 1>(node._tail);
}

template <sizetype Idx, class T>
constexpr const auto& get(const T& v) noexcept {
    return v;
}

template <sizetype Idx, class T>
constexpr auto& get(T& v) noexcept {
    return v;
}

template <class T> struct is_std_tuple : std::false_type {};
template <class... Ts> struct is_std_tuple<std::tuple<Ts...>> : std::true_type {};
template <class T> inline constexpr bool is_std_tuple_v = is_std_tuple<std::remove_cvref_t<T>>::value;

template <class Chunk, class Reg>
raze_always_inline void assign_chunk(Chunk& slot, Reg&& reg) noexcept {
    auto& s = ustorage(slot);
    using S = std::remove_cvref_t<decltype(s)>;
    if constexpr (intrin_type<S>) s = as<S>(std::forward<Reg>(reg));
    else s = std::forward<Reg>(reg);
}

template <sizetype Offset, class Dst, class Reg>
raze_always_inline void assign_regs(Dst& dst, Reg&& reg) noexcept
    requires (!is_std_tuple_v<std::remove_cvref_t<Reg>>)
{
    assign_chunk(dst.template __get<Offset>(), std::forward<Reg>(reg));
}

template <sizetype Offset, class Dst, class Tuple>
raze_always_inline void assign_regs(Dst& dst, Tuple&& regs) noexcept
    requires(is_std_tuple_v<std::remove_cvref_t<Tuple>>)
{
    using T = std::remove_cvref_t<Tuple>;
    [&] <sizetype... Ks> (std::index_sequence<Ks...>) raze_always_inline_lambda {
        (assign_chunk(dst.template __get<Offset + Ks>(), std::get<Ks>(std::forward<Tuple>(regs))), ...);
    }(std::make_index_sequence<std::tuple_size_v<T>>{});
}

template <class Tuple, class F>
raze_always_inline void visit_chunk_by_index(Tuple& tuple, sizetype i, F&& f) noexcept
    requires(is_simd_tuple<std::remove_cvref_t<Tuple>>::value)
{
    [&] <std::size_t... I> (std::index_sequence<I...>) raze_always_inline_lambda {
        auto current = i;

        ([&] () raze_always_inline_lambda {
            using Chunk = std::decay_t<decltype(get<I>(tuple))>;

            if (current < Chunk::size) {
                f(get<I>(tuple), current);
                return true;
            }

            current -= Chunk::size;
            return false;
        }() || ... );
    }(std::make_index_sequence<simd_tuple_size<std::remove_cvref_t<Tuple>>::value>{});
}

struct chunk_visitor {
    template <std::size_t Idx, sizetype CurrentI, class Tuple, class F>
    static raze_always_inline void visit(Tuple& t, F&& f) noexcept {
        using TupleType = std::remove_cvref_t<Tuple>;
        constexpr std::size_t size = simd_tuple_size<TupleType>::value;

        if constexpr (Idx < size) {
            using Chunk = std::decay_t<decltype(get<Idx>(t))>;

            if constexpr (CurrentI < Chunk::size) f(get<Idx>(t), std::integral_constant<sizetype, CurrentI>{});
            else visit<Idx + 1, CurrentI - Chunk::size>(t, std::forward<F>(f));
        }
    }
};

template <class Tuple, sizetype I, class F>
raze_always_inline void visit_chunk_by_index(Tuple& tuple, 
    std::integral_constant<sizetype, I> i, F&& f) noexcept 
{
    chunk_visitor::template visit<0, i>(tuple, std::forward<F>(f));
}

template <class Tuple, class F, class ... Args>
raze_always_inline void for_each_tuple(Tuple& tuple, F&& f, Args&& ... args) noexcept
    requires(is_simd_tuple<std::remove_cvref_t<Tuple>>::value)
{
    [&] <sizetype ... I> (std::integer_sequence<sizetype, I...>) raze_always_inline_lambda {
        ([&] (auto current) raze_always_inline_lambda {
            f(get<current>(tuple), get<current>(std::forward<Args>(args))...);
        }(std::integral_constant<std::size_t, I>{}), ...);
    }(std::make_integer_sequence<sizetype, simd_tuple_size<std::remove_cvref_t<Tuple>>::value>{});
}

template <class Tuple, class F, class ... Args>
raze_always_inline bool for_each_tuple_all_of(Tuple& tuple, F&& f, Args&& ... args) noexcept
    requires(is_simd_tuple<std::remove_cvref_t<Tuple>>::value)
{
    return [&] <sizetype ... I> (std::integer_sequence<sizetype, I...>) raze_always_inline_lambda {
        return ([&] (auto current) raze_always_inline_lambda {
            return f(get<current>(tuple), get<current>(std::forward<Args>(args))...);
        }(std::integral_constant<std::size_t, I>{}) && ...);
    }(std::make_integer_sequence<sizetype, simd_tuple_size<std::remove_cvref_t<Tuple>>::value>{});
}

template <class Tuple, class F, class ... Args>
raze_always_inline bool for_each_tuple_any_of(Tuple& tuple, F&& f, Args&& ... args) noexcept
    requires(is_simd_tuple<std::remove_cvref_t<Tuple>>::value)
{
    return [&] <sizetype ... I> (std::integer_sequence<sizetype, I...>) raze_always_inline_lambda {
        return ([&] (auto current) raze_always_inline_lambda {
            return f(get<current>(tuple), get<current>(std::forward<Args>(args))...);
        }(std::integral_constant<std::size_t, I>{}) || ...);
    }(std::make_integer_sequence<sizetype, simd_tuple_size<std::remove_cvref_t<Tuple>>::value>{});
}

template <class Tuple, class F, class ... Args>
raze_always_inline void for_each_tuple_reverse(Tuple& tuple, F&& f, Args&& ... args) noexcept
    requires(is_simd_tuple<std::remove_cvref_t<Tuple>>::value)
{
    constexpr sizetype N = simd_tuple_size<std::remove_cvref_t<Tuple>>::value;
    [&] <sizetype ... I> (std::integer_sequence<sizetype, I...>) raze_always_inline_lambda {
        ([&] (auto current) raze_always_inline_lambda {
            f(get<current>(tuple), get<current>(std::forward<Args>(args))...);
        }(std::integral_constant<std::size_t, N - 1 - I>{}), ...);
    }(std::make_integer_sequence<sizetype, N>{});
}

template <class Tuple, class F, class ... Args>
raze_always_inline bool for_each_tuple_all_of_reverse(Tuple& tuple, F&& f, Args&& ... args) noexcept
    requires(is_simd_tuple<std::remove_cvref_t<Tuple>>::value)
{
    constexpr sizetype N = simd_tuple_size<std::remove_cvref_t<Tuple>>::value;
    return [&] <sizetype ... I> (std::integer_sequence<sizetype, I...>) raze_always_inline_lambda {
        return ([&] (auto current) raze_always_inline_lambda {
            return f(get<current>(tuple), get<current>(std::forward<Args>(args))...);
        }(std::integral_constant<std::size_t, N - 1 - I>{}) && ...);
    }(std::make_integer_sequence<sizetype, N>{});
}

template <class Tuple, class F, class ... Args>
raze_always_inline bool for_each_tuple_any_of_reverse(Tuple& tuple, F&& f, Args&& ... args) noexcept
    requires(is_simd_tuple<std::remove_cvref_t<Tuple>>::value)
{
    constexpr sizetype N = simd_tuple_size<std::remove_cvref_t<Tuple>>::value;
    return [&] <sizetype ... I> (std::integer_sequence<sizetype, I...>) raze_always_inline_lambda {
        return ([&] (auto current) raze_always_inline_lambda {
            return f(get<current>(tuple), get<current>(std::forward<Args>(args))...);
        }(std::integral_constant<std::size_t, N - 1 - I>{}) || ...);
    }(std::make_integer_sequence<sizetype, N>{});
}


__RAZE_VX_NAMESPACE_END
