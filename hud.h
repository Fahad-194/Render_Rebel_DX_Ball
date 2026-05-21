#pragma once
#include <GL/glut.h>
#include <cstdio>
#include <cstring>

#define HUD_HEIGHT 36.0f

void drawText(float x, float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    for (int i = 0; text[i]; i++)
        glutBitmapCharacter(font, text[i]);
}

void drawTextLarge(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (int i = 0; text[i]; i++)
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);
}

// ── Particle System ─────────────────────────────────────────────────
struct Particle {
    float x, y, vx, vy;
    float r, g, b;
    float life, maxLife;
    bool  active;
};

#define MAX_PARTICLES 300

struct ParticleSystem {
    Particle p[MAX_PARTICLES];

    void clear() { for (int i = 0; i < MAX_PARTICLES; i++) p[i].active = false; }

    void burst(float x, float y, float r, float g, float b, int count = 12) {
        int spawned = 0;
        for (int i = 0; i < MAX_PARTICLES && spawned < count; i++) {
            if (!p[i].active) {
                p[i].x = x; p[i].y = y;
                float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
                float spd   = 1.5f + (rand() % 30) / 10.0f;
                p[i].vx = cosf(angle) * spd;
                p[i].vy = sinf(angle) * spd;
                p[i].r = r; p[i].g = g; p[i].b = b;
                p[i].life = p[i].maxLife = 0.45f + (rand() % 10) / 20.0f;
                p[i].active = true;
                spawned++;
            }
        }
    }

    void update(float dt) {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!p[i].active) continue;
            p[i].x  += p[i].vx;
            p[i].y  += p[i].vy;
            p[i].vy -= 0.05f;
            p[i].life -= dt;
            if (p[i].life <= 0) p[i].active = false;
        }
    }

    void draw() {
        glPointSize(3.5f);
        glBegin(GL_POINTS);
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!p[i].active) continue;
            float a = p[i].life / p[i].maxLife;
            glColor4f(p[i].r, p[i].g, p[i].b, a);
            glVertex2f(p[i].x, p[i].y);
        }
        glEnd();
        glPointSize(1.0f);
    }
};

// ── Draw a filled heart shape ───────────────────────────────────────
void drawHeart(float cx, float cy, float size) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy - size * 0.5f); // bottom tip
    for (int i = 0; i <= 60; i++) {
        float t = -3.14159f + i * (2.0f * 3.14159f / 60.0f);
        // parametric heart
        float hx = size * 0.6f * (16.0f * sinf(t)*sinf(t)*sinf(t)) / 16.0f;
        float hy = size * 0.6f * (13.0f*cosf(t) - 5.0f*cosf(2*t)
                                  - 2.0f*cosf(3*t) - cosf(4*t)) / 16.0f;
        glVertex2f(cx + hx, cy + hy);
    }
    glEnd();
}

// ── HUD ──────────────────────────────────────────────────────────────
struct HUD {
    int   score, lives, level;
    float time;
    char  perkMsg[64];
    float perkMsgTimer;
    bool  fireballActive;
    float fireballTimer;

    void init() {
        score = 0; lives = 3; level = 1; time = 0;
        perkMsg[0] = '\0'; perkMsgTimer = 0;
        fireballActive = false; fireballTimer = 0;
    }

    void showPerkMessage(const char* msg) {
        strncpy(perkMsg, msg, 63);
        perkMsgTimer = 2.5f;
    }

    void update(float dt) {
        time += dt;
        if (perkMsgTimer  > 0) perkMsgTimer  -= dt;
        if (fireballTimer > 0) {
            fireballTimer -= dt;
            if (fireballTimer <= 0) fireballActive = false;
        }
    }

    void draw(float winW, float winH) {
        // ── Top HUD bar ──
        // Background
        glColor4f(0.0f, 0.0f, 0.12f, 0.92f);
        glBegin(GL_QUADS);
        glVertex2f(0, winH - HUD_HEIGHT);
        glVertex2f(winW, winH - HUD_HEIGHT);
        glVertex2f(winW, winH);
        glVertex2f(0, winH);
        glEnd();

        // Bottom line of HUD bar
        glColor3f(0.2f, 0.5f, 1.0f);
        glLineWidth(1.5f);
        glBegin(GL_LINES);
        glVertex2f(0, winH - HUD_HEIGHT);
        glVertex2f(winW, winH - HUD_HEIGHT);
        glEnd();
        glLineWidth(1.0f);

        char buf[128];
        float textY = winH - 24.0f;

        // SCORE
        glColor3f(1.0f, 0.85f, 0.1f);
        snprintf(buf, sizeof(buf), "SCORE: %d", score);
        drawText(12, textY, buf);

        // LEVEL
        glColor3f(0.35f, 0.85f, 1.0f);
        snprintf(buf, sizeof(buf), "LEVEL: %d / 3", level);
        drawText(winW / 2.0f - 50, textY, buf);

        // TIME
        glColor3f(0.75f, 0.75f, 0.85f);
        int mins = (int)time / 60, secs = (int)time % 60;
        snprintf(buf, sizeof(buf), "%02d:%02d", mins, secs);
        drawText(winW - 220, textY, buf);

        // LIVES label
        glColor3f(1.0f, 0.3f, 0.35f);
        drawText(winW - 155, textY, "LIVES:", GLUT_BITMAP_HELVETICA_18);

        // Lives as number  e.g.  x3
        glColor3f(1.0f, 1.0f, 1.0f);
        snprintf(buf, sizeof(buf), "x%d", lives);
        drawText(winW - 75, textY, buf, GLUT_BITMAP_HELVETICA_18);

        // Draw small heart icon next to number
        glColor3f(1.0f, 0.2f, 0.3f);
        drawHeart(winW - 28, textY - 2, 9.0f);

        // ── Fireball timer bar ──
        if (fireballActive && fireballTimer > 0) {
            float maxT = 8.0f;
            float ratio = fireballTimer / maxT;
            // background
            glColor4f(0.3f, 0.0f, 0.0f, 0.7f);
            glBegin(GL_QUADS);
            glVertex2f(winW/2-80, winH - HUD_HEIGHT - 10);
            glVertex2f(winW/2+80, winH - HUD_HEIGHT - 10);
            glVertex2f(winW/2+80, winH - HUD_HEIGHT - 4);
            glVertex2f(winW/2-80, winH - HUD_HEIGHT - 4);
            glEnd();
            // fill
            glColor3f(1.0f, 0.4f + ratio*0.3f, 0.0f);
            glBegin(GL_QUADS);
            glVertex2f(winW/2-80, winH - HUD_HEIGHT - 10);
            glVertex2f(winW/2-80+160*ratio, winH - HUD_HEIGHT - 10);
            glVertex2f(winW/2-80+160*ratio, winH - HUD_HEIGHT - 4);
            glVertex2f(winW/2-80, winH - HUD_HEIGHT - 4);
            glEnd();
            // label
            glColor3f(1.0f, 0.6f, 0.0f);
            drawText(winW/2 - 30, winH - HUD_HEIGHT - 22, "FIREBALL!", GLUT_BITMAP_HELVETICA_12);
        }

        // ── Perk message (center screen) ──
        if (perkMsgTimer > 0) {
            float alpha = (perkMsgTimer > 0.5f) ? 1.0f : perkMsgTimer * 2.0f;
            glColor4f(0.15f, 1.0f, 0.45f, alpha);
            drawTextLarge(winW/2 - 90, winH/2 + 30, perkMsg);
        }
    }
};
