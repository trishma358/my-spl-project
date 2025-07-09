 #include <iostream>
#include <conio.h>
#include <windows.h>
#include <vector>
#include <ctime>
#include <set>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
using namespace std;

const int width = 80, height = 20;
int x, y, fruitX, fruitY, bonusX, bonusY, poisonX, poisonY;
int score, nTail, bonusTimer = 0, level = 1, customSpeed = 0;
int highScoreEasy = 0, highScoreMedium = 0, highScoreHard = 0;
bool gameOver, bonusAlive = false, poisonAlive = false, soundOn = true;
int obstacleLimit = 0;
unsigned long bonusStartTick = 0;
vector<pair<int, int>> tail;
vector<pair<int, int>> obstacles;
enum eDirecton { STOP = 0, LEFT, RIGHT, UP, DOWN };
eDirecton dir = STOP;

int theme_bg = 7, theme_wall = 3, theme_snake = 10, theme_snakebody = 11, theme_obs = 12;
int theme_fruit = 10, theme_bonus = 12, theme_poison = 13;

void setColor(int color) { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color); }
void gotoxy(int x, int y) { COORD c = { (SHORT)x, (SHORT)y }; SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c); }
void clearScreen() { system("cls"); }
void playEatSound() { if(soundOn) PlaySound(TEXT("SystemAsterisk"), NULL, SND_ASYNC); }
void playBonusSound() { if(soundOn) PlaySound(TEXT("SystemExclamation"), NULL, SND_ASYNC); }
void playPoisonSound() { if(soundOn) PlaySound(TEXT("SystemHand"), NULL, SND_ASYNC); }
void playGameOverSound() { if(soundOn) PlaySound(TEXT("SystemExit"), NULL, SND_ASYNC); }
void playLevelUpSound() { if(soundOn) PlaySound(TEXT("SystemStart"), NULL, SND_ASYNC); }
void playSelectSound() { if(soundOn) Beep(880,70); }
void randomPos(int &xx, int &yy) { xx = rand() % width; yy = rand() % height; }

void showStartAnimation() {
    clearScreen();
    for (int i = 0; i < 4; i++) {
        setColor(14 + (i%4));
        cout << "\n\n\n\t\t";
        setColor(10 + (i%4));
        cout << "████████████████████\n\t\t";
        setColor(12 + (i%2));
        cout << "   S T A R T   G A M E   \n\t\t";
        setColor(10 + (i%3));
        cout << "████████████████████";
        setColor(7); cout << endl;
        Sleep(400);
        clearScreen();
    }
    setColor(10);
    cout << "\n\n\n\n\t\t   S T A R T   G A M E   \n";
    setColor(7);
    Sleep(1200);
}

void printMenu() {
    clearScreen();
    setColor(14); cout << "\n\t";
    //setColor(12); cout << "♥ "; setColor(11); cout << "♥ "; setColor(10); cout << "♥ ";
    setColor(14); cout << "======= SNAKE GAME =======";
    //setColor(10); cout << " ♥ "; setColor(11); cout << "♥ "; setColor(12); cout << "♥ ";
    setColor(7);
    cout << "\n\n\t"; setColor(11); cout << "1. "; setColor(10); cout << "Play Game\n\t";
    setColor(11); cout << "2. "; setColor(13); cout << "Settings\n\t";
    setColor(11); cout << "3. "; setColor(14); cout << "High Score\n\t";
    setColor(11); cout << "4. "; setColor(12); cout << "Exit\n";
    setColor(7); cout << "\n\tChoose (1-4): ";
}

int printLevelMenu() {
    clearScreen();
    setColor(11); cout << "\n\t---- ";
    setColor(13); cout << "Select Game Level";
    setColor(11); cout << " ----\n";
    setColor(7);
    setColor(10); cout << "\t1. Easy   "; setColor(8); cout << "(No Obstacles)\n";
    setColor(14); cout << "\t2. Medium "; setColor(8); cout << "(5 Obstacles, +1 every 10 points)\n";
    setColor(12); cout << "\t3. Hard   "; setColor(8); cout << "(10 Obstacles, +1 every 5 points)\n";
    setColor(11); cout << "\t4. "; setColor(14); cout << "Back\n";
    setColor(7); cout << "\n\tChoose (1-4): ";
    int lv; cin >> lv; return lv;
}

void printSettings() {
    clearScreen();
    setColor(13); cout << "\n\t---- Settings ----\n";
    setColor(7);
    setColor(11); cout << "\t1. "; setColor(10); cout << "Background Theme\n";
    setColor(11); cout << "\t2. "; setColor(10); cout << "Snake Color\n";
    setColor(11); cout << "\t3. "; setColor(10); cout << "Wall Color\n";
    setColor(11); cout << "\t4. "; setColor(14); cout << "Sound ";
    cout << (soundOn ? "[ON]" : "[OFF]") << endl;
    setColor(11); cout << "\t5. "; setColor(12); cout << "Back\n";
    setColor(7); cout << "\n\tChoose: ";
}

