
#include "ScriptEngine.h"

int currentFrame = 0;
std::unordered_multimap<int, std::experimental::coroutine_handle<> > thingsToDo;

