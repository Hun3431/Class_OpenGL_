#include <GLUT/GLUT.h>
#include <iostream>
#include <cmath>
#include <random>
#include <ctime>
#define    PI               M_PI
#define    WIDTH            1000
#define    HEIGHT           1000
#define    STATE_BREAK      0
#define    STATE_ONE        1
#define    STATE_TWO        2
#define    MODE_DEFAULT     0
#define    RECTANGLE_BLOCK_NUM 19
#define    WALL_NUM         9
#define    READY            0
#define    RUN              1
#define    GAMEOVER         2
#define    CLEAR            3

/*
 *  구조체 선언부
 */
/// 좌표들의 정보를 나타내는 구조체
typedef struct _Point {
    float    x = 0.0;
    float    y = 0.0;
} Point;

/// 색상(RGBC)의 정보를 나타내는 구조체
typedef struct _Color {
    float   red;
    float   green;
    float   blue;
    float   clamp = 0.0;
} Color;

/// 벽돌의 정보를 나타내는 구조체
typedef struct _Block {
    Point leftTop = { 0, 0 };
    Point leftBottom = { 0, 0 };
    Point rightTop = { 0, 0 };
    Point rightBottom = { 0, 0 };
    int mode = MODE_DEFAULT;
    int state = STATE_TWO;
} Block;

/// 하단의 슬라이딩 바의 정보를 나타내는 구조체
typedef struct _Bar {
    Point center;
    int len;
    int weight;
} Bar;

/// 벡터를 나타내는 구조체
struct Vector {
    float x;
    float y;

    Vector(float _x = 0.0f, float _y = 0.0f) {
        x = _x;
        y = _y;
    }

    // 벡터 정규화
    void normalize() {
        float length = std::sqrt(x * x + y * y);
        x /= length;
        y /= length;
    }

    // 벡터 내적
    float dot(const Vector& other) const {
        return x * other.x + y * other.y;
    }
    
    // 벡터의 반사
    Vector reflect(const Vector& normal) const {
        float dotProduct = dot(normal);
        Vector reflection = *this - (normal * (2.0f * dotProduct));
        return reflection;
    }

    // 벡터 뺄셈 연산자 오버로딩
    Vector operator-(const Vector& other) const {
        return Vector(x - other.x, y - other.y);
    }

    // 스칼라 곱 연산자 오버로딩
    Vector operator*(float scalar) const {
        return Vector(x * scalar, y * scalar);
    }
};

struct Space {
    float x;
    float y;
    Color c;
};

/*
 *  변수 선언부
 */
/// 가상 공간의 그리기 영역 좌측과 하단을 나타내는 변수
int left = 0;
int bottom = 0;

/// 하단의 슬라이딩 바 선언 및 초기화
int slidingBarLen = 200;
int slidingBarWeight = 20;
int slidingBarSpeed = 10;
Bar slidingBar = { WIDTH / 2, 0, slidingBarLen, slidingBarWeight };

/// Sliding Bar Power Hit Mode 변수 선언 및 초기화
bool powerHitCheck = false;
float powerHitMax = 50;
float powerHitGauge = powerHitMax;
float powerHitVariation;
bool powerShut = false;

/// 공 선언 및 초기화
float ballRadius = 10.0;
Point ballPosition = { WIDTH / 2, slidingBarWeight + ballRadius };
float speedX = 1.0;
float speedY = 6.0;
float speedSum = sqrt(speedX * speedX + speedY * speedY);
Vector ballSpeed = { speedX, speedY };
int beforeTouch = -1;

