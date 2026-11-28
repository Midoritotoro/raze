#pragma once

#include <src/raze/meta/tuple/Storage.h>


__RAZE_META_NAMESPACE_BEGIN

/**
 * @ingroup raze_meta_tuple
 * @struct tuple raze/meta/Tuple.h <raze/meta/Tuple.h>
 * @brief Zero-overhead, ABI-friendly aggregate tuple with flat storage and full C++23 protocol support.
 *
 * `raze::meta::tuple` is a lightweight aggregate alternative to `std::tuple` designed for
 * register-friendly ABI passing (preserving Homogeneous Vector Aggregates / HVA on x86-64
 * and ARM64 without `#pragma pack`), fast compile times, and expressive compile-time indexing.
 *
 * ### Key Characteristics
 * - **Aggregate & triviality:** Non-reference tuples of 1..16 elements use flat fields without
 *   inheritance (`std::is_aggregate_v` is always `true`), preserving trivial copyability,
 *   standard layout (when element types allow), and native SIMD register passing.
 * - **Empty-element optimization:** Automatically applies `[[no_unique_address]]` only when
 *   empty element types are present, avoiding unnecessary ABI differences on dense numeric tuples.
 * - **Reference & proxy assignment:** Tuples holding lvalue/rvalue references (`tie`, `forward_as_tuple`,
 *   `*_ref` views) propagate assignments to their referents and support C++23 `const`-qualified
 *   proxy assignment (`const tuple<Ts...>& operator=`).
 * - **Standard protocol interoperability:** Specializes `std::tuple_size`, `std::tuple_element`,
 *   `std::common_type`, and `std::basic_common_reference`, and works seamlessly with structured
 *   bindings and ADL `get<I>` / `get<T>`.
 *
 * ### Example
 * ```cpp
 * #include <raze/meta/Tuple.h>
 *
 * using namespace raze::meta;
 *
 * constexpr auto t = make_tuple(42, 3.5, 'a');
 * static_assert(t[0_c] == 42);
 * static_assert(t[type_v<char>] == 'a');
 * auto [i, d, c] = t;
 * ```
 *
 * @tparam Ts Element types stored in the tuple.
 */
template <class ... Ts>
struct tuple {
    /** @brief Underlying storage backend type selected for `Ts...`. */
    using storage_type = __storage_t<Ts...>;

    /** @brief Number of elements in the tuple (`sizeof...(Ts)`). */
    static constexpr sizetype tuple_size_v = sizeof...(Ts);
    /** @brief `true` if any element type in `Ts...` is an lvalue or rvalue reference. */
    static constexpr bool has_references = !__no_refs_v<Ts...>;
    /** @brief `true` if the tuple is non-empty and all element types are identical. */
    static constexpr bool is_homogeneous = (sizeof...(Ts) >= 1) && __all_same_v<Ts...>;

    /// @cond
    storage_type _impl;
    /// @endcond

    /**
     * @brief Returns the number of elements in the tuple.
     * @return `sizeof...(Ts)`.
     */
    raze_nodiscard static raze_always_inline constexpr sizetype size() noexcept {
        return sizeof...(Ts);
    }

    /**
     * @brief Checks whether the tuple contains zero elements.
     * @return `false` for non-empty tuples (`true` for the `tuple<>` specialization).
     */
    raze_nodiscard static raze_always_inline constexpr bool empty() noexcept {
        return false;
    }

    /**
     * @brief Accesses the element at compile-time index `I`, preserving value category and cv-qualifiers.
     *
     * Follows exact `std::get<I>` reference-collapsing semantics for value, lvalue-reference,
     * and rvalue-reference members.
     *
     * @tparam I Zero-based element index (`I < sizeof...(Ts)`).
     * @return Reference to the `I`-th element.
     */
    template <sizetype I, class Self>
        requires (I < sizeof...(Ts))
    raze_nodiscard raze_always_inline constexpr decltype(auto) get(this Self&& self) noexcept {
        return std::forward<Self>(self)._impl.template get<I>();
    }

