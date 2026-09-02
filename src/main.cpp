#include <raylib.h>
#include <raymath.h>
#include <vector>
#include "Robot.h"




int main()
{
    Color pink = {255, 204, 240, 255};
    Color darkPink = {255, 102, 178, 255};
    int screenWidth = 800;
    int screenHeight = 800;

    std::vector<Rectangle> walls = {
        {50, 50, 100, 500},
        {587, 29, 70, 200},
        {300, 710, 300, 80}};

    Vector2 targetPosition = {-10, -10};
    float targetRadius = 10.0f;
    bool targetSet = false;
 

    InitWindow(screenWidth, screenHeight, "Little Robot");
    SetTargetFPS(180);
    Robot littleRobot = Robot(pink, darkPink, 20.0f, 5.0f, {400,400}, 0.0f);

    // Game Loop
    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        // 1. Event Handling + 2. Update State

        // Calculate robot/sensor geometry
        // Sensor direction and endpoint logic

        littleRobot.updateSensors(walls, screenWidth, screenHeight);

        littleRobot.avoidObstacles(deltaTime);

        if (targetSet && !littleRobot.recovering)
        {
            littleRobot.steerTowardTarget(targetPosition, deltaTime);
        }

        if (targetSet)
        {       
            littleRobot.move(deltaTime, screenWidth, screenHeight, walls);
        }

        
        // Target logic
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !targetSet)
        {
            bool invalidTarget = false;
            targetPosition = GetMousePosition();
            for (int i = 0; i < walls.size(); i++)
            {
                bool collision = CheckCollisionCircleRec(targetPosition, targetRadius, walls[i]);
                if (collision == true)
                {
                    invalidTarget = true;
                    targetPosition = {-10, -10};
                    break;
                }
            }
            if (!invalidTarget)
            {
                targetSet = true;
            }
        }

    
        // Found target logic
        float distanceToTarget = Vector2Distance(littleRobot.robotPosition, targetPosition);

       if  (distanceToTarget <= littleRobot.robotRadius + targetRadius  && targetSet == true) {
            targetSet = false;
       }

        // 3. Drawing
        BeginDrawing();
        ClearBackground(WHITE);

        // Robot
        littleRobot.draw();

        // Sensors
        littleRobot.drawSensors();

        // Walls
        for (int i = 0; i < walls.size(); i++)
        {
            DrawRectangleRec(walls[i], GRAY);
        }

        // Sensor Readings
        littleRobot.readSensors();

        // Target
        if (targetSet == true) {
            DrawCircle(targetPosition.x, targetPosition.y, targetRadius, BLUE);
            DrawLineV(littleRobot.robotPosition, targetPosition, BLUE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
