#pragma once
#include <raylib.h>
#include <raymath.h>
#include <vector>

class Robot{
public:
    Color robotColor; 
    Color noseColor; 
    Vector2 robotPosition; 
    float robotRadius; 
    float noseRadius;
    float robotSpeed; 
    float robotAngleRadians; 
    float avoidanceDistance;
    float maxTurnSpeed;
    bool recovering;
    int recoveryDirection;

    float sensorRange; 
    std::vector<int> sensorAngleOffsets;
    std::vector<float> sensorDistances;
    std::vector<Vector2> sensorHitPoints;

    Robot(Color robotColor, Color noseColor, float robotRadius, float noseRadius, Vector2 robotPosition, float robotAngleDegrees);

    void draw();

    void drawSensors();

    void updateSensors(const std::vector<Rectangle>& walls, int screenWidth, int screenHeight);

    void avoidObstacles(float deltaTime);

    void steerTowardTarget(Vector2 targetPosition, float deltaTime);

    void move(float deltaTime, float screenWidth, float screenHeight, const std::vector<Rectangle> &walls);

    void collisionHandling(const std::vector<Rectangle> &walls, Vector2 oldPosition);

    float getRightAverage();

    float getLeftAverage();

    void readSensors();

};