    /**
     * @brief Accesses the unique element of type `T`, preserving value category and cv-qualifiers.
     *
     * Participates in overload resolution only when `T` appears exactly once in `Ts...`.
     *
     * @tparam T Unique element type to retrieve.
     * @return Reference to the element of type `T`.
     */
    template <class T, class Self>
        requires (__index_of_unique_type<T, Ts...>() < sizeof...(Ts))
    raze_nodiscard raze_always_inline constexpr decltype(auto) get(this Self&& self) noexcept {
        constexpr sizetype idx = __index_of_unique_type<T, Ts...>();
        return std::forward<Self>(self)._impl.template get<idx>();
    }

    /**
     * @brief Subscripts the tuple with a compile-time `index_t<I>` tag (e.g., `t[0_c]` or `t[index<0>]`).
     * @tparam I Zero-based element index (`I < sizeof...(Ts)`).
     * @return Reference to the `I`-th element with preserved value category.
     */
    template <sizetype I, class Self>
        requires (I < sizeof...(Ts))
    raze_nodiscard raze_always_inline constexpr decltype(auto) operator[](this Self&& self, index_t<I>) noexcept {
        return std::forward<Self>(self)._impl.template get<I>();
    }

    /**
     * @brief Subscripts the tuple with a `std::integral_constant<sizetype, I>` tag.
     * @tparam I Zero-based element index (`I < sizeof...(Ts)`).
     * @return Reference to the `I`-th element with preserved value category.
     */
    template <sizetype I, class Self>
        requires (I < sizeof...(Ts))
    raze_nodiscard raze_always_inline constexpr decltype(auto) operator[](
        this Self&& self, std::integral_constant<sizetype, I>) noexcept
    {
        return std::forward<Self>(self)._impl.template get<I>();
    }

    /**
     * @brief Subscripts the tuple by unique element type using a `type_t<T>` tag (e.g., `t[type_v<int>]`).
     * @tparam T Unique element type present in `Ts...`.
     * @return Reference to the element of type `T` with preserved value category.
     */
    template <class T, class Self>
        requires (__index_of_unique_type<T, Ts...>() < sizeof...(Ts))
    raze_nodiscard raze_always_inline constexpr decltype(auto) operator[](this Self&& self, type_t<T>) noexcept {
        constexpr sizetype idx = __index_of_unique_type<T, Ts...>();
        return std::forward<Self>(self)._impl.template get<idx>();
    }

    /**
     * @brief Element-wise converting copy assignment from a heterogeneous tuple of equal arity.
     * @tparam Us Element types of `rhs` (`sizeof...(Us) == sizeof...(Ts)`).
     * @param rhs Source tuple whose elements are assigned into `*this`.
     * @return `*this`.
     */
    template <class ... Us>
        requires (sizeof...(Us) == sizeof...(Ts))
              && (!std::same_as<tuple, tuple<Us...>>)
              && (std::is_assignable_v<Ts&, const Us&> && ...)
    constexpr tuple& operator=(const tuple<Us...>& rhs)
        noexcept((std::is_nothrow_assignable_v<Ts&, const Us&> && ...))
    {
        [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            ((this->template get<Is>() = rhs.template get<Is>()), ...);
        }(std::index_sequence_for<Ts...>{});

        return *this;
    }

    /**
     * @brief Element-wise converting move assignment from a heterogeneous tuple of equal arity.
     * @tparam Us Element types of `rhs` (`sizeof...(Us) == sizeof...(Ts)`).
     * @param rhs Source rvalue tuple whose elements are move-assigned into `*this`.
     * @return `*this`.
     */
    template <class ... Us>
        requires (sizeof...(Us) == sizeof...(Ts))
              && (!std::same_as<tuple, tuple<Us...>>)
              && (std::is_assignable_v<Ts&, Us&&> && ...)
    constexpr tuple& operator=(tuple<Us...>&& rhs)
        noexcept((std::is_nothrow_assignable_v<Ts&, Us&&> && ...))
    {
        [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            ((this->template get<Is>() = std::move(rhs).template get<Is>()), ...);
        }(std::index_sequence_for<Ts...>{});

        return *this;
    }

