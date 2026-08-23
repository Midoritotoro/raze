#pragma once 

#include <src/raze/vx/hw/Mask.h>
#include <raze/vx/Algorithm.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskBroadcast.h>
#include <src/raze/vx/hw/x86/mask/operations/LoadMask.h>
#include <src/raze/vx/hw/x86/mask/operations/StoreMask.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>
#include <src/raze/vx/Storage.h>
#include <src/raze/vx/reference/SimdMaskReference.h>
#include <bitset>
#include <src/raze/utility/UninitializedTag.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class T, class Abi>
class simd_mask {
	static_assert(traits::is_vector_type_supported_v<T>);
public:
	using storage_type = _Mask_storage<T, Abi>;
	using element_type = T;
	using reference_type = _Simd_mask_reference<simd_mask>;
	using value_type = T;
	using abi_type = Abi;

	raze_always_inline simd_mask() noexcept {
		_storage.__for_each_chunk([&] <class Chunk> (Chunk& chunk) raze_always_inline_lambda {
			chunk = _Mask_zero<abi_type::isa, typename Chunk::unwrapped_type>()();
		});
	}

	simd_mask(const simd_mask&) = default;
	simd_mask(simd_mask&&) = default;
	~simd_mask() = default;

	raze_always_inline simd_mask(uninitialized_tag) noexcept {}

	raze_no_stack_protector raze_always_inline explicit simd_mask(bool v) noexcept {
		_storage.__for_each_chunk([&] <class Chunk> (Chunk& chunk) raze_always_inline_lambda {
			chunk = _Mask_broadcast<abi_type::isa, Chunk::size, typename Chunk::unwrapped_type, value_type>()(v);
		});
	}

	template <std::forward_iterator FwdIt, class Policy = __unaligned_policy>
	raze_no_stack_protector raze_always_inline simd_mask(FwdIt it, Policy&& policy = {}) noexcept {
		copy_from(it, policy);
	}

	template <std::forward_iterator FwdIt, class Policy = __unaligned_policy>
	raze_no_stack_protector raze_always_inline void copy_from(FwdIt it, Policy&& policy = {}) noexcept
		requires(std::convertible_to<std::iter_value_t<_ForwardIterator_>, bool>)
	{
		using Unwrapped = algorithm::unwrapped_iterator_type<FwdIt>;
		using Value = std::iter_value_t<Unwrapped>;

		if constexpr (sizeof(Value) == 1 && std::contiguous_iterator<Unwrapped> && sizeof(T) == 1) {
			auto current = reinterpret_cast<const bool*>(std::to_address(it));

			__for_each_chunk([&] <class Chunk> (Chunk& chunk) raze_always_inline_lambda {
				chunk = _Load_mask<abi_type::isa, Chunk::size, typename Chunk::unwrapped_type, value_type>()(current, policy);
				algorithm::advance_bytes(current, Chunk::size * sizeof(value_type));
			});
		}
		else {
			for (auto i = 0; i < size(); ++i)
				insert(i, static_cast<bool>(*it++));
		}
	}

	template <class OutIt, class Policy = __unaligned_policy>
	raze_always_inline void copy_to(OutIt out, Policy&& policy = {}) noexcept {
		using Unwrapped = algorithm::unwrapped_iterator_type<OutIt>;
		using Value = std::iter_value_t<Unwrapped>;

		if constexpr (sizeof(Value) == 1 && std::contiguous_iterator<Unwrapped> && sizeof(T) == 1) {
			auto current = reinterpret_cast<bool*>(std::to_address(out));

			__for_each_chunk([&] <class Chunk> (const Chunk& chunk) raze_always_inline_lambda {
				_Store_mask<abi_type::isa, Chunk::size, value_type>()(current, chunk.data(), policy);
				algorithm::advance_bytes(current, Chunk::size * sizeof(value_type));
			});
		}
		else {
			for (auto i = 0; i < size(); ++i)
				*out++ = __extract(i);
		}
	}

	raze_always_inline bool operator[](i32 i) const noexcept {
		return __extract(i);
	}

	raze_always_inline auto operator[](i32 i) noexcept {
		return reference_type(*this, i);
	}

	raze_nodiscard raze_always_inline static constexpr auto size() noexcept {
		return abi_type::size;
	}

	raze_always_inline simd_mask& operator=(const simd_mask& other) noexcept {
		_storage = other._storage;
		return *this;
	}

	friend raze_always_inline simd_mask operator&(const simd_mask& x, const simd_mask& y) noexcept {
		return bit_and(x, y);
	}

	friend raze_always_inline simd_mask operator|(const simd_mask& x, const simd_mask& y) noexcept {
		return bit_or(x, y);
	}

