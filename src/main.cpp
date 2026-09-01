#include <raylib.h>
#include <raymath.h>
#include <vector>

float findEndpoint(float position, float directionComponent, float distance) {
    return position + directionComponent * distance;
}

void checkSensorCollision(Vector2 sensorStart, Vector2 sensorEnd, Vector2 boundaryStart, Vector2 boundaryEnd, float &closestDistance, Vector2 &closestCollisionPoint){
    Vector2 collisionPoint;
    bool hit = CheckCollisionLines(sensorStart, sensorEnd, boundaryStart, boundaryEnd, &collisionPoint);
    if (hit == true){
        float distance = Vector2Distance(sensorStart, collisionPoint); 
        if (distance < closestDistance) {
            closestDistance = distance;
            closestCollisionPoint = collisionPoint;
        }
    }
}

void checkRectangleWallCollision(std::vector<Rectangle> &walls, Vector2 sensorStart, Vector2 sensorEnd, float &closestDistance, Vector2 &closestCollisionPoint){
    for (int j = 0; j < walls.size(); j++) {
        Vector2 topLeft     = {walls[j].x, walls[j].y };
        Vector2 topRight    = {walls[j].x + walls[j].width, walls[j].y};
        Vector2 bottomLeft  = {walls[j].x, walls[j].y + walls[j].height};
        Vector2 bottomRight = {walls[j].x + walls[j].width , walls[j].y + walls[j].height};
        
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

void checkWindowCollision(Vector2 sensorStart, Vector2 sensorEnd, float &closestDistance, Vector2 &closestCollisionPoint, int width, int height){
    Vector2 windowTopLeft = {0, 0};
    Vector2 windowTopRight = {width, 0};
    Vector2 windowBottomLeft = {0, height};
    Vector2 windowBottomRight = {width, height};

    // Check top edge
    checkSensorCollision(sensorStart, sensorEnd, windowTopLeft, windowTopRight, closestDistance, closestCollisionPoint);
    
    // Check bottom edge
    checkSensorCollision(sensorStart, sensorEnd, windowBottomLeft, windowBottomRight, closestDistance, closestCollisionPoint);
    
    // Check right edge
    checkSensorCollision(sensorStart, sensorEnd, windowTopRight, windowBottomRight, closestDistance, closestCollisionPoint);
    
    // Check left edge
    checkSensorCollision(sensorStart, sensorEnd, windowTopLeft, windowBottomLeft, closestDistance, closestCollisionPoint);
}

int main() {
    Color pink = {255, 204, 240, 255};
    Color darkPink = {255, 102, 178, 255};
    Vector2 robotPosition = {400, 400};
    float robotSpeed = 300.0f;
    float robotAngleRadians = 0.0f;
    float sensorRange = 250.0f;
    int screenWidth = 800;
    int screenHeight = 800;
    std::vector<int> sensorAngleOffsets = {-60, -30, 0, 30, 60};

    std::vector<Rectangle> walls = {
        {50, 50, 100, 500},
        {587, 29, 70, 200},
        {300, 710, 300, 80}
    };

    InitWindow(screenWidth, screenHeight, "Little Robot");
    SetTargetFPS(180);

   
    // Game Loop
    while (!WindowShouldClose()) {
        Vector2 robotDirection = {0, 0};
        // 1. Event Handling + 2. Update State
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D) ) {
            robotDirection.x += 1;
        } 
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
            robotDirection.x -= 1;
        } 
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
            robotDirection.y += 1;
        } 
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
            robotDirection.y -= 1;


        // Calculate robot/sensor geometry
        if (robotDirection.x != 0 || robotDirection.y != 0)
        {   
            // normalizing the direction vector to keep a constant speed 
            robotDirection = Vector2Normalize(robotDirection);

            // calculating the angle from the direction
            robotAngleRadians = atan2f(robotDirection.y, robotDirection.x);
        }


        float deltaTime = GetFrameTime();
        robotPosition.x += robotDirection.x * robotSpeed * deltaTime;
        robotPosition.y += robotDirection.y * robotSpeed * deltaTime;

        // Nose position logic
        Vector2 nosePosition = {findEndpoint(robotPosition.x, cosf(robotAngleRadians), 25), findEndpoint(robotPosition.y, sinf(robotAngleRadians), 25)};
        
        // Sensor direction and endpoint logic
        std::vector<float> sensorDistances;
        std::vector<Vector2> sensorHitPoints;
        for (int i = 0; i < sensorAngleOffsets.size(); i++) {
            float sensorAngleRadians = robotAngleRadians + sensorAngleOffsets[i] * DEG2RAD;
            Vector2 sensorDirection = {cosf(sensorAngleRadians), sinf(sensorAngleRadians)};
            Vector2 sensorEnd = { findEndpoint(robotPosition.x, sensorDirection.x, sensorRange), findEndpoint(robotPosition.y, sensorDirection.y, sensorRange) };
            float closestDistance = sensorRange;
            Vector2 closestCollisionPoint = sensorEnd;
            

            // Sensor collison with walls
            checkRectangleWallCollision(walls, robotPosition, sensorEnd, closestDistance, closestCollisionPoint);


            // Sensor collison with window edges
            checkWindowCollision(robotPosition, sensorEnd, closestDistance, closestCollisionPoint, screenWidth, screenHeight);

            sensorDistances.push_back(closestDistance);
            sensorHitPoints.push_back(closestCollisionPoint);


        }
    

        // 3. Drawing
        BeginDrawing();
        ClearBackground(WHITE);

        // Robot
        DrawCircle(robotPosition.x, robotPosition.y, 20, pink);
        DrawCircle(nosePosition.x, nosePosition.y, 5, darkPink);

        // Sensors
        for (int i = 0; i < sensorHitPoints.size(); i++){
            DrawLineV(robotPosition, sensorHitPoints[i], RED);

            if (sensorDistances[i] < sensorRange) {
            DrawCircle(sensorHitPoints[i].x, sensorHitPoints[i].y, 5, RED);
        }
        }

        // Walls
        for (int i = 0; i < walls.size(); i++) {
             DrawRectangleRec(walls[i], GRAY);
        }
   


        EndDrawing();
    }

    CloseWindow();
    return 0;
}
