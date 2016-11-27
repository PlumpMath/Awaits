#include "../TenInts/DebugStream.h"

#include <iostream>
#include <experimental/generator>
#include <experimental/resumable>

#include <cstdint>
#include <iostream>
#include <future>
#include <string>

using namespace std;
using namespace std::experimental;


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
			return std::experimental::suspend_always(); // this is critical to not delete value before it's used
		}

		void yield_value(const T& v)
		{
			value = &v;
		}

		const T* value;
	};

	coroutine_handle<promise_type> m_coroutine = nullptr;

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
};

my_generator<int> years()
{
	for (int i = 2010; i <= 2011; ++i)
		co_yield i;
}

int main()
{
	FreeConsole();
	static OutputDebugStringBuf<char> charDebugOutput;
	std::cout.rdbuf(&charDebugOutput);

	auto gen = years();
	while (auto i = gen.getNext())
		std::cout << *i << std::endl;
}
