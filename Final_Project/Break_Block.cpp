#include <GLUT/GLUT.h>
#include <cmath>
#define    PI               M_PI
#define    WIDTH            1000
#define    HEIGHT           1000
#define    STATE_BREAK      0
#define    STATE_ONE        1
#define    STATE_TWO        2
#define    MODE_DEFAULT     0

typedef struct _Point {
    float    x;
    float    y;
} Point;

typedef struct _Color {
    float   red;
    float   green;
    float   blue;
    float   clamp = 0.0;
} Color;

typedef struct _Block {
    Point leftTop;
    Point leftBottom;
    Point rightTop;
    Point rightBottom;
    int mode = MODE_DEFAULT;
    int state = STATE_ONE;
} Block;

typedef struct _Bar {
    Point center;
    int len;
    int weight;
} Bar;

int left = 0;
int bottom = 0;

/// 슬라이딩 바 초기값
int slidingBarLen = 200;
int slidingBarWeight = 20;
int slidingBarPosition = 0;
int slidingBarSpeed = 10;

/// 공 초기값 및 선언
float ballRadius = 10.0;
Point ballPosition = { WIDTH / 2, slidingBarWeight + ballRadius };

Point Wall[] = {
    {  150,    0 },
    {  150,  350 },
    {    0,  700 },
    {  300,  700 },
    {  500, 1000 },
    {  700,  700 },
    { 1000,  700 },
    {  850,  350 },
    {  850,    0 }
};

Color backGroundColor = { 0.1, 0.1, 0.1 };
Color wallColor = { 0.9, 0.8, 0.5 };
Color slidingBarColor = { 0.5, 0.8, 0.7 };
Color ballColor = { 0.97, 0.95, 0.99 };

Bar slidingBar = { WIDTH / 2, 0, slidingBarLen, slidingBarWeight };

void ShowBall() {
    int num = 36;
    float delta = 2 * PI / num;
    glBegin(GL_POLYGON);
    for(int i = 0; i < num; i ++) {
        glVertex2f(ballPosition.x + ballRadius * cos(delta * i), ballPosition.y + ballRadius * sin(delta * i));
    }
    glEnd();
}

void ShowSlidingBar() {
    glBegin(GL_POLYGON);
    glVertex2i(slidingBarPosition + slidingBar.center.x - slidingBar.len / 2, slidingBar.center.y);
    glVertex2i(slidingBarPosition + slidingBar.center.x - slidingBar.len / 2, slidingBar.center.y + slidingBar.weight);
    glVertex2i(slidingBarPosition + slidingBar.center.x + slidingBar.len / 2, slidingBar.center.y + slidingBar.weight);
    glVertex2i(slidingBarPosition + slidingBar.center.x + slidingBar.len / 2, slidingBar.center.y);
    glEnd();
}

void ShowWall() {
    glLineWidth(3.0);
    glBegin(GL_LINE_STRIP);
    for(int i = 0; i < 9; i ++) {
        glVertex2f(Wall[i].x, Wall[i].y);
    }
    glEnd();
}

void MySpecialKey(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:
            slidingBarPosition -= slidingBarPosition + slidingBar.center.x - slidingBar.len / 2 > Wall[0].x ? slidingBarSpeed : 0;
            break;
        case GLUT_KEY_RIGHT:
            slidingBarPosition += slidingBarPosition + slidingBar.center.x + slidingBar.len / 2 < Wall[8].x ? slidingBarSpeed : 0;
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

void MyReshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(left, left + WIDTH, bottom, bottom + HEIGHT);
}

void RenderScene(void) {
    glClearColor(backGroundColor.red, backGroundColor.green, backGroundColor.blue, backGroundColor.clamp);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glColor3f(ballColor.red, ballColor.green, ballColor.blue);
    ShowBall();
    
    glColor3f(slidingBarColor.red, slidingBarColor.green, slidingBarColor.blue);
    ShowSlidingBar();
    
    glColor3f(wallColor.red, wallColor.green, wallColor.blue);
    ShowWall();
    
    glutSwapBuffers();
    glFlush();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowPosition(0, 0);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Break the Block!");
    glutReshapeFunc(MyReshape);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(MySpecialKey);
    glutMainLoop();
}
