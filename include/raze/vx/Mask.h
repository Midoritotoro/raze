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


__RAZE_VX_NAMESPACE_BEGIN

template <
	class _FirstMaskType_,
	class _SecondMaskType_>
concept __compatible_mask = (sizeof(typename _FirstMaskType_::element_type) == sizeof(typename _SecondMaskType_::element_type)) &&
	(_FirstMaskType_::__isa == _SecondMaskType_::__isa) && (_FirstMaskType_::__width == _SecondMaskType_::__width);

template <class _Type_, class _Abi_>
class simd_mask {
	static_assert(type_traits::__is_vector_type_supported_v<_Type_>);
public:
	using storage_type = _Mask_storage<_Type_, _Abi_>;
	using element_type = _Type_;
	using reference_type = _Simd_mask_reference<simd_mask>;
	using value_type = _Type_;
	using abi_type = _Abi_;

	simd_mask() = default;
	simd_mask(const simd_mask&) = default;
	simd_mask(simd_mask&&) = default;
	~simd_mask() = default;

	simd_mask(bool __value) noexcept {
		_storage.__for_each_chunk([&] <class _Chunk> (_Chunk& __chunk) raze_always_inline_lambda {
			__chunk = _Mask_broadcast<abi_type::isa, _Chunk::size, typename _Chunk::mask_type, value_type>()(__value);
		});
	}

	template <class _ForwardIterator_, class _AlignmentPolicy_ = __unaligned_policy>
	simd_mask(const _ForwardIterator_ __first, _AlignmentPolicy_&& __alignment_policy = {}) noexcept
		requires(type_traits::is_iterator_v<_ForwardIterator_> && type_traits::is_iterator_forward_ranges_v<_ForwardIterator_>)
	{
		copy_from(__first, __alignment_policy);
	}

	template <class _ForwardIterator_, class _AlignmentPolicy_ = __unaligned_policy>
	raze_always_inline void copy_from(_ForwardIterator_ __first, _AlignmentPolicy_&& __alignment_policy = {}) noexcept
		requires(std::convertible_to<type_traits::iterator_value_type<_ForwardIterator_>, bool>)
	{
		using _ItType = algorithm::__unwrapped_iterator_type<_ForwardIterator_>;
		using _ItValueType = type_traits::iterator_value_type<_ItType>;

		if constexpr (sizeof(_ItValueType) == 1 && type_traits::is_iterator_contiguous_v<_ItType>) {
			auto __current = reinterpret_cast<const bool*>(std::to_address(__first));

			__for_each_chunk([&] <class _Chunk> (_Chunk& __chunk) raze_always_inline_lambda {
				__chunk = _Load_mask<abi_type::isa, _Chunk::size, typename _Chunk::mask_type, value_type>()(__current, __alignment_policy);
				algorithm::__advance_bytes(__current, _Chunk::size * sizeof(value_type));
			});
		}
		else {
			for (auto __i = 0; __i < size(); ++__i)
				__insert(__i, static_cast<bool>(*__first++));
		}
	}

	template <class _OutputIterator_, class _AlignmentPolicy_ = __unaligned_policy>
	raze_always_inline void copy_to(_OutputIterator_ __first, _AlignmentPolicy_&& __alignment_policy = {}) noexcept {
		using _ItType = algorithm::__unwrapped_iterator_type<_OutputIterator_>;
		using _ItValueType = type_traits::iterator_value_type<_ItType>;

		if constexpr (sizeof(_ItValueType) == 1 && type_traits::is_iterator_contiguous_v<_ItType>) {
			auto __current = reinterpret_cast<bool*>(std::to_address(__first));

			__for_each_chunk([&] <class _Chunk> (const _Chunk& __chunk) raze_always_inline_lambda {
				_Store_mask<abi_type::isa, _Chunk::size, value_type>()(__current, __chunk.data(), __alignment_policy);
				algorithm::__advance_bytes(__current, _Chunk::size * sizeof(value_type));
			});
		}
		else {
			for (auto __i = 0; __i < size(); ++__i)
				*__first++ = __extract(__i);
		}
	}

	raze_always_inline bool operator[](i32 __index) const noexcept {
		return reference_type(*this, __index);
	}

	raze_always_inline auto operator[](i32 __index) noexcept {
		return reference_type(*this, __index);
	}
	
	//template <u32 _Elements_>
	//friend raze_always_inline simd_mask operator>>(const simd_mask& __x,
	//	std::integral_constant<u32, _Elements_>	__elements) noexcept
	//{
	//	return slide_right(__x, __elements);
	//}

	//friend raze_always_inline simd_mask operator>>(const simd_mask& __x, u32 __elements) noexcept {
	//	return slide_right(__x, __elements);
	//}

