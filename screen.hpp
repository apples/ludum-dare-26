#ifndef SCREEN_H
#define SCREEN_H

#include <functional>

class Screen
{
public:
    Screen();
    Screen(const Screen& in);

    std::function<void()> tick;
    std::function<void()> draw;
    std::function<void()> enter;
    std::function<void()> leave;

    bool opaque;
    bool freeze;
};

#endif // SCREEN_H
