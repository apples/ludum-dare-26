#ifndef ENTITYTIMER_H
#define ENTITYTIMER_H

#include "entity.hpp"

#include <functional>

class EntityTimer : public Entity
{
public:
    EntityTimer(int t, std::function<void()> f);
    virtual ~EntityTimer();

    virtual void tick() override;

    virtual bool isDead() const override;

private:
    std::function<void()> func;
    int timer;
};

#endif // ENTITYTIMER_H
