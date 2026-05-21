#include <GL/glut.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>

#include "ball.h"
#include "paddle.h"
#include "block.h"
#include "perk.h"
#include "hud.h"
#include "menu.h"

// ── Window ───────────────────────────────────────────────────────────
static const float WIN_W = 800.0f;
static const float WIN_H = 650.0f;

// ── Global objects ────────────────────────────────────────────────────
Ball         ball;
Paddle       paddle;
BlockGrid    grid;
PerkSystem   perks;
HUD          hud;
Menu         menu;
ParticleSystem particles;

GameState    state = STATE_MENU;

// Key states — arrow keys AND < > keys
bool keyLeft  = false;
bool keyRight = false;

float speedTimer = 0;
const float SPEED_INTERVAL = 8.0f;

// Multi-ball
#define MAX_BALLS 4
Ball extraBalls[MAX_BALLS];
bool extraBallActive[MAX_BALLS];

// ── Helpers ───────────────────────────────────────────────────────────
void startNewGame() {
    srand((unsigned)time(NULL));
    ball.init(WIN_W, WIN_H);
    paddle.init(WIN_W);
    grid.buildLevel(hud.level, WIN_W, WIN_H);
    perks.clear();
    particles.clear();
    speedTimer = 0;
    for (int i = 0; i < MAX_BALLS; i++) extraBallActive[i] = false;
}

void spawnMultiBalls() {
    int spawned = 0;
    for (int i = 0; i < MAX_BALLS && spawned < 2; i++) {
        if (!extraBallActive[i]) {
            extraBalls[i]         = ball;
            extraBalls[i].vx      = ball.vx * (spawned == 0 ? 1.3f : -1.3f);
            extraBalls[i].vy      = fabsf(ball.vy);
            extraBalls[i].active  = true;
            extraBalls[i].launched= true;
            extraBallActive[i]    = true;
            spawned++;
        }
    }
}

// ── Background ────────────────────────────────────────────────────────
void drawBackground() {
    glBegin(GL_QUADS);
    glColor3f(0.02f,0.02f,0.10f); glVertex2f(0,0);       glVertex2f(WIN_W,0);
    glColor3f(0.04f,0.02f,0.14f); glVertex2f(WIN_W,WIN_H); glVertex2f(0,WIN_H);
    glEnd();
    // subtle grid
    glColor4f(0.07f,0.07f,0.20f,0.45f);
    glLineWidth(0.5f);
    for (int x=0;x<=(int)WIN_W;x+=40){
        glBegin(GL_LINES); glVertex2f((float)x,0); glVertex2f((float)x,WIN_H); glEnd();
    }
    for (int y=0;y<=(int)WIN_H;y+=40){
        glBegin(GL_LINES); glVertex2f(0,(float)y); glVertex2f(WIN_W,(float)y); glEnd();
    }
    glLineWidth(1.0f);
}

// ── Display ───────────────────────────────────────────────────────────
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    switch (state) {
    case STATE_MENU:
        menu.drawMainMenu(WIN_W, WIN_H);
        break;
    case STATE_HELP:
        menu.drawHelp(WIN_W, WIN_H);
        break;
    case STATE_HIGHSCORE:
        menu.drawHighScores(WIN_W, WIN_H);
        break;
    case STATE_PLAYING:
    case STATE_PAUSED:
        drawBackground();
        grid.draw();
        perks.draw();
        particles.draw();
        paddle.draw();
        ball.draw(hud.fireballActive, true);   // main ball — cyan ring
        for (int i = 0; i < MAX_BALLS; i++)
            if (extraBallActive[i]) extraBalls[i].draw(hud.fireballActive, false); // extra — yellow
        hud.draw(WIN_W, WIN_H);
        if (state == STATE_PAUSED)
            menu.drawPauseMenu(WIN_W, WIN_H);
        break;
    case STATE_GAMEOVER:
        menu.drawGameOver(WIN_W, WIN_H, hud.score, hud.time);
        break;
    case STATE_WIN:
        menu.drawWin(WIN_W, WIN_H, hud.score, hud.time, hud.level);
        break;
    }

    glutSwapBuffers();
}