void HighScoreScreen() {
    clearScreen();
    setColor(11); cout << "\n\t---- ";
    setColor(13); cout << "High Scores (Per Level)";
    setColor(11); cout << " ----\n";
    setColor(10); cout << "\n\tEasy   : " << highScoreEasy;
    setColor(14); cout << "\n\tMedium : " << highScoreMedium;
    setColor(12); cout << "\n\tHard   : " << highScoreHard;
    setColor(7); cout << "\n\n\t";
    setColor(8); cout << "(Press any key to go back...)";
    setColor(7); _getch();
}

void placeObstacles(int numObs) {
    set<pair<int, int>> forbidden;
    forbidden.insert({x, y});
    forbidden.insert({fruitX, fruitY});
    forbidden.insert({bonusX, bonusY});
    forbidden.insert({poisonX, poisonY});
    for (auto t : tail) forbidden.insert(t);
    for (auto obs : obstacles) forbidden.insert(obs);

    int placed = obstacles.size(), tryCount = 0;
    while (placed < numObs && tryCount < numObs*20) {
        int ox = rand() % width;
        int oy = rand() % height;
        if (forbidden.count({ox, oy}) == 0) {
            obstacles.push_back({ox, oy});
            forbidden.insert({ox, oy});
            placed++;
        }
        tryCount++;
    }
}

void tryAddObstacle() {
    int currentLimit = obstacleLimit;
    if (level == 2 && score > 0 && score % 10 == 0) currentLimit++;
    if (level == 3 && score > 0 && score % 5 == 0) currentLimit++;
    if (currentLimit > obstacles.size()) placeObstacles(currentLimit);
}

void Setup() {
    gameOver = false; dir = STOP;
    x = width / 2; y = height / 2;
    randomPos(fruitX, fruitY);
    score = 0; nTail = 0; tail.clear();
    bonusAlive = false; poisonAlive = false; bonusTimer = 0;
    obstacles.clear();
    if (level==2) obstacleLimit = 5;
    else if (level==3) obstacleLimit = 10;
    else obstacleLimit = 0;
    if (obstacleLimit > 0) placeObstacles(obstacleLimit);
    customSpeed = 0;
}

void Draw() {
    gotoxy(0, 0);
    setColor(theme_wall);
    for (int i = 0; i < width+2; i++) cout << "#";
    cout << endl;

    for (int i = 0; i < height; i++) {
        setColor(theme_wall); cout << "#";
        for (int j = 0; j < width; j++) {
            bool drawn = false;
            for (auto obs : obstacles) if (obs.first == j && obs.second == i) {
                setColor(theme_obs); cout << "#"; drawn=true; break;
            }
            if (drawn) continue;
            if (i == y && j == x) { setColor(theme_snake); cout << "O"; }
            else if (i == fruitY && j == fruitX) { setColor(theme_fruit); cout << "@"; }
            else if (bonusAlive && i == bonusY && j == bonusX) { setColor(theme_bonus); cout << "$"; }
            else if (poisonAlive && i == poisonY && j == poisonX) { setColor(theme_poison); cout << "X"; }
            else {
                bool print = false;
                for (int k = 0; k < nTail; k++) {
                    if (tail[k].first == j && tail[k].second == i) { setColor(theme_snakebody); cout << "o"; print = true; }
                }
                if (!print) { setColor(theme_bg); cout << " "; }
            }
        }
        setColor(theme_wall); cout << "#\n";
    }
    setColor(theme_wall);
    for (int i = 0; i < width+2; i++) cout << "#";
    setColor(7);

    cout << "\n";
    setColor(14); cout << "Score: "; setColor(10); cout << score;
    setColor(8); cout << "   [Level: ";
    setColor(level==1?10:level==2?14:12);
    cout << (level==1?"Easy":level==2?"Medium":"Hard"); setColor(8); cout << "]";
    setColor(7); cout << "   ";
    setColor(10); cout << (soundOn ? "Sound: ON" : "Sound: OFF");
    setColor(7);

    cout << "\n";
    setColor(10); cout << "@(green) "; setColor(7); cout << "+10 | ";
    setColor(12); cout << "$(red) "; setColor(7); cout << "+25 | ";
    setColor(13); cout << "X(magenta) "; setColor(7); cout << "-10 ";
    setColor(12); cout << "| #(red)=Obstacle";
    setColor(7);

    setColor(8); cout << "\nPause[P] | Exit[X] | W/A/S/D Move | [+] Faster [-] Slower";
    setColor(7); cout << endl;
}

void Input() {
    if (_kbhit()) {
        switch (_getch()) {
            case 'a': case 'A': dir = LEFT; break;
            case 'd': case 'D': dir = RIGHT; break;
            case 'w': case 'W': dir = UP; break;
            case 's': case 'S': dir = DOWN; break;
            case '+': if (customSpeed > -50) customSpeed -= 5; break;
            case '-': if (customSpeed < 200) customSpeed += 5; break;
            case 'x': case 'X': gameOver = true; break;
            case 'p': case 'P':
                setColor(14); cout << "\nPaused. Press any key to continue..."; setColor(7); _getch(); break;
        }
    }
}