    /**
     * @brief C++23 `const`-qualified proxy copy assignment (e.g., assigning through `const tuple<int&, double&>`).
     * @tparam Us Element types of `rhs` (`sizeof...(Us) == sizeof...(Ts)`).
     * @param rhs Source tuple whose elements are assigned into the const-assignable elements of `*this`.
     * @return `*this`.
     */
    template <class ... Us>
        requires (sizeof...(Us) == sizeof...(Ts))
              && (std::is_assignable_v<const Ts&, const Us&> && ...)
    constexpr const tuple& operator=(const tuple<Us...>& rhs) const
        noexcept((std::is_nothrow_assignable_v<const Ts&, const Us&> && ...))
    {
        [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            ((this->template get<Is>() = rhs.template get<Is>()), ...);
        }(std::index_sequence_for<Ts...>{});

        return *this;
    }

    /**
     * @brief C++23 `const`-qualified proxy move assignment.
     * @tparam Us Element types of `rhs` (`sizeof...(Us) == sizeof...(Ts)`).
     * @param rhs Source rvalue tuple whose elements are move-assigned into `*this`.
     * @return `*this`.
     */
    template <class ... Us>
        requires (sizeof...(Us) == sizeof...(Ts))
              && (std::is_assignable_v<const Ts&, Us&&> && ...)
    constexpr const tuple& operator=(tuple<Us...>&& rhs) const
        noexcept((std::is_nothrow_assignable_v<const Ts&, Us&&> && ...))
    {
        [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            ((this->template get<Is>() = std::move(rhs).template get<Is>()), ...);
        }(std::index_sequence_for<Ts...>{});

        return *this;
    }

    /**
     * @brief Element-wise converting copy conversion to `tuple<Us...>`.
     *
     * Conditionally `explicit` if any `const Ts&` is not implicitly convertible to its corresponding `Us`.
     *
     * @tparam Us Target element types (`sizeof...(Us) == sizeof...(Ts)`).
     * @return Converted `tuple<Us...>`.
     */
    template <class ... Us>
        requires (sizeof...(Us) == sizeof...(Ts))
              && (!std::same_as<tuple, tuple<Us...>>)
              && (std::is_constructible_v<Us, const Ts&> && ...)
#if !defined(RAZE_DOXYGEN_INVOKED)
    raze_nodiscard explicit(!(std::is_convertible_v<const Ts&, Us> && ...))
#endif
    constexpr operator tuple<Us...>() const&
        noexcept((std::is_nothrow_constructible_v<Us, const Ts&> && ...))
    {
        return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            return tuple<Us...>{ __make_storage<Us...>(static_cast<Us>(this->template get<Is>())...) };
        }(std::index_sequence_for<Ts...>{});
    }

    /**
     * @brief Element-wise converting move conversion to `tuple<Us...>`.
     *
     * Conditionally `explicit` if any `Ts&&` is not implicitly convertible to its corresponding `Us`.
     *
     * @tparam Us Target element types (`sizeof...(Us) == sizeof...(Ts)`).
     * @return Converted `tuple<Us...>`.
     */
    template <class ... Us>
        requires (sizeof...(Us) == sizeof...(Ts))
              && (!std::same_as<tuple, tuple<Us...>>)
              && (std::is_constructible_v<Us, Ts&&> && ...)
#if !defined(RAZE_DOXYGEN_INVOKED)
    raze_nodiscard explicit(!(std::is_convertible_v<Ts&&, Us> && ...))
#endif
    constexpr operator tuple<Us...>() &&
        noexcept((std::is_nothrow_constructible_v<Us, Ts&&> && ...))
    {
        return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            return tuple<Us...>{ __make_storage<Us...>(static_cast<Us>(std::move(*this).template get<Is>())...) };
        }(std::index_sequence_for<Ts...>{});
    }

    /**
     * @brief Swaps every element of `*this` with the corresponding element of `rhs` via ADL `swap`.
     * @param rhs Tuple to swap elements with.
     */
    constexpr void swap(tuple& rhs)
        noexcept((std::is_nothrow_swappable_v<Ts> && ...))
        requires (std::is_swappable_v<Ts> && ...)
    {
        using std::swap;

        [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            (swap(this->template get<Is>(), rhs.template get<Is>()), ...);
        }(std::index_sequence_for<Ts...>{});
    }

    /**
     * @brief C++23 `const`-qualified proxy swap for tuples of swappable references/proxies.
     * @param rhs Tuple to swap referents with.
     */
    constexpr void swap(const tuple& rhs) const
        noexcept((std::is_nothrow_swappable_v<const Ts> && ...))
        requires (std::is_swappable_v<const Ts> && ...)
    {
        using std::swap;

        [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            (swap(this->template get<Is>(), rhs.template get<Is>()), ...);
        }(std::index_sequence_for<Ts...>{});
    }

    /**
     * @brief ADL-discoverable swap for two `tuple` instances.
     * @param a First tuple.
     * @param b Second tuple.
     */
    friend constexpr void swap(tuple& a, tuple& b)
        noexcept(noexcept(a.swap(b)))
        requires (std::is_swappable_v<Ts> && ...)
    {
        a.swap(b);
    }

    /**
     * @brief ADL-discoverable `const`-qualified proxy swap for two `tuple` instances.
     * @param a First tuple.
     * @param b Second tuple.
     */
    friend constexpr void swap(const tuple& a, const tuple& b)
        noexcept(noexcept(a.swap(b)))
        requires (std::is_swappable_v<const Ts> && ...)
    {
        a.swap(b);
    }
};

