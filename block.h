#pragma once
#include <GL/glut.h>
#include <cstring>
#include <cmath>

void floodFillRect(float x,float y,float w,float h,float r,float g,float b){
    glColor3f(r,g,b);
    glBegin(GL_QUADS);
    glVertex2f(x,y); glVertex2f(x+w,y);
    glVertex2f(x+w,y+h); glVertex2f(x,y+h);
    glEnd();
}

void boundaryFillBorder(float x,float y,float w,float h,float r,float g,float b){
    glColor3f(r,g,b);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x,y); glVertex2f(x+w,y);
    glVertex2f(x+w,y+h); glVertex2f(x,y+h);
    glEnd();
    glLineWidth(1.0f);
}

// HP tiers:  1, 3, 5, 10, 20, 999(unbreakable)
enum PerkType { PERK_NONE=0, PERK_EXTRALIFE=1, PERK_FASTBALL=2, PERK_WIDER=3,
                PERK_FIREBALL=4, PERK_SHRINK=5, PERK_MULTIBALL=6 };

struct Block {
    float x, y, w, h;
    int   maxHp, hp;
    bool  unbreakable;
    PerkType perk;
    bool  alive;
    float flashTimer;

    // Color based on HP tier — bright, no black
    void getColor(float& r, float& g, float& b) {
        if (unbreakable) {
            // Steel/silver shimmer
            r=0.75f; g=0.80f; b=0.85f; return;
        }
        if (flashTimer > 0) { r=g=b=1.0f; return; }
        switch (maxHp) {
            case 1:  r=0.25f; g=0.85f; b=1.00f; break; // cyan
            case 3:  r=0.30f; g=1.00f; b=0.35f; break; // green
            case 5:  r=1.00f; g=0.85f; b=0.10f; break; // yellow
            case 10: r=1.00f; g=0.45f; b=0.10f; break; // orange
            case 20: r=1.00f; g=0.15f; b=0.20f; break; // red
            default: r=0.80f; g=0.20f; b=1.00f; break; // purple fallback
        }
        // Darken as HP drops
        if (!unbreakable && maxHp > 1) {
            float ratio = (float)hp / (float)maxHp;
            float dark  = 0.45f + ratio * 0.55f;
            r *= dark; g *= dark; b *= dark;
        }
    }

    void init(float bx,float by,float bw,float bh,int hpVal,bool unbr,PerkType pt){
        x=bx; y=by; w=bw; h=bh;
        maxHp=hpVal; hp=hpVal;
        unbreakable=unbr;
        perk=pt;
        alive=true;
        flashTimer=0;
    }

    PerkType hit(){
        if(!alive||unbreakable) return PERK_NONE;
        flashTimer=0.12f;
        hp--;
        if(hp<=0){ alive=false; return perk; }
        return PERK_NONE;
    }

    void update(float dt){ if(flashTimer>0) flashTimer-=dt; }

    void draw(){
        if(!alive) return;
        float r,g,b;
        getColor(r,g,b);

        // Flood fill body
        floodFillRect(x+1,y+1,w-2,h-2,r,g,b);

        // Shine strip (top half)
        glBegin(GL_QUADS);
        glColor4f(1,1,1,0.22f);
        glVertex2f(x+2,y+h/2); glVertex2f(x+w-2,y+h/2);
        glColor4f(1,1,1,0.0f);
        glVertex2f(x+w-2,y+h-2); glVertex2f(x+2,y+h-2);
        glEnd();

        // Border
        if(unbreakable){
            // Hatched border for steel
            glColor3f(0.5f,0.55f,0.6f);
        } else {
            glColor3f(r*0.45f,g*0.45f,b*0.45f);
        }
        boundaryFillBorder(x,y,w,h,r*0.45f,g*0.45f,b*0.45f);

        // Unbreakable: X pattern
        if(unbreakable){
            glColor3f(0.4f,0.45f,0.5f);
            glLineWidth(1.2f);
            glBegin(GL_LINES);
            glVertex2f(x+4,y+4); glVertex2f(x+w-4,y+h-4);
            glVertex2f(x+w-4,y+4); glVertex2f(x+4,y+h-4);
            glEnd();
            glLineWidth(1.0f);
        }

        // HP indicator — white dots with dark background for contrast
        if(!unbreakable && maxHp>1){
            int show = hp; if(show>10) show=10;
            float gap=6.0f;
            float totalW = (show-1)*gap;
            float startX = x + w/2 - totalW/2;
            // Dark strip behind dots
            glColor4f(0.0f,0.0f,0.0f,0.55f);
            glBegin(GL_QUADS);
            glVertex2f(startX-4, y+h/2-4);
            glVertex2f(startX+totalW+4, y+h/2-4);
            glVertex2f(startX+totalW+4, y+h/2+5);
            glVertex2f(startX-4, y+h/2+5);
            glEnd();
            // Bright white dots
            glColor3f(1.0f, 1.0f, 1.0f);
            glPointSize(3.5f);
            glBegin(GL_POINTS);
            for(int i=0;i<show;i++)
                glVertex2f(startX+i*gap, y+h/2);
            glEnd();
            glPointSize(1.0f);
        }

        // Perk star indicator
        if(perk!=PERK_NONE){
            glColor3f(1.0f,1.0f,0.0f);
            glPointSize(4.5f);
            glBegin(GL_POINTS);
            glVertex2f(x+w-8,y+h-7);
            glEnd();
            glPointSize(1.0f);
        }
    }
};

