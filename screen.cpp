#include "screen.hpp"

Screen::Screen()
    : tick([]{})
    , draw([]{})
    , enter([]{})
    , leave([]{})
    , opaque(false)
    , freeze(false)
{}

Screen::Screen(const Screen& in)
    : tick(in.tick)
    , draw(in.draw)
    , enter(in.enter)
    , leave(in.leave)
    , opaque(in.opaque)
    , freeze(in.freeze)
{}