/**
 * @ingroup raze_meta_tuple
 * @brief Empty specialization of `raze::meta::tuple` containing zero elements.
 */
template <>
struct tuple<> {
    /** @brief Always `0`. */
    static constexpr sizetype tuple_size_v = 0;
    /** @brief Always `false`. */
    static constexpr bool has_references = false;
    /** @brief Always `false`. */
    static constexpr bool is_homogeneous = false;

    /**
     * @brief Returns `0`.
     * @return `0`.
     */
    raze_nodiscard static raze_always_inline constexpr sizetype size() noexcept {
        return 0;
    }

    /**
     * @brief Returns `true`.
     * @return `true`.
     */
    raze_nodiscard static raze_always_inline constexpr bool empty() noexcept {
        return true;
    }

    /** @brief No-op swap for empty tuples. */
    raze_always_inline constexpr void swap(tuple&) noexcept {}
    /** @brief No-op `const`-qualified swap for empty tuples. */
    raze_always_inline constexpr void swap(const tuple&) const noexcept {}

    /** @brief ADL no-op swap for empty tuples. */
    friend raze_always_inline constexpr void swap(tuple&, tuple&) noexcept {}
    /** @brief ADL `const`-qualified no-op swap for empty tuples. */
    friend raze_always_inline constexpr void swap(const tuple&, const tuple&) noexcept {}
};

template <class ... Ts>
tuple(Ts...) -> tuple<std::unwrap_ref_decay_t<Ts>...>;

/**
 * @ingroup raze_meta_tuple
 * @brief Lexicographical element-wise equality comparison between two tuples of equal size.
 *
 * Participates in overload resolution only when `sizeof...(Ts) == sizeof...(Us)` and every pair
 * `(a == b)` is convertible to `bool` (safely SFINAE-ing out types such as raw SIMD vectors whose
 * `operator==` returns a vector mask rather than a boolean).
 *
 * @tparam Ts Element types of `lhs`.
 * @tparam Us Element types of `rhs`.
 * @param lhs Left-hand tuple.
 * @param rhs Right-hand tuple.
 * @return `true` if all corresponding elements compare equal (always `true` for empty tuples).
 */
template <class ... Ts, class ... Us>
    requires (sizeof...(Ts) == sizeof...(Us))
          && (requires (const Ts& a, const Us& b) { { a == b } -> std::convertible_to<bool>; } && ...)
raze_nodiscard constexpr bool operator==(const tuple<Ts...>& lhs, const tuple<Us...>& rhs)
    noexcept((noexcept(static_cast<bool>(std::declval<const Ts&>() == std::declval<const Us&>())) && ...))
{
    if constexpr (sizeof...(Ts) == 0) {
        return true;
    }
    else {
        return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            return (static_cast<bool>(lhs.template get<Is>() == rhs.template get<Is>()) && ...);
        }(std::index_sequence_for<Ts...>{});
    }
}

/**
 * @ingroup raze_meta_tuple
 * @brief Lexicographical synthesized three-way comparison between two tuples of equal size.
 *
 * Uses `<=>` when available and falls back to `<`-based `std::weak_ordering` synthesis for
 * legacy comparable types, short-circuiting on the first non-equivalent element.
 *
 * @tparam Ts Element types of `lhs`.
 * @tparam Us Element types of `rhs`.
 * @param lhs Left-hand tuple.
 * @param rhs Right-hand tuple.
 * @return The common comparison category ordering (`std::strong_ordering::equal` for empty tuples).
 */
