#include <cstdint>
#include <iostream>
#include <future>
#include <string>

#include <experimental/generator>
#include <experimental/resumable>

using namespace std;
using namespace std::experimental;


my_generator<int> produceValues()
{
    ...
        co_yield 1;
    ...
        co_yield 2;
    ...
        co_yield 3;
    ...
}

void stackless(const int& a)
{
    ...
        suspend
        ...
        a = 5;
}

void helper()
{
    suspend
}

void stackful(const int& a)
{
    ...
        helper()
        ...
        a = 5;
}

struct File;

void verifyDone(void* userData)
{
    std::cout << "File ok!";
}

int writeDone(void* userData)
{
    auto file = (File*)userData;
    file.asyncVerify(verifyDone, file);
}

void writeAndVerify(uint8_t* buf)
{
    auto file = new File("file.txt");
    file.asyncWrite(buf, writeDone, file);
}

void writeAndVerify(uint8_t* buf)
{
    auto file = new File("file.txt");
    await file.write(buf);
    await file.verify(file);
    std::cout << "File ok!";
}


generator<int> tenInts()
{
    cout << "Start";
    for (int i = 0; i < 10; ++i)
    {
        cout << "Next: " << i;
        co_yield i;
    }
    cout << "End";
}

int main()
{
    for (auto i : tenInts())
        std::cout << i;
}

int main()
{
    for (auto i : tenInts())
    {
        std::cout << i;
        ...
            ...
    }
}

// Output: { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }

struct path;

future<path> cacheUrl(string url);

// Downloads url to cache and
// returns cache file path.
future<path> cacheUrl(string url)
{
    cout << "Downloading url.";
    string text = co_await downloadAsync(url);

    cout << "Saving in cache.";
    path p = randomFileName();
    co_await saveInCacheAsync(p, text);

    return p;
}

int main()
{
    auto f = writeAndVerify(buf);
    ...
        ...
        ...
        f.wait();
}


future<void> writeAndVerify(uint8_t* buf)
{
    auto file = new File("file.txt");

    co_await file.asyncWrite(buf);

    std::cout << "Written!";

    co_await file.asyncVerify();

    std::cout << "Verified!";
}

future<void> File::asyncWrite(uint8_t* buf)
{
    ...
}

future<void> File::asyncVerify()
{
    ...
}


future<int> compute()
{
    ...
}

future<int> compute()
{
    auto context = new ComputeContext();
    auto& promise = context->get_promise();
    __return = promise.get_return_object();

    co_await promise.initial_suspend();

    ...

        final_suspend:
    co_await promise.final_suspend();
}


future<int> compute()
{
    co_return 5;
}

future<int> compute()
{
    ...
        __return = promise.get_return_object();

    co_await promise.initial_suspend();

    promise.return_value(5);
    goto final_suspend;

final_suspend:
    co_await promise.final_suspend();
}


future<int> compute()
{
    int v = co_await think();
    ...
}

future<int> compute()
{
    ...


        auto&& a = think();
    if (!await_ready(a))
    {

        await_suspend(a, coroutine_handle);

        suspend
    }

    int v = await_resume(a);

    ...
}



int tcp_reader(int total)
{
    char buf[4 * 1024];
    auto conn = Tcp::Connect("127.0.0.1", 1337);
    while (true)
    {
        auto bytesRead = conn.Read(buf, sizeof(buf));
        total -= bytesRead;
        if (total <= 0 || bytesRead == 0) return total;
    }
}

future<int> tcp_reader(int total)
{
    char buf[4 * 1024];
    auto conn = co_await Tcp::Connect("127.0.0.1", 1337);
    while (true)
    {
        auto bytesRead = co_await conn.Read(buf, sizeof(buf));
        total -= bytesRead;
        if (total <= 0 || bytesRead == 0) co_return total;
    }
}

future<int> tcp_reader(int64_t total) {
    struct State {
        char buf[4 * 1024];
        int64_t total;
        Tcp::Connection conn;
        explicit State(int64_t total) : total(total) {}
    };
    auto state = make_shared<State>(total);
    return Tcp::Connect("127.0.0.1", 1337).then(
        [state](future<Tcp::Connection> conn) {
        state->conn = std::move(conn.get());
        return do_while([state]()->future<bool> {
            if (state->total <= 0) return make_ready_future(false);
            return state->conn.read(state->buf, sizeof(state->buf)).then(
                [state](future<int> nBytesFut) {
                auto nBytes = nBytesFut.get()
                    if (nBytes == 0) return make_ready_future(false);
                state->total -= nBytes;
                return make_ready_future(true);
            }); // read
        }); // do_while
    }).then([state](future<void>) {return make_ready_future(state->total)});
}

