#pragma once 

#include <raze/arch/ProcessorInformation.h>

#if defined(raze_os_win)
#  include <src/raze/concurrency/WindowsThread.h>
#endif // defined(raze_os_win)

#include <raze/concurrency/ThreadId.h>
#include <raze/concurrency/ThreadHandle.h>

#include <src/raze/concurrency/ThreadYield.h>
#include <chrono>
#include <thread>

__RAZE_CONCURRENCY_NAMESPACE_BEGIN

class thread final {
    static constexpr auto __default_stack_size = 1024 * 1024; // 1 Mb
    static constexpr auto __default_stack_priority = Priority::NormalPriority;
public:
	using handle_type	= thread_handle;
	using id			= thread_id;

	raze_nodiscard_constructor thread() noexcept;
    raze_nodiscard_constructor thread(thread&& __other) noexcept;

    thread(const thread&) = delete;
    thread& operator=(const thread&) = delete;

    ~thread() noexcept;

    /**
     * @brief Управляет поведением деструктора объекта потока.
     *
     * Если флаг установлен в true, то при уничтожении объекта поток
     * завершается немедленно, без ожидания выполнения задачи.
     * Если флаг равен false, то деструктор вызывает join(), дожидаясь
     * завершения работы потока, и только затем освобождает ресурсы.
     *
     * @param __terminate_on_destroy Логический флаг, определяющий поведение деструктора.
     */
    void set_terminate_on_destroy(bool __terminate_on_destroy) noexcept;
    raze_nodiscard raze_always_inline bool terminate_on_destroy() const noexcept;


    /**
     * @brief Устанавливает размер стека для нового потока.
     *
     * По умолчанию используется значение defaultStackSize.
     *
     * @param __bytes Размер стека в байтах.
     */
    void set_stack_size(sizetype __bytes) noexcept;
    raze_nodiscard raze_always_inline sizetype stack_size() const noexcept;


    /**
     * @brief Устанавливает приоритет выполнения потока.
     *
     * По умолчанию используется defaultStackPriority (нормальный приоритет).
     *
     * @param __priority Значение приоритета.
     */
    void set_priority(Priority __priority) noexcept;
    raze_nodiscard raze_always_inline Priority priority() const noexcept;

    thread& operator=(thread&& __other) noexcept;
    void swap(thread& __other) noexcept;

    /**
     * @brief Блокирует текущий поток до завершения работы потока,
     *        принадлежащего данному объекту.
     *
     * После завершения освобождаются связанные ресурсы.
     */
    void join();

    /**
     * @brief Отделяет поток выполнения от объекта.
     *
     * Поток продолжает работу независимо, а объект освобождает свои ресурсы.
     * Завершение потока произойдёт автоматически, без участия объекта.
     */
    void detach();

    /**
     * @brief Принудительно уничтожает поток выполнения.
     *
     * Завершает поток немедленно, без ожидания его нормального окончания.
     */
    void terminate();

    /**
     * @brief Запускает новый поток выполнения, исполняющий переданную задачу.
     *
     * Принимает вызываемый объект
     * и создаёт поток, который будет выполнять его без дополнительных аргументов.
     *
     * @tparam _Task_   Тип вызываемого объекта.
     * @param __task    Вызываемый объект, который будет выполнен в новом потоке.
     */
    template <class _Task_>
    inline void start(_Task_&& __task);

    /**
     * @brief Запускает новый поток выполнения, исполняющий переданную задачу
     *        с аргументами, аналогично std::invoke.
     *
     * Данный метод принимает вызываемый объект
     * и аргументы, которые будут переданы при вызове. Первый аргумент используется
     * как объект для вызова указателя на метод или как первый параметр функции,
     * остальные аргументы передаются далее.
     *
     * @tparam _Task_          Тип вызываемого объекта.
     * @tparam _FirstArgument_ Тип первого аргумента.
     * @tparam _Args_          Типы остальных аргументов.
     *
     * @param __task            Вызываемый объект, который будет выполнен в новом потоке.
     * @param __first_argument  Первый аргумент, используемый для вызова (например, объект для метода).
     * @param __args            Дополнительные аргументы, передаваемые вызываемому объекту.
     */
    template <
        class       _Task_,
        class       _FirstArgument_,
        class...    _Args_>
    inline void start(
        _Task_&&            __task,
        _FirstArgument_&&   __first_argument,
        _Args_&& ...        __args);


    raze_nodiscard raze_always_inline bool joinable() const noexcept;

    /**
     * @brief Возвращает количество аппаратных потоков выполнения,
     *        доступных на текущей системе.
     * @return Количество аппаратных потоков.
     */
    raze_nodiscard raze_always_inline static uint32 hardware_concurrency() noexcept;

    /**
     * @brief Возвращает системный дескриптор потока.
     * @return Дескриптор потока.
     */
    raze_nodiscard raze_always_inline handle_type handle() noexcept;


    /**
    * @brief Проверяет, соответствует ли данный объект текущему потоку выполнения.
    *
    * Возвращает true, если объект управляет текущим выполняющимся потоком.
    *
    * @return Значение, указывающее на совпадение с текущим потоком.
    */
    raze_nodiscard raze_always_inline bool is_current_thread() const noexcept;
private:
    handle_type _handle;
	id _id = 0;

    bool _terminate_on_destroy = false;
    bool _joinable = false;

    sizetype _stack_size = __default_stack_size;
    Priority _priority = __default_stack_priority;
};

thread::thread() noexcept 
{}