/// 내부 벽 선언 및 초기화
Point Wall[WALL_NUM] = {
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

/// 내부 벽 법선벡터
Vector nomalWall[WALL_NUM - 1];

/// 직사각형 벽돌 선언 및 초기화
Block rectangleBlock[RECTANGLE_BLOCK_NUM];
int rectangleBlockLen = 100;
int rectangleBlockWeight = 50;

bool pause = true;
int  mode  = READY;

const int star_num = 100;
Space star[star_num];


/*
 *  Shape Color
 */
Color backGroundColor = { 0.1, 0.1, 0.1 };
Color wallColor = { 0.9, 0.8, 0.5 };
Color slidingBarColor = { 0.6, 0.8, 0.95 };
Color ballColor = { 0.97, 0.95, 0.99 };


/*
 *  Bitmap Setting
 */
/// READ Bitmap
bool READ[4][5][5] = {
    /// G
    {
        { 0, 1, 1, 1, 0 },
        { 1, 0, 0, 0, 0 },
        { 1, 0, 1, 1, 0 },
        { 1, 0, 0, 0, 1 },
        { 0, 1, 1, 1, 0 }
    },
    /// A
    {
        { 0, 1, 1, 1, 0 },
        { 1, 0, 0, 0, 1 },
        { 1, 0, 1, 1, 1 },
        { 1, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 1 }
    },
    /// M
    {
        { 0, 1, 0, 1, 0 },
        { 1, 0, 1, 0, 1 },
        { 1, 0, 1, 0, 1 },
        { 1, 0, 1, 0, 1 },
        { 1, 0, 0, 0, 1 }
    },
    /// E
    {
        { 0, 1, 1, 1, 1 },
        { 1, 0, 0, 0, 0 },
        { 1, 0, 1, 1, 1 },
        { 1, 0, 0, 0, 0 },
        { 0, 1, 1, 1, 1 }
    }
};

/// START Bitmap
bool START[5][5][5] = {
    /// S
    {
        { 0, 1, 1, 1, 1 },
        { 1, 0, 0, 0, 0 },
        { 0, 1, 1, 1, 0 },
        { 0, 0, 0, 0, 1 },
        { 1, 1, 1, 1, 0 }
    },
    /// T
    {
        { 1, 1, 1, 1, 1 },
        { 0, 0, 1, 0, 0 },
        { 0, 0, 1, 0, 0 },
        { 0, 0, 1, 0, 0 },
        { 0, 0, 1, 0, 0 }
    },
    /// A
    {
        { 0, 1, 1, 1, 0 },
        { 1, 0, 0, 0, 1 },
        { 1, 0, 1, 1, 1 },
        { 1, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 1 }
    },
    /// R
    {
        { 1, 1, 1, 1, 0 },
        { 1, 0, 0, 0, 1 },
        { 1, 0, 1, 1, 0 },
        { 1, 0, 1, 0, 0 },
        { 1, 0, 0, 1, 1 }
    },
    /// T
    {
        { 1, 1, 1, 1, 1 },
        { 0, 0, 1, 0, 0 },
        { 0, 0, 1, 0, 0 },
        { 0, 0, 1, 0, 0 },
        { 0, 0, 1, 0, 0 }
    }
};

bool EXIT[4][5][5] {
    /// E
    {
        { 0, 1, 1, 1, 0 },
        { 1, 0, 0, 0, 1 },
        { 1, 1, 1, 1, 1 },
        { 1, 0, 0, 0, 0 },
        { 0, 1, 1, 1, 1 }
    },
    /// X
    {
        { 1, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 1 },
        { 0, 1, 1, 1, 0 },
        { 1, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 1 }
    },
    /// I
    {
        { 1, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0 },
        { 1, 0, 0, 0, 0 },
        { 1, 0, 0, 0, 0 },
        { 1, 0, 0, 0, 0 }
    },
    /// T
    {
        { 1, 0, 0, 0, 0 },
        { 1, 1, 1, 0, 0 },
        { 1, 0, 0, 0, 0 },
        { 1, 0, 0, 0, 0 },
        { 0, 1, 1, 1, 0 }
    }
};


/*
 *  Initial Function
 */
///
void InitWall() {
    for(int i = 0; i < WALL_NUM - 1; i ++) {
        Vector v = { Wall[i + 1].x - Wall[i].x, Wall[i + 1].y - Wall[i].y };
        nomalWall[i] = { v.y, -v.x };
        std::cout << i << ":" << nomalWall[i].x << "|" << nomalWall[i].y << std::endl;
    }
}

void InitSpace() {
    srand(time(NULL));
    for(int i = 0; i < star_num; i ++) {
        Color color = {
            red:static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
            green:static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
            blue:static_cast <float> (rand()) / static_cast <float> (RAND_MAX)
        };
        star[i].c = color;
        star[i].x = rand() % WIDTH;
        star[i].y = rand() % HEIGHT;
    }
}



/*
 *  Create Function
 */
/// 직사각형 벽돌을 생성해주는 함수
void CreateRectangleBlock() {
    int startX = 0;
    int xVariation = 0;
    int startY = 0;
    for(int i = 0; i < RECTANGLE_BLOCK_NUM; i ++) {
        if(i < 3) {
            startX = 350;
            startY = 650;
            xVariation = i;
        }
        else if(i < 7) {
            startX = 300;
            startY = 600;
            xVariation = i % 4;
        }
        else if(i < 12) {
            startX = 250;
            startY = 550;
            xVariation = i % 7;
        }
        else if(i < 16) {
            startX = 300;
            startY = 500;
            xVariation = i % 12;
        }
        else if(i < 19) {
            startX = 350;
            startY = 450;
            xVariation = i % 16;
        }

        rectangleBlock[i].leftTop.x = startX + xVariation * rectangleBlockLen;
        rectangleBlock[i].leftTop.y = startY;

        rectangleBlock[i].leftBottom.x = startX + xVariation * rectangleBlockLen;
        rectangleBlock[i].leftBottom.y = startY - rectangleBlockWeight;

        rectangleBlock[i].rightTop.x = startX + xVariation * rectangleBlockLen + rectangleBlockLen;
        rectangleBlock[i].rightTop.y = startY;

        rectangleBlock[i].rightBottom.x = startX + xVariation * rectangleBlockLen + rectangleBlockLen;
        rectangleBlock[i].rightBottom.y = startY - rectangleBlockWeight;
    }
}


/*
 *  Math Function
 */
/// 기울기를 반환해주는 함수
float inclination(Point a1, Point a2) {
    return (a1.x == a2.x) ? 0 : (a2.y - a1.y) / (a2.x - a1.x);
}

/// 두 점과 y 값을 넣으면 x 값을 반환해주는 함수
float return_X(float y, Point a1, Point a2) {
    return (y - a1.y) / inclination(a1, a2) + a1.x;
}

/// 두 점과 x 값을 넣으면 y 값을 반환해주는 함수
float return_Y(float x, Point a1, Point a2) {
    return inclination(a1, a2) * x - a1.x + a1.y;
}

/// 벽돌의 갯수를 반환하는 함수
int CountBlock() {
    int count = 0;
    for(int i = 0; i < RECTANGLE_BLOCK_NUM; i ++) {
        count += rectangleBlock[i].state;
    }
    return count;
}

/// 공의 속도를 변환하는 함수
void ChangeSpeed(float change) {
    speedX *= change;
    speedY *= change;
    speedSum = sqrt(speedX * speedX + speedY * speedY);
}

/// 두 선분의 교점을 구하는 함수
Point MeetPoint(Point a1, Point a2, Point b1, Point b2) {
    float x = ((a1.x * a2.y - a1.y * a2.x) * (b1.x - b2.x) - (a1.x - a2.x) * (b1.x * b2.y - b1.y * b2.x)) / ((a1.x - a2.x) * (b1.y - b2.y) - (a1.y - a2.y) * (b1.x - b2.x));
    float y = ((a1.x * a2.y - a1.y * a2.x) * (b1.y - b2.y) - (a1.y - a2.y) * (b1.x * b2.y - b1.y * b2.x)) / ((a1.x - a2.x) * (b1.y - b2.y) - (a1.y - a2.y) * (b1.x - b2.x));
    Point p = { x, y };
    
    return p;
}


/*
 *  SpecialMode Function
 */
/// Sliding Bar Power Hit Mode
void PowerHit() {
    //std::cout << powerHitGauge << std::endl;
    if(powerHitCheck) {
        if(powerHitGauge > slidingBar.center.y) {
            slidingBar.center.y += powerHitVariation;
        }
        else {
            powerShut = false;
            ChangeSpeed(1.2);
            speedY *= speedY < 0 ? -1 : 1;
            powerHitCheck = false;
            powerHitGauge = 0;
        }
    }
    else {
        if(slidingBar.center.y > 0) {
            slidingBar.center.y -= powerHitVariation;
        }
        if(slidingBar.center.y < 0) {
            slidingBar.center.y = 0;
        }
        if(slidingBar.center.y == 0) {
            powerHitGauge += powerHitGauge < powerHitMax ? 0.1 : 0;
        }
    }
}


/*
 *  CollisionDetection Function
 */
/// 외부 벽과의 충돌을 확인하는 함수
void CollisionDetectionToWindow() {
    ballSpeed.x *= ballPosition.x + ballRadius >= WIDTH ? -1 : 1;
    ballSpeed.x *= ballPosition.x - ballRadius <= 0 ? -1 : 1;
    ballSpeed.y *= ballPosition.y + ballRadius >= HEIGHT ? -1 : 1;
    ballSpeed.y *= ballPosition.y - ballRadius <= 0 ? -1 : 1;
}

/// 내부 벽과의 충돌을 확인하는 함수
void CollisionDetectionToWall(void){
    for(int i = 0; i < WALL_NUM - 1; i ++) {
        /// 좌측 기울기가 - 인 대각선
        if(i == 1 && i != beforeTouch) {
            if (Wall[i].y <= ballPosition.y && Wall[i + 1].y >= ballPosition.y) {
                if (return_X(ballPosition.y, Wall[i], Wall[i+1]) >= ballPosition.x - ballRadius) {
                    // 공 위치 조정(벽을 넘어가지 않도록)
                    Point before = { ballPosition.x - ballSpeed.x, ballPosition.y - ballSpeed.y };
                    Point meet = MeetPoint(Wall[i], Wall[i + 1], ballPosition, before);
                    float angle = atan(ballSpeed.y / ballSpeed.x);
                    
                    std::cout << "좌하단" << std::endl;
                    
                    ballPosition.x = meet.x + abs(ballRadius * cos(angle));
                    ballPosition.y = meet.y + abs(ballRadius * sin(angle));
                    
                    // 벡터의 정규화
                    ballSpeed.normalize();
                    nomalWall[i].normalize();

                    // 반사 벡터 계산
                    Vector v = ballSpeed.reflect(nomalWall[i]);
                    float vSum = sqrt(v.x * v.x + v.y * v.y);
                                        
                    ballSpeed.x = v.x * (speedSum / vSum / 2);
                    ballSpeed.y = v.y * (speedSum / vSum / 2);
                    
                    beforeTouch = i;
                    
                    return;
                }
            }
        }
        /// 상단 기울기가 - 인 대각선
        else if(i == 4 && i != beforeTouch) {
            if (Wall[i + 1].y <= ballPosition.y && Wall[i].y >= ballPosition.y) {
                if (return_X(ballPosition.y, Wall[i], Wall[i+1]) <= ballPosition.x + ballRadius) {
                    // 공 위치 조정(벽을 넘어가지 않도록)
                    Point before = { ballPosition.x - ballSpeed.x, ballPosition.y - ballSpeed.y };
                    Point meet = MeetPoint(Wall[i], Wall[i + 1], ballPosition, before);
                    float angle = atan(ballSpeed.y / ballSpeed.x);
                    
                    std::cout << "우상단" << std::endl;
                    
                    ballPosition.x = meet.x - abs(ballRadius * cos(angle));
                    ballPosition.y = meet.y - abs(ballRadius * sin(angle));
                    
                    // 벡터의 정규화
                    ballSpeed.normalize();
                    nomalWall[i].normalize();

                    // 반사 벡터 계산
                    Vector v = ballSpeed.reflect(nomalWall[i]);
                    float vSum = sqrt(v.x * v.x + v.y * v.y);
                                        
                    ballSpeed.x = v.x * (speedSum / vSum / 2);
                    ballSpeed.y = v.y * (speedSum / vSum / 2);
                    
                    beforeTouch = i;
                    
                    return;
                }
            }
        }
        /// 상단 기울기가 + 인 대각선
        else if(i == 3 && i != beforeTouch) {
            if (Wall[i].y <= ballPosition.y && Wall[i + 1].y >= ballPosition.y) {
                if (return_X(ballPosition.y, Wall[i], Wall[i+1]) >= ballPosition.x - ballRadius) {
                    // 공 위치 조정(벽을 넘어가지 않도록)
                    Point before = { ballPosition.x - ballSpeed.x, ballPosition.y - ballSpeed.y };
                    Point meet = MeetPoint(Wall[i], Wall[i + 1], ballPosition, before);
                    float angle = atan(ballSpeed.y / ballSpeed.x);
                    
                    std::cout << "좌상단" << std::endl;
                    
                    ballPosition.x = meet.x + abs(ballRadius * cos(angle));
                    ballPosition.y = meet.y - abs(ballRadius * sin(angle));
                    
                    // 벡터의 정규화
                    ballSpeed.normalize();
                    nomalWall[i].normalize();

                    // 반사 벡터 계산
                    Vector v = ballSpeed.reflect(nomalWall[i]);
                    float vSum = sqrt(v.x * v.x + v.y * v.y);
                                        
                    ballSpeed.x = v.x * (speedSum / vSum / 2);
                    ballSpeed.y = v.y * (speedSum / vSum / 2);
                    
                    beforeTouch = i;
                    
                    return;
                }
            }
        }
        /// 우측 기울기가 + 인 대각선
        else if(i == 6 && i != beforeTouch) {
            if (Wall[i + 1].y <= ballPosition.y && Wall[i].y >= ballPosition.y) {
                if (return_X(ballPosition.y, Wall[i], Wall[i+1]) <= ballPosition.x + ballRadius) {
                    // 공 위치 조정(벽을 넘어가지 않도록)
                    Point before = { ballPosition.x - ballSpeed.x, ballPosition.y - ballSpeed.y };
                    Point meet = MeetPoint(Wall[i], Wall[i + 1], ballPosition, before);
                    float angle = atan(ballSpeed.y / ballSpeed.x);
                    
                    std::cout << "우하단" << std::endl;
                
                    ballPosition.x = meet.x - abs(ballRadius * cos(angle));
                    ballPosition.y = meet.y + abs(ballRadius * sin(angle));
                    
                    // 벡터의 정규화
                    ballSpeed.normalize();
                    nomalWall[i].normalize();

                    // 반사 벡터 계산
                    Vector v = ballSpeed.reflect(nomalWall[i]);
                    float vSum = sqrt(v.x * v.x + v.y * v.y);
                                        
                    ballSpeed.x = v.x * (speedSum / vSum / 2);
                    ballSpeed.y = v.y * (speedSum / vSum / 2);
                    
                    beforeTouch = i;
                    
                    return;
                }
            }
        }
        /// 좌측 y 축에 평행한 직선
        else if(i == 0 && i != beforeTouch) {
            if(Wall[i].x >= ballPosition.x - ballRadius && Wall[i].y <= ballPosition.y && Wall[i + 1].y >= ballPosition.y) {
                ballSpeed.x *= -1;
                ballPosition.x = Wall[i].x + ballRadius + 2;
                
                beforeTouch = i;
                
                return;
            }
        }
        /// 우측 y 축에 평행한 직선
        else if(i == 7 && i != beforeTouch) {
            if(Wall[i].x <= ballPosition.x + ballRadius && Wall[i + 1].y <= ballPosition.y && Wall[i].y >= ballPosition.y) {
                ballSpeed.x *= -1;
                ballPosition.x = Wall[i].x - ballRadius - 2;
                
                beforeTouch = i;
                
                return;
            }
        }
        /// 좌측 x 축에 평행한 직선
        else if(i == 2 && i != beforeTouch) {
            if(Wall[i].x <= ballPosition.x + ballRadius && Wall[i + 1].x >= ballPosition.x - ballRadius) {
                if(Wall[i].y <= ballPosition.y + ballRadius) {
                    ballSpeed.y *= -1;
                    ballPosition.y = Wall[i].y - ballRadius - 2;
                    
                    beforeTouch = i;
                    
                    return;
                }
            }
        }
        /// 우측 x 축에 평행한 직선
        else if(i == 5 && i != beforeTouch) {
            if(Wall[i].x <= ballPosition.x + ballRadius && Wall[i + 1].x >= ballPosition.x - ballRadius) {
                if(Wall[i].y <= ballPosition.y + ballRadius) {
                    ballSpeed.y *= -1;
                    ballPosition.y = Wall[i].y - ballRadius - 2;
                    
                    beforeTouch = i;
                    
                    return;
                }
            }
        }
    }
}

/// 슬라이딩바와 충돌을 확인하는 함수
void CollisionDetectionToSlidingBar() {
    if (beforeTouch != RECTANGLE_BLOCK_NUM + WALL_NUM + 1) {
        float distance = pow(ballPosition.x - (slidingBar.center.x - slidingBarLen / 2), 2) + pow(ballPosition.y - (slidingBar.center.y + slidingBarWeight), 2);
        if(distance <= ballRadius * ballRadius) {
            std::cout << "슬라이딩 바 좌측 상단 충돌" << std::endl;
            Vector v = { -1.0, 1.0 };
            float vSum = sqrt(v.x * v.x + v.y * v.y);
            
            ballSpeed.x = v.x * (speedSum / vSum / 2);
            ballSpeed.y = v.y * (speedSum / vSum / 2);
            
            beforeTouch = RECTANGLE_BLOCK_NUM + WALL_NUM + 1;
            
            return;
        }
        
        distance = pow(ballPosition.x - (slidingBar.center.x + slidingBarLen / 2), 2) + pow(ballPosition.y - (slidingBar.center.y + slidingBarWeight), 2);
        if(distance <= ballRadius * ballRadius) {
            std::cout << "슬라이딩 바 우측 상단 충돌" << std::endl;
            Vector v = { 1.0, 1.0 };
            float vSum = sqrt(v.x * v.x + v.y * v.y);
            
            ballSpeed.x = v.x * (speedSum / vSum / 2);
            ballSpeed.y = v.y * (speedSum / vSum / 2);
            
            beforeTouch = RECTANGLE_BLOCK_NUM + WALL_NUM + 1;
            
            return;
        }
        
        if(ballPosition.y - ballRadius < slidingBar.center.y + slidingBar.weight) {
            if(ballPosition.x < slidingBar.center.x + slidingBarLen / 2 && ballPosition.x > slidingBar.center.x - slidingBarLen / 2){
                if(powerHitCheck) {
                    powerShut = true;
                }
                ballSpeed.y *= -1;
                beforeTouch = RECTANGLE_BLOCK_NUM + WALL_NUM + 1;
            }
        }
    }
}

/// 직사각형 벽돌과의 충돌을 확인하는 함수
void CollisionDetectionToRectangleBlock() {
    for(int i = 0; i < RECTANGLE_BLOCK_NUM; i ++) {
        if(rectangleBlock[i].state && i + WALL_NUM != beforeTouch) {
            // 벽돌의 모서리와의 충돌을 확인
            // 좌측 하단
            float distance = pow(ballPosition.x - rectangleBlock[i].leftBottom.x, 2) + pow(ballPosition.y - rectangleBlock[i].leftBottom.y, 2);
            if(distance <= ballRadius * ballRadius) {
                std::cout << "벽돌 좌측 하단 충돌" << std::endl;
                Vector v = { -1.0, -1.0 };
                float vSum = sqrt(v.x * v.x + v.y * v.y);
                                    
                ballSpeed.x = v.x * (speedSum / vSum / 2);
                ballSpeed.y = v.y * (speedSum / vSum / 2);
                
                rectangleBlock[i].state --;
                
                beforeTouch = i + WALL_NUM;
                
                return;
            }
            
            // 우측 하단
            distance = pow(ballPosition.x - rectangleBlock[i].rightBottom.x, 2) + pow(ballPosition.y - rectangleBlock[i].rightBottom.y, 2);
            if(distance <= ballRadius * ballRadius) {
                std::cout << "벽돌 우측 하단 충돌" << std::endl;
                Vector v = { 1.0, -1.0 };
                float vSum = sqrt(v.x * v.x + v.y * v.y);
                                    
                ballSpeed.x = v.x * (speedSum / vSum / 2);
                ballSpeed.y = v.y * (speedSum / vSum / 2);
                
                rectangleBlock[i].state --;
                
                beforeTouch = i + WALL_NUM;
                
                return;
            }
            
            // 우측 상단
            distance = pow(ballPosition.x - rectangleBlock[i].rightTop.x, 2) + pow(ballPosition.y - rectangleBlock[i].rightTop.y, 2);
            if(distance <= ballRadius * ballRadius) {
                std::cout << "벽돌 우측 상단 충돌" << std::endl;
                Vector v = { 1.0, 1.0 };
                float vSum = sqrt(v.x * v.x + v.y * v.y);
                                    
                ballSpeed.x = v.x * (speedSum / vSum / 2);
                ballSpeed.y = v.y * (speedSum / vSum / 2);
                
                rectangleBlock[i].state --;
                
                beforeTouch = i + WALL_NUM;
                
                return;
            }
            
            // 좌측 상단
            distance = pow(ballPosition.x - rectangleBlock[i].leftTop.x, 2) + pow(ballPosition.y - rectangleBlock[i].leftTop.y, 2);
            if(distance <= ballRadius * ballRadius) {
                std::cout << "벽돌 좌측 상단 충돌" << std::endl;
                Vector v = { -1.0, 1.0 };
                float vSum = sqrt(v.x * v.x + v.y * v.y);
                                    
                ballSpeed.x = v.x * (speedSum / vSum / 2);
                ballSpeed.y = v.y * (speedSum / vSum / 2);
                
                rectangleBlock[i].state --;
                
                beforeTouch = i + WALL_NUM;
                
                return;
            }
            
            // 공이 벽돌과 충돌을 확인
            if(ballPosition.y + ballRadius >= rectangleBlock[i].leftBottom.y && ballPosition.y - ballRadius <= rectangleBlock[i].leftTop.y) {
                if(ballPosition.x + ballRadius >= rectangleBlock[i].leftBottom.x && ballPosition.x - ballRadius <= rectangleBlock[i].rightBottom.x){
                    
                    rectangleBlock[i].state --;
                    
                    Point _block;
                    _block.x = rectangleBlock[i].leftBottom.x + rectangleBlockLen / 2;
                    _block.y = rectangleBlock[i].leftBottom.y + rectangleBlockWeight / 2;
                    float inc = inclination(ballPosition, _block);
                    
                    if(inc < 0.5 && inc > -0.5) {
                        ballSpeed.x *= -1;
                    }
                    else {
                        ballSpeed.y *= -1;
                    }
                    
                    beforeTouch = i + WALL_NUM;
                    
                    return;
                }
            }
        }
    }
}

/// 벽면 모서리와의 충돌을 확인하는 함수
void CollisionDetectionToCorner() {
    for(int i = 1; i < WALL_NUM - 1; i ++) {
        float distance = (ballPosition.x - Wall[i].x) * (ballPosition.x - Wall[i].x) + (ballPosition.y - Wall[i].y) * (ballPosition.y - Wall[i].y);
        if(distance <= ballRadius * ballRadius) {
            Vector v = nomalWall[i - 1] - nomalWall[i];
            float vSum = sqrt(v.x * v.x + v.y * v.y);
                                
            ballSpeed.x = v.x * (speedSum / vSum / 2);
            ballSpeed.y = v.y * (speedSum / vSum / 2);
        }
    }
}



/*
 *  Show Window Function
 */
/// 게임 플레이에 사용되는 공을 그려주는 함수
void ShowBall() {
    int num = 36;
    float delta = 2 * PI / num;
    glBegin(GL_POLYGON);
    ballPosition.y = powerShut ? slidingBar.center.y + slidingBarWeight + ballRadius + 3 : ballPosition.y;
    for(int i = 0; i < num; i ++) {
        glVertex2f(ballPosition.x + ballRadius * cos(delta * i), ballPosition.y + ballRadius * sin(delta * i));
    }
    glEnd();
}

/// 하단의 슬라이딩 바를 그려주는 함수
void ShowSlidingBar() {
    PowerHit();
    glBegin(GL_POLYGON);
    glVertex2i(slidingBar.center.x - slidingBar.len / 2, slidingBar.center.y);
    glVertex2i(slidingBar.center.x - slidingBar.len / 2, slidingBar.center.y + slidingBar.weight);
    glVertex2i(slidingBar.center.x + slidingBar.len / 2, slidingBar.center.y + slidingBar.weight);
    glVertex2i(slidingBar.center.x + slidingBar.len / 2, slidingBar.center.y);
    glEnd();
}

/// Window 화면에 게임이 진행될 벽을 그려주는 함수
void ShowWall() {
    glLineWidth(3.0);
    glBegin(GL_LINE_STRIP);
    for(int i = 0; i < WALL_NUM; i ++) {
        glVertex2f(Wall[i].x, Wall[i].y);
    }
    glEnd();
}

/// 직사각형 벽돌을 출력해주는 함수
void ShowRectangleBlock() {
    for(int i = 0; i < RECTANGLE_BLOCK_NUM; i ++) {
        if(rectangleBlock[i].state) {
            switch (rectangleBlock[i].state) {
                case STATE_ONE:
                    glColor3f(0.99, 0.4, 0.4);
                    break;
                case STATE_TWO:
                    glColor3f(0.5, 0.8, 0.7);
                default:
                    break;
            }
            glBegin(GL_POLYGON);
            glVertex2f(rectangleBlock[i].leftTop.x, rectangleBlock[i].leftTop.y);
            glVertex2f(rectangleBlock[i].leftBottom.x, rectangleBlock[i].leftBottom.y);
            glVertex2f(rectangleBlock[i].rightBottom.x, rectangleBlock[i].rightBottom.y);
            glVertex2f(rectangleBlock[i].rightTop.x, rectangleBlock[i].rightTop.y);
            glEnd();

            glColor3f(0, 0, 0);
            glBegin(GL_LINE_LOOP);
            glVertex2f(rectangleBlock[i].leftTop.x, rectangleBlock[i].leftTop.y);
            glVertex2f(rectangleBlock[i].leftBottom.x, rectangleBlock[i].leftBottom.y);
            glVertex2f(rectangleBlock[i].rightBottom.x, rectangleBlock[i].rightBottom.y);
            glVertex2f(rectangleBlock[i].rightTop.x, rectangleBlock[i].rightTop.y);
            glEnd();
        }
    }
}


/*
 *  내부 화면들
 */
///
void Draw_Circle(float x, float y, float c_radius) {
   float    delta;
   int      num = 36;

   delta = 2 * PI / num;
   glBegin(GL_POLYGON);
   for (int i = 0; i < num; i++)
      glVertex2f(x + c_radius*cos(delta*i), y + c_radius*sin(delta*i));
   glEnd();
}

void DrawSpace() {
    glBegin(GL_POLYGON);
    for(int i = 0; i < star_num; i ++) {
        glColor3f(star[i].c.red, star[i].c.green, star[i].c.blue);
        Draw_Circle(star[i].x, star[i].y,rand()%3);
    }
}

void DrawRectangle(int x, int y, int w) {
    glBegin(GL_POLYGON);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + w);
    glVertex2f(x, y + w);
    glEnd();
}

