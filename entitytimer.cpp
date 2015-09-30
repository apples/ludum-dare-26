#include "entitytimer.hpp"

EntityTimer::EntityTimer(int t, std::function<void()> f)
    : timer(t)
    , func(f)
{}

EntityTimer::~EntityTimer()
{}

void EntityTimer::tick()
{
    if (!isDead()) if (--timer <= 0) func();
}

bool EntityTimer::isDead() const
{
    return (timer<=0);
}
