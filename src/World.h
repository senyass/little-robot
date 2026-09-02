#pragma once
#include <raylib.h>
#include <vector>


class World{
public:
    float screenWidth;
    float screenHeight;
    std::vector<Rectangle> walls;
    Color wallsColor;

    World(float screenWidth, float screenHeight, Color wallsColor);

    void draw();
};