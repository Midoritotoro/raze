#pragma once 

#include <raze/Types.h>
#include <raze/compatibility/Compatibility.h>
#include <src/raze/algorithm/AdvanceBytes.h>
#include <ostream>

__RAZE_CONCURRENCY_NAMESPACE_BEGIN

class thread_id {
public:
	using thread_id_type = dword_t;

	thread_id() noexcept;
	thread_id(thread_id_type id) noexcept;

#if raze_has_cxx20
	raze_always_inline friend std::strong_ordering operator<=>(
		const thread_id& left,
		const thread_id& right) noexcept;
#else
	raze_always_inline friend bool operator==(
		const thread_id& left, 
		const thread_id& right) noexcept;
	raze_always_inline friend bool operator!=(
		const thread_id& left,
		const thread_id& right) noexcept;

	raze_always_inline friend bool operator<(
		const thread_id& left,
		const thread_id& right) noexcept;
	raze_always_inline friend bool operator>(
		const thread_id& left, 
		const thread_id& right) noexcept;

	raze_always_inline friend bool operator<=(
		const thread_id& left, 
		const thread_id& right) noexcept;
	raze_always_inline friend bool operator>=(
		const thread_id& left, 
		const thread_id& right) noexcept;
#endif // raze_has_cxx20

	template <
		class _Char_,
		class _Traits_>
	friend std::basic_ostream<_Char_, _Traits_>& operator<<(
		std::basic_ostream<_Char_, _Traits_>&	stream,
		concurrency::thread_id					id);

	raze_nodiscard raze_always_inline thread_id_type id() const noexcept;
private:
	thread_id_type _id = 0;
};

thread_id::thread_id() noexcept {}

thread_id::thread_id(thread_id_type id) noexcept:
	_id(id)
{}

#if raze_has_cxx20

std::strong_ordering operator<=>(
	const thread_id& left,
	const thread_id& right) noexcept
{
	return left._id <=> right._id;
}

#else

bool operator==(
	const thread_id& left,
	const thread_id& right) noexcept 
{
	return left._id == right._id;
}

bool operator!=(
	const thread_id& left,
	const thread_id& right) noexcept
{
	return left._id != right._id;
}

bool operator<(
	const thread_id& left,
	const thread_id& right) noexcept
{
	return left._id < right._id;
}

bool operator>(
	const thread_id& left,
	const thread_id& right) noexcept
{
	return left._id > right._id;
}

bool operator<=(
	const thread_id& left,
	const thread_id& right) noexcept 
{
	return left._id <= right._id;
}

bool operator>=(
	const thread_id& left,
	const thread_id& right) noexcept
{
	return left._id >= right._id;
}

#endif // raze_has_cxx20

thread_id::thread_id_type thread_id::id() const noexcept {
	return _id;
}

template <
	class _Char_, 
	class _Traits_>
std::basic_ostream<_Char_, _Traits_>& operator<<(
	std::basic_ostream<_Char_, _Traits_>&	stream, 
	concurrency::thread_id					id)
{
	static_assert(sizeof(concurrency::thread_id) == 4);
	_Char_ buffer[11];

	_Char_* end = std::end(buffer);
	*--end = static_cast<_Char_>('\0');

	end = algorithm::__unsigned_integral_to_buffer(end, id._id);
	return stream << end;
}

__RAZE_CONCURRENCY_NAMESPACE_END
