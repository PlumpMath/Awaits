#include <iostream>
#include <thread>
#include <future>

#include <experimental/generator>
#include <experimental/resumable>

#include <boost/optional.hpp>

using namespace std::experimental;

namespace std
{
namespace experimental
{

struct my_f {};

template<typename T>
struct coroutine_traits<std::future<T>, my_f>
{
    struct suspend_then_resume_on_thread
    {
        std::thread m_thread;

        suspend_then_resume_on_thread() noexcept {}
        suspend_then_resume_on_thread(suspend_then_resume_on_thread&&) noexcept = default;

        bool await_ready() noexcept
        {
            return false;
        }

        void await_suspend(coroutine_handle<> h) noexcept
        {
            std::cout << "Suspended in thread:" << std::this_thread::get_id() << std::endl;
            m_thread.swap(std::thread([h]() { h.resume(); }));
        }

        void await_resume() noexcept
        {
            std::cout << "Resumed in thread: " << std::this_thread::get_id() << std::endl;
        }

        ~suspend_then_resume_on_thread() noexcept
        {
            std::cout << "Destroying in thread: " << std::this_thread::get_id() << std::endl;
        }
    };

    struct promise_type
    {
        std::promise<T> m_promise;

        std::future<T> get_return_object()
        {
            return m_promise.get_future();
        }

        auto initial_suspend()
        {
            return suspend_then_resume_on_thread();
        }

        auto final_suspend()
        {
            return std::experimental::suspend_never();
        }

        template<typename U>
        void return_value(U&& v)
        {
            m_promise.set_value(std::forward<U>(v));
        }

        void set_exception(std::exception_ptr exc)
        {
            m_promise.set_exception(exc);
        }
    };
};


} // namespace experimental
} // namespace std

std::future<int> computeInBackground(my_f)
{
    co_return 5;
}

void funma()
{
    auto f = computeInBackground(my_f());
    auto v = f.get();
    std::cout << "Value: " << v << std::endl;
}
