#include "raylib.h"
#include <cmath>
#include <vector>

bool IsWall(Color color) {
    return (color.r == 15 && color.g == 15 && color.b == 15);
}
bool IsTrack(Color color) {
    return (color.r == 35 && color.g == 35 && color.b == 35);
}
bool IsGrass(Color color) {
    return (color.r == 34 && color.g == 177 && color.b == 76);
}
float GetFrictionMultiplier(Color color) {
    if (IsWall(color)) return 999.0f;
    if (IsGrass(color)) return 3.0f;
    if (IsTrack(color)) return 1.0f;
    return 1.0f;
}

struct Checkpoint {
    Vector2 start; 
    Vector2 end; 
    bool crossed;
    
    // Check if a line segment crosses this checkpoint
    bool CheckCrossing(Vector2 prevPos, Vector2 currentPos) {
        // Line intersection algorithm
        float x1 = prevPos.x, y1 = prevPos.y;
        float x2 = currentPos.x, y2 = currentPos.y;
        float x3 = start.x, y3 = start.y;

        float x4 = end.x, y4 = end.y;
        
        float denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
        if (fabs(denom) < 0.001f) return false;
        
        float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
        float u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denom;
        
        return (t >= 0 && t <= 1 && u >= 0 && u <= 1);
    }
};

