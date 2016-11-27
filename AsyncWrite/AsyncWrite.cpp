#include "../TenInts/DebugStream.h"

#include <iostream>
#include <experimental/generator>
#include <experimental/resumable>
#include <future>
#include <thread>

using namespace std;
using namespace std::experimental;
using namespace std::literals::chrono_literals;

struct File
{
	File(const char*) {}

	future<void> asyncWrite(const char* buf)
	{
		return async(std::launch::async, [buf] {
			cout << "Writing..." << buf << endl;
			this_thread::sleep_for(3s);
			cout << "Writing done." << endl;
		});
	}

	future<void> asyncVerify()
	{
		return async(std::launch::async, [] {
			cout << "Verifying..." << endl;
			this_thread::sleep_for(3s);
			cout << "Verification done." << endl;
		});
	}
};

future<void> writeAndVerifyCoro(const char* buf)
{
	File file("file.txt");
	co_await file.asyncWrite(buf);
	co_await file.asyncVerify();
	cout << "File ok!" << endl;
}

int main()
{
	FreeConsole();
	static OutputDebugStringBuf<char> charDebugOutput;
	std::cout.rdbuf(&charDebugOutput);

	auto fut = writeAndVerifyCoro("Stuff");
	fut.wait();
}
