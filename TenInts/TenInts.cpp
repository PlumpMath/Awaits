#include "DebugStream.h"

#include <iostream>
#include <experimental/generator>
#include <experimental/resumable>

using namespace std;
using namespace std::experimental;

generator<int> tenInts()
{
	cout << "Start" << endl;

	for (int i = 0; i < 3; ++i)
	{
		cout << "Yield: " << i << endl;
		co_yield i;
	}

	cout << "End" << endl;
}

int main()
{
	FreeConsole();
	static OutputDebugStringBuf<char> charDebugOutput;
	std::cout.rdbuf(&charDebugOutput);

	auto ints = tenInts();
	for (auto i : ints)
	{
		std::cout << "i = " << i << endl;
	}
}
