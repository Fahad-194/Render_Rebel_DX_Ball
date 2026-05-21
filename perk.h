#pragma once
#include <GL/glut.h>
#include <cmath>
#include "block.h"

struct PerkDrop {
    float x, y;
    float vy;
    PerkType type;
    bool  active;
    float angle;

    static constexpr float COLORS[7][3] = {
        {0,0,0},
        {0.1f,1.0f,0.4f},   // EXTRALIFE  green
        {1.0f,0.55f,0.0f},  // FASTBALL   orange
        {0.1f,0.55f,1.0f},  // WIDER      blue
        {1.0f,0.2f,0.0f},   // FIREBALL   red
        {0.65f,0.0f,1.0f},  // SHRINK     purple
        {1.0f,1.0f,0.0f},   // MULTIBALL  yellow
    };

    static const char* label(PerkType t) {
        switch(t){
            case PERK_EXTRALIFE: return "+LIFE";
            case PERK_FASTBALL:  return "FAST";
            case PERK_WIDER:     return "WIDE";
            case PERK_FIREBALL:  return "FIRE";
            case PERK_SHRINK:    return "SHRK";
            case PERK_MULTIBALL: return "MULTI";
            default: return "";
        }
    }

    void spawn(float bx, float by, PerkType t) {
        x=bx; y=by; vy=-2.8f; type=t; active=true; angle=0;
    }

    bool update(float dt, float px, float pw, float py, float ph) {
        if (!active) return false;
        y += vy; angle += 3.5f;
        if (y < 0) { active=false; return false; }
        if (y <= py+ph && y >= py && x >= px && x <= px+pw) {
            active=false; return true;
        }
        return false;
    }

    void draw() {
        if (!active) return;
        float r=COLORS[type][0], g=COLORS[type][1], b=COLORS[type][2];

        glPushMatrix();
        glTranslatef(x, y, 0);
        glRotatef(angle, 0, 0, 1);

        // Glow
        glColor4f(r, g, b, 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(-16,0); glVertex2f(0,16);
        glVertex2f(16,0);  glVertex2f(0,-16);
        glEnd();
        // Diamond fill
        glColor4f(r, g, b, 0.55f);
        glBegin(GL_QUADS);
        glVertex2f(-12,0); glVertex2f(0,12);
        glVertex2f(12,0);  glVertex2f(0,-12);
        glEnd();
        // Border
        glColor3f(r, g, b);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(-12,0); glVertex2f(0,12);
        glVertex2f(12,0);  glVertex2f(0,-12);
        glEnd();
        glLineWidth(1.0f);

        glPopMatrix();

        // Label
        glColor3f(1,1,1);
        glRasterPos2f(x-14, y-4);
        const char* lbl = label(type);
        for (int i=0; lbl[i]; i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, lbl[i]);
    }
};

constexpr float PerkDrop::COLORS[7][3];

#define MAX_PERKS 20

struct PerkSystem {
    PerkDrop drops[MAX_PERKS];

    void clear() { for(int i=0;i<MAX_PERKS;i++) drops[i].active=false; }

    void spawn(float x, float y, PerkType t) {
        for(int i=0;i<MAX_PERKS;i++){
            if(!drops[i].active){ drops[i].spawn(x,y,t); return; }
        }
    }

    PerkType update(float dt, float px, float pw, float py, float ph) {
        PerkType caught=PERK_NONE;
        for(int i=0;i<MAX_PERKS;i++){
            if(drops[i].active){
                bool c=drops[i].update(dt,px,pw,py,ph);
                if(c) caught=drops[i].type;
            }
        }
        return caught;
    }

    void draw(){ for(int i=0;i<MAX_PERKS;i++) drops[i].draw(); }
};
