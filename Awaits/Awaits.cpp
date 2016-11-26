
#include <iostream>
#include <thread>
#include <future>

#include <experimental/generator>
#include <experimental/resumable>

#include <boost/optional.hpp>

using namespace std::experimental;

//template<typename... T>
//using coroutine_handle = std::experimental::coroutine_handle<T...>;

template<typename T>
struct resumable_optional
{
    struct promise_type
    {
        resumable_optional get_return_object()
        {
            return resumable_optional(coroutine_handle<promise_type>::from_promise(*this));
        }

        auto initial_suspend()
        {
            return std::experimental::suspend_never();
        }

        auto final_suspend()
        {
            return std::experimental::suspend_always(); // this is critical to not delete value before it's used; see "An Introduction to C++ Coroutines - James McNellis - Meeting C++ 2015 " https://www.youtube.com/watch?v=YYtzQ355_Co 30m
        }

        void return_value(boost::none_t)
        {
        }

        void return_value(T v)
        {
            value = v;
        }

        boost::optional<T> value;
    };

    coroutine_handle<promise_type> m_coroutine = nullptr;

    explicit resumable_optional(coroutine_handle<promise_type> coroutine)
        : m_coroutine(coroutine)
    {}

    resumable_optional() = default;
    resumable_optional(const resumable_optional&) = delete;
    resumable_optional& operator=(const resumable_optional&) = delete;

    resumable_optional(resumable_optional&& other)
        : m_coroutine(other.m_coroutine)
    {
        other.m_coroutine = nullptr;
    }

    resumable_optional& operator=(resumable_optional&& other)
    {
        if (this != &other)
        {
            m_coroutine = other.m_coroutine;
            other.m_coroutine = nullptr;
        }
    }

    ~resumable_optional()
    {
        if (m_coroutine)
            m_coroutine.destroy();
    }

    bool await_ready()
    {
        return true;
    }

    void await_suspend(coroutine_handle<> ch)
    {
    }

    T await_resume() const
    {
        m_coroutine.resume();
        return get();
    }

    T get() const
    {
        auto val = m_coroutine.promise().value;
        if (val)
            return *val;
        throw "Koza";
    }
};

namespace std
{
namespace experimental
{

/// await_transform:
/// We would like to provide an ability for the coroutine type author to specify an await_transform member in the promise_type of the coroutine.
/// If present, every await expr in that coroutine would be as if it was await $promise.await_transform(expr).
/// Dzięki temu można robić transformacje takie jak:
///     auto bytesRead = await conn.Read(buf, len);
/// kompilator zamieni na:
///     auto bytesRead = await CheckCancel(cancelToken, conn.Read(buf, len));
/// Gdy await transform to:
///     auto await_transform(T&& awaitable)
///     { return CheckCancel(cancelToken, awaitable); }
/// To pytanie: skąd promise ma znać cancel token?
/// Todo: Spróbować takie coś napisać.

/// Todo: napisać tracer na podstawie await_transform taj jak to jest napisane tutaj:
///    http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0054r0.html

/// Obiekt zwrócony z funkcji musi mieć jakiś kontakt z promisem.
/// Dla future jest to kontakt niebezpośredni i nie kontroluje wznawiania funkcji - tylko czeka na ustawienie ostatecznego wyniku.
/// Dla generatorów jest to bezpośrednie wznawianie funkcji w celu odebrania pośrednich wyników.

/// coroutine_traits has extra arguments for extra customization, see https://www.reddit.com/r/cpp/comments/4ctj2a/visual_studio_2015_update_2_co_awaitco_return/
/// The customization takes type parameters from coroutine function parameters. See http://open-std.org/JTC1/SC22/WG21/docs/papers/2016/p0057r3.pdf 8.4.4.3
/// Por. await_transform
template<typename T>
struct coroutine_traits<boost::optional<T>>
{
    struct promise_type
    {
        boost::optional<T> value;
        boost::optional<T> get_return_object()
        {
            return value; // won't work (always return empty optional), because there is no way await_resume() can communicate through value to this promise! coroutine_handle is needed (or something else?)
        }

        auto initial_suspend()
        {
            return std::experimental::suspend_never();
        }