struct MyType {};
struct Awaitable {};
struct WrapperAwaitable {};

struct promise_type
{
    MyType get_return_object()
    {
        return MyType(coroutine_handle<promise_type>::from_promise(*this));
    }

    auto initial_suspend()
    {
        return std::experimental::suspend_never();
    }

    auto final_suspend()
    {
        return std::experimental::suspend_always();
    }

    void return_value(int v);

    void return_void();

    void set_exception(std::exception_ptr exc);

    void yield_value(int i);

    WrapperAwaitable await_transform(Awaitable a)
    {
        return WrapperAwaitable(a);
    }
};


struct coroutine_handle<void>
{
    void resume() const;

    void destroy();

    bool done() const;
};

template <typename Promise>
struct coroutine_handle : coroutine_handle<void>
{
    Promise& promise();
    static coroutine_handle from_promise(Promise& promise);
};

template <typename Promise>
struct coroutine_handle
{
    void resume() const;

    void destroy();

    bool done() const;

    Promise& promise();

    static coroutine_handle from_promise(Promise& promise);
};

generator<int> get_return_object()
{
    return generator<int>(coroutine_handle<promise_type>::from_promise(*this));
}

struct T;

bool await_ready(MyType& val);

void await_suspend(MyType& val, coroutine_handle<> ch);

T await_resume(MyType& val);


bool await_ready(future<T>& f)
{
    return f.is_ready();
}

void await_suspend(future<T>& f, std::experimental::coroutine_handle<> coro)
{
    f.then([coro](const future<T>& f) { coro.resume(); });
}

T await_resume(const future<T>& f)
{
    return f.get();
}

template<typename T>
struct coroutine_traits<future<T>>
{
    struct promise_type
    {
        promise<T> promise;

        future<T> get_return_object()
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

        void return_value(T v)
        {
            promise.set_value(v);
        }

        void set_exception(std::exception_ptr exc)
        {
            promise.set_exception(exc);
        }
    };
};

void fun()
{
co_await suspend_never;

co_await suspend_always;
}

template<typename T>
struct my_generator
{
    const T* getNext();
};

my_generator<int> years()
{
    for (int i = 2010; i <= 2015; ++i)
        co_yield i;
}

int main()
{
    auto gen = years();
    while (auto i = gen.getNext())
        std::cout << *i << std::endl;
}


template<typename T>
struct my_generator
{
    struct promise_type { ... };

    coroutine_handle<promise_type> m_coroutine = nullptr;

    explicit my_generator(coroutine_handle<promise_type> coroutine)
        : m_coroutine(coroutine)
    {}

    ~my_generator()
    {
        if (m_coroutine)
            m_coroutine.destroy();
    }

    const T* getNext()
    {
        m_coroutine.resume();
        if (m_coroutine.done())
        {
            m_coroutine = nullptr;
            return nullptr;
        }

        return m_coroutine.promise().value;
    }
};


struct my_generator;

struct promise_type
{
    my_generator get_return_object()
    {
        return my_generator(coroutine_handle<promise_type>::from_promise(*this));
    }

    const T* value;

    void yield_value(const T& v)
    {
        value = &v;
    }

    auto initial_suspend()
    {
        return std::experimental::suspend_always();
    }

    auto final_suspend()
    {
        return std::experimental::suspend_always();
    }
};

#include <vector>

generator<int> produce()
{
    vector<int> vec{ 1, 2, 3 };
    for (auto i : vec)
        co_yield i;
}

generator<int> years()
{
    for (int i = 2010; i <= 2015; ++i)
        co_yield i;
}

generator<int> more_years()
{
    co_yield 2009;

    co_yield years();

    co_yield 2016;
}

generator<int> years()
{
    int start = co_await get_year();
    for (int i = start; i <= 2025; ++i)
        co_yield i;
}

generator<int> years()
{
    int start = get_year().get();
    for (int i = start; i <= 2025; ++i)
        co_yield i;
}

template<typename T>
struct async_generator;

async_generator<int> squares(async_generator<int> gen)
{
    for co_await (auto v : gen)
        co_yield v * v;
}

template <typename _Uty>
_Uty && await_transform(_Uty &&_Whatever)
{
    static_assert(_Always_false<_Uty>::value,
                  "co_await is not supported in coroutines of type std::experiemental::generator");
    return _STD forward<_Uty>(_Whatever);
}