template <class ... Ts, class ... Us>
    requires (sizeof...(Ts) == sizeof...(Us))
          && (requires (const Ts& a, const Us& b) { __synth_three_way(a, b); } && ...)
raze_nodiscard constexpr auto operator<=>(const tuple<Ts...>& lhs, const tuple<Us...>& rhs)
    noexcept((noexcept(__synth_three_way(std::declval<const Ts&>(), std::declval<const Us&>())) && ...))
{
    if constexpr (sizeof...(Ts) == 0) {
        return std::strong_ordering::equal;
    }
    else {
        using cat_t = std::common_comparison_category_t<
            decltype(__synth_three_way(std::declval<const Ts&>(), std::declval<const Us&>()))...>;

        cat_t res = std::strong_ordering::equal;

        [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            static_cast<void>((([&] () raze_always_inline_lambda {
                if (auto c = __synth_three_way(lhs.template get<Is>(), rhs.template get<Is>()); c != 0) {
                    res = c;
                    return true;
                }

                return false;
            }()) || ...));
        }(std::index_sequence_for<Ts...>{});

        return res;
    }
}

/**
 * @ingroup raze_meta_tuple
 * @brief ADL-discoverable indexed element accessor for mutable lvalue `tuple`.
 * @tparam I Zero-based element index (`I < sizeof...(Ts)`).
 * @tparam Ts Element types of `t`.
 * @param t Source tuple.
 * @return Reference to the `I`-th element of `t`.
 */
template <sizetype I, class ... Ts>
    requires (I < sizeof...(Ts))
raze_nodiscard raze_always_inline constexpr decltype(auto) get(tuple<Ts...>& t) noexcept {
    return t.template get<I>();
}

/**
 * @ingroup raze_meta_tuple
 * @brief ADL-discoverable indexed element accessor for `const` lvalue `tuple`.
 * @tparam I Zero-based element index (`I < sizeof...(Ts)`).
 * @tparam Ts Element types of `t`.
 * @param t Source tuple.
 * @return Const-qualified reference to the `I`-th element of `t`.
 */
template <sizetype I, class ... Ts>
    requires (I < sizeof...(Ts))
raze_nodiscard raze_always_inline constexpr decltype(auto) get(const tuple<Ts...>& t) noexcept {
    return t.template get<I>();
}

/**
 * @ingroup raze_meta_tuple
 * @brief ADL-discoverable indexed element accessor for mutable rvalue `tuple`.
 * @tparam I Zero-based element index (`I < sizeof...(Ts)`).
 * @tparam Ts Element types of `t`.
 * @param t Source rvalue tuple.
 * @return Rvalue/forwarded reference to the `I`-th element of `t`.
 */
template <sizetype I, class ... Ts>
    requires (I < sizeof...(Ts))
raze_nodiscard raze_always_inline constexpr decltype(auto) get(tuple<Ts...>&& t) noexcept {
    return std::move(t).template get<I>();
}

/**
 * @ingroup raze_meta_tuple
 * @brief ADL-discoverable indexed element accessor for `const` rvalue `tuple`.
 * @tparam I Zero-based element index (`I < sizeof...(Ts)`).
 * @tparam Ts Element types of `t`.
 * @param t Source const rvalue tuple.
 * @return Const rvalue/forwarded reference to the `I`-th element of `t`.
 */
template <sizetype I, class ... Ts>
    requires (I < sizeof...(Ts))
raze_nodiscard raze_always_inline constexpr decltype(auto) get(const tuple<Ts...>&& t) noexcept {
    return std::move(t).template get<I>();
}

/**
 * @ingroup raze_meta_tuple
 * @brief ADL-discoverable unique-type element accessor for mutable lvalue `tuple`.
 * @tparam T Unique element type present in `Ts...`.
 * @tparam Ts Element types of `t`.
 * @param t Source tuple.
 * @return Reference to the unique element of type `T`.
 */
template <class T, class ... Ts>
    requires (__index_of_unique_type<T, Ts...>() < sizeof...(Ts))