void Logic() {
    int prevX = x, prevY = y, prev2X, prev2Y;
    if (nTail > 0) {
        prev2X = tail[0].first; prev2Y = tail[0].second;
        tail[0] = {x, y};
        for (int i = 1; i < nTail; i++) {
            int tempX = tail[i].first, tailY = tail[i].second;
            tail[i] = {prev2X, prev2Y};
            prev2X = tempX; prev2Y = tailY;
        }
    }

    switch (dir) {
        case LEFT: x--; break;
        case RIGHT: x++; break;
        case UP: y--; break;
        case DOWN: y++; break;
        default: break;
    }

    if (x < 0 || x >= width || y < 0 || y >= height) { playGameOverSound(); gameOver = true; }
    for (int i = 0; i < nTail; i++)
        if (tail[i].first == x && tail[i].second == y) { playGameOverSound(); gameOver = true; }
    for (auto obs : obstacles)
        if (obs.first == x && obs.second == y) { playGameOverSound(); gameOver = true; }

    if (x == fruitX && y == fruitY) {
        playEatSound(); score += 10;
        nTail++; tail.push_back({x, y});
        randomPos(fruitX, fruitY);
        if (!bonusAlive && rand()%4==0) {
            randomPos(bonusX, bonusY);
            bonusAlive = true;
            bonusStartTick = GetTickCount();
        }
        if (!poisonAlive && rand()%5==0) { randomPos(poisonX, poisonY); poisonAlive = true; }
        tryAddObstacle();
    }
    if (bonusAlive && (GetTickCount() - bonusStartTick > 5000)) { // 5 sec
        bonusAlive = false;
    }
    if (bonusAlive && x == bonusX && y == bonusY) {
        playBonusSound(); score += 25; nTail+=2;
        tail.push_back({x, y}); tail.push_back({x, y});
        bonusAlive = false;
        tryAddObstacle();
    }
    if (poisonAlive && x == poisonX && y == poisonY) {
        playPoisonSound(); score -= 10;
        if (nTail > 0) { nTail--; tail.pop_back(); }
        poisonAlive = false;
        if (score < 0) score = 0;
        tryAddObstacle();
    }
    if (bonusAlive) bonusTimer++;
}

void GameLoop() {
    showStartAnimation();
    Setup();
    dir = RIGHT;

    int baseEasy = 130;
    int baseMedium = 70;
    int baseHard = 40;
    int areaK = 1;

    while (!gameOver) {
        Draw();
        Input();
        Logic();

        int area = width * height;
        int delay = baseEasy;
        if (level == 2) delay = baseMedium;
        if (level == 3) delay = baseHard;

        delay -= (area / (level == 1 ? 60 : 50)) * areaK;
        delay += customSpeed;
        if (delay < 10) delay = 10;
        Sleep(delay);
    }
    if (level==1 && score > highScoreEasy) highScoreEasy = score;
    else if (level==2 && score > highScoreMedium) highScoreMedium = score;
    else if (level==3 && score > highScoreHard) highScoreHard = score;

    setColor(12);
    cout << "\n\n\t";
    setColor(14); cout << "████ ";
    setColor(12); cout << "GAME OVER!";
    setColor(14); cout << " ████";
    setColor(7);
    cout << "\n\tYour score: ";
    setColor(10); cout << score << endl;
    setColor(8); cout << "\t(Press any key to return to menu...)";
    setColor(7); _getch();
}

void Settings() {
    int op = 0;
    while (true) {
        printSettings();
        cin >> op;
        if (op==1) { setColor(7); cout << "\nTheme change is disabled for simplicity!\n"; Sleep(700);}
        else if (op==2) { setColor(7); cout << "\nSnake color change is disabled for simplicity!\n"; Sleep(700);}
        else if (op==3) { setColor(7); cout << "\nWall color change is disabled for simplicity!\n"; Sleep(700);}
        else if (op==4) { soundOn = !soundOn;
            cout << "\n\tSound is now " << (soundOn ? "ON!" : "OFF!") << endl;
            playSelectSound(); Sleep(500);
        }
        else break;
    }
}

int main() {
    srand((unsigned)time(0));
    int op;
    while (true) {
        printMenu(); cin >> op;
        if (op==1) {
            while (true) {
                int lv = printLevelMenu();
                if (lv==1) { level = 1; playLevelUpSound(); GameLoop(); break; }
                else if (lv==2) { level = 2; playLevelUpSound(); GameLoop(); break; }
                else if (lv==3) { level = 3; playLevelUpSound(); GameLoop(); break; }
                else if (lv==4) break;
            }
        }
        else if (op==2) Settings();
        else if (op==3) HighScoreScreen();
        else if (op==4) break;
    }
    setColor(7);
    return 0;
}
