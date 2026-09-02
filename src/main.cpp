#include <raylib.h>
#include <raymath.h>
#include <vector>
#include "Robot.h"
#include "World.h"



int main()
{
    Color pink = {255, 204, 240, 255};
    Color darkPink = {255, 102, 178, 255};
 

    Vector2 targetPosition = {-10, -10};
    float targetRadius = 10.0f;
    bool targetSet = false;
 
    World gameWorld = World(800, 800, GRAY);
    InitWindow(gameWorld.screenWidth, gameWorld.screenHeight, "Little Robot");
    SetTargetFPS(180);
    Robot littleRobot = Robot(pink, darkPink, 20.0f, 5.0f, {400,400}, 0.0f);

    // Game Loop
    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        // 1. Event Handling + 2. Update State

        littleRobot.updateSensors(gameWorld.walls, gameWorld.screenWidth,  gameWorld.screenHeight);

        float avoidanceStrength = 0.0f;
        float avoidanceTurn = littleRobot.avoidObstacles(deltaTime, avoidanceStrength);
        float targetTurn = 0;

        if (targetSet && !littleRobot.recovering)
        {
            targetTurn = littleRobot.steerTowardTarget(targetPosition, deltaTime);
        }
        float targetWeight = 1.0f - (0.2f * avoidanceStrength);

        float finalTurn = targetTurn*targetWeight + avoidanceTurn;

        if (targetSet)
        {       
            littleRobot.move(deltaTime, gameWorld.screenWidth, gameWorld.screenHeight, gameWorld.walls, finalTurn);
        }

        
        // Target logic
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !targetSet)
        {
            bool invalidTarget = false;
            targetPosition = GetMousePosition();
            for (int i = 0; i <  gameWorld.walls.size(); i++)
            {
                bool collision = CheckCollisionCircleRec(targetPosition, targetRadius,  gameWorld.walls[i]);
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
        gameWorld.draw();

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
