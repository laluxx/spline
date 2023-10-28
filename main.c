#include "raylib.h"
#include <stdbool.h>

#define MAX_POINTS 100

typedef struct {
    Vector2 position;
    Color color;
} Point;

Vector2 lerp(Vector2 a, Vector2 b, float t) {
    return (Vector2){ (1 - t) * a.x + t * b.x, (1 - t) * a.y + t * b.y };
}

int main() {
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "splines");
    SetTargetFPS(144);

    Point controlPoints[MAX_POINTS] = {0};
    int pointCount = 0;
    int selectedPoint = -1;

    Color colors[] = {(Color){0xFE, 0x11, 0x55, 0xFF}, (Color){0x35, 0xBB, 0xFD, 0xFF},
                      (Color){0x12, 0xFE, 0xAD, 0xFF}, (Color){0xFD, 0xDC, 0x56, 0xFF}};

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_D) && pointCount > 0) {
            pointCount--;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && pointCount < MAX_POINTS) {
            Vector2 mousePos = GetMousePosition();
            bool pointSelected = false;
            
            // Check if we're clicking an existing point
            for (int i = 0; i < pointCount; i++) {
                if (CheckCollisionPointCircle(mousePos, controlPoints[i].position, 20)) {
                    selectedPoint = i;
                    pointSelected = true;
                    break;
                }
            }

            // If no point was selected, create a new one
            if (!pointSelected) {
                controlPoints[pointCount].position = mousePos;
                controlPoints[pointCount].color = colors[pointCount % 4];
                pointCount++;
            }
        }

        // Allow moving of selected control point
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && selectedPoint != -1) {
            controlPoints[selectedPoint].position = GetMousePosition();
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            selectedPoint = -1;
        }

        BeginDrawing();
        ClearBackground((Color){0x0E, 0x1A, 0x25, 0xFF});
        
        // Draw the Bézier curve visualization
        Vector2 prevPoint = {0};
        for (float t = 0; t <= 1; t += 0.01f) {
            Vector2 points[MAX_POINTS];
            for (int i = 0; i < pointCount; i++) {
                points[i] = controlPoints[i].position;
            }

            for (int i = 1; i < pointCount; i++) {
                for (int j = 0; j < pointCount - i; j++) {
                    points[j] = lerp(points[j], points[j + 1], t);
                }
            }

            // The last point is on the Bézier curve
            if (t > 0 && pointCount > 1) {
                DrawLineEx(prevPoint, points[0], 4.0f, WHITE);
            }
            prevPoint = points[0];
        }

        // Draw control points and lines
        for (int i = 0; i < pointCount; i++) {
            if (i > 0) {
                DrawLineEx(controlPoints[i-1].position, controlPoints[i].position, 2.0f, controlPoints[i].color);
            }
            DrawCircleV(controlPoints[i].position, 8, controlPoints[i].color);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
