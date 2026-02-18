#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <raze/Types.h>

__RAZE_SYSTEM_NAMESPACE_BEGIN

class handle {
public:
	using native_handle_type	= system_handle_t;
	using deleter_type			= system_bool_t(*)(native_handle_type);

	handle() noexcept;
	~handle() noexcept;

	handle(native_handle_type __handle) noexcept;

	raze_always_inline handle& operator=(const handle& __other) noexcept;
	raze_always_inline handle& operator=(const native_handle_type __other) noexcept;

	handle(
		native_handle_type	__handle,
		bool				__auto_delete,
		deleter_type		__deleter) noexcept;

	raze_always_inline void set_deleter(deleter_type __deleter) noexcept;
	raze_nodiscard raze_always_inline deleter_type deleter() const noexcept;

	raze_always_inline void set_auto_delete(bool __autoDelete) noexcept;
	raze_nodiscard raze_always_inline bool auto_delete() const noexcept;

	raze_always_inline void set_native_handle(
		native_handle_type	__handle,
		bool				__deletePrevious = true) noexcept;
	raze_nodiscard raze_always_inline native_handle_type native_handle() noexcept;

	raze_always_inline bool destroy() noexcept;
	raze_nodiscard raze_always_inline bool available() const noexcept;
	
	raze_always_inline friend bool operator==(
		const handle& __left,
		const handle& __right) noexcept;

	raze_always_inline friend bool operator!=(
		const handle& __left,
		const handle& __right) noexcept;
protected:
	native_handle_type	_native_handle	= nullptr;
	deleter_type		_deleter		= nullptr;

	bool _auto_delete = true;
};

handle::handle() noexcept {}

handle::~handle() noexcept {
	if (_auto_delete)
		destroy();
}

handle::handle(native_handle_type __handle) noexcept:
	_native_handle(__handle)
{}

handle::handle(
	native_handle_type	__handle,
	bool				__auto_delete,
	deleter_type		__deleter
) noexcept:
	_native_handle(__handle),
	_auto_delete(__auto_delete),
	_deleter(std::move(__deleter))
{}

handle& handle::operator=(const native_handle_type __other) noexcept {
	_native_handle = __other;
	return *this;
}

void handle::set_deleter(deleter_type __deleter) noexcept {
	_deleter = std::move(__deleter);
}

handle::deleter_type handle::deleter() const noexcept {
	return _deleter;
}

void handle::set_auto_delete(bool __auto_delete) noexcept {
	_auto_delete = __auto_delete;
}

bool handle::auto_delete() const noexcept {
	return _auto_delete;
}

void handle::set_native_handle(
	native_handle_type	__handle,
	bool				__delete_previous) noexcept
{
	if (__handle == _native_handle)
		return;

	if (__delete_previous)
		destroy();

	_native_handle = __handle;
}

handle::native_handle_type handle::native_handle() noexcept {
	return _native_handle;
}

bool handle::destroy() noexcept {
	if (_native_handle != nullptr) {
		_deleter(_native_handle);
		_native_handle = nullptr;

		return true;
	}

	return false;
}

bool handle::available() const noexcept {
	return (_native_handle != nullptr);
}

handle& handle::operator=(const handle& __other) noexcept {
	_native_handle = __other._native_handle;
	return *this;
}

raze_nodiscard bool operator==(
	const handle& __left,
	const handle& __right) noexcept
{
	return __left._native_handle == __right._native_handle;
}

raze_nodiscard bool operator!=(
	const handle& __left,
	const handle& __right) noexcept
{
	return __left._native_handle != __right._native_handle;
}

__RAZE_SYSTEM_NAMESPACE_END