int main(void){

    //Screen setup
    const int screenWidth = 900;
    const int screenHeight = 900;
    InitWindow(screenWidth, screenHeight, "Speed Racer");

    //Physics Constants
    const float MAX_SPEED = 300.0f;
    const float ACCELERATION = 150.0f;
    const float BRAKE_FORCE = 200.0f;
    const float FRICTION = 50.0f;
    const float TURN_SPEED_BASE = 3.0f;
    const float TURN_SPEED_FACTOR = 0.3f;

    //Car State
    Vector2 position = {430, 92};
    Vector2 velocity = {0, 0};
    float angle = 0.0f;
    float speed = 0.0f;

    //Loading assets
    Image trackImage = LoadImage("raceTrack.png");
    Texture2D trackTexture = LoadTextureFromImage(trackImage);
    Texture2D carTexture = LoadTexture("racecarTransparent.png");

    //Defining checkpoints
    std::vector<Checkpoint> checkpoints;
    
    // Format: start point (x,y) and end point (x,y) forming a line across the track
    checkpoints.push_back({{450,35}, {450, 150}, false});  // Finish line (checkpoint 0)
    checkpoints.push_back({{719, 260}, {850, 260}, false});  // Checkpoint 1
    checkpoints.push_back({{850, 665}, {723, 665}, false});  // Checkpoint 2
    checkpoints.push_back({{523, 482}, {625, 517}, false});  // Checkpoint 3
    checkpoints.push_back({{409, 438}, {295, 413}, false});  // Checkpoint 4
    checkpoints.push_back({{139, 672}, {49, 672}, false});  // Checkpoint 5
    checkpoints.push_back({{138, 205}, {49, 205}, false});  // Checkpoint 6
    
    //Race State
    int currentLap = 0;
    int totalLaps = 3;
    float currentLapTime = 0.0f;
    float bestLapTime = 999999.0f;
    std::vector<float> lapTimes;
    bool raceStarted = false;
    bool raceFinished = false;
    int nextCheckpoint = 0;  // Which checkpoint we're looking for next

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        Vector2 prevPosition = position;
        
        // Update race timer
        if (raceStarted && !raceFinished) {
            currentLapTime += dt;
        }
        
        // Input Handling
        float accelerationInput = 0.0f;
        float steeringInput = 0.0f;

        if (IsKeyDown(KEY_UP)) accelerationInput = 1.0f;
        if (IsKeyDown(KEY_DOWN)) {
            if (speed > 0.1f) {
                speed -= BRAKE_FORCE * dt;
            } else {
                accelerationInput = -0.4f;
            }
        }
        if (IsKeyDown(KEY_LEFT)) steeringInput = -1.0f;
        if (IsKeyDown(KEY_RIGHT)) steeringInput = 1.0f;
        
        // Reset race
        if (IsKeyPressed(KEY_R)) {
            position = {430, 92};
            velocity = {0, 0};
            speed = 0;
            angle = 0;
            currentLap = 0;
            currentLapTime = 0;
            raceStarted = false;
            raceFinished = false;
            nextCheckpoint = 0;
            lapTimes.clear();
            for (auto& checkpoint : checkpoints) {
                checkpoint.crossed = false;
            }
        }
        
        // Detecting the surface

        //casting to int
        int checkPixelX = (int)position.x;
        int checkPixelY = (int)position.y;

        float surfaceFriction = 1.0f;
        
        if (checkPixelX >= 0 && checkPixelX < trackImage.width && checkPixelY >= 0 && checkPixelY < trackImage.height) {
            Color surfaceColor = GetImageColor(trackImage, checkPixelX, checkPixelY);
            surfaceFriction = GetFrictionMultiplier(surfaceColor);
        }
        
        // Physics //

        //applying acceleration/friction
        speed += accelerationInput * ACCELERATION * dt;
        
        float frictionToApply = FRICTION; 
        if (accelerationInput == 0.0f) {
            frictionToApply = FRICTION * surfaceFriction;
        }
        
        if (speed > 0) {
            speed -= frictionToApply * dt;
            if (speed < 0) speed = 0;
        } else if (speed < 0) {
            speed += frictionToApply * dt;
            if (speed > 0) speed = 0;
        }
        
        float maxSpeedOnSurface = MAX_SPEED;
        if (surfaceFriction > 2.0f) {
            maxSpeedOnSurface = MAX_SPEED * 0.5f;
        }
        //max speed clamps   
        if (speed > maxSpeedOnSurface) speed = maxSpeedOnSurface;
        if (speed < -maxSpeedOnSurface * 0.5f) speed = -maxSpeedOnSurface * 0.5f;
        
        //steering (weaker at higher speeds)
        float speedFactor = 1.0f / (1.0f + fabs(speed) / MAX_SPEED * TURN_SPEED_FACTOR);
        float turnRate = TURN_SPEED_BASE * speedFactor;
        
        if (fabs(speed) > 1.0f) {
            angle += steeringInput * turnRate * dt * (speed / fabs(speed));
        }
        
        //updating velocity and position
        velocity.x = cos(angle) * speed;
        velocity.y = sin(angle) * speed;
        
        position.x += velocity.x * dt;
        position.y += velocity.y * dt;

        // Start race on first movement
        if (!raceStarted && fabs(speed) > 1.0f) {
            raceStarted = true;
        }

        // Collision detection
        int pixelX = (int)position.x;
        int pixelY = (int)position.y;
        bool inBounds = false;
        bool hitWall = false;
        Color currentColor = {0, 0, 0, 0};

        if (pixelX >= 0 && pixelX < trackImage.width && pixelY >= 0 && pixelY < trackImage.height) {
            inBounds = true;
            currentColor = GetImageColor(trackImage, pixelX, pixelY);
            
            if (IsWall(currentColor)) {
                hitWall = true;
                position = prevPosition;
                speed *= -0.3f;
            }
        } else {
            position = prevPosition;
            speed *= -0.3f;
        }
    
        // Checkpoint detection
        if (raceStarted && !raceFinished) //while race is ongoing
        {
            Checkpoint& cp = checkpoints[nextCheckpoint];
            
            if (cp.CheckCrossing(prevPosition, position)) {
                cp.crossed = true;
                
                // If this was the finish line (checkpoint 0) and all checkpoints crossed
                if (nextCheckpoint == 0 && currentLap > 0) {
                    bool allCrossed = true;
                    for (int i = 1; i < checkpoints.size(); i++) {
                        if (!checkpoints[i].crossed) {
                            allCrossed = false;
                            break;
                        }
                    }
                    
                    if (allCrossed) {
                        // Lap finished
                        lapTimes.push_back(currentLapTime);
                        if (currentLapTime < bestLapTime) {
                            bestLapTime = currentLapTime;
                        }
                        
                        currentLap++;
                        currentLapTime = 0.0f;
                        
                        // Reset checkpoints for next lap
                        for (auto& checkpoint : checkpoints) {
                            checkpoint.crossed = false;
                        }
                        nextCheckpoint = 1;  // Start looking for checkpoint 1
                        
                        // Check if race finished
                        if (currentLap >= totalLaps) {
                            raceFinished = true;
                        }
                    }
                } 
                else if (nextCheckpoint == 0) {
                    // Crossed finish line for the first time - start lap 1
                    currentLap = 1;
                    currentLapTime = 0.0f;
                    cp.crossed = false;  // Keep finish line uncrossed until lap complete
                    nextCheckpoint = 1;
                } 
                else {
                    // Regular checkpoint crossed
                    nextCheckpoint = (nextCheckpoint + 1) % checkpoints.size();
                }
            }
        }

        // DRAWING CODE
        BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawTexture(trackTexture, 0, 0, WHITE);

            // iterating through checkpoints and drawing
            for (int i = 0; i < checkpoints.size(); i++) {
                Color cpColor = (i == 0) ? RED : YELLOW;  // Finish line is red
                if (checkpoints[i].crossed) cpColor = GREEN;
                if (i == nextCheckpoint) cpColor = BLUE;  // Next checkpoint to cross
                
                DrawLineEx(checkpoints[i].start, checkpoints[i].end, 3, cpColor);
                
                // Draw checkpoint number
                Vector2 mid = { (checkpoints[i].start.x + checkpoints[i].end.x)/2, (checkpoints[i].start.y + checkpoints[i].end.y)/2};
                DrawText(TextFormat("%d", i), mid.x - 10, mid.y - 10, 20, cpColor);
            }

            // Race info
            DrawText("SpeedRacer!", 10, 10, 20, RED);
            DrawText(TextFormat("Speed: %.0f", fabs(speed)), 10, 30, 20, LIGHTGRAY);
            
            // Lap and time info
            DrawText(TextFormat("Lap: %d / %d", currentLap, totalLaps), 10, 50, 20, LIGHTGRAY);
            DrawText(TextFormat("Time: %.2fs", currentLapTime), 10, 70, 20, LIGHTGRAY);
            
            if (bestLapTime < 999999.0f) {
                DrawText(TextFormat("Best: %.2fs", bestLapTime), 10, 90, 20, GOLD);
            }
            
            if (raceFinished) {
                DrawText("RACE FINISHED!", screenWidth/2 - 100, screenHeight/2, 30, RED);
                DrawText("Press R to restart", screenWidth/2 - 90, screenHeight/2 + 40, 20, RED);
            } 
            
            //DrawText("Press R to restart", 10, 160, 16, LIGHTGRAY);
            //DrawText(TextFormat("Next CP: %d", nextCheckpoint), 10, 180, 16, BLUE);

            // Drawing Car Texture
            float carTextureScale = 0.15f;
            Rectangle source = { 0, 0, (float)carTexture.width, (float)carTexture.height };
            Rectangle dest = { position.x, position.y, carTexture.width * carTextureScale, carTexture.height * carTextureScale};
            Vector2 origin = { carTexture.width * carTextureScale / 2.0f, carTexture.height * carTextureScale / 2.0f };
            DrawTexturePro(carTexture, source, dest, origin, angle * RAD2DEG, WHITE);
            

        EndDrawing();
    }

    UnloadTexture(carTexture);
    UnloadTexture(trackTexture);
    UnloadImage(trackImage);
    CloseWindow();

    return 0;
}
