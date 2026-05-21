#pragma once
#include <GL/glut.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include "hud.h"

enum GameState { STATE_MENU=0, STATE_PLAYING=1, STATE_PAUSED=2,
                 STATE_GAMEOVER=3, STATE_WIN=4, STATE_HELP=5, STATE_HIGHSCORE=6 };

#define MAX_SCORES 5
struct ScoreEntry { char name[32]; int score; float time; int level; };

struct HighScoreTable {
    ScoreEntry entries[MAX_SCORES];
    int count;
    void init() { count=0; }
    void add(const char* name, int score, float time, int level) {
        if(count<MAX_SCORES){
            strncpy(entries[count].name,name,31);
            entries[count].score=score;
            entries[count].time=time;
            entries[count].level=level;
            count++;
        }
        for(int i=0;i<count-1;i++)
            for(int j=i+1;j<count;j++)
                if(entries[j].score>entries[i].score){
                    ScoreEntry tmp=entries[i]; entries[i]=entries[j]; entries[j]=tmp;
                }
        if(count>MAX_SCORES) count=MAX_SCORES;
    }
};

// ── Draw filled rounded-rect (simulated) ────────────────────────────
void drawPanel(float x,float y,float w,float h,float r,float g,float b,float a){
    glColor4f(r,g,b,a);
    glBegin(GL_QUADS);
    glVertex2f(x,y); glVertex2f(x+w,y);
    glVertex2f(x+w,y+h); glVertex2f(x,y+h);
    glEnd();
}

void drawPanelBorder(float x,float y,float w,float h,float r,float g,float b){
    glColor3f(r,g,b);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x,y); glVertex2f(x+w,y);
    glVertex2f(x+w,y+h); glVertex2f(x,y+h);
    glEnd();
    glLineWidth(1.0f);
}

struct Menu {
    int   selectedItem;
    float pulse;
    bool  hasResume;
    HighScoreTable scores;

    void init(){ selectedItem=0; pulse=0; hasResume=false; scores.init(); }

    // ── Starfield ──
    void drawBackground(float winW,float winH){
        // gradient background
        glBegin(GL_QUADS);
        glColor3f(0.00f,0.00f,0.08f); glVertex2f(0,0); glVertex2f(winW,0);
        glColor3f(0.02f,0.01f,0.12f); glVertex2f(winW,winH); glVertex2f(0,winH);
        glEnd();
        // stars
        srand(42);
        glPointSize(1.8f);
        glBegin(GL_POINTS);
        for(int i=0;i<150;i++){
            float bri=0.4f+(rand()%60)/100.0f;
            glColor3f(bri,bri,bri);
            float sx=fmodf((float)(rand()%10000),winW);
            float sy=fmodf((float)(rand()%10000),winH);
            glVertex2f(sx,sy);
        }
        glEnd();
        glPointSize(1.0f);
        srand((unsigned)time(NULL));
    }

    // ── Animated logo ──
    void drawLogo(float winW,float winH){
        float bob=sinf(pulse)*5.0f;
        float cy=winH-90.0f+bob;

        // Glow behind text
        for(int i=5;i>0;i--){
            glColor4f(0.1f,0.5f,1.0f,0.04f*i);
            drawPanel(winW/2-115,(float)(cy-8-i*3),230,50,0,0,0,0);
        }

        // Shadow
        glColor4f(0.0f,0.3f,0.8f,0.4f);
        glRasterPos2f(winW/2-110+3,cy-3);
        const char* title="DX BALL";
        for(int i=0;title[i];i++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,title[i]);

        // Main title — cyan-white
        glColor3f(0.55f,0.95f,1.0f);
        glRasterPos2f(winW/2-110,cy);
        for(int i=0;title[i];i++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,title[i]);

        // Subtitle
        glColor4f(0.6f,0.45f,1.0f,0.9f);
        drawText(winW/2-52,cy-26,"by  RenderRebels",GLUT_BITMAP_HELVETICA_12);

        // Divider line
        float p2=(sinf(pulse*1.5f)+1)/2.0f;
        glColor3f(0.1f+p2*0.2f,0.5f,1.0f);
        glLineWidth(1.2f);
        glBegin(GL_LINES);
        glVertex2f(winW/2-120,cy-34); glVertex2f(winW/2+120,cy-34);
        glEnd();
        glLineWidth(1.0f);
    }

