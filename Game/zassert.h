#pragma once

inline void zassert(bool cond)
{
    if (!cond)
        throw "Assertion failed.";
}
