#pragma once
#include <GL/glut.h>
#include <cmath>

#define HUD_HEIGHT 36.0f

// Midpoint Circle Algorithm (from lab file)
void drawCircleMidpoint(int cx, int cy, int r) {
    int x = 0, y = r;
    float d = 5.0f / 4.0f - r;
    auto plot8 = [&](int px, int py) {
        glVertex2i(cx + px, cy + py);
        glVertex2i(cx - px, cy + py);
        glVertex2i(cx + px, cy - py);
        glVertex2i(cx - px, cy - py);
        glVertex2i(cx + py, cy + px);
        glVertex2i(cx - py, cy + px);
        glVertex2i(cx + py, cy - px);
        glVertex2i(cx - py, cy - px);
    };
    glBegin(GL_POINTS);
    plot8(x, y);
    while (y > x) {
        if (d < 0) { x++; d += 2 * x + 1; }
        else       { y--; x++; d += 2 * (x - y) + 1; }
        plot8(x, y);
    }
    glEnd();
}

void drawFilledCircle(int cx, int cy, int r) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f((float)cx, (float)cy);
    for (int i = 0; i <= 360; i++) {
        float angle = i * 3.14159f / 180.0f;
        glVertex2f(cx + r * cosf(angle), cy + r * sinf(angle));
    }
    glEnd();
}

struct Ball {
    float x, y;
    float vx, vy;
    int   radius;
    float baseSpeed;
    float speedMultiplier;
    bool  active;
    bool  launched;

    float trailX[10], trailY[10];
    int   trailCount;

    void init(float wx, float wy) {
        x = wx / 2.0f;
        y = 80.0f;
        radius = 10;
        baseSpeed = 4.0f;
        speedMultiplier = 1.0f;
        vx = baseSpeed * 0.7f;
        vy = baseSpeed;
        active = true;
        launched = false;
        trailCount = 0;
        for (int i = 0; i < 10; i++) trailX[i] = trailY[i] = 0;
    }

    void launch() {
        launched = true;
        vx = baseSpeed * 0.8f;
        vy = baseSpeed;
    }

    float speed() { return baseSpeed * speedMultiplier; }

    void increaseSpeed(float factor) {
        speedMultiplier *= factor;
        float spd = speed();
        float angle = atan2f(vy, vx);
        vx = spd * cosf(angle);
        vy = spd * sinf(angle);
    }

    void update(float winW, float winH,
                float paddleX, float paddleW,
                float paddleY, float paddleH) {
        if (!launched || !active) return;

        for (int i = 9; i > 0; i--) {
            trailX[i] = trailX[i-1];
            trailY[i] = trailY[i-1];
        }
        trailX[0] = x; trailY[0] = y;
        if (trailCount < 10) trailCount++;

        x += vx;
        y += vy;

        // Left/right walls
        if (x - radius <= 0)    { x = (float)radius;     vx =  fabsf(vx); }
        if (x + radius >= winW) { x = winW - radius;     vx = -fabsf(vx); }

        // Ceiling: bounce BELOW the HUD bar, not into it
        float ceiling = winH - HUD_HEIGHT - radius - 2;
        if (y >= ceiling) { y = ceiling; vy = -fabsf(vy); }

        // Paddle collision
        if (vy < 0 &&
            y - radius <= paddleY + paddleH &&
            y - radius >= paddleY - 6.0f &&
            x >= paddleX && x <= paddleX + paddleW) {
            float hitPos = (x - paddleX) / paddleW;
            float ang    = (hitPos - 0.5f) * 2.0f;
            float spd    = speed();
            vx = spd * ang * 1.2f;
            vy = fabsf(vy);
            y  = paddleY + paddleH + radius + 1;
        }
    }

    // isMain=true  → main ball (white + cyan ring)
    // isMain=false → extra ball (yellow tint + orange ring)
    void draw(bool fireballMode, bool isMain = true) {
        if (!active) return;

        // Trail
        for (int i = 0; i < trailCount; i++) {
            float alpha = 1.0f - (float)(i+1) / 10.0f;
            float sz = (float)(radius - i);
            if (sz < 1) sz = 1;
            if (fireballMode)
                glColor4f(1.0f, 0.35f - i*0.02f, 0.0f, alpha * 0.55f);
            else if (isMain)
                glColor4f(0.2f, 0.8f, 1.0f, alpha * 0.5f);
            else
                glColor4f(1.0f, 0.8f, 0.1f, alpha * 0.45f);
            drawFilledCircle((int)trailX[i], (int)trailY[i], (int)sz);
        }

        if (fireballMode) {
            // Outer fire glow
            glColor4f(1.0f, 0.1f, 0.0f, 0.22f);
            drawFilledCircle((int)x, (int)y, radius + 10);
            glColor4f(1.0f, 0.45f, 0.0f, 0.38f);
            drawFilledCircle((int)x, (int)y, radius + 6);
            // Body hot orange
            glColor3f(1.0f, 0.68f, 0.05f);
            drawFilledCircle((int)x, (int)y, radius);
            // Red outline
            glColor3f(1.0f, 0.08f, 0.0f);
            glPointSize(2.0f);
            drawCircleMidpoint((int)x, (int)y, radius);
            glPointSize(1.0f);
            // White-hot core
            glColor3f(1.0f, 1.0f, 0.88f);
            drawFilledCircle((int)x, (int)y, radius / 2);

        } else if (isMain) {
            // Large outer glow — always visible
            glColor4f(0.3f, 0.9f, 1.0f, 0.25f);
            drawFilledCircle((int)x, (int)y, radius + 8);
            // Inner glow
            glColor4f(0.5f, 1.0f, 1.0f, 0.35f);
            drawFilledCircle((int)x, (int)y, radius + 4);
            // White body
            glColor3f(1.0f, 1.0f, 1.0f);
            drawFilledCircle((int)x, (int)y, radius);
            // Bright cyan outline — thick, easy to see
            glColor3f(0.0f, 0.9f, 1.0f);
            glPointSize(2.5f);
            drawCircleMidpoint((int)x, (int)y, radius);
            glPointSize(1.0f);
            // Cyan ring outside — marks it as MAIN ball
            glColor3f(0.0f, 0.7f, 1.0f);
            glPointSize(2.0f);
            drawCircleMidpoint((int)x, (int)y, radius + 4);
            glPointSize(1.0f);

        } else {
            // Extra ball — yellow/orange so player can tell apart
            glColor4f(1.0f, 0.75f, 0.0f, 0.25f);
            drawFilledCircle((int)x, (int)y, radius + 6);
            // Body: light yellow
            glColor3f(1.0f, 0.95f, 0.55f);
            drawFilledCircle((int)x, (int)y, radius);
            // Orange outline
            glColor3f(1.0f, 0.5f, 0.0f);
            glPointSize(2.0f);
            drawCircleMidpoint((int)x, (int)y, radius);
            glPointSize(1.0f);
        }
    }
};
