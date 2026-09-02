#include <raylib.h>
#include <raymath.h>
#include <vector>

float findEndpoint(float position, float directionComponent, float distance)
{
    return position + directionComponent * distance;
}

void checkSensorCollision(Vector2 sensorStart, Vector2 sensorEnd, Vector2 boundaryStart, Vector2 boundaryEnd, float &closestDistance, Vector2 &closestCollisionPoint)
{
    Vector2 collisionPoint;
    bool hit = CheckCollisionLines(sensorStart, sensorEnd, boundaryStart, boundaryEnd, &collisionPoint);
    if (hit == true)
    {
        float distance = Vector2Distance(sensorStart, collisionPoint);
        if (distance < closestDistance)
        {
            closestDistance = distance;
            closestCollisionPoint = collisionPoint;
        }
    }
}

void checkRectangleWallCollision(const std::vector<Rectangle> &walls, Vector2 sensorStart, Vector2 sensorEnd, float &closestDistance, Vector2 &closestCollisionPoint)
{
    for (int j = 0; j < walls.size(); j++)
    {
        Vector2 topLeft = {walls[j].x, walls[j].y};
        Vector2 topRight = {walls[j].x + walls[j].width, walls[j].y};
        Vector2 bottomLeft = {walls[j].x, walls[j].y + walls[j].height};
        Vector2 bottomRight = {walls[j].x + walls[j].width, walls[j].y + walls[j].height};

        // Check top border
        checkSensorCollision(sensorStart, sensorEnd, topLeft, topRight, closestDistance, closestCollisionPoint);

        // Check bottom border
        checkSensorCollision(sensorStart, sensorEnd, bottomLeft, bottomRight, closestDistance, closestCollisionPoint);

        // Check right border
        checkSensorCollision(sensorStart, sensorEnd, topRight, bottomRight, closestDistance, closestCollisionPoint);

        // Check left border
        checkSensorCollision(sensorStart, sensorEnd, topLeft, bottomLeft, closestDistance, closestCollisionPoint);
    }
}

void checkWindowCollision(Vector2 sensorStart, Vector2 sensorEnd, float &closestDistance, Vector2 &closestCollisionPoint, int width, int height, float robotRadius)
{
    Vector2 windowTopLeft = {robotRadius, robotRadius};
    Vector2 windowTopRight = {width - robotRadius, robotRadius};
    Vector2 windowBottomLeft = {robotRadius, height - robotRadius};
    Vector2 windowBottomRight = {width - robotRadius, height - robotRadius};

    // Check top edge
    checkSensorCollision(sensorStart, sensorEnd, windowTopLeft, windowTopRight, closestDistance, closestCollisionPoint);

    // Check bottom edge
    checkSensorCollision(sensorStart, sensorEnd, windowBottomLeft, windowBottomRight, closestDistance, closestCollisionPoint);

    // Check right edge
    checkSensorCollision(sensorStart, sensorEnd, windowTopRight, windowBottomRight, closestDistance, closestCollisionPoint);

    // Check left edge
    checkSensorCollision(sensorStart, sensorEnd, windowTopLeft, windowBottomLeft, closestDistance, closestCollisionPoint);
}

