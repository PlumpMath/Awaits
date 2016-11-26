#include <cstdint>
#include <iostream>
#include <future>
#include <string>

#include <experimental/generator>
#include <experimental/resumable>

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>

using namespace std;
using namespace std::experimental;

namespace SimpleGenerator
{

template<typename T>
struct my_generator
{
    struct promise_type
    {
        my_generator get_return_object()
        {
            return my_generator(coroutine_handle<promise_type>::from_promise(*this));
        }

        auto initial_suspend()
        {
            return std::experimental::suspend_always();
        }

        auto final_suspend()
        {
            return std::experimental::suspend_always(); // this is critical to not delete value before it's used; see "An Introduction to C++ Coroutines - James McNellis - Meeting C++ 2015 " https://www.youtube.com/watch?v=YYtzQ355_Co 30m
        }

        void yield_value(const my_generator<T>& v)
        {
            assert(!gen);
            assert(!value);
            gen = std::addressof(v);
        }

        void yield_value(const T& v)
        {
            assert(!gen);
            assert(!value);
            value = std::addressof(v);
        }

        mutable boost::optional<const T*> value;
        mutable boost::optional<const my_generator<T>*> gen;
    };

    coroutine_handle<promise_type> m_coroutine = nullptr;

    explicit my_generator(coroutine_handle<promise_type> coroutine)
        : m_coroutine(coroutine)
    {}

    my_generator() = default;
    my_generator(const my_generator&) = delete;
    my_generator& operator=(const my_generator&) = delete;

    my_generator(my_generator&& other)
        : m_coroutine(other.m_coroutine)
    {
        other.m_coroutine = nullptr;
    }

    my_generator& operator=(my_generator&& other)
    {
        if (this != &other)
        {
            m_coroutine = other.m_coroutine;
            other.m_coroutine = nullptr;
        }
    }

    ~my_generator()
    {
        if (m_coroutine)
            m_coroutine.destroy();
    }

    boost::optional<T> getNext() const
    {
        do {
            if (m_coroutine.promise().gen)
            {
                auto val = m_coroutine.promise().gen.get()->getNext();
                if (val)
                    return val;

                m_coroutine.promise().gen.reset();
            }

            m_coroutine.resume();
            if (m_coroutine.done())
                return boost::none;

        } while(m_coroutine.promise().gen);

        auto val = m_coroutine.promise().value;
        m_coroutine.promise().value.reset();
        return *val.get();
    }
};

my_generator<int> years()
{
    for (int i = 2010; i <= 2015; ++i)
        co_yield i;
}

my_generator<int> years2()
{
    for (int i = 2017; i <= 2018; ++i)
        co_yield i;
    co_yield years();
}

}

void funga()
{
    auto gen = SimpleGenerator::years2();
    while (auto i = gen.getNext())
        std::cout << i << std::endl;
}
