#pragma once

#include "zassert.h"

#include <exception>
#include <functional>
#include <memory>
#include <experimental/resumable>

#include <boost/optional.hpp>

template<typename T>
class sfuture;

template<typename T>
class sstate
{
public:
    bool empty()
    {
        return !value && !exception;
    }

public:
    boost::optional<T> value;
    std::exception_ptr exception;
    std::function<void(const sfuture<T>& fut)> continuation;
};

template<>
class sstate<void>
{
public:
    sstate()
        : value(false)
    {}

    bool empty()
    {
        return !value && !exception;
    }

public:
    bool value;
    std::exception_ptr exception;
    std::function<void(const sfuture<void>& fut)> continuation;
};

template<typename T>
class sfuture
{
private:
    template<typename U>
    friend class spromise;

    sfuture(std::shared_ptr<sstate<T>> state)
        : state(state)
    {
    }

public:
    bool is_ready() const
    {
        return !state->empty();
    }

    T get()
    {
        zassert(is_ready());
        if (state->exception)
            std::rethrow_exception(state->exception);
        return *state->value;
    }

    template<typename F>
    void then(F&& f)
    {
        zassert(!state->continuation);
        state->continuation = std::forward<F>(f);
    }

private:
    std::shared_ptr<sstate<T>> state;
};

template<>
class sfuture<void>
{
private:
    template<typename U>
    friend class spromise;

    sfuture(std::shared_ptr<sstate<void>> state)
        : state(state)
    {
    }

public:
    bool is_ready() const
    {
        return !state->empty();
    }

    void get()
    {
        zassert(is_ready());
        if (state->exception)
            std::rethrow_exception(state->exception);
    }

    void wait()
    {
        get();
    }

    template<typename F>
    void then(F&& f)
    {
        zassert(!state->continuation);
        state->continuation = std::forward<F>(f);
    }

private:
    std::shared_ptr<sstate<void>> state;
};

template<typename T>
class spromise
{
public:
    spromise()
        : state(std::make_shared<sstate<T>>())
    {}

    sfuture<T> get_future()
    {
        return sfuture<T>(state);
    }

    void set_value(const T& v)
    {
        zassert(state->empty());
        state->value = v;

        if (state->continuation)
        {
            sfuture<T> fut(state);
            state->continuation(fut);
        }
    }

    void set_exception(std::exception_ptr exc)
    {
        zassert(state->empty());
        state->exception = exc;

        if (state->continuation)
        {
            sfuture<T> fut(state);
            state->continuation(fut);
        }
    }

private:
    std::shared_ptr<sstate<T>> state;
};


template<>
class spromise<void>
{
public:
    spromise()
        : state(std::make_shared<sstate<void>>())
    {}

    sfuture<void> get_future()
    {
        return sfuture<void>(state);
    }

    void set_value()
    {
        zassert(state->empty());
        state->value = true;

        if (state->continuation)
        {
            sfuture<void> fut(state);
            state->continuation(fut);
        }
    }

     void set_exception(std::exception_ptr exc)
    {
        zassert(state->empty());
        state->exception = exc;

        if (state->continuation)
        {
            sfuture<void> fut(state);
            state->continuation(fut);
        }
    }

private:
    std::shared_ptr<sstate<void>> state;
};

namespace std
{
    namespace experimental
    {

        template<typename T, typename... Ts>
        struct coroutine_traits<sfuture<T>, Ts...>
        {
            struct promise_type
            {
                spromise<T> promise;
                sfuture<T> get_return_object()
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

                template<typename U>
                void return_value(U&& v)
                {
                    promise.set_value(std::forward<U>(v));
                }

                void set_exception(std::exception_ptr exc)
                {
                    promise.set_exception(exc);
                }
            };
        };

        template<typename... Ts>
        struct coroutine_traits<sfuture<void>, Ts...>
        {
            struct promise_type
            {
                spromise<void> promise;
                sfuture<void> get_return_object()
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

template<typename T>
bool await_ready(sfuture<T>& f)
{
    return f.is_ready();
}

template<typename T>
void await_suspend(sfuture<T>& f, std::experimental::coroutine_handle<> coro)
{
    f.then([coro](const sfuture<T>& f) { coro.resume(); });
}

template<typename T>
T await_resume(const sfuture<T>& f)
{
    return f.get();
}

auto await_resume(sfuture<void>& f)
{
    f.get();
}