// ── Apply perk ────────────────────────────────────────────────────────
void applyPerk(PerkType pt) {
    switch (pt) {
    case PERK_EXTRALIFE:
        hud.lives++; if(hud.lives > 5) hud.lives = 5; if (hud.lives > 5) hud.lives = 5;
        hud.showPerkMessage("+ EXTRA LIFE!");
        particles.burst(WIN_W/2, WIN_H/2, 0.1f,1.0f,0.35f, 35);
        break;
    case PERK_FASTBALL:
        ball.increaseSpeed(1.22f);
        hud.showPerkMessage("SPEED BOOST!");
        particles.burst(ball.x,ball.y, 1.0f,0.5f,0.0f, 20);
        break;
    case PERK_WIDER:
        paddle.applyWider();
        hud.showPerkMessage("WIDER PADDLE!");
        particles.burst(paddle.x+paddle.width/2,paddle.y+paddle.height, 0.1f,0.8f,1.0f, 20);
        break;
    case PERK_FIREBALL:
        hud.fireballActive = true;
        hud.fireballTimer  = 8.0f;
        hud.showPerkMessage("FIREBALL!");
        particles.burst(ball.x,ball.y, 1.0f,0.35f,0.0f, 30);
        break;
    case PERK_SHRINK:
        paddle.applyShrink();
        hud.showPerkMessage("PADDLE SHRUNK :(");
        particles.burst(paddle.x+paddle.width/2,paddle.y+paddle.height, 0.8f,0.0f,0.8f, 15);
        break;
    case PERK_MULTIBALL:
        spawnMultiBalls();
        hud.showPerkMessage("MULTI BALL!");
        particles.burst(WIN_W/2,WIN_H/2, 1.0f,1.0f,0.0f, 35);
        break;
    default: break;
    }
}

// ── Respawn ball on paddle ────────────────────────────────────────────
void respawnBall() {
    ball.init(WIN_W, WIN_H);
    ball.active   = true;
    ball.launched = false;
    ball.x = paddle.x + paddle.width / 2.0f;
    ball.y = paddle.y + paddle.height + ball.radius + 3;
    ball.trailCount = 0;
    // clear extra balls on respawn
    for (int i = 0; i < MAX_BALLS; i++) extraBallActive[i] = false;
}

// ── Process one ball's block collisions ──────────────────────────────
void processBallBlocks(Ball& b) {
    if (!b.active || !b.launched) return;
    PerkType dropped = grid.checkBallCollision(
        b.x, b.y, (float)b.radius,
        b.vx, b.vy, hud.score, hud.fireballActive);
    if (dropped != PERK_NONE) {
        perks.spawn(b.x, b.y, dropped);
        particles.burst(b.x, b.y, 0.9f,0.7f,0.2f, 10);
    }
}