	friend raze_always_inline simd_mask operator^(const simd_mask& x, const simd_mask& y) noexcept {
		return bit_xor(x, y);
	}

	raze_always_inline simd_mask& operator&=(const simd_mask& other) noexcept {
		return *this = (*this & other);
	}

	raze_always_inline simd_mask& operator|=(const simd_mask& other) noexcept {
		return *this = (*this | other);
	}

	raze_always_inline simd_mask& operator^=(const simd_mask& other) noexcept {
		return *this = (*this ^ other);
	}

	raze_always_inline simd_mask operator!() const noexcept {
		return bit_not(*this);
	}

	friend raze_always_inline simd_mask operator!=(const simd_mask& x, const simd_mask& y) noexcept {
		return bit_xor(x, y);
	}

	friend raze_always_inline simd_mask operator==(const simd_mask& x, const simd_mask& y) noexcept {
		return !(x != y);
	}

	raze_nodiscard raze_always_inline const storage_type& __storage() const noexcept {
		return _storage;
	}

	raze_nodiscard raze_always_inline storage_type& __storage() noexcept {
		return _storage;
	}

	raze_nodiscard raze_always_inline static constexpr bool __is_native() noexcept {
		return storage_type::is_native();
	}

	raze_nodiscard raze_always_inline static constexpr auto __chunks_count() noexcept {
		return storage_type::chunks_count();
	}

	template <sizetype _I_>
	raze_always_inline auto __get() const noexcept {
		return raze::vx::__get<_I_>(_storage.storage());
	}

	template <class _Function_, class ... _Args_>
	raze_always_inline void __for_each_chunk(_Function_&& __f, _Args_&& ... __args) noexcept {
		_storage.__for_each_chunk(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
	}

	template <class _Function_, class ... _Args_>
	raze_always_inline void __for_each_chunk_reverse(_Function_&& __f, _Args_&& ... __args) noexcept {
		_storage.__for_each_chunk_reverse(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
	}

	template <class _Function_, class ... _Args_>
	raze_always_inline void __for_each_chunk(_Function_&& __f, _Args_&& ... __args) const noexcept {
		_storage.__for_each_chunk(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
	}

	template <class _Function_, class ... _Args_>
	raze_always_inline void __for_each_chunk_reverse(_Function_&& __f, _Args_&& ... __args) const noexcept {
		_storage.__for_each_chunk_reverse(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
	}

	template <class _Function_, class ... _Args_>
	raze_always_inline bool __for_each_chunk_any_of(_Function_&& __f, _Args_&& ... __args) noexcept {
		return _storage.__for_each_chunk_any_of(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
	}

	template <class _Function_, class ... _Args_>
	raze_always_inline bool __for_each_chunk_any_of_reverse(_Function_&& __f, _Args_&& ... __args) noexcept {
		return _storage.__for_each_chunk_any_of_reverse(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
	}

	template <class _Function_, class ... _Args_>
	raze_always_inline bool __for_each_chunk_any_of(_Function_&& __f, _Args_&& ... __args) const noexcept {
		return _storage.__for_each_chunk_any_of(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
	}

	template <class _Function_, class ... _Args_>
	raze_always_inline bool __for_each_chunk_any_of_reverse(_Function_&& __f, _Args_&& ... __args) const noexcept {
		return _storage.__for_each_chunk_any_of_reverse(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
	}

	template <class _Function_, class ... _Args_>
	raze_always_inline bool __for_each_chunk_all_of(_Function_&& __f, _Args_&& ... __args) noexcept {
		return _storage.__for_each_chunk_all_of(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
	}

	template <class _Function_, class ... _Args_>
	raze_always_inline bool __for_each_chunk_all_of_reverse(_Function_&& __f, _Args_&& ... __args) noexcept {
		return _storage.__for_each_chunk_all_of_reverse(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
	}

	template <class _Function_, class ... _Args_>
	raze_always_inline bool __for_each_chunk_all_of(_Function_&& __f, _Args_&& ... __args) const noexcept {
		return _storage.__for_each_chunk_all_of(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
	}

	template <class _Function_, class ... _Args_>
	raze_always_inline bool __for_each_chunk_all_of_reverse(_Function_&& __f, _Args_&& ... __args) const noexcept {
		return _storage.__for_each_chunk_all_of_reverse(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
	}

	raze_always_inline void __insert(i32 __i, bool __v) noexcept {
		_storage.insert(__i, __v);
    }

    raze_nodiscard raze_always_inline bool __extract(i32 i) const noexcept {
		return _storage.extract(i);
    }
private:
	raze_no_unique_address storage_type _storage;
};

__RAZE_VX_NAMESPACE_END