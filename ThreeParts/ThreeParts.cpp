#include "../TenInts/DebugStream.h"

#include <iostream>
#include <experimental/resumable>

using namespace std;
using namespace std::experimental;

struct PrintOnDestroy
{
    ~PrintOnDestroy() { cout << "~PrintOnDestroy" << endl; }
};

struct Coro
{
    coroutine_handle<> m_coro;
    Coro(coroutine_handle<> coro) : m_coro(coro) {}

    Coro(const Coro&) = delete;
    Coro& operator=(const Coro&) = delete;
    Coro(Coro&& other) : m_coro(std::move(other.m_coro)) {}

    struct promise_type
    {
        Coro get_return_object()
        {
            return Coro(coroutine_handle<promise_type>::from_promise(*this));
        }

        ~promise_type() { cout << "~promise_type" << endl; }

        auto initial_suspend() { return false; }
        auto final_suspend() { return true; }
        void return_void() {}
    };
};

Coro threeParts()
{
    PrintOnDestroy p;

    cout << "First part: immediately!" << endl;
    co_await suspend_always{};

    cout << "After resume." << endl;
    co_await suspend_always{};

    cout << "After second resume." << endl;
}

int main()
{
    FreeConsole();
    static OutputDebugStringBuf<char> charDebugOutput;
    std::cout.rdbuf(&charDebugOutput);

    Coro coro = threeParts();

    coro.m_coro.resume();
    coro.m_coro.resume();
    cout << "Done? " << coro.m_coro.done() << endl;
    coro.m_coro.destroy();
}
