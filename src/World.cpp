#include "World.h"

World::World(float screenWidth, float screenHeight, Color wallsColor){
    this->screenWidth = screenWidth;
    this->screenHeight= screenHeight;
    this->wallsColor = wallsColor;
    walls =  {
        {50, 50, 100, 500},
        {50, 50, 300, 100},
        {450, 360, 200, 250},
        {587, 29, 70, 200},
        {300, 710, 300, 80}};
}

void World::draw(){
    for (int i = 0; i < walls.size(); i++)
        {
            DrawRectangleRec(walls[i], wallsColor);
        }
}