thread::thread(thread&& __other) noexcept :
    _handle(std::exchange(__other._handle, {})),
    _id(std::exchange(__other._id, {})),
    _terminate_on_destroy(std::exchange(__other._terminate_on_destroy, {})),
    _stack_size(std::exchange(__other._stack_size, {})),
    _priority(std::exchange(__other._priority, {}))
{
    raze_debug_assert(!__other.joinable());
    _joinable = std::exchange(__other._joinable, {});
}

thread::~thread() noexcept {
    if (_joinable && _terminate_on_destroy)
        __terminate_thread(_handle.native_handle());
    else
        _handle.destroy();
}

void thread::set_terminate_on_destroy(bool __terminate_on_destroy) noexcept {
    _terminate_on_destroy = __terminate_on_destroy;
}

bool thread::terminate_on_destroy() const noexcept {
    return _terminate_on_destroy;
}

void thread::set_stack_size(sizetype __bytes) noexcept {
    _stack_size = __bytes;
}

sizetype thread::stack_size() const noexcept {
    return _stack_size;
}

void thread::set_priority(Priority __priority) noexcept {
    _priority = __priority;
}

Priority thread::priority() const noexcept {
    return _priority;
}

void thread::swap(thread& __other) noexcept {
    algorithm::swap(_handle, __other._handle);
    algorithm::swap(_id, __other._id);
    algorithm::swap(_stack_size, __other._stack_size);
    algorithm::swap(_joinable, __other._joinable);
    algorithm::swap(_priority, __other._priority);
    algorithm::swap(_terminate_on_destroy, __other._terminate_on_destroy);
}

void thread::join() {
    if (_handle.available() == false)
        return;

    const auto result = __wait_for_thread(_handle.native_handle());
    raze_debug_assert(result != __thread_result::__error);

    _handle.destroy();
    _joinable = false;
    _id = 0;
}

void thread::detach() {
    _handle.set_auto_delete(false);
    concurrency::__detach_thread(_handle.native_handle());

    _id = 0;
    _handle.set_native_handle(nullptr, false);
}

void thread::terminate() {
    if (_handle.available()) {
        concurrency::__terminate_thread(_handle.native_handle());

        _handle.destroy();
        _id = 0;
        _joinable = false;
    }
}

thread::handle_type thread::handle() noexcept {
    return _handle;
}

bool thread::is_current_thread() const noexcept {
    return (concurrency::__current_thread_id() == _id.id());
}

uint32 thread::hardware_concurrency() noexcept {
    return arch::ProcessorInformation::hardware_concurrency();
}

bool thread::joinable() const noexcept {
    return _joinable;
}

thread& thread::operator=(thread&& __other) noexcept {
    if (_joinable)
        terminate();

    _id     = std::exchange(__other._id, {});
    _handle = std::exchange(__other._handle, {});

    return *this;
}

template <class _Task_>
void thread::start(_Task_&& __task)
{
    if (_handle.available())
        terminate();

    const auto __result = concurrency::__create_thread(
        __thread_creation_flags::__suspend_after_creation, _stack_size,
        std::forward<_Task_>(__task));

    concurrency::__set_thread_priority(__result.__handle, _priority);

    _joinable = true;

    _handle = __result.__handle;
    _id = __result.__id;

    _handle.set_deleter(CloseHandle);
    concurrency::__resume_suspended_thread(result.handle);
}

template <
    class       _Task_,
    class       _FirstArgument_,
    class...    _Args_>
void thread::start(
    _Task_&&            task,
    _FirstArgument_&&   firstArgument,
    _Args_&& ...        args)
{
    if (_handle.available())
        terminate();

    const auto __result = concurrency::__create_thread(
        __thread_creation_flags::__suspend_after_creation, _stack_size,
        std::forward<_Task_>(__task), std::forward<_FirstArgument_>(__first_argument), 
        std::forward<_Args_>(__args)...);

    concurrency::__set_thread_priority(__result.__handle, _priority);

    _joinable = true;

    _handle = __result.__handle;
    _id = __result.__id;

    _handle.set_deleter(CloseHandle);
    concurrency::__resume_suspended_thread(__result.__handle);
}

namespace this_thread {
    Priority get_priority() noexcept {
        return static_cast<Priority>(concurrency::__thread_priority(concurrency::__current_thread()));
    }

    thread_id get_id() noexcept {
        return concurrency::__current_thread_id();
    }

    raze_always_inline void yield() noexcept {
        concurrency::__yield();
    }

    template <
        class _Clock_,
        class _Duration_>
    void sleep_until(const std::chrono::time_point<_Clock_, _Duration_>& __absolute_time) {
        constexpr auto __maximum_sleep_ms = std::chrono::milliseconds(std::chrono::hours(24));

        while (true) {
            const auto __now = _Clock_::now();

            if (__absolute_time <= __now)
                return;

            std::chrono::milliseconds __ms;
            const auto __remaining_time = (__absolute_time - __now);

            if (__remaining_time > __maximum_sleep_ms)
                __ms = __maximum_sleep_ms;
            else
                __ms = std::chrono::ceil<std::chrono::milliseconds>(__remaining_time);

            concurrency::__current_thread_sleep(ms.count());
        }
    }

    template <
        class _TickCountType_,
        class _Period_>
    void sleep_for(const std::chrono::duration<_TickCountType_, _Period_>& __relative_time) {
        sleep_until(concurrency::__to_absolute_time(__relative_time));
    }
} // namespace this_thread

__RAZE_CONCURRENCY_NAMESPACE_END