    // ── Single menu item ──
    void drawMenuItem(float x,float y,float itemW,const char* text,bool selected){
        if(selected){
            float glow=(sinf(pulse*2.5f)+1)/2.0f;
            // Selection box
            drawPanel(x-14,y-7,itemW,30,0.05f,0.25f+glow*0.1f,0.65f,0.55f);
            drawPanelBorder(x-14,y-7,itemW,30,0.3f,0.7f+glow*0.2f,1.0f);
            // Arrow
            glColor3f(0.4f,0.9f,1.0f);
            drawText(x-28,y,">");
            glColor3f(1.0f,1.0f,1.0f);
        } else {
            glColor3f(0.55f,0.60f,0.70f);
        }
        drawText(x,y,text);
    }

    int itemCount(){ return hasResume?5:4; }

    void drawMainMenu(float winW,float winH){
        drawBackground(winW,winH);
        drawLogo(winW,winH);

        // Center panel
        float panW=280,panH=220;
        float panX=winW/2-panW/2, panY=winH/2-panH/2-20;
        drawPanel(panX,panY,panW,panH,0.02f,0.04f,0.15f,0.75f);
        drawPanelBorder(panX,panY,panW,panH,0.15f,0.35f,0.7f);

        // item x starts just inside the panel left edge
        float cx   = panX + 30;
        float itemW= panW - 36;   // stays inside panel
        float sy   = panY + panH - 44;
        float step = 42;
        int idx = 0;
        if(hasResume){ drawMenuItem(cx,sy-idx*step,itemW,"Resume Game", selectedItem==idx); idx++; }
        drawMenuItem(cx,sy-idx*step,itemW,"New Game",    selectedItem==idx); idx++;
        drawMenuItem(cx,sy-idx*step,itemW,"High Scores", selectedItem==idx); idx++;
        drawMenuItem(cx,sy-idx*step,itemW,"Help",        selectedItem==idx); idx++;
        drawMenuItem(cx,sy-idx*step,itemW,"Exit",        selectedItem==idx);

        // Controls hint at bottom
        drawPanel(0,0,winW,22,0.0f,0.0f,0.08f,0.85f);
        glColor3f(0.35f,0.40f,0.55f);
        drawText(winW/2-160,5,"UP / DOWN : Navigate      ENTER / Click : Select",
                 GLUT_BITMAP_HELVETICA_12);
    }

    void drawPauseMenu(float winW,float winH){
        // Dark overlay
        glColor4f(0.0f,0.0f,0.08f,0.78f);
        glBegin(GL_QUADS);
        glVertex2f(0,0); glVertex2f(winW,0);
        glVertex2f(winW,winH); glVertex2f(0,winH);
        glEnd();

        float pw=280,ph=230,px=winW/2-pw/2,py=winH/2-ph/2;
        drawPanel(px,py,pw,ph,0.02f,0.04f,0.15f,0.92f);
        drawPanelBorder(px,py,pw,ph,0.3f,0.6f,1.0f);

        glColor3f(1.0f,0.75f,0.1f);
        drawTextLarge(winW/2-50,py+ph-42,"PAUSED");

        float lx=px+30, ly=py+ph-90, ls=34;
        glColor3f(0.85f,0.90f,1.0f);
        drawText(lx,ly,       "P  —  Resume");
        drawText(lx,ly-ls,    "R  —  Restart Level");
        drawText(lx,ly-ls*2,  "M  —  Main Menu");
        drawText(lx,ly-ls*3,  "ESC — Exit Game");
    }

    void drawGameOver(float winW,float winH,int score,float time){
        drawBackground(winW,winH);
        float pw=320,ph=240,px=winW/2-pw/2,py=winH/2-ph/2+20;
        drawPanel(px,py,pw,ph,0.12f,0.01f,0.03f,0.88f);
        drawPanelBorder(px,py,pw,ph,1.0f,0.2f,0.2f);

        glColor3f(1.0f,0.2f,0.2f);
        drawTextLarge(winW/2-80,py+ph-46,"GAME  OVER");

        char buf[64];
        glColor3f(1.0f,0.85f,0.1f);
        snprintf(buf,sizeof(buf),"Score : %d",score);
        drawText(winW/2-65,py+ph-90,buf);

        int m=(int)time/60,s=(int)time%60;
        snprintf(buf,sizeof(buf),"Time  : %02d:%02d",m,s);
        glColor3f(0.7f,0.75f,0.85f);
        drawText(winW/2-65,py+ph-124,buf);

        glColor3f(0.4f,0.85f,1.0f);
        drawText(winW/2-90,py+30,"ENTER — Main Menu");
        drawText(winW/2-90,py+6, "R     — Try Again");
    }