int main()
{
    Color pink = {255, 204, 240, 255};
    Color darkPink = {255, 102, 178, 255};
    Vector2 robotPosition = {400, 400};
    Vector2 oldRobotPosition = robotPosition;
    float robotRadius = 20.0f;
    float robotSpeed = 300.0f;
    float robotAngleRadians = 0.0f;
    float sensorRange = 180.0f;
    int screenWidth = 800;
    int screenHeight = 800;
    std::vector<int> sensorAngleOffsets = {-90, -60, -30, 0, 30, 60, 90};

    std::vector<Rectangle> walls = {
        {50, 50, 100, 500},
        {587, 29, 70, 200},
        {300, 710, 300, 80}};

    float avoidanceDistance = 60.0f;
    float maxTurnSpeed = 360.0f;
    bool recovering = false;
    int recoveryDirection = 0;

    Vector2 targetPosition = {-10, -10};
    float targetRadius = 10.0f;
    bool targetSet = false;
    Vector2 targetDirection = {-10, -10};
    float targetAngleRadians = 0.0f;
    float angleDifference = 0.0f;

    InitWindow(screenWidth, screenHeight, "Little Robot");
    SetTargetFPS(180);

    // Game Loop
    while (!WindowShouldClose())
    {

        oldRobotPosition = robotPosition;
        Vector2 robotDirection = {0, 0};
        float deltaTime = GetFrameTime();

        // 1. Event Handling + 2. Update State

        // Calculate robot/sensor geometry
        // Sensor direction and endpoint logic
        std::vector<float> sensorDistances;
        std::vector<Vector2> sensorHitPoints;
        for (int i = 0; i < sensorAngleOffsets.size(); i++)
        {
            float sensorAngleRadians = robotAngleRadians + sensorAngleOffsets[i] * DEG2RAD;
            Vector2 sensorDirection = {cosf(sensorAngleRadians), sinf(sensorAngleRadians)};
            Vector2 sensorEnd = {findEndpoint(robotPosition.x, sensorDirection.x, sensorRange), findEndpoint(robotPosition.y, sensorDirection.y, sensorRange)};
            float closestDistance = sensorRange;
            Vector2 closestCollisionPoint = sensorEnd;

            // Sensor collison with walls
            checkRectangleWallCollision(walls, robotPosition, sensorEnd, closestDistance, closestCollisionPoint);

            // Sensor collison with window edges
            checkWindowCollision(robotPosition, sensorEnd, closestDistance, closestCollisionPoint, screenWidth, screenHeight, robotRadius);

            sensorDistances.push_back(closestDistance);
            sensorHitPoints.push_back(closestCollisionPoint);
        }

        // Swerving logic
        float closestSensorDistance = sensorRange;
        int dangerSensor = 3;
        for (int i = 2; i < 5; i++)
        {
            if (sensorDistances[i] < closestSensorDistance)
            {
                closestSensorDistance = sensorDistances[i];
                dangerSensor = i;
            }
        }

        float leftAverage = (sensorDistances[0] + sensorDistances[1] + sensorDistances[2]) / 3.0f;
        float rightAverage = (sensorDistances[4] + sensorDistances[5] + sensorDistances[6]) / 3.0f;

        if (closestSensorDistance <= avoidanceDistance && recovering == false)
        {
            float steeringStrength = (avoidanceDistance - closestSensorDistance) / avoidanceDistance;
            float actualTurnSpeed = steeringStrength * maxTurnSpeed;

            if (dangerSensor < 3)
            {
                robotAngleRadians += actualTurnSpeed * deltaTime * DEG2RAD;
            }
            else if (dangerSensor > 3)
            {
                robotAngleRadians -= actualTurnSpeed * deltaTime * DEG2RAD;
            }
            else
            {
                if (leftAverage <= rightAverage)
                {
                    robotAngleRadians += actualTurnSpeed * deltaTime * DEG2RAD;
                }
                else
                {
                    robotAngleRadians -= actualTurnSpeed * deltaTime * DEG2RAD;
                }
            }
        } else if (targetSet && !recovering) {
            targetDirection = targetPosition - robotPosition;
            targetAngleRadians = atan2f(targetDirection.y, targetDirection.x);
            angleDifference = targetAngleRadians - robotAngleRadians;

            if (angleDifference < -PI)
            {
                angleDifference += 2 * PI;
            }
            else if (angleDifference > PI)
            {
                angleDifference -= 2 * PI;
            }

            float turnThisFrame = maxTurnSpeed * deltaTime * DEG2RAD;
            float actualTargetTurn = Clamp(angleDifference, -turnThisFrame, turnThisFrame);
            robotAngleRadians += actualTargetTurn;
        }

        robotDirection.x = cosf(robotAngleRadians);
        robotDirection.y = sinf(robotAngleRadians);

        if (targetSet) {
            robotPosition.x += robotDirection.x * robotSpeed * deltaTime;
            robotPosition.y += robotDirection.y * robotSpeed * deltaTime;
        }

        // Avoid falling off the map
        robotPosition.x = Clamp(robotPosition.x, robotRadius, screenWidth - robotRadius);
        robotPosition.y = Clamp(robotPosition.y, robotRadius, screenHeight - robotRadius);

        // Avoid colliding with walls
        bool collidedThisFrame = false;
        for (int i = 0; i < walls.size(); i++)
        {
            bool collision = CheckCollisionCircleRec(robotPosition, robotRadius, walls[i]);
            if (collision == true)
            {

                collidedThisFrame = true;
                robotPosition = oldRobotPosition;

                if (recovering == false)
                {
                    if (leftAverage <= rightAverage)
                    {
                        robotAngleRadians += 20 * DEG2RAD;
                        recoveryDirection = 1;
                    }
                    else
                    {
                        robotAngleRadians -= 20 * DEG2RAD;
                        recoveryDirection = -1;
                    }
                    recovering = true;
                }
                else
                {
                    if (recoveryDirection == 1)
                    {
                        robotAngleRadians += 20 * DEG2RAD;
                    }
                    else if (recoveryDirection == -1)
                    {
                        robotAngleRadians -= 20 * DEG2RAD;
                    }
                }
                break;
            }
        }

        if (collidedThisFrame == false)
        {
            recovering = false;
            recoveryDirection = 0;
        }

        // Nose position logic
        Vector2 nosePosition = {findEndpoint(robotPosition.x, cosf(robotAngleRadians), 25), findEndpoint(robotPosition.y, sinf(robotAngleRadians), 25)};

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
        float distanceToTarget = Vector2Distance(robotPosition, targetPosition);

       if  (distanceToTarget <= robotRadius + targetRadius  && targetSet == true) {
            targetSet = false;
       }

        // 3. Drawing
        BeginDrawing();
        ClearBackground(WHITE);

        // Robot
        DrawCircle(robotPosition.x, robotPosition.y, robotRadius, pink);
        DrawCircle(nosePosition.x, nosePosition.y, 5, darkPink);

        // Sensors
        for (int i = 0; i < sensorHitPoints.size(); i++)
        {
            DrawLineV(robotPosition, sensorHitPoints[i], RED);

            if (sensorDistances[i] < sensorRange)
            {
                DrawCircle(sensorHitPoints[i].x, sensorHitPoints[i].y, 5, RED);
            }
        }

        // Walls
        for (int i = 0; i < walls.size(); i++)
        {
            DrawRectangleRec(walls[i], GRAY);
        }

        // Sensor Readings
        for (int i = 0; i < sensorDistances.size(); i++)
        {
            const char *sensorInfo = TextFormat("Sensor %d : %.1f", i, sensorDistances[i]);
            int posY = 20 + 25 * i;
            DrawText(sensorInfo, 20, posY, 20, BLACK);
        }

        // Target
        if (targetSet == true) {
            DrawCircle(targetPosition.x, targetPosition.y, targetRadius, BLUE);
            DrawLineV(robotPosition, targetDirection + robotPosition, BLUE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
