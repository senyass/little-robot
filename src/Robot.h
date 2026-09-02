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


    // ==================== Constructor ====================

    Robot(Color robotColor, Color noseColor, float robotRadius, float noseRadius, Vector2 robotPosition, float robotAngleDegrees);


    // ==================== Drawing ====================

    void draw();

    void drawSensors();

    void readSensors();


    // ==================== Sensor Helpers ====================

    float getLeftAverage();

    float getRightAverage();


    // ==================== Sensor Update ====================

    void updateSensors(const std::vector<Rectangle>& walls, int screenWidth, int screenHeight);


    // ==================== Steering ====================

    float avoidObstacles(float deltaTime);

    float steerTowardTarget(Vector2 targetPosition, float deltaTime);


    // ==================== Movement & Collision ====================

    void move(float deltaTime, float screenWidth, float screenHeight, const std::vector<Rectangle> &walls, float finalTurnAngle);

    void collisionHandling(const std::vector<Rectangle> &walls, Vector2 oldPosition);

};