    void drawWin(float winW,float winH,int score,float time,int level){
        drawBackground(winW,winH);
        float pw=340,ph=240,px=winW/2-pw/2,py=winH/2-ph/2+20;
        float glow=(sinf(pulse*3)+1)/2.0f;
        drawPanel(px,py,pw,ph,0.01f,0.12f,0.03f,0.88f);
        drawPanelBorder(px,py,pw,ph,0.2f+glow*0.4f,1.0f,0.3f);

        glColor3f(0.2f+glow*0.5f,1.0f,0.3f);
        drawTextLarge(winW/2-70,py+ph-46,level>=10?"YOU WIN! ALL 10 LEVELS!":"LEVEL CLEAR!");

        char buf[64];
        glColor3f(1.0f,0.85f,0.1f);
        snprintf(buf,sizeof(buf),"Score : %d",score);
        drawText(winW/2-65,py+ph-90,buf);

        int m=(int)time/60,s=(int)time%60;
        snprintf(buf,sizeof(buf),"Time  : %02d:%02d",m,s);
        glColor3f(0.7f,0.75f,0.85f);
        drawText(winW/2-65,py+ph-124,buf);

        glColor3f(0.4f,0.85f,1.0f);
        if(level<10){
            drawText(winW/2-100,py+30,"ENTER — Next Level");
        } else {
            drawText(winW/2-100,py+30,"ENTER — Main Menu");
        }
        drawText(winW/2-100,py+6,"M     — Main Menu");
    }

    void drawHighScores(float winW,float winH){
        drawBackground(winW,winH);
        float pw=420,ph=280,px=winW/2-pw/2,py=winH/2-ph/2;
        drawPanel(px,py,pw,ph,0.02f,0.04f,0.14f,0.88f);
        drawPanelBorder(px,py,pw,ph,0.25f,0.55f,1.0f);

        glColor3f(1.0f,0.85f,0.1f);
        drawTextLarge(winW/2-80,py+ph-42,"HIGH  SCORES");

        char buf[80];
        for(int i=0;i<scores.count;i++){
            int m=(int)scores.entries[i].time/60;
            int s=(int)scores.entries[i].time%60;
            snprintf(buf,sizeof(buf),"%d.  %-14s  %5d pts   %02d:%02d  Lv%d",
                     i+1,scores.entries[i].name,scores.entries[i].score,m,s,
                     scores.entries[i].level);
            float br=1.0f-i*0.15f;
            glColor3f(br,br*0.8f,0.3f);
            drawText(px+20,py+ph-90-i*36,buf,GLUT_BITMAP_HELVETICA_12);
        }
        if(scores.count==0){
            glColor3f(0.45f,0.50f,0.60f);
            drawText(winW/2-70,winH/2,"No scores yet. Play!");
        }
        glColor3f(0.35f,0.40f,0.55f);
        drawText(winW/2-80,py+10,"ESC / ENTER — Back",GLUT_BITMAP_HELVETICA_12);
    }

