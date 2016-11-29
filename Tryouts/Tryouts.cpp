
#include <iostream>
#include <experimental/generator>
#include <experimental/resumable>
#include <experimental/generator>
#include <future>

using namespace std;
using namespace std::experimental;

struct coro_ret {};

namespace std
{
namespace experimental
{

template<>
struct coroutine_traits<coro_ret>
{
	struct promise_type
	{
		coro_ret get_return_object()
		{
			return {};
		}

		auto initial_suspend()
		{
			return std::experimental::suspend_never();
		}

		auto final_suspend()
		{
			return std::experimental::suspend_always();
		}

		void yield_value(int i)
		{
		}
#if 1
		template<typename U>
		void return_value(U&& v)
		{
		}
#endif
#if 0
		void return_void()
		{
		}
#endif
		void set_exception(std::exception_ptr exc)
		{
		}
	};
};

} // namespace experimental
} // namespace std

coro_ret someInts()
{
	cout << "Start" << endl;

	for (int i = 0; i < 3; ++i)
	{
		cout << "Yield: " << i << endl;
		co_yield i;
	}

	cout << "End" << endl;

	co_await future<int>();
	co_return 5;
	//co_return;
}

generator<int> tenInts()
{
	cout << "Start" << endl;

	for (int i = 0; i < 3; ++i)
	{
		cout << "Yield: " << i << endl;
		co_yield i;
	}

	cout << "End" << endl;

	//co_await future<int>();
	//co_return 5;
}

int main()
{
	auto ints = tenInts();
	for (auto i : ints)
	{
		std::cout << "i = " << i << endl;
	}
}