#define MAX_BLOCKS 300

struct BlockGrid {
    Block blocks[MAX_BLOCKS];
    int   count, alive;

    void buildLevel(int level, float winW, float winH){
        count=0; alive=0;

        int cols=13;
        // rows: level1=4 rows, increases by 1 each level, max 13
        int rows = 3 + level;
        if(rows > 13) rows = 13;
        float bw=(winW-40.0f)/cols;
        float bh=22.0f;
        float startY=winH-125.0f;

        // max HP allowed per level
        int maxHpAllowed = 1;
        if(level>=3)  maxHpAllowed=3;
        if(level>=5)  maxHpAllowed=5;
        if(level>=7)  maxHpAllowed=10;
        if(level>=9)  maxHpAllowed=20;

        for(int r=0;r<rows&&count<MAX_BLOCKS;r++){
            for(int c=0;c<cols&&count<MAX_BLOCKS;c++){
                int idx=r*cols+c;
                bool unbr=false;
                int hpVal=1;

                // HP based on row ratio
                float ratio=(rows>1)?(float)r/(rows-1):0;
                if     (ratio<0.25f) hpVal=1;
                else if(ratio<0.50f) hpVal=(maxHpAllowed>=3)?3:1;
                else if(ratio<0.72f) hpVal=(maxHpAllowed>=5)?5:(maxHpAllowed>=3)?3:1;
                else if(ratio<0.88f) hpVal=(maxHpAllowed>=10)?10:(maxHpAllowed>=5)?5:3;
                else                 hpVal=maxHpAllowed;

                // Unbreakable from level 4, frequency increases
                if(level>=4){
                    int freq=16-level; if(freq<4) freq=4;
                    if(idx%freq==0) unbr=true;
                }

                // Perks
                PerkType pt=PERK_NONE;
                if(!unbr){
                    if     (idx%22==0) pt=PERK_EXTRALIFE;
                    else if(idx%17==0) pt=PERK_WIDER;
                    else if(idx%13==0) pt=PERK_FASTBALL;
                    else if(idx%25==0) pt=PERK_FIREBALL;
                    else if(idx%31==0) pt=PERK_SHRINK;
                    else if(idx%33==0) pt=PERK_MULTIBALL;
                }

                float bx=20.0f+c*bw;
                float by=startY-r*(bh+4);
                blocks[count].init(bx,by,bw-3,bh,hpVal,unbr,pt);
                count++;
                if(!unbr) alive++;
            }
        }
    }

    PerkType checkBallCollision(float bx,float by,float br,
                                 float& vx,float& vy,
                                 int& score,bool fireball){
        PerkType dropped=PERK_NONE;
        for(int i=0;i<count;i++){
            Block& bl=blocks[i];
            if(!bl.alive) continue;

            float cx=fmaxf(bl.x,fminf(bx,bl.x+bl.w));
            float cy=fmaxf(bl.y,fminf(by,bl.y+bl.h));
            float dx=bx-cx, dy=by-cy;
            if(dx*dx+dy*dy < br*br){
                bool wasAlive=bl.alive;
                PerkType p=bl.hit();
                if(p!=PERK_NONE) dropped=p;

                if(wasAlive&&!bl.alive){
                    if(bl.maxHp==20)      score+=100;
                    else if(bl.maxHp==10) score+=50;
                    else if(bl.maxHp==5)  score+=30;
                    else if(bl.maxHp==3)  score+=20;
                    else                  score+=10;
                    alive--;
                }

                if(!fireball){
                    float ovX=(dx>0)?(bl.x+bl.w-(bx-br)):((bx+br)-bl.x);
                    float ovY=(dy>0)?(bl.y+bl.h-(by-br)):((by+br)-bl.y);
                    if(fabsf(ovX)<fabsf(ovY)) vx=-vx;
                    else vy=-vy;
                }
                break;
            }
        }
        return dropped;
    }

    void update(float dt){ for(int i=0;i<count;i++) blocks[i].update(dt); }
    void draw()          { for(int i=0;i<count;i++) blocks[i].draw(); }
};