	//template <u32 _Elements_>
	//friend raze_always_inline simd_mask operator<<(const simd_mask&	__x,
	//	std::integral_constant<u32, _Elements_>	__elements) noexcept
	//{
	//	return slide_left(__x, __elements);
	//}

	//friend raze_always_inline simd_mask operator<<(const simd_mask& __x, u32 __elements) noexcept {
	//	return slide_left(__x, __elements);
	//}

	raze_nodiscard raze_always_inline static constexpr auto size() noexcept {
		return abi_type::size;
	}

	//template <u32 _Elements_>
	//raze_always_inline simd_mask& operator<<=(std::integral_constant<u32, _Elements_> __elements) noexcept {
	//	return *this = (*this << __elements);
	//}

	//raze_always_inline simd_mask& operator<<=(u32 __elements) noexcept {
	//	return *this = (*this << __elements);
	//}

	//template <u32 _Elements_>
	//raze_always_inline simd_mask& operator>>=(std::integral_constant<u32, _Elements_> __elements) noexcept {
	//	return *this = (*this >> __elements);
	//}

	//raze_always_inline simd_mask& operator>>=(u32 __elements) noexcept {
	//	return *this = (*this >> __elements);
	//}

	raze_always_inline simd_mask& operator=(const simd_mask& __other) noexcept {
		_storage = __other._storage;
		return *this;
	}

	friend raze_always_inline simd_mask operator&(const simd_mask& __x, const simd_mask& __y) noexcept {
		return bit_and(__x, __y);
	}

	friend raze_always_inline simd_mask operator|(const simd_mask&__x, const simd_mask& __y) noexcept {
		return bit_or(__x, __y);
	}

	friend raze_always_inline simd_mask operator^(const simd_mask&__x, const simd_mask& __y) noexcept {
		return bit_xor(__x, __y);
	}

	raze_always_inline simd_mask& operator&=(const simd_mask& __other) noexcept {
		return *this = (*this & __other);
	}

	raze_always_inline simd_mask& operator|=(const simd_mask& __other) noexcept {
		return *this = (*this | __other);
	}

	raze_always_inline simd_mask& operator^=(const simd_mask& __other) noexcept {
		return *this = (*this ^ __other);
	}

	raze_always_inline simd_mask operator!() const noexcept {
		return bit_not(*this);
	}

	friend raze_always_inline simd_mask operator!=(const simd_mask& __x, const simd_mask& __y) noexcept {
		return bit_xor(__x, __y);
	}

	friend raze_always_inline simd_mask operator==(const simd_mask& __x, const simd_mask& __y) noexcept {
		return !(__x != __y);
	}

	raze_nodiscard explicit raze_always_inline operator storage_type() const noexcept {
		return _storage;
	}

	raze_nodiscard explicit raze_always_inline operator storage_type&() noexcept {
		return _storage;
	}

	raze_nodiscard raze_always_inline storage_type __storage() const noexcept {
		return _storage;
	}

	raze_nodiscard raze_always_inline storage_type& __storage() noexcept {
		return _storage;
	}

	template <class _Function_, class ... _Args_>
	raze_always_inline void __for_each_chunk(_Function_&& __f, _Args_&& ... __args) noexcept {
		_storage.__for_each_chunk(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
	}

	template <class _Function_, class ... _Args_>
	raze_always_inline void __for_each_chunk(_Function_&& __f, _Args_&& ... __args) const noexcept {
		_storage.__for_each_chunk(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
	}

	template <class _Function_, class ... _Args_>
	raze_always_inline bool __for_each_chunk_any_of(_Function_&& __f, _Args_&& ... __args) noexcept {
		return _storage.__for_each_chunk_any_of(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
	}

	template <class _Function_, class ... _Args_>
	raze_always_inline bool __for_each_chunk_any_of(_Function_&& __f, _Args_&& ... __args) const noexcept {
		return _storage.__for_each_chunk_any_of(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
	}

	template <class _Function_, class ... _Args_>
	raze_always_inline bool __for_each_chunk_all_of(_Function_&& __f, _Args_&& ... __args) noexcept {
		return _storage.__for_each_chunk_all_of(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
	}

	template <class _Function_, class ... _Args_>
	raze_always_inline bool __for_each_chunk_all_of(_Function_&& __f, _Args_&& ... __args) const noexcept {
		return _storage.__for_each_chunk_all_of(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
	}

	raze_always_inline void __insert(i32 __i, bool __v) noexcept {
		_storage.__insert(__i, __v);
    }

    raze_nodiscard raze_always_inline bool __extract(i32 __i) const noexcept {
		return _storage.__extract(__i);
    }
private:
	storage_type _storage;
};

__RAZE_VX_NAMESPACE_END