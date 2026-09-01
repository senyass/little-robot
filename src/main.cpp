#include <raylib.h>
#include <raymath.h>

float findNosePosition(float position, float directionComponent, float distance) {
    return position + directionComponent * distance;
}

int main() {
    Color pink = {255, 204, 240, 255};
    Color darkPink = {255, 102, 178, 255};
    Vector2 robotPosition = {400, 400};
    float robotSpeed = 300.0f;
    float robotAngleRadians = 0.0f;
    InitWindow(800, 800, "Little Robot");
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

        Vector2 nosePosition = {findNosePosition(robotPosition.x, cosf(robotAngleRadians), 25), findNosePosition(robotPosition.y, sinf(robotAngleRadians), 25)};
        

        // 3. Drawing
        BeginDrawing();
        ClearBackground(WHITE);
        DrawCircle(robotPosition.x, robotPosition.y, 20, pink);
        DrawCircle(nosePosition.x, nosePosition.y, 5, darkPink);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