// ── Update ────────────────────────────────────────────────────────────
void update(int) {
    const float dt = 1.0f / 60.0f;

    menu.update(dt);

    if (state == STATE_PLAYING) {
        hud.update(dt);

        // Keyboard paddle: arrow keys + < >
        if (keyLeft)  paddle.moveLeft();
        if (keyRight) paddle.moveRight(WIN_W);

        paddle.update(WIN_W, dt);

        // Gradual speed increase
        speedTimer += dt;
        if (speedTimer >= SPEED_INTERVAL) {
            speedTimer = 0;
            ball.increaseSpeed(1.08f);
        }

        // Attach ball to paddle before launch
        if (!ball.launched) {
            ball.x = paddle.x + paddle.width / 2.0f;
            ball.y = paddle.y + paddle.height + ball.radius + 2;
        }

        // Update main ball
        ball.update(WIN_W, WIN_H, paddle.x, paddle.width, paddle.y, paddle.height);

        // Extra balls — পড়লে শুধু সেটাই মরে, life কমে না
        for (int i = 0; i < MAX_BALLS; i++) {
            if (!extraBallActive[i]) continue;
            extraBalls[i].update(WIN_W, WIN_H, paddle.x, paddle.width, paddle.y, paddle.height);
            if (extraBalls[i].y < 0) {
                extraBallActive[i] = false;
                particles.burst(extraBalls[i].x, 15, 1.0f, 0.5f, 0.1f, 8);
            }
        }

        // Main ball পড়লে
        if (ball.active && ball.y < 0) {
            ball.active = false;
            particles.burst(ball.x, 15, 1.0f, 0.2f, 0.2f, 25);

            // Extra ball বেঁচে আছে? promote করো — life কমবে না
            bool promoted = false;
            for (int i = 0; i < MAX_BALLS; i++) {
                if (extraBallActive[i]) {
                    ball = extraBalls[i];
                    ball.active   = true;
                    ball.launched = true;
                    extraBallActive[i] = false;
                    promoted = true;
                    break;
                }
            }

            if (!promoted) {
                // সব ball শেষ — life কমবে
                hud.lives--;
                if (hud.lives <= 0) {
                    menu.scores.add("RenderRebels", hud.score, hud.time, hud.level);
                    state = STATE_GAMEOVER;
                } else {
                    respawnBall();
                }
            }
        }

        // Block collisions
        processBallBlocks(ball);
        for (int i = 0; i < MAX_BALLS; i++)
            if (extraBallActive[i]) processBallBlocks(extraBalls[i]);

        // Perks
        PerkType caught = perks.update(dt, paddle.x, paddle.width, paddle.y, paddle.height);
        if (caught != PERK_NONE) applyPerk(caught);

        particles.update(dt);
        grid.update(dt);

        // Win check
        if (grid.alive <= 0) {
            menu.scores.add("RenderRebels", hud.score, hud.time, hud.level);
            state = STATE_WIN;
        }
    } else {
        particles.update(dt);
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// ── Menu select ───────────────────────────────────────────────────────
void handleMenuSelect() {
    int idx = menu.selectedItem;
    if (menu.hasResume) {
        if (idx == 0) { state = STATE_PLAYING; return; }
        idx--;
    }
    switch (idx) {
    case 0: // New Game
        hud.init(); hud.level = 1;
        startNewGame();
        menu.hasResume = true;
        state = STATE_PLAYING;
        break;
    case 1: state = STATE_HIGHSCORE; break;
    case 2: state = STATE_HELP;      break;
    case 3: exit(0);
    }
}

// ── Keyboard ──────────────────────────────────────────────────────────
void keyDown(unsigned char key, int /*x*/, int /*y*/) {
    switch (state) {
    case STATE_MENU:
        if (key == 13) handleMenuSelect();
        break;

    case STATE_PLAYING:
        if (key == ' ')              { if (!ball.launched) ball.launch(); }
        if (key == 'p'||key == 'P')  state = STATE_PAUSED;
        if (key == 'm'||key == 'M')  { state = STATE_MENU; }
        if (key == 'r'||key == 'R')  { startNewGame(); }
        if (key == 27)               exit(0);
        // < > keys for paddle
        if (key == ',')              keyLeft  = true;
        if (key == '.')              keyRight = true;
        break;

    case STATE_PAUSED:
        if (key == 'p'||key == 'P')  state = STATE_PLAYING;
        if (key == 'r'||key == 'R')  { startNewGame(); state = STATE_PLAYING; }
        if (key == 'm'||key == 'M')  state = STATE_MENU;
        if (key == 27)               exit(0);
        break;

    case STATE_GAMEOVER:
        if (key == 13)               { hud.init(); hud.level=1; startNewGame(); state=STATE_MENU; }
        if (key == 'r'||key == 'R')  { hud.init(); hud.level=1; startNewGame(); state=STATE_PLAYING; }
        break;

    case STATE_WIN:
        if (key == 13) {
            if (hud.level < 10) { hud.level++; startNewGame(); state=STATE_PLAYING; }
            else                { state=STATE_MENU; }
        }
        if (key == 'm'||key == 'M') state=STATE_MENU;
        break;

    case STATE_HELP:
    case STATE_HIGHSCORE:
        if (key == 27||key == 13) state=STATE_MENU;
        break;
    }
}

void keyUp(unsigned char key, int /*x*/, int /*y*/) {
    if (key == ','||key == '<') keyLeft  = false;
    if (key == '.'||key == '>') keyRight = false;
}

void specialKeyDown(int key, int /*x*/, int /*y*/) {
    if (state==STATE_MENU||state==STATE_HIGHSCORE||state==STATE_HELP) {
        if (key==GLUT_KEY_UP)   menu.navigate(-1);
        if (key==GLUT_KEY_DOWN) menu.navigate(1);
    }
    if (state==STATE_PLAYING) {
        if (key==GLUT_KEY_LEFT)  keyLeft  = true;
        if (key==GLUT_KEY_RIGHT) keyRight = true;
    }
}

void specialKeyUp(int key, int /*x*/, int /*y*/) {
    if (key==GLUT_KEY_LEFT)  keyLeft  = false;
    if (key==GLUT_KEY_RIGHT) keyRight = false;
}

// ── Mouse ─────────────────────────────────────────────────────────────
void mouseMotion(int x, int /*y*/) {
    if (state==STATE_PLAYING)
        paddle.setTarget((float)x * (WIN_W / glutGet(GLUT_WINDOW_WIDTH)));
}

void mouseClick(int button, int mstate, int x, int y) {
    if (button==GLUT_LEFT_BUTTON && mstate==GLUT_DOWN) {
        if (state==STATE_PLAYING) {
            if (!ball.launched) ball.launch();   // first click: launch
            else                state=STATE_PAUSED; // subsequent: pause + show menu
        }
        if (state==STATE_PAUSED)     state=STATE_PLAYING; // click again: resume
        if (state==STATE_MENU)       handleMenuSelect();
    }
}

// ── Init ──────────────────────────────────────────────────────────────
void init() {
    glClearColor(0.02f,0.02f,0.10f,1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0,WIN_W,0.0,WIN_H);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    srand((unsigned)time(NULL));
    hud.init();
    menu.init();
    particles.clear();
    for (int i=0;i<MAX_BALLS;i++) extraBallActive[i]=false;
}

// ── Main ──────────────────────────────────────────────────────────────
int main(int argc,char** argv) {
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize((int)WIN_W,(int)WIN_H);
    glutInitWindowPosition(100,50);
    glutCreateWindow("DX Ball  —  RenderRebels  |  CSE 426");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutSpecialFunc(specialKeyDown);
    glutSpecialUpFunc(specialKeyUp);
    glutPassiveMotionFunc(mouseMotion);
    glutMotionFunc(mouseMotion);
    glutMouseFunc(mouseClick);
    glutTimerFunc(16,update,0);

    glutMainLoop();
    return 0;
}
