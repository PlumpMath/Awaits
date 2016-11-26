#pragma once

#include <SFML/Graphics.hpp>

#include <vector>

extern std::vector<int> guyWalkLeft;
extern std::vector<int> guyWalkRight;
extern std::vector<int> guyStandLeft;
extern std::vector<int> guyStandRight;

struct Guy
{
    Guy(sf::Vector2f position)
        : position(position)
        , walking(false)
        , facingLeft(false)
        , animFrame(0)
    {}

    sf::Vector2f position;
    bool walking;
    bool facingLeft;
    int animFrame;
    std::string talk;

    const std::vector<int>& getFrames() const
    {
        if (walking)
            return facingLeft ? guyWalkLeft : guyWalkRight;
        else
            return facingLeft ? guyStandLeft : guyStandRight;
    }

    void advanceFrame()
    {
        const auto& frames = getFrames();
        animFrame++;
        animFrame %= frames.size();
    }
};
