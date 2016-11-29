
#include <experimental/generator>
#include <experimental/resumable>
#include <future>

using namespace std;
using namespace std::experimental;

future<void> suspendOnce()
{
	co_await suspend_never{};
	co_await suspend_always{};
}

generator<int> generate()
{
	co_yield 1;
}