void DrawREADY() {
    int size = 32;
    glColor3f(1.0f, 1.0f, 1.0f);
    for(int index = 0; index < 4; index ++) {
        for(int y = 0; y < 5; y ++) {
            for(int x = 0; x < 5; x ++) {
                if(READ[index][y][x]) {
                    DrawRectangle(x * size + 132 + index * (size * 6), (4 - y) * size + 700, size);
                }
            }
        }
    }
}

void DrawSTART() {
    int size = 15;
    glColor3f(1.0f, 1.0f, 1.0f);
    for(int index = 0; index < 5; index ++) {
        for(int y = 0; y < 5; y ++) {
            for(int x = 0; x < 5; x ++) {
                if(START[index][y][x]) {
                    DrawRectangle(x * size + 350 + index * (size * 6), (4 - y) * size + 450, size);
                }
            }
        }
    }
}

void DrawEXIT() {
    int size = 15;
    glColor3f(1.0f, 1.0f, 1.0f);
    for(int index = 0; index < 4; index ++) {
        if (index == 3) {
            for(int y = 0; y < 5; y ++) {
                for(int x = 0; x < 5; x ++) {
                    if(EXIT[index][y][x]) {
                        DrawRectangle(x * size + 350 + index * (size * 6) - (size * 4), (4 - y) * size + 300, size);
                    }
                }
            }
            break;
        }
        for(int y = 0; y < 5; y ++) {
            for(int x = 0; x < 5; x ++) {
                if(EXIT[index][y][x]) {
                    DrawRectangle(x * size + 350 + index * (size * 6), (4 - y) * size + 300, size);
                }
            }
        }
    }
}

