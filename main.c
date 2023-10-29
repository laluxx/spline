#include "raylib.h"
#include <stdbool.h>
#include "screen.h"
#include "theme.h"
#include "panels.h"
#include "ui.h"

#define MAX_POINTS 100
#define BASE_RADIUS 8.0f
#define HOVER_RADIUS 12.0f
#define curveThickness 4
#define lineThickness 3


typedef struct {
    Vector2 position;
    Color color;
    float targetRadius;
    float currentRadius;
} Point;

Point points[MAX_POINTS];
int pointCount = 0;
int selectedPoint = -1;
Color colors[] = {(Color){0xFE, 0x11, 0x55, 0xFF}, (Color){0x35, 0xBB, 0xFD, 0xFF},
                  (Color){0x12, 0xFE, 0xAD, 0xFF}, (Color){0xFD, 0xDC, 0x56, 0xFF}};

void handleKeys(Vector2 mousePos);
void drawAll();
void addOrSelectPoint(Vector2 mousePos);
void moveSelectedPoint(Vector2 mousePos);
void updatePointRadii(Vector2 mousePos);
void drawBezierCurve();
void drawPointsAndLines();
Vector2 lerp(Vector2 a, Vector2 b, float t);
Vector2 vector2_add(Vector2 v1, Vector2 v2);
float lerpFloat(float a, float b, float t);


int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "splines");
    SetExitKey(0); // Disable the exit key
    SetTargetFPS(144);

    InitializeThemes();

    Vector2 mousePos = GetMousePosition();
    updateMousePanelState(mousePos);

    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();
        handleKeys(mousePos);
        drawAll();
    }
    CloseWindow();
    return 0;
}

void handleKeys(Vector2 mousePos) {
    if (IsKeyPressed(KEY_D) && pointCount > 0) {
        pointCount--;
    }
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        addOrSelectPoint(mousePos);
    }
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && selectedPoint != -1) {
        moveSelectedPoint(mousePos);
    }
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        selectedPoint = -1;
    }

    if (IsKeyPressed(KEY_O)) {
        panel.centerPanelVisible = !panel.centerPanelVisible;
    }

    // THEME KEYBINDS
    if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) {  // Check if Alt key is pressed
        if (IsKeyPressed(KEY_MINUS)) {    // Check if '-' key is pressed
            PreviousTheme();    // Go to the previous theme
        } else if (IsKeyPressed(KEY_EQUAL)) {    // Check if '=' key is pressed
            NextTheme();    // Go to the next theme
        }
    }
    updatePointRadii(mousePos);
}

void drawAll() {
    BeginDrawing();
    ClearBackground(CURRENT_THEME.background);
    drawPointsAndLines();
    drawBezierCurve();


    // UI
    ClosePanel('T'); // Close top panel
    ClosePanel('B'); // Close right panel
    ClosePanel('L'); // Close left panel
    /* ClosePanel('R'); // Close right panel */
    showBottomRightCorner = 1;
    showBottomLeftCorner = 1;
    showTopLeftCorner = 1;
    showTopRightCorner = 1;
    DrawPanel('R', 280.0f); // for a fixed right panel
    /* UpdatePanelsDimensions(); */


    HandleFrameKeyBindings();
    HandleFrameMouseInteractions();
    DrawFrames();

    DrawPanels();

    float minibufferHeight = 21.0f; // Default height, change as needed
    UpdateModelinePosition();
    DrawModeline(SCREEN_WIDTH, minibufferHeight);
    UpdateMinibufferKeyChord();
    DrawMiniBuffer(SCREEN_WIDTH, minibufferHeight);

    /* DrawModeBar(); */ // port in ui
    DrawFPSWidget();
    DrawAltIndicator();

    EndDrawing();
}

void addOrSelectPoint(Vector2 mousePos) {
    // Create rectangles for the main panels based on the global 'panel' struct.
    Rectangle topPanel = {0, 0, SCREEN_WIDTH, panel.topHeight};
    Rectangle bottomPanel = {0, SCREEN_HEIGHT - panel.bottomHeight, SCREEN_WIDTH, panel.bottomHeight};
    Rectangle leftPanel = {0, panel.topHeight, panel.leftWidth, SCREEN_HEIGHT - panel.topHeight - panel.bottomHeight};
    Rectangle rightPanel = {SCREEN_WIDTH - panel.rightWidth, panel.topHeight, panel.rightWidth, SCREEN_HEIGHT - panel.topHeight - panel.bottomHeight};

    // Create rectangles for Modeline and Minibuffer using the same logic as your Draw functions.
    Rectangle modelineRect = {0, SCREEN_HEIGHT - minibuffer.height - modeline.height, SCREEN_WIDTH, modeline.height};
    Rectangle minibufferRect = {0, SCREEN_HEIGHT - minibuffer.height, SCREEN_WIDTH, minibuffer.height};

    // Check if the mouse click is within any of the main panels, Modeline, or Minibuffer.
    if (CheckCollisionPointRec(mousePos, topPanel) ||
        CheckCollisionPointRec(mousePos, bottomPanel) ||
        CheckCollisionPointRec(mousePos, leftPanel) ||
        CheckCollisionPointRec(mousePos, rightPanel) ||
        CheckCollisionPointRec(mousePos, modelineRect) ||
        CheckCollisionPointRec(mousePos, minibufferRect)) {
        return;  // Click occurred on a panel, don't add a point.
    }

    // Existing logic to select or add a point.
    bool pointSelected = false;
    for (int i = 0; i < pointCount && !pointSelected; i++) {
        if (CheckCollisionPointCircle(mousePos, points[i].position, HOVER_RADIUS)) {
            selectedPoint = i;
            pointSelected = true;
        }
    }
    if (!pointSelected && pointCount < MAX_POINTS) {
        points[pointCount].position = mousePos;
        points[pointCount].color = colors[pointCount % 4];
        points[pointCount].currentRadius = BASE_RADIUS;
        points[pointCount].targetRadius = BASE_RADIUS;
        pointCount++;
    }
}