raze_nodiscard raze_always_inline constexpr decltype(auto) get(tuple<Ts...>& t) noexcept {
    return t.template get<T>();
}

/**
 * @ingroup raze_meta_tuple
 * @brief ADL-discoverable unique-type element accessor for `const` lvalue `tuple`.
 * @tparam T Unique element type present in `Ts...`.
 * @tparam Ts Element types of `t`.
 * @param t Source tuple.
 * @return Const-qualified reference to the unique element of type `T`.
 */
template <class T, class ... Ts>
    requires (__index_of_unique_type<T, Ts...>() < sizeof...(Ts))
raze_nodiscard raze_always_inline constexpr decltype(auto) get(const tuple<Ts...>& t) noexcept {
    return t.template get<T>();
}

/**
 * @ingroup raze_meta_tuple
 * @brief ADL-discoverable unique-type element accessor for mutable rvalue `tuple`.
 * @tparam T Unique element type present in `Ts...`.
 * @tparam Ts Element types of `t`.
 * @param t Source rvalue tuple.
 * @return Rvalue/forwarded reference to the unique element of type `T`.
 */
template <class T, class ... Ts>
    requires (__index_of_unique_type<T, Ts...>() < sizeof...(Ts))
raze_nodiscard raze_always_inline constexpr decltype(auto) get(tuple<Ts...>&& t) noexcept {
    return std::move(t).template get<T>();
}

/**
 * @ingroup raze_meta_tuple
 * @brief ADL-discoverable unique-type element accessor for `const` rvalue `tuple`.
 * @tparam T Unique element type present in `Ts...`.
 * @tparam Ts Element types of `t`.
 * @param t Source const rvalue tuple.
 * @return Const rvalue/forwarded reference to the unique element of type `T`.
 */
template <class T, class ... Ts>
    requires (__index_of_unique_type<T, Ts...>() < sizeof...(Ts))
raze_nodiscard raze_always_inline constexpr decltype(auto) get(const tuple<Ts...>&& t) noexcept {
    return std::move(t).template get<T>();
}

template <class T, class Seq>
struct __repeat_tuple_impl;

template <class T, sizetype ... Is>
struct __repeat_tuple_impl<T, std::index_sequence<Is...>> {
    using type = tuple<__repeat_elem_t<T, Is>...>;
};

/**
 * @ingroup raze_meta_tuple
 * @brief Type alias producing a homogeneous `tuple<T, ..., T>` with `N` copies of `T`.
 * @tparam T Element type to repeat.
 * @tparam N Number of elements.
 */
template <class T, sizetype N>
using repeat_tuple_t = typename __repeat_tuple_impl<T, std::make_index_sequence<N>>::type;

/**
 * @ingroup raze_meta_tuple
 * @brief Constructs a `tuple<Ts...>` with exact explicitly specified element types.
 *
 * Unlike `make_tuple`, does not decay or unwrap `Ts...`, allowing exact construction of
 * tuples containing references, cv-qualified types, or explicit target types.
 *
 * @tparam Ts Exact element types of the resulting tuple.
 * @tparam Args Deduced constructor argument types (`sizeof...(Args) == sizeof...(Ts)`).
 * @param args Values forwarded to initialize each element.
 * @return Newly constructed `tuple<Ts...>`.
 */
template <class ... Ts, class ... Args>
    requires (sizeof...(Ts) == sizeof...(Args))
raze_nodiscard raze_always_inline constexpr tuple<Ts...> make_tuple_exact(Args&& ... args)
    noexcept((std::is_nothrow_constructible_v<Ts, Args&&> && ...))
{
    if constexpr (sizeof...(Ts) == 0)
        return tuple<>{};
    else
        return tuple<Ts...>{ __make_storage<Ts...>(std::forward<Args>(args)...) };
}

/**
 * @ingroup raze_meta_tuple
 * @brief Constructs a `tuple` by decaying arguments and unwrapping `std::reference_wrapper`.
 *
 * Equivalent to `make_tuple_exact<std::unwrap_ref_decay_t<Ts>...>(std::forward<Ts>(ts)...)`.
 *
 * @tparam Ts Deduced argument types.
 * @param ts Values used to initialize tuple elements.
 * @return `tuple<std::unwrap_ref_decay_t<Ts>...>`.
 */
