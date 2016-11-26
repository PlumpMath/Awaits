#pragma once

#include <iostream>
#include <chrono>
#include <unordered_map>

#include <experimental/resumable>

extern int currentFrame;
extern std::unordered_multimap<int, std::experimental::coroutine_handle<> > thingsToDo;

struct wait_few_frames
{
    int numFrames;
    wait_few_frames(int numFrames) : numFrames(numFrames) {}

    bool await_ready() const
    {
        return numFrames == 0;
    }

    void await_suspend(std::experimental::coroutine_handle<> coro) const
    {
        thingsToDo.insert(std::make_pair(currentFrame + numFrames, coro));
    }

    void await_resume() const
    {
    }
};

template<class Rep, class Period>
inline wait_few_frames wait_some_time(const std::chrono::duration<Rep, Period>& time)
{
    using namespace std::chrono_literals;
    auto frames = static_cast<int>(time / 16ms);
    return wait_few_frames(frames);
}

inline void executeOneFrame()
{
    while (!thingsToDo.empty())
    {
        auto cur = thingsToDo.find(currentFrame);
        if (cur == thingsToDo.end())
        {
            break;
        }

        auto coro = cur->second;
        thingsToDo.erase(cur);
        coro.resume();
    }

    currentFrame++;
}
