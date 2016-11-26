#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION

//#include <future>
//#include <experimental/future>
//#include <boost/thread/future.hpp>

#include <experimental/resumable>

#include <iostream>
#include <chrono>
#include <unordered_map>
#include <thread>

#include "sfuture.h"

using namespace std::experimental;
using namespace std::chrono_literals;

int currentFrame = 0;
std::unordered_multimap<int, coroutine_handle<> > thingsToDo;

struct wait_few_frames
{
    int numFrames;
    wait_few_frames(int numFrames) : numFrames(numFrames) {}

    bool await_ready() const
    {
        return numFrames == 0;
    }

    void await_suspend(coroutine_handle<> coro) const
    {
        thingsToDo.insert(std::make_pair(currentFrame + numFrames, coro));
    }

    void await_resume() const
    {
    }
};

#if 0
namespace std
{
namespace experimental
{

template<typename T>
struct coroutine_traits<boost::future<T>>
{
    struct promise_type
    {
        boost::promise<T> promise;
        boost::future<T> get_return_object()
        {
            return promise.get_future();
        }

        auto initial_suspend()
        {
            return std::experimental::suspend_never();
        }

        auto final_suspend()
        {
            return std::experimental::suspend_never();
        }

#if 0
        template<typename U>
        void return_value(U&& v)
        {
            promise.set_value(std::forward<U>(v));
        }
#endif
        void return_void()
        {
            promise.set_value();
        }

        void set_exception(std::exception_ptr exc)
        {
            promise.set_exception(exc);
        }
    };
};

} // namespace experimental
} // namespace std

namespace boost
{

template<typename T>
bool await_ready(boost::future<T>& f)
{
    return f.is_ready();
}

template<typename T>
void await_suspend(boost::future<T>& f, coroutine_handle<> coro)
{
    f.then([coro](const boost::future<T>& f) { coro.resume(); });
}

#if 0
template<typename T>
T await_resume(const boost::future<T>& f)
{
    return f.get();
}
#endif

auto await_resume(boost::future<void>& f)
{
    f.wait();
}

} // namespace boost

#endif

sfuture<void> thinkDeeply()
{
    std::cout << "I'm thinking deeply." << std::endl;
    co_await wait_few_frames(2);
}


sfuture<void> think()
{
    std::cout << "I'm thinking." << std::endl;
    co_await wait_few_frames(2);
    
    co_await thinkDeeply();

    std::cout << "Eureka!" << std::endl;
    co_await wait_few_frames(1);
    std::cout << "Finished thinking." << std::endl;
}

int main()
{
    auto f = think();

    while (!thingsToDo.empty())
    {
        auto cur = thingsToDo.find(currentFrame);
        if (cur == thingsToDo.end())
        {
            std::this_thread::sleep_for(1s);
            currentFrame++;
            std::cout << "Frame: " << currentFrame << std::endl;
            continue;
        }

        auto coro = cur->second;
        coro.resume();
        thingsToDo.erase(cur);
    }

    std::cout << "Done." << std::endl;
    f.get();
}