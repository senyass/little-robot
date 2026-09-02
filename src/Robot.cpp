#include "Robot.h"

float findEndpoint(float position, float directionComponent, float distance);


// ==================== Constructor ====================

Robot::Robot(Color robotColor, Color noseColor, float robotRadius, float noseRadius, Vector2 robotPosition, float robotAngleDegrees) {
    this->robotColor = robotColor;
    this->noseColor = noseColor;
    this->robotRadius = robotRadius;
    this->noseRadius = noseRadius;
    this->robotPosition = robotPosition;
    robotAngleRadians = robotAngleDegrees * DEG2RAD;
    robotSpeed = 300.0f;
    sensorRange = 100.0f;
    sensorAngleOffsets = {-90, -60, -30, 0, 30, 60, 90};
    avoidanceDistance = 40.0f;
    maxTurnSpeed = 360.0f;
    recovering = false;
    recoveryDirection = 0;
    avoidanceSide = 0;
}



// ==================== Drawing ====================

void Robot::draw() {
    Vector2 nosePosition = {findEndpoint(robotPosition.x, cosf(robotAngleRadians), 25), findEndpoint(robotPosition.y, sinf(robotAngleRadians), 25)};

    DrawCircle(robotPosition.x, robotPosition.y, robotRadius, robotColor);
    DrawCircle(nosePosition.x, nosePosition.y, noseRadius, noseColor);
}


void Robot::drawSensors() {
    for (int i = 0; i < sensorHitPoints.size(); i++)
    {
        DrawLineV(robotPosition, sensorHitPoints[i], RED);

        if (sensorDistances[i] < sensorRange)
        {
            DrawCircle(sensorHitPoints[i].x, sensorHitPoints[i].y, 5, RED);
        }
    }
}


void Robot::readSensors(){
    int smallest = 0;
    float dist = 180.0f;
    for (int i = 2; i < 5; i++)
    {
        const char *sensorInfo = TextFormat("Sensor %d : %.1f", i, sensorDistances[i]);
        int posY = 20 + 25 * i;
        DrawText(sensorInfo, 20, posY, 20, BLACK);

        if (sensorDistances[i] < dist){
            dist = sensorDistances[i];
            smallest = i;
        }
        

    }
    const char *danger = TextFormat("Danger Sensor: %d ", smallest);
        DrawText(danger, 20, 195, 20, RED);

}



// ==================== Sensor Helpers ====================