/*
 *  GAMEMODE PAGE
 */
void ShowREADY(){
    DrawSpace();
    DrawREADY();
    DrawSTART();
    DrawEXIT();
}


/*
 *  Event Callback Function
 */
/// 스페셜 키가 입력되면 실행되는 콜백함수
void MySpecialKey(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:
            slidingBar.center.x -= slidingBar.center.x - slidingBar.len / 2 > Wall[0].x ? slidingBarSpeed : 0;
            break;
        case GLUT_KEY_RIGHT:
            slidingBar.center.x += slidingBar.center.x + slidingBar.len / 2 < Wall[8].x ? slidingBarSpeed : 0;
            break;
        // 슬라이딩바를 움직여 공의 속도 상승
        case 32:
            if (powerHitGauge > 30){
                powerHitCheck = true;
                powerHitVariation = powerHitGauge / 20;
            }
            break;
        // 게임 일시정지
        case 27:
            pause = pause ? false : true;
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

/// Window 화면의 크기가 변경되면 실행되는 콜백함수
void MyReshape(int w, int h) {
    glViewport((w - WIDTH) / 2, (h - HEIGHT) / 2, WIDTH, HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(left, left + WIDTH, bottom, bottom + HEIGHT);
}

/// Window 화면을 출력할 때 실행되는 콜백함수
void RenderScene(void) {
    if(mode == READY) {
        glClearColor(backGroundColor.red, backGroundColor.green, backGroundColor.blue, backGroundColor.clamp);
        glClear(GL_COLOR_BUFFER_BIT);
        
        ShowREADY();
        
        glutSwapBuffers();
        glFlush();
    }
    else if(mode == RUN){
        glClearColor(backGroundColor.red, backGroundColor.green, backGroundColor.blue, backGroundColor.clamp);
        glClear(GL_COLOR_BUFFER_BIT);

        // 충돌 검증
        CollisionDetectionToWindow();
        CollisionDetectionToCorner();
        CollisionDetectionToWall();
        CollisionDetectionToSlidingBar();
        CollisionDetectionToRectangleBlock();
        
        // 요소 출력
        glColor3f(ballColor.red, ballColor.green, ballColor.blue);
        ShowBall();

        glColor3f(slidingBarColor.red, slidingBarColor.green, slidingBarColor.blue);
        ShowSlidingBar();

        glColor3f(wallColor.red, wallColor.green, wallColor.blue);
        ShowWall();

        ShowRectangleBlock();

        
        if(CountBlock() && pause) {
            // 공의 위치 결정
            ballPosition.x += powerShut ? 0.0 : ballSpeed.x;
            ballPosition.y += ballSpeed.y;
        }

        glutSwapBuffers();
        glFlush();
    }
    else if(mode == GAMEOVER) {
        
    }
    else if(mode == CLEAR) {
        
    }
}

int main(int argc, char** argv) {
    InitSpace();
    InitWall();
    glutInit(&argc, argv);
    CreateRectangleBlock();
    glutInitWindowPosition(0, 0);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Break the Block!");
    glutReshapeFunc(MyReshape);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(MySpecialKey);
    glutIdleFunc(RenderScene);
    glutMainLoop();
}