        auto final_suspend()
        {
            return std::experimental::suspend_always(); // this is critical to not delete value before it's used; see "An Introduction to C++ Coroutines - James McNellis - Meeting C++ 2015 " https://www.youtube.com/watch?v=YYtzQ355_Co 30m
        }

        template<typename U>
        void return_value(U&& v)
        {
            value = std::forward<U>(v);
        }

#if 0
        void set_exception(std::exception_ptr exc)
        {
            // we don't really need that for optionals
        }
#endif
    };
};

} // namespace experimental
} // namespace std

/// Dwa całkowicie oddzielne aspekty:
///  - możliwość poczekania na wartość (await_ready/await_suspend/await_resume)
///  - możliwość zwrócenia wartości z funkcji wznawialnej (promise_type / corutine_traits).

template<typename T>
bool await_ready(boost::optional<T>& val)
{
    return true;
}

template<typename T>
void await_suspend(boost::optional<T>& val, coroutine_handle<> ch)
{
}

template<typename T>
T await_resume(boost::optional<T>& val)
{
    if (val)
        return *val;

    throw "None!";
}


std::future<int> make_ready_fut()
{
    co_return 5;
}

std::future<int> fint()
{
    return std::async(std::launch::async, [](){
        return 20;
    });
}

std::future<int> many()
{
    auto x = co_await fint();
    std::cout << x << std::endl;
    co_return x;
}

resumable_optional<int> ropt()
{
    co_return 10;
}

resumable_optional<int> ropto()
{
    auto x = co_await ropt();
    co_return x;
}

boost::optional<int> bopt()
{
    co_return 5;
}

boost::optional<int> bopto()
{
    auto x = bopt();
    co_return x;
}

/// Todo: "for await"

/// Todo: napisać generatory, które nie mają poniższego ograniczenia (co_await i generatory w środku mają działać).
/// Recursive generators: page 6 of https://isocpp.org/files/papers/N4402.pdf
/// See also: "undo yet another "simplification" from N4402" in http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0054r0.html
generator<int> ints()
{
    //auto x = co_await ropt();   // co_await is not supported in coroutines of type std::experiemental::generator - this is probably for performance reasons, see https://blogs.msdn.microsoft.com/vcblog/2014/11/12/resumable-functions-in-c/ (std::experimental::generator<T> does not support recursive invocations)
                                  // because as it is now every resume yeields another value, otherwise we would need to store flag that says whether yield was called (and so new value is ready)
                                  // oprócz tego możliwe by były równoległe yieldy
    for (int i = 0; i < 10; ++i)
        co_yield i;
}

generator<int> ints2()
{
    for (auto i : ints())
        co_yield i;
}

/// @todo:
///future<int> deep_thought() {
///    await 7'500'000'000h;
///    return 42;
///}

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

struct ala
{
    /// See: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0054r0.html
    auto operator await()
    {
        return std::experimental::suspend_always();
    }
};

/// See: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0054r0.html
/// Moreover, even though coroutines allow asynchrous code to be written nearly as simple as synchronous, they do not eliminate the need to think about and properly design the lifetime of the asynchronous activity.
/// Const-ref parameters const& that are perfectly fine to consume in a normal function may result in a crash, information disclosure and more if the function is a coroutine which lifetime extends beyond the
/// lifetime of the object bound to that const& parameter.

/// There are competitive proposals: stackful coroutines (fibers), resumable expressions, etc...



std::future<void> ala()
{
    await ala();
}

void funma();
void funga();

void main()
{
    try
    {
        funga();
        funma();
        return;

        for (auto i : ints())
            std::cout << "i = " << i << std::endl;

        auto gen = years2();
        for (auto v = gen.getNext(); v; v = gen.getNext())
        {
            std::cout << "year = " << *v << std::endl;
        }

        auto y = ropt().get();
        auto x = bopt().get();
        //auto y = ropto().get();
        //auto x = bopto().get();
        auto z = make_ready_fut().get();
        auto w = many().get();
    }
    catch (...)
    {
        std::cout << "Exception." << std::endl;
    }
    std::cout << "Bye." << std::endl;
}
