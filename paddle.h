#pragma once
#include <GL/glut.h>
#include <cmath>

// Bresenham's Line (from lab file)
void bresenhamLine(int x1, int y1, int x2, int y2) {
    int dx = abs(x2-x1), dy = abs(y2-y1);
    int incx = (x2>x1)?1:-1, incy = (y2>y1)?1:-1;
    int x = x1, y = y1;
    glBegin(GL_POINTS);
    if (dx >= dy) {
        int e = 2*dy - dx;
        for (int i = 0; i <= dx; i++) {
            glVertex2i(x,y);
            if (e >= 0) { y += incy; e += 2*(dy-dx); } else e += 2*dy;
            x += incx;
        }
    } else {
        int e = 2*dx - dy;
        for (int i = 0; i <= dy; i++) {
            glVertex2i(x,y);
            if (e >= 0) { x += incx; e += 2*(dx-dy); } else e += 2*dx;
            y += incy;
        }
    }
    glEnd();
}

struct Paddle {
    float x, y;
    float width, height;
    float speed;
    float targetX;
    float normalWidth;
    float widerWidth;
    bool  widerPerk;
    float perkTimer;
    bool  shrunk;
    float shrinkTimer;

    void init(float winW) {
        normalWidth = 120.0f;
        widerWidth  = 200.0f;
        width  = normalWidth;
        height = 16.0f;
        x = winW/2.0f - width/2.0f;
        y = 40.0f;
        speed = 9.0f;
        targetX = x;
        widerPerk = false; perkTimer  = 0;
        shrunk    = false; shrinkTimer= 0;
    }

    // Keyboard move (LEFT/RIGHT arrow AND < > keys)
    void moveLeft()            { x -= speed; if (x < 0) x = 0; }
    void moveRight(float winW) { x += speed; if (x + width > winW) x = winW - width; }

    // Mouse target
    void setTarget(float mx) { targetX = mx - width/2.0f; }

    void applyWider() {
        widerPerk = true;
        shrunk    = false;
        width     = widerWidth;
        perkTimer = 12.0f;
    }

    void applyShrink() {
        shrunk     = true;
        widerPerk  = false;
        width      = normalWidth * 0.55f;
        shrinkTimer= 8.0f;
    }

    void update(float winW, float dt) {
        // Smooth mouse follow
        float diff = targetX - x;
        x += diff * 0.18f;
        if (x < 0)              x = 0;
        if (x + width > winW)   x = winW - width;

        if (widerPerk) {
            perkTimer -= dt;
            if (perkTimer <= 0) { widerPerk = false; width = normalWidth; }
        }
        if (shrunk) {
            shrinkTimer -= dt;
            if (shrinkTimer <= 0) { shrunk = false; width = normalWidth; }
        }
    }

    void draw() {
        float px=x, py=y, pw=width, ph=height;

        // Shadow
        glColor4f(0,0,0,0.3f);
        glBegin(GL_QUADS);
        glVertex2f(px+4,py-4); glVertex2f(px+pw+4,py-4);
        glVertex2f(px+pw+4,py+ph-4); glVertex2f(px+4,py+ph-4);
        glEnd();

        // Body gradient
        glBegin(GL_QUADS);
        if (widerPerk) {
            glColor3f(0.0f,0.9f,0.4f); glVertex2f(px,py); glVertex2f(px+pw,py);
            glColor3f(0.0f,0.5f,0.2f); glVertex2f(px+pw,py+ph); glVertex2f(px,py+ph);
        } else if (shrunk) {
            glColor3f(0.9f,0.1f,0.5f); glVertex2f(px,py); glVertex2f(px+pw,py);
            glColor3f(0.6f,0.0f,0.3f); glVertex2f(px+pw,py+ph); glVertex2f(px,py+ph);
        } else {
            glColor3f(0.1f,0.6f,1.0f); glVertex2f(px,py); glVertex2f(px+pw,py);
            glColor3f(0.0f,0.3f,0.8f); glVertex2f(px+pw,py+ph); glVertex2f(px,py+ph);
        }
        glEnd();

        // Shine strip
        glColor4f(1,1,1,0.35f);
        glBegin(GL_QUADS);
        glVertex2f(px+6,py+ph-4); glVertex2f(px+pw-6,py+ph-4);
        glVertex2f(px+pw-6,py+ph-2); glVertex2f(px+6,py+ph-2);
        glEnd();

        // Bresenham top edge highlight
        glColor3f(0.5f,0.95f,1.0f);
        glPointSize(1.5f);
        bresenhamLine((int)px,(int)(py+ph),(int)(px+pw),(int)(py+ph));
        glPointSize(1.0f);

        // Perk border glow
        if (widerPerk || shrunk) {
            glColor3f(widerPerk?0.0f:1.0f, widerPerk?1.0f:0.1f, widerPerk?0.5f:0.5f);
            glLineWidth(2.0f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(px-2,py-2); glVertex2f(px+pw+2,py-2);
            glVertex2f(px+pw+2,py+ph+2); glVertex2f(px-2,py+ph+2);
            glEnd();
            glLineWidth(1.0f);
        }
    }
};
