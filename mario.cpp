#include <iostream>
#include <vector>
#include <ctime>
#include <algorithm>
#include <iomanip>
#include <clocale>

// Кросс-платформенная поддержка
#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
#else
    #include <unistd.h>
    #include <termios.h>
    #include <sys/ioctl.h>
    #include <sys/select.h>
#endif

using namespace std;

// Кросс-платформенные функции для работы с консолью
#ifndef _WIN32
// Для Linux/Unix систем
struct termios orig_termios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);
    
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int _kbhit() {
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}

int _getch() {
    int r;
    unsigned char c;
    if ((r = read(STDIN_FILENO, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
}

void Sleep(int milliseconds) {
    usleep(milliseconds * 1000);
}
#endif

const int WIDTH = 40;
const int HEIGHT = 20;
const int GROUND_LEVEL = HEIGHT - 3;

class Mario {
public:
    int x, y;
    int velocityY;
    bool isJumping;
    int score;
    int lives;
    
    Mario() {
        x = 5;
        y = GROUND_LEVEL;
        velocityY = 0;
        isJumping = false;
        score = 0;
        lives = 3;
    }
    
    void jump() {
        if (!isJumping && y == GROUND_LEVEL) {
            isJumping = true;
            velocityY = -3;
        }
    }
    
    void update() {
        if (isJumping) {
            y += velocityY;
            velocityY++;
            
            if (y >= GROUND_LEVEL) {
                y = GROUND_LEVEL;
                isJumping = false;
                velocityY = 0;
            }
        }
    }
    
    void moveLeft() {
        if (x > 0) x--;
    }
    
    void moveRight() {
        if (x < WIDTH - 1) x++;
    }
};

class Obstacle {
public:
    int x, y;
    int width;
    bool active;
    
    Obstacle(int startX, int startY, int w) {
        x = startX;
        y = startY;
        width = w;
        active = true;
    }
    
    void move() {
        x--;
        if (x + width < 0) {
            active = false;
        }
    }
};

class Coin {
public:
    int x, y;
    bool active;
    
    Coin(int startX, int startY) {
        x = startX;
        y = startY;
        active = true;
    }
    
    void move() {
        x--;
        if (x < 0) {
            active = false;
        }
    }
};

class Game {
private:
    Mario mario;
    vector<Obstacle> obstacles;
    vector<Coin> coins;
    int frameCount;
    bool gameOver;
    
    void setCursorPosition(int x, int y) {
#ifdef _WIN32
        COORD coord;
        coord.X = x;
        coord.Y = y;
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#else
        printf("\033[%d;%dH", y + 1, x + 1);
        fflush(stdout);
#endif
    }
    
    void hideCursor() {
#ifdef _WIN32
        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        info.dwSize = 100;
        info.bVisible = FALSE;
        SetConsoleCursorInfo(consoleHandle, &info);
#else
        printf("\033[?25l");
        fflush(stdout);
#endif
    }
    
    void showCursor() {
#ifdef _WIN32
        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        info.dwSize = 100;
        info.bVisible = TRUE;
        SetConsoleCursorInfo(consoleHandle, &info);
#else
        printf("\033[?25h");
        fflush(stdout);
#endif
    }
    
public:
    Game() {
        frameCount = 0;
        gameOver = false;
        srand(time(0));
    }
    
    void spawnObstacle() {
        if (rand() % 100 < 2) {
            obstacles.push_back(Obstacle(WIDTH - 1, GROUND_LEVEL, 2));
        }
    }
    
    void spawnCoin() {
        if (rand() % 100 < 3) {
            int coinY = GROUND_LEVEL - 3 - rand() % 3;
            coins.push_back(Coin(WIDTH - 1, coinY));
        }
    }
    
    bool checkCollision() {
        for (auto& obs : obstacles) {
            if (!obs.active) continue;
            
            for (int i = 0; i < obs.width; i++) {
                if (mario.x == obs.x + i && mario.y == obs.y) {
                    return true;
                }
            }
        }
        return false;
    }
    
    void collectCoins() {
        for (auto& coin : coins) {
            if (coin.active && coin.x == mario.x && coin.y == mario.y) {
                coin.active = false;
                mario.score += 10;
            }
        }
    }
    
    void update() {
        frameCount++;
        
        mario.update();
        
        spawnObstacle();
        spawnCoin();
        
        for (auto& obs : obstacles) {
            obs.move();
        }
        
        for (auto& coin : coins) {
            coin.move();
        }
        
        collectCoins();
        
        if (checkCollision()) {
            mario.lives--;
            if (mario.lives <= 0) {
                gameOver = true;
            } else {
                mario.x = 5;
                mario.y = GROUND_LEVEL;
                mario.isJumping = false;
                mario.velocityY = 0;
                obstacles.clear();
            }
        }
        
        // Очистка неактивных объектов
        obstacles.erase(remove_if(obstacles.begin(), obstacles.end(),
            [](const Obstacle& o) { return !o.active; }), obstacles.end());
        coins.erase(remove_if(coins.begin(), coins.end(),
            [](const Coin& c) { return !c.active; }), coins.end());
    }
    
    void draw() {
        setCursorPosition(0, 0);
        
        // Создаем игровое поле
        char screen[HEIGHT][WIDTH + 1];
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                screen[i][j] = ' ';
            }
            screen[i][WIDTH] = '\0';
        }
        
        // Рисуем землю
        for (int i = 0; i < WIDTH; i++) {
            screen[GROUND_LEVEL + 1][i] = '=';
        }
        
        // Рисуем препятствия
        for (auto& obs : obstacles) {
            if (obs.active) {
                for (int i = 0; i < obs.width; i++) {
                    if (obs.x + i >= 0 && obs.x + i < WIDTH) {
                        screen[obs.y][obs.x + i] = '#';
                    }
                }
            }
        }
        
        // Рисуем монеты
        for (auto& coin : coins) {
            if (coin.active && coin.x >= 0 && coin.x < WIDTH && coin.y >= 0 && coin.y < HEIGHT) {
                screen[coin.y][coin.x] = 'O';
            }
        }
        
        // Рисуем Марио
        if (mario.x >= 0 && mario.x < WIDTH && mario.y >= 0 && mario.y < HEIGHT) {
            screen[mario.y][mario.x] = 'M';
        }
        
        // Выводим экран
        for (int i = 0; i < HEIGHT; i++) {
            cout << screen[i] << endl;
        }
        
        // Выводим информацию
        cout << "Счет: " << mario.score << "  Жизни: " << mario.lives << "  [Пробел - прыжок, A - влево, D - вправо, Q - выход]" << endl;
    }
    
    void handleInput() {
        if (_kbhit()) {
            char key = _getch();
            if (key == ' ') {
                mario.jump();
            } else if (key == 'a' || key == 'A') {
                mario.moveLeft();
            } else if (key == 'd' || key == 'D') {
                mario.moveRight();
            } else if (key == 'q' || key == 'Q') {
                gameOver = true;
            }
        }
    }
    
    void run() {
#ifndef _WIN32
        enableRawMode();
#endif
        hideCursor();
        
        cout << "=== МАРИО ===" << endl;
        cout << "Управление: Пробел - прыжок, A - влево, D - вправо" << endl;
        cout << "Собирайте монеты (O) и избегайте препятствий (#)" << endl;
        cout << "Нажмите любую клавишу для начала..." << endl;
        _getch();
        
        // Очистка экрана вместо system("cls")
        setCursorPosition(0, 0);
        for (int i = 0; i < HEIGHT + 5; i++) {
            for (int j = 0; j < WIDTH + 50; j++) {
                cout << " ";
            }
            cout << endl;
        }
        
        while (!gameOver) {
            handleInput();
            update();
            draw();
            Sleep(50);
        }
        
        // Очистка экрана вместо system("cls")
        setCursorPosition(0, 0);
        for (int i = 0; i < HEIGHT + 5; i++) {
            for (int j = 0; j < WIDTH + 50; j++) {
                cout << " ";
            }
            cout << endl;
        }
        
        setCursorPosition(0, 5);
        cout << "\n\n";
        cout << "    ╔═══════════════════════════════════╗\n";
        cout << "    ║         ИГРА ОКОНЧЕНА!            ║\n";
        cout << "    ╠═══════════════════════════════════╣\n";
        cout << "    ║                                   ║\n";
        cout << "    ║    Ваш финальный счет: " << setw(6) << mario.score << "    ║\n";
        cout << "    ║                                   ║\n";
        cout << "    ╚═══════════════════════════════════╝\n";
        cout << "\n\n";
        
        showCursor();
#ifndef _WIN32
        disableRawMode();
#endif
    }
};

int main() {
    // Установка кодировки для русского языка
#ifdef _WIN32
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
#else
    // Для Linux/Unix используем UTF-8 по умолчанию
    setlocale(LC_ALL, "");
#endif
    
    Game game;
    game.run();
    
    return 0;
}