/* void moveSelectedPoint(Vector2 mousePos) { */
/*     points[selectedPoint].position = mousePos; */
/* } */


void moveSelectedPoint(Vector2 mousePos) {
    if (selectedPoint == -1) return;  // No point selected.

    float currentRadius = points[selectedPoint].currentRadius;

    Rectangle topPanel = {0, 0, SCREEN_WIDTH, panel.topHeight};
    Rectangle bottomPanel = {0, SCREEN_HEIGHT - panel.bottomHeight, SCREEN_WIDTH, panel.bottomHeight};
    Rectangle leftPanel = {0, panel.topHeight, panel.leftWidth, SCREEN_HEIGHT - panel.topHeight - panel.bottomHeight};
    Rectangle rightPanel = {SCREEN_WIDTH - panel.rightWidth, panel.topHeight, panel.rightWidth, SCREEN_HEIGHT - panel.topHeight - panel.bottomHeight};
    Rectangle modelineRect = {0, SCREEN_HEIGHT - minibuffer.height - modeline.height, SCREEN_WIDTH, modeline.height + minibuffer.height};

    // Adjust for top panel
    if (mousePos.y - currentRadius < topPanel.height) {
        mousePos.y = topPanel.height + currentRadius;
    }
    // Adjust for bottom panel
    if (mousePos.y + currentRadius > bottomPanel.y) {
        mousePos.y = bottomPanel.y - currentRadius;
    }
    // Adjust for left panel
    if (mousePos.x - currentRadius < leftPanel.width) {
        mousePos.x = leftPanel.width + currentRadius;
    }
    // Adjust for right panel
    if (mousePos.x + currentRadius > rightPanel.x) {
        mousePos.x = rightPanel.x - currentRadius;
    }
    // Adjust for modeline (taking minibuffer into account)
    if (mousePos.y + currentRadius > modelineRect.y) {
        mousePos.y = modelineRect.y - currentRadius;
    }

    points[selectedPoint].position = mousePos;
}



void updatePointRadii(Vector2 mousePos) {
    for (int i = 0; i < pointCount; i++) {
        if (CheckCollisionPointCircle(mousePos, points[i].position, HOVER_RADIUS)) {
            points[i].targetRadius = HOVER_RADIUS;
        } else {
            points[i].targetRadius = BASE_RADIUS;
        }
        points[i].currentRadius = lerpFloat(points[i].currentRadius, points[i].targetRadius, 0.1f);
    }
}

void drawBezierCurve() {
    Vector2 prevPoint = {0};
    for (float t = 0; t <= 1; t += 0.01f) {
        Vector2 tempPoints[MAX_POINTS];
        for (int i = 0; i < pointCount; i++) {
            tempPoints[i] = points[i].position;
        }
        for (int i = 1; i < pointCount; i++) {
            for (int j = 0; j < pointCount - i; j++) {
                tempPoints[j] = lerp(tempPoints[j], tempPoints[j + 1], t);
            }
        }
        if (t > 0 && pointCount > 1) {
            DrawLineEx(prevPoint, tempPoints[0], curveThickness, WHITE);
        }
        prevPoint = tempPoints[0];
    }
}

void drawPointsAndLines() {
    // Draw all the lines first
    for (int i = 1; i < pointCount; i++) {
        DrawLineEx(points[i-1].position, points[i].position, lineThickness, points[i].color);
    }

    // Draw all the points
    for (int i = 0; i < pointCount; i++) {
        DrawCircleV(points[i].position, points[i].currentRadius, points[i].color);
    }
}


Vector2 lerp(Vector2 a, Vector2 b, float t) {
    return (Vector2){ (1 - t) * a.x + t * b.x, (1 - t) * a.y + t * b.y };
}

Vector2 vector2_add(Vector2 v1, Vector2 v2) {
    return (Vector2){ v1.x + v2.x, v1.y + v2.y };
}

float lerpFloat(float a, float b, float t) {
    return (1 - t) * a + t * b;
}
