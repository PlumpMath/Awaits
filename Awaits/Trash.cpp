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