    void drawHelp(float winW,float winH){
        drawBackground(winW,winH);

        // Bigger box — takes up most of the window
        float pw=680, ph=540;
        float px=winW/2-pw/2, py=winH/2-ph/2;
        drawPanel(px,py,pw,ph,0.02f,0.04f,0.14f,0.92f);
        drawPanelBorder(px,py,pw,ph,0.25f,0.55f,1.0f);

        // Title
        glColor3f(0.4f,0.85f,1.0f);
        drawTextLarge(winW/2-72,py+ph-38,"HOW  TO  PLAY");

        // Divider under title
        glColor3f(0.2f,0.45f,0.85f);
        glLineWidth(1.0f);
        glBegin(GL_LINES);
        glVertex2f(px+20,py+ph-50); glVertex2f(px+pw-20,py+ph-50);
        glEnd();

        float ls = 20.0f;   // line spacing
        float col1x = px+24;
        float col2x = px+pw/2+10;
        float topY  = py+ph-72;

        // ── LEFT COLUMN: Controls ──────────────────────────────
        struct Row { const char* key; const char* desc; };
        Row ctrl[]={
            {"CONTROLS:",""},
            {"LEFT/RIGHT Arrow", "Move Paddle"},
            {"< >  keys",        "Also Move Paddle"},
            {"Mouse",            "Smooth Paddle"},
            {"SPACE / L-Click",  "Launch Ball"},
            {"P",                "Pause / Resume"},
            {"R",                "Restart Level"},
            {"M",                "Main Menu"},
            {"ESC",              "Exit"},
        };
        int nc=sizeof(ctrl)/sizeof(ctrl[0]);
        for(int i=0;i<nc;i++){
            bool hdr=(ctrl[i].desc[0]=='\0');
            if(hdr){ glColor3f(1.0f,0.85f,0.1f); }
            else    { glColor3f(0.75f,0.82f,0.95f); }
            char buf[80];
            if(hdr) snprintf(buf,sizeof(buf),"%s",ctrl[i].key);
            else     snprintf(buf,sizeof(buf),"%-18s %s",ctrl[i].key,ctrl[i].desc);
            drawText(col1x, topY-i*ls, buf, GLUT_BITMAP_HELVETICA_12);
        }

        // ── LEFT COLUMN continued: Block types ─────────────────
        float blockY = topY - (nc+1)*ls;
        Row blk[]={
            {"BLOCK TYPES:",""},
            {"Cyan   (1 hit)",  "Normal"},
            {"Green  (3 hits)", "Medium"},
            {"Yellow (5 hits)", "Hard"},
            {"Orange (10 hits)","Very Hard"},
            {"Red    (20 hits)","Boss Block"},
            {"Silver   X",      "Unbreakable!"},
        };
        int nb=sizeof(blk)/sizeof(blk[0]);
        for(int i=0;i<nb;i++){
            bool hdr=(blk[i].desc[0]=='\0');
            if(hdr){ glColor3f(1.0f,0.85f,0.1f); }
            else    { glColor3f(0.75f,0.82f,0.95f); }
            char buf[80];
            if(hdr) snprintf(buf,sizeof(buf),"%s",blk[i].key);
            else     snprintf(buf,sizeof(buf),"%-18s %s",blk[i].key,blk[i].desc);
            drawText(col1x, blockY-i*ls, buf, GLUT_BITMAP_HELVETICA_12);
        }

        // ── RIGHT COLUMN: Perks ────────────────────────────────
        Row prk[]={
            {"PERKS:",""},
            {"+LIFE (green)",   "Extra Life"},
            {"FAST  (orange)",  "Faster Ball"},
            {"WIDE  (blue)",    "Wider Paddle"},
            {"FIRE  (red)",     "Fireball mode!"},
            {"SHRK  (purple)",  "Shrinks Paddle"},
            {"MULTI (yellow)",  "Multi Ball!"},
        };
        int np=sizeof(prk)/sizeof(prk[0]);
        for(int i=0;i<np;i++){
            bool hdr=(prk[i].desc[0]=='\0');
            if(hdr){ glColor3f(1.0f,0.85f,0.1f); }
            else    { glColor3f(0.75f,0.82f,0.95f); }
            char buf[80];
            if(hdr) snprintf(buf,sizeof(buf),"%s",prk[i].key);
            else     snprintf(buf,sizeof(buf),"%-16s %s",prk[i].key,prk[i].desc);
            drawText(col2x, topY-i*ls, buf, GLUT_BITMAP_HELVETICA_12);
        }

        // ── RIGHT COLUMN: Tips ─────────────────────────────────
        float tipY = topY - (np+1)*ls;
        Row tips[]={
            {"TIPS:",""},
            {"Ball speeds up",   "every 15 sec"},
            {"Fireball ball",    "passes through!"},
            {"Explosive block",  "destroys nearby"},
            {"Unbreakable",      "never breaks"},
            {"3 Levels total",   "get high score!"},
        };
        int nt=sizeof(tips)/sizeof(tips[0]);
        for(int i=0;i<nt;i++){
            bool hdr=(tips[i].desc[0]=='\0');
            if(hdr){ glColor3f(1.0f,0.85f,0.1f); }
            else    { glColor3f(0.65f,0.72f,0.85f); }
            char buf[80];
            if(hdr) snprintf(buf,sizeof(buf),"%s",tips[i].key);
            else     snprintf(buf,sizeof(buf),"%-16s %s",tips[i].key,tips[i].desc);
            drawText(col2x, tipY-i*ls, buf, GLUT_BITMAP_HELVETICA_12);
        }

        // Bottom hint — inside box
        glColor3f(0.30f,0.38f,0.55f);
        drawText(winW/2-80, py+12, "ESC / ENTER — Back", GLUT_BITMAP_HELVETICA_12);
    }

    void update(float dt){ pulse+=dt*1.8f; }
    void navigate(int dir){ selectedItem=(selectedItem+dir+itemCount())%itemCount(); }
};