template <class ... Ts>
raze_nodiscard raze_always_inline constexpr auto make_tuple(Ts&& ... ts)
    noexcept((std::is_nothrow_constructible_v<std::unwrap_ref_decay_t<Ts>, Ts&&> && ...))
{
    return meta::make_tuple_exact<std::unwrap_ref_decay_t<Ts>...>(std::forward<Ts>(ts)...);
}

/**
 * @ingroup raze_meta_tuple
 * @brief Constructs a tuple of lvalue references bound to `ts...`.
 * @tparam Ts Referent types.
 * @param ts Lvalues to bind.
 * @return `tuple<Ts&...>`.
 */
template <class ... Ts>
raze_nodiscard raze_always_inline constexpr tuple<Ts&...> tie(Ts& ... ts) noexcept {
    return meta::make_tuple_exact<Ts&...>(ts...);
}

/**
 * @ingroup raze_meta_tuple
 * @brief Constructs a tuple of forwarding references preserving the exact value categories of `ts...`.
 * @tparam Ts Forwarded reference types.
 * @param ts Arguments to bind as forwarding references.
 * @return `tuple<Ts&&...>`.
 */
template <class ... Ts>
raze_nodiscard raze_always_inline constexpr tuple<Ts&&...> forward_as_tuple(Ts&& ... ts) noexcept {
    return meta::make_tuple_exact<Ts&&...>(std::forward<Ts>(ts)...);
}

template <sizetype N, class Gen>
raze_always_inline constexpr auto __build_sequenced(Gen&& gen) {
    if constexpr (N == 0) {
        return tuple<>{};
    }
    else {
        return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            using ret_t = tuple<std::unwrap_ref_decay_t<decltype(gen(index<Is>))>...>;
            using storage_t = typename ret_t::storage_type;

            if constexpr (requires { storage_t::is_leaf_storage; })
                return ret_t{ storage_t{ __leaf<Is, __nth_type_t<Is, std::unwrap_ref_decay_t<decltype(gen(index<Is>))>...>>{ gen(index<Is>) }... } };
            else
                return ret_t{ storage_t{ gen(index<Is>)... } };
        }(std::make_index_sequence<N>{});
    }
}

template <class ... Ts, class Gen>
raze_always_inline constexpr tuple<Ts...> __build_typed_sequenced(Gen&& gen) {
    constexpr sizetype n = sizeof...(Ts);

    if constexpr (n == 0) {
        return tuple<>{};
    }
    else {
        using storage_t = __storage_t<Ts...>;

        return [&] <sizetype ... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
            if constexpr (requires { storage_t::is_leaf_storage; })
                return tuple<Ts...>{ storage_t{ __leaf<Is, Ts>{ gen(index<Is>) }... } };
            else
                return tuple<Ts...>{ storage_t{ gen(index<Is>)... } };
        }(std::make_index_sequence<n>{});
    }
}

__RAZE_META_NAMESPACE_END

template <class ... Ts>
struct std::tuple_size<raze::meta::tuple<Ts...>> :
    std::integral_constant<std::size_t, sizeof...(Ts)>
{};

template <std::size_t I, class ... Ts>
    requires (I < sizeof...(Ts))
struct std::tuple_element<I, raze::meta::tuple<Ts...>> {
    using type = raze::meta::__nth_type_t<I, Ts...>;
};

template <class ... Ts, class ... Us,
          template <class> class TQual, template <class> class UQual>
    requires (sizeof...(Ts) == sizeof...(Us))
          && (requires { typename std::common_reference_t<TQual<Ts>, UQual<Us>>; } && ...)
struct std::basic_common_reference<raze::meta::tuple<Ts...>, raze::meta::tuple<Us...>, TQual, UQual> {
    using type = raze::meta::tuple<std::common_reference_t<TQual<Ts>, UQual<Us>>...>;
};

template <class ... Ts, class ... Us>
    requires (sizeof...(Ts) == sizeof...(Us))
          && (requires { typename std::common_type_t<Ts, Us>; } && ...)
struct std::common_type<raze::meta::tuple<Ts...>, raze::meta::tuple<Us...>> {
    using type = raze::meta::tuple<std::common_type_t<Ts, Us>...>;
};