float findEndpoint(float position, float directionComponent, float distance){
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


void checkWindowCollision(Vector2 sensorStart, Vector2 sensorEnd, float &closestDistance, Vector2 &closestCollisionPoint, int width, int height, float robotRadius){
    Vector2 windowTopLeft = {0, 0};
    Vector2 windowTopRight = {width, 0};
    Vector2 windowBottomLeft = {0, height};
    Vector2 windowBottomRight = {width, height};

    Vector2 sensorDirection = sensorEnd - sensorStart;

    // Check top edge
    if (sensorDirection.y < 0){
        checkSensorCollision(sensorStart, sensorEnd, windowTopLeft, windowTopRight, closestDistance, closestCollisionPoint);
    }

    // Check bottom edge
    if (sensorDirection.y > 0){
        checkSensorCollision(sensorStart, sensorEnd, windowBottomLeft, windowBottomRight, closestDistance, closestCollisionPoint);
    }

    // Check right edge
    if (sensorDirection.x > 0){
        checkSensorCollision(sensorStart, sensorEnd, windowTopRight, windowBottomRight, closestDistance, closestCollisionPoint);
    }

    // Check left edge
    if (sensorDirection.x < 0){
        checkSensorCollision(sensorStart, sensorEnd, windowTopLeft, windowBottomLeft, closestDistance, closestCollisionPoint);
    }
}


float Robot::getLeftAverage()
{
    return (sensorDistances[0] + sensorDistances[1] + sensorDistances[2]) / 3.0f;
}


float Robot::getRightAverage()
{
    return (sensorDistances[4] + sensorDistances[5] + sensorDistances[6]) / 3.0f;
}



// ==================== Sensor Update ====================

void Robot::updateSensors(const std::vector<Rectangle>& walls, int screenWidth, int screenHeight){
    sensorDistances.clear();
    sensorHitPoints.clear();

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
}



// ==================== Steering ====================

float Robot::avoidObstacles(float deltaTime, float &avoidanceStrength) {
    float closestSensorDistance = sensorRange;
    int dangerSensor = 3;
    avoidanceStrength = 0.0f;

    for (int i = 2; i < 5; i++)
    {
        if (sensorDistances[i] < closestSensorDistance)
        {
            closestSensorDistance = sensorDistances[i];
            dangerSensor = i;
        }
    }

    float leftAverage = getLeftAverage();
    float rightAverage = getRightAverage();



    float clearDistance = avoidanceDistance + 20.0f;

if (avoidanceSide == 1)
{
    if (sensorDistances[3] > clearDistance &&
        sensorDistances[2] > clearDistance)
    {
        avoidanceSide = 0;
    }
}
else if (avoidanceSide == -1)
{
    if (sensorDistances[3] > clearDistance &&
        sensorDistances[4] > clearDistance)
    {
        avoidanceSide = 0;
    }
}

    if (closestSensorDistance <= avoidanceDistance && recovering == false) {
        avoidanceStrength = (avoidanceDistance - closestSensorDistance) / avoidanceDistance;
        float actualTurnSpeed = avoidanceStrength * maxTurnSpeed;
        

        if (avoidanceSide == 0) {
            if (dangerSensor < 3)
            {   
                avoidanceSide = 1;
            }
            else if (dangerSensor > 3)
            {
                avoidanceSide = -1;
            }
            else
            {
                if (leftAverage <= rightAverage)
                {
                    avoidanceSide = 1;
                }
                else
                {
                    avoidanceSide = -1;
                }
            }
        }

        if (avoidanceSide == 1) {
            return actualTurnSpeed * deltaTime * DEG2RAD;
        }
        else {
            return -(actualTurnSpeed * deltaTime * DEG2RAD);
        }
    }

    return 0;
}

float Robot::steerTowardTarget(Vector2 targetPosition, float deltaTime){

    Vector2 targetDirection = targetPosition - robotPosition;
    float targetAngleRadians = atan2f(targetDirection.y, targetDirection.x);
    float angleDifference = targetAngleRadians - robotAngleRadians;

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
    return actualTargetTurn;
}



// ==================== Movement & Collision ====================

void Robot::move(float deltaTime, float screenWidth, float screenHeight, const std::vector<Rectangle> &walls, float finalTurnAngle){
    Vector2 robotOldPosition = robotPosition;
    robotAngleRadians += finalTurnAngle;
    Vector2 robotDirection = {cosf(robotAngleRadians), sinf(robotAngleRadians)};
    robotPosition.x += robotDirection.x * robotSpeed * deltaTime;
    robotPosition.y += robotDirection.y * robotSpeed * deltaTime;

    // Avoid falling off the map
    robotPosition.x = Clamp(robotPosition.x, robotRadius, screenWidth - robotRadius);
    robotPosition.y = Clamp(robotPosition.y, robotRadius, screenHeight - robotRadius);

    collisionHandling(walls, robotOldPosition);
}


void Robot::collisionHandling(const std::vector<Rectangle> &walls, Vector2 oldPosition){
    bool collidedThisFrame = false;
    float leftAverage = getLeftAverage();
    float rightAverage = getRightAverage();

    for (int i = 0; i < walls.size(); i++)
    {
        bool collision = CheckCollisionCircleRec(robotPosition, robotRadius, walls[i]);

        if (collision == true)
        {
            collidedThisFrame = true;
            robotPosition = oldPosition;

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
}