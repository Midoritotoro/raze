#pragma once 

#include <raze/vx/Abi.h>
#include <src/raze/vx/storage/Size.h>
#include <src/raze/vx/storage/ElementWise.h>
#include <src/raze/vx/storage/BestChunk.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class T, class Abi>
class vector_storage {
    static constexpr std::size_t bytes = Abi::size * sizeof(T);
    static constexpr bool use_native = (bytes == 16 && has_sse2<Abi::isa>) ||
        (bytes == 32 && has_avx<Abi::isa>) || (bytes == 64 && has_avx512f<Abi::isa>);
public:
    using abi_type = Abi;
    using tuple_type = std::conditional_t<use_native, typename best_chunk<
        Type, Abi, Abi::size>::type, simd_vector_tuple_type<_Type_, _Abi_>>;
    
    raze_always_inline vector_storage() noexcept = default;
    raze_always_inline vector_storage(const vector_storage&) noexcept = default;
    raze_always_inline vector_storage(vector_storage&&) noexcept = default;

    raze_always_inline ~vector_storage() = default;

    raze_always_inline vector_storage& operator=(const vector_storage&) noexcept = default;
    raze_always_inline vector_storage& operator=(vector_storage&&) noexcept = default;

    raze_nodiscard static constexpr bool is_native() noexcept {
        return use_native;
    }

    raze_nodiscard static constexpr auto chunks_count() noexcept {
        if constexpr (is_native()) return 1;
        else return simd_tuple_size<std::remove_cvref_t<tuple_type>>::value;
    }

    raze_always_inline tuple_type storage() const noexcept {
        return _data;
    }

    raze_always_inline tuple_type& storage() noexcept {
        return _data;
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline void __for_each_chunk(_Function_&& __f, _Args_&& ... __args) noexcept {
        if constexpr (use_native) __f(_data, std::forward<_Args_>(__args)...);
        else __for_each_tuple(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline void __for_each_chunk_reverse(_Function_&& __f, _Args_&& ... __args) noexcept {
        if constexpr (use_native) __f(_data, std::forward<_Args_>(__args)...);
        else __for_each_tuple_reverse(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline auto __for_each_chunk_all_of(_Function_&& __f, _Args_&& ... __args) noexcept {
        if constexpr (use_native) return __f(_data, std::forward<_Args_>(__args)...);
        else return __for_each_tuple_all_of(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline auto __for_each_chunk_all_of_reverse(_Function_&& __f, _Args_&& ... __args) noexcept {
        if constexpr (use_native) return __f(_data, std::forward<_Args_>(__args)...);
        else return __for_each_tuple_all_of_reverse(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline auto __for_each_chunk_any_of(_Function_&& __f, _Args_&& ... __args) noexcept {
        if constexpr (use_native) return __f(_data, std::forward<_Args_>(__args)...);
        else return __for_each_tuple_any_of(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline auto __for_each_chunk_any_of_reverse(_Function_&& __f, _Args_&& ... __args) noexcept {
        if constexpr (use_native) return __f(_data, std::forward<_Args_>(__args)...);
        else return __for_each_tuple_any_of_reverse(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline void __for_each_chunk(_Function_&& __f, _Args_&& ... __args) const noexcept {
        if constexpr (use_native) __f(_data, std::forward<_Args_>(__args)...);
        else __for_each_tuple(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline void __for_each_chunk_reverse(_Function_&& __f, _Args_&& ... __args) const noexcept {
        if constexpr (use_native) __f(_data, std::forward<_Args_>(__args)...);
        else __for_each_tuple_reverse(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline auto __for_each_chunk_all_of(_Function_&& __f, _Args_&& ... __args) const noexcept {
        if constexpr (use_native) return __f(_data, std::forward<_Args_>(__args)...);
        else return __for_each_tuple_all_of(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline auto __for_each_chunk_all_of_reverse(_Function_&& __f, _Args_&& ... __args) const noexcept {
        if constexpr (use_native) return __f(_data, std::forward<_Args_>(__args)...);
        else return __for_each_tuple_all_of_reverse(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline auto __for_each_chunk_any_of(_Function_&& __f, _Args_&& ... __args) const noexcept {
        if constexpr (use_native) return __f(_data, std::forward<_Args_>(__args)...);
        else return __for_each_tuple_any_of(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline auto __for_each_chunk_any_of_reverse(_Function_&& __f, _Args_&& ... __args) const noexcept {
        if constexpr (use_native) return __f(_data, std::forward<_Args_>(__args)...);
        else return __for_each_tuple_any_of_reverse(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    raze_always_inline void __insert(i32 i, T v) noexcept {
        raze_debug_assert(i >= 0 && i < Abi::size);

        if constexpr (use_native) insert_<Abi::isa>(ustorage(_data), i, v);
        else visit_chunk_by_index(_data, __i, [&](auto& chunk, i32 lane) raze_always_inline_lambda {
            insert_<Abi::isa>(ustorage(chunk), lane, v);
        });
    }

    raze_nodiscard raze_always_inline T __extract(i32 i) const noexcept {
        raze_debug_assert(i >= 0 && i < Abi::size);
        T r{};

        if constexpr (use_native) r = extract_<Abi::isa, T>(ustorage(_data), i);
        else visit_chunk_by_index(_data, __i, [&](const auto& chunk, i32 lane) raze_always_inline_lambda {
            r = extract_<Abi::isa, T>(ustorage(chunk), lane);
        });

        return r;
    }

    template <sizetype I>
    raze_nodiscard raze_always_inline T __extract(std::integral_constant<sizetype, I> i) const noexcept {
        static_assert(i >= 0 && i < Abi::size);
        T r{};

        if constexpr (use_native) r = extract_<Abi::isa, T>(ustorage(_data), i);
        else visit_chunk_by_index(_data, __i, [&](const auto& chunk, auto current) raze_always_inline_lambda{
            r = extract_<Abi::isa, T>(ustorage(chunk), current);
        });

        return r;
    }
private:
    raze_no_unique_address tuple_type _data;
};

__RAZE_VX_NAMESPACE_END
