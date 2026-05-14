#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "raylib.h"
#include "world.h"
#include "patterns.h"
#include "file_io.h"
#include "barriers.h"
#include "levels.h"

// ========== НАСТРОЙКИ ОКНА ==========
#define PANEL_WIDTH 300
#define MIN_CELL_SIZE 5
#define MAX_CELL_SIZE 35

// ========== ЦВЕТА ==========
#define COLOR_DEAD   (Color){ 40, 40, 40, 255 }
#define COLOR_ALIVE  (Color){ 100, 200, 100, 255 }
#define COLOR_GRID   (Color){ 60, 60, 60, 255 }
#define COLOR_PANEL  (Color){ 30, 30, 40, 255 }
#define COLOR_BUTTON (Color){ 60, 60, 80, 255 }
#define COLOR_BUTTON_HOVER (Color){ 90, 90, 120, 255 }
#define COLOR_BUTTON_ACTIVE (Color){ 120, 120, 160, 255 }
#define COLOR_BARRIER (Color){ 255, 80, 80, 255 }

// ========== РЕЖИМЫ ==========
typedef enum {
    GAME_MODE_CLASSIC,
    GAME_MODE_CAMPAIGN
} GameMode;

typedef enum {
    CAMPAIGN_PHASE_PLACING,
    CAMPAIGN_PHASE_SIMULATING,
    CAMPAIGN_PHASE_COMPLETE,
    CAMPAIGN_PHASE_FAILED
} CampaignPhase;

// ========== СОСТОЯНИЕ ==========
typedef struct {
    GameMode gameMode;
    int cellSize;
    bool paused;
    bool showGrid;
    float speed;
    float timeAccumulator;
    
    // Классический режим
    int classicWorld[HEIGHT][WIDTH];
    int classicGeneration;
    
    // Кампания
    Level *currentLevel;
    int **campaignWorld;
    int campaignGeneration;
    int startCellsPlaced;
    CampaignPhase campaignPhase;
    bool levelCompleted;
    int currentLevelId;
    
    bool showMenu;
} GameState;

// ========== ПРОТОТИПЫ ==========
void init_classic(GameState *state);
void init_campaign(GameState *state, int levelId);
void free_campaign(GameState *state);
void draw_button(const char *text, int x, int y, int w, int h, bool hover, bool active);
void draw_main_menu(GameState *state, Vector2 mouse);
void draw_classic_ui(GameState *state, Vector2 mouse);
void draw_campaign_ui(GameState *state, Vector2 mouse);
void draw_barriers(Barriers *b, int cellSize, int offsetX, int offsetY);
void handle_main_menu_input(GameState *state, Vector2 mouse);
void handle_classic_input(GameState *state, Vector2 mouse);
void handle_campaign_input(GameState *state, Vector2 mouse);
void auto_solve_campaign(GameState *state);
bool check_campaign_win(GameState *state);

// ========== РЕАЛИЗАЦИЯ ==========

void init_classic(GameState *state) {
    pattern_random(state->classicWorld, 25);
    state->classicGeneration = 0;
    state->paused = true;
    state->speed = 5.0f;
    state->timeAccumulator = 0.0f;
    state->showGrid = true;
    int maxCellWidth = (GetScreenWidth() - PANEL_WIDTH) / WIDTH;
    int maxCellHeight = GetScreenHeight() / HEIGHT;
    state->cellSize = fmin(maxCellWidth, maxCellHeight);
    state->cellSize = fmax(MIN_CELL_SIZE, fmin(state->cellSize, MAX_CELL_SIZE));
}

void init_campaign(GameState *state, int levelId) {
    if (state->currentLevel) free_level(state->currentLevel);
    state->currentLevel = load_level(levelId);
    if (!state->currentLevel) {
        printf("Failed to load level %d\n", levelId);
        state->showMenu = true;
        return;
    }
    // Создаём мир для кампании
    if (state->campaignWorld) {
        for (int i = 0; i < state->currentLevel->height; i++) free(state->campaignWorld[i]);
        free(state->campaignWorld);
    }
    int h = state->currentLevel->height, w = state->currentLevel->width;
    state->campaignWorld = (int**)malloc(h * sizeof(int*));
    for (int i = 0; i < h; i++) {
        state->campaignWorld[i] = (int*)calloc(w, sizeof(int));
    }
    state->campaignGeneration = 0;
    state->startCellsPlaced = 0;
    state->campaignPhase = CAMPAIGN_PHASE_PLACING;
    state->levelCompleted = false;
    state->currentLevelId = levelId;
    
    // Размер клетки
    int maxCellWidth = (GetScreenWidth() - PANEL_WIDTH) / w;
    int maxCellHeight = GetScreenHeight() / h;
    state->cellSize = fmin(maxCellWidth, maxCellHeight);
    state->cellSize = fmax(MIN_CELL_SIZE, fmin(state->cellSize, MAX_CELL_SIZE));
}

void free_campaign(GameState *state) {
    if (state->campaignWorld) {
        for (int i = 0; i < state->currentLevel->height; i++) free(state->campaignWorld[i]);
        free(state->campaignWorld);
        state->campaignWorld = NULL;
    }
    if (state->currentLevel) {
        free_level(state->currentLevel);
        state->currentLevel = NULL;
    }
}

void draw_button(const char *text, int x, int y, int w, int h, bool hover, bool active) {
    Color col = active ? COLOR_BUTTON_ACTIVE : (hover ? COLOR_BUTTON_HOVER : COLOR_BUTTON);
    DrawRectangle(x, y, w, h, col);
    DrawRectangleLines(x, y, w, h, LIGHTGRAY);
    int tw = MeasureText(text, 16);
    DrawText(text, x + (w - tw)/2, y + (h - 16)/2, 16, WHITE);
}

void draw_main_menu(GameState *state, Vector2 mouse) {
    ClearBackground(BLACK);
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    DrawText("CONWAY'S GAME OF LIFE", sw/2 - 200, 100, 30, WHITE);
    
    Rectangle classicBtn = { sw/2 - 100, 250, 200, 50 };
    Rectangle campaignBtn = { sw/2 - 100, 330, 200, 50 };
    bool classicHover = CheckCollisionPointRec(mouse, classicBtn);
    bool campaignHover = CheckCollisionPointRec(mouse, campaignBtn);
    draw_button("Classic Mode", classicBtn.x, classicBtn.y, classicBtn.width, classicBtn.height, classicHover, false);
    draw_button("Campaign", campaignBtn.x, campaignBtn.y, campaignBtn.width, campaignBtn.height, campaignHover, false);
}

void handle_main_menu_input(GameState *state, Vector2 mouse) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        int sw = GetScreenWidth(), sh = GetScreenHeight();
        Rectangle classicBtn = { sw/2 - 100, 250, 200, 50 };
        Rectangle campaignBtn = { sw/2 - 100, 330, 200, 50 };
        if (CheckCollisionPointRec(mouse, classicBtn)) {
            state->gameMode = GAME_MODE_CLASSIC;
            init_classic(state);
            state->showMenu = false;
        }
        if (CheckCollisionPointRec(mouse, campaignBtn)) {
            state->gameMode = GAME_MODE_CAMPAIGN;
            init_campaign(state, 1);
            state->showMenu = false;
        }
    }
}

void draw_classic_ui(GameState *state, Vector2 mouse) {
    int panelX = WIDTH * state->cellSize;
    DrawRectangle(panelX, 0, PANEL_WIDTH, GetScreenHeight(), COLOR_PANEL);
    DrawLine(panelX, 0, panelX, GetScreenHeight(), LIGHTGRAY);
    
    int y = 20;
    DrawText("CLASSIC MODE", panelX + 20, y, 20, WHITE); y += 35;
    DrawText(TextFormat("Gen: %d", state->classicGeneration), panelX + 20, y, 16, YELLOW); y += 25;
    int live = 0; for (int i=0;i<HEIGHT;i++) for (int j=0;j<WIDTH;j++) live += state->classicWorld[i][j];
    DrawText(TextFormat("Live: %d", live), panelX + 20, y, 16, YELLOW); y += 25;
    DrawText(TextFormat("Speed: %.1f", state->speed), panelX + 20, y, 16, YELLOW); y += 40;
    
    DrawText("CONTROLS:", panelX + 20, y, 16, WHITE); y += 25;
    DrawText("Space - Pause", panelX + 30, y, 14, GRAY); y += 20;
    DrawText("Enter - Step", panelX + 30, y, 14, GRAY); y += 20;
    DrawText("+/- - Speed", panelX + 30, y, 14, GRAY); y += 20;
    DrawText("G - Grid", panelX + 30, y, 14, GRAY); y += 20;
    DrawText("1-5 - Rules", panelX + 30, y, 14, GRAY); y += 20;
    DrawText("F1-F4 - Patterns", panelX + 30, y, 14, GRAY); y += 20;
    
    // Кнопки паттернов
    y += 20;
    Rectangle randomBtn = { panelX + 20, y, 120, 30 };
    Rectangle gliderBtn = { panelX + 150, y, 120, 30 };
    y += 40;
    Rectangle blinkerBtn = { panelX + 20, y, 120, 30 };
    Rectangle clearBtn  = { panelX + 150, y, 120, 30 };
    y += 50;
    Rectangle menuBtn = { panelX + 20, y, 260, 40 };
    
    bool randomH = CheckCollisionPointRec(mouse, randomBtn);
    bool gliderH = CheckCollisionPointRec(mouse, gliderBtn);
    bool blinkerH = CheckCollisionPointRec(mouse, blinkerBtn);
    bool clearH = CheckCollisionPointRec(mouse, clearBtn);
    bool menuH = CheckCollisionPointRec(mouse, menuBtn);
    
    draw_button("Random", randomBtn.x, randomBtn.y, randomBtn.width, randomBtn.height, randomH, false);
    draw_button("Glider", gliderBtn.x, gliderBtn.y, gliderBtn.width, gliderBtn.height, gliderH, false);
    draw_button("Blinker", blinkerBtn.x, blinkerBtn.y, blinkerBtn.width, blinkerBtn.height, blinkerH, false);
    draw_button("Clear", clearBtn.x, clearBtn.y, clearBtn.width, clearBtn.height, clearH, false);
    draw_button("Back to Menu", menuBtn.x, menuBtn.y, menuBtn.width, menuBtn.height, menuH, false);
    
    // Сохраняем кнопки в структуре или обрабатываем сразу здесь – в обработчике пересчитываем координаты
}

void handle_classic_input(GameState *state, Vector2 mouse) {
    int panelX = WIDTH * state->cellSize;
    int y = 20;
    // вычисляем координаты кнопок (аналогично draw)
    y += 35 + 25*3 + 40 + 25*6 + 20; // пролистывание до кнопок паттернов
    Rectangle randomBtn = { panelX + 20, y, 120, 30 };
    Rectangle gliderBtn = { panelX + 150, y, 120, 30 };
    y += 40;
    Rectangle blinkerBtn = { panelX + 20, y, 120, 30 };
    Rectangle clearBtn  = { panelX + 150, y, 120, 30 };
    y += 50;
    Rectangle menuBtn = { panelX + 20, y, 260, 40 };
    
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mouse, randomBtn)) {
            pattern_random(state->classicWorld, 25);
            state->classicGeneration = 0;
        }
        if (CheckCollisionPointRec(mouse, gliderBtn)) {
            for (int i=0;i<HEIGHT;i++) for (int j=0;j<WIDTH;j++) state->classicWorld[i][j]=0;
            pattern_glider(state->classicWorld, HEIGHT/2, WIDTH/2);
            state->classicGeneration = 0;
        }
        if (CheckCollisionPointRec(mouse, blinkerBtn)) {
            for (int i=0;i<HEIGHT;i++) for (int j=0;j<WIDTH;j++) state->classicWorld[i][j]=0;
            pattern_blinker(state->classicWorld, HEIGHT/2, WIDTH/2-1);
            state->classicGeneration = 0;
        }
        if (CheckCollisionPointRec(mouse, clearBtn)) {
            for (int i=0;i<HEIGHT;i++) for (int j=0;j<WIDTH;j++) state->classicWorld[i][j]=0;
            state->classicGeneration = 0;
        }
        if (CheckCollisionPointRec(mouse, menuBtn)) {
            state->showMenu = true;
            state->gameMode = GAME_MODE_CLASSIC; // не важно
        }
    }
    // Клавиатурные команды (пробел, +/-, G, Enter, 1-5, F1-F4)
    if (IsKeyPressed(KEY_SPACE)) state->paused = !state->paused;
    if (IsKeyPressed(KEY_ENTER) && state->paused) {
        next_generation(state->classicWorld);
        state->classicGeneration++;
    }
    if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) state->speed = fmin(30, state->speed+1);
    if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) state->speed = fmax(1, state->speed-1);
    if (IsKeyPressed(KEY_G)) state->showGrid = !state->showGrid;
    if (IsKeyPressed(KEY_C)) {
        for (int i=0;i<HEIGHT;i++) for (int j=0;j<WIDTH;j++) state->classicWorld[i][j]=0;
        state->classicGeneration = 0;
    }
    if (IsKeyPressed(KEY_R)) pattern_random(state->classicWorld, 25);
    // Правила (1-5) – в классике используем стандартные, показываем но не меняем
    // Паттерны F1-F4
    if (IsKeyPressed(KEY_F1)) {
        pattern_random(state->classicWorld, 25);
        state->classicGeneration = 0;
    }
    if (IsKeyPressed(KEY_F2)) {
        for (int i=0;i<HEIGHT;i++) for (int j=0;j<WIDTH;j++) state->classicWorld[i][j]=0;
        pattern_glider(state->classicWorld, HEIGHT/2, WIDTH/2);
        state->classicGeneration = 0;
    }
    if (IsKeyPressed(KEY_F3)) {
        for (int i=0;i<HEIGHT;i++) for (int j=0;j<WIDTH;j++) state->classicWorld[i][j]=0;
        pattern_blinker(state->classicWorld, HEIGHT/2, WIDTH/2-1);
        state->classicGeneration = 0;
    }
}

void draw_barriers(Barriers *b, int cellSize, int offsetX, int offsetY) {
    if (!b) return;
    for (int y = 0; y < b->height; y++) {
        for (int x = 0; x < b->width - 1; x++) {
            if (has_h_barrier(b, y, x)) {
                int x1 = offsetX + (x+1)*cellSize;
                int y1 = offsetY + y*cellSize + cellSize/2;
                int x2 = x1, y2 = y1;
                DrawLine(x1, y1, x2, y2, COLOR_BARRIER);
            }
        }
    }
    for (int y = 0; y < b->height - 1; y++) {
        for (int x = 0; x < b->width; x++) {
            if (has_v_barrier(b, y, x)) {
                int x1 = offsetX + x*cellSize + cellSize/2;
                int y1 = offsetY + (y+1)*cellSize;
                int x2 = x1, y2 = y1;
                DrawLine(x1, y1, x2, y2, COLOR_BARRIER);
            }
        }
    }
}

void draw_campaign_ui(GameState *state, Vector2 mouse) {
    if (!state->currentLevel) return;
    int w = state->currentLevel->width, h = state->currentLevel->height;
    int panelX = w * state->cellSize;
    DrawRectangle(panelX, 0, PANEL_WIDTH, GetScreenHeight(), COLOR_PANEL);
    DrawLine(panelX, 0, panelX, GetScreenHeight(), LIGHTGRAY);
    
    int y = 20;
    DrawText("CAMPAIGN", panelX + 20, y, 20, WHITE); y += 30;
    DrawText(TextFormat("Level %d", state->currentLevelId), panelX + 20, y, 16, YELLOW); y += 25;
    DrawText(TextFormat("Gen: %d / %d", state->campaignGeneration, state->currentLevel->target_generations), panelX + 20, y, 16, YELLOW); y += 25;
    int live = 0; for (int i=0;i<h;i++) for (int j=0;j<w;j++) live += state->campaignWorld[i][j];
    DrawText(TextFormat("Infected: %d / %d", live, w*h), panelX + 20, y, 16, YELLOW); y += 25;
    DrawText(TextFormat("Start cells: %d / %d", state->startCellsPlaced, state->currentLevel->max_start_cells), panelX + 20, y, 16, YELLOW); y += 35;
    
    if (state->campaignPhase == CAMPAIGN_PHASE_PLACING) {
        DrawText("PHASE: PLACE CELLS", panelX + 20, y, 16, WHITE); y += 30;
        Rectangle startBtn = { panelX + 20, y, 120, 35 };
        Rectangle resetBtn = { panelX + 150, y, 120, 35 };
        y += 45;
        Rectangle autoBtn = { panelX + 20, y, 260, 35 };
        y += 50;
        Rectangle menuBtn = { panelX + 20, y, 260, 40 };
        
        bool startH = CheckCollisionPointRec(mouse, startBtn);
        bool resetH = CheckCollisionPointRec(mouse, resetBtn);
        bool autoH = CheckCollisionPointRec(mouse, autoBtn);
        bool menuH = CheckCollisionPointRec(mouse, menuBtn);
        draw_button("Start", startBtn.x, startBtn.y, startBtn.width, startBtn.height, startH, false);
        draw_button("Reset", resetBtn.x, resetBtn.y, resetBtn.width, resetBtn.height, resetH, false);
        draw_button("Auto Solve", autoBtn.x, autoBtn.y, autoBtn.width, autoBtn.height, autoH, false);
        draw_button("Back to Menu", menuBtn.x, menuBtn.y, menuBtn.width, menuBtn.height, menuH, false);
    } else if (state->campaignPhase == CAMPAIGN_PHASE_SIMULATING) {
        DrawText("PHASE: SIMULATING", panelX + 20, y, 16, WHITE); y += 30;
        Rectangle pauseBtn = { panelX + 20, y, 120, 35 };
        y += 45;
        Rectangle menuBtn = { panelX + 20, y, 260, 40 };
        bool pauseH = CheckCollisionPointRec(mouse, pauseBtn);
        bool menuH = CheckCollisionPointRec(mouse, menuBtn);
        draw_button(state->paused ? "Run" : "Pause", pauseBtn.x, pauseBtn.y, pauseBtn.width, pauseBtn.height, pauseH, false);
        draw_button("Back to Menu", menuBtn.x, menuBtn.y, menuBtn.width, menuBtn.height, menuH, false);
    } else if (state->campaignPhase == CAMPAIGN_PHASE_COMPLETE) {
        DrawText("LEVEL COMPLETE!", panelX + 20, y, 20, GREEN); y += 40;
        Rectangle nextBtn = { panelX + 20, y, 120, 35 };
        Rectangle menuBtn = { panelX + 150, y, 120, 35 };
        y += 50;
        bool nextH = CheckCollisionPointRec(mouse, nextBtn);
        bool menuH = CheckCollisionPointRec(mouse, menuBtn);
        draw_button("Next Level", nextBtn.x, nextBtn.y, nextBtn.width, nextBtn.height, nextH, false);
        draw_button("Menu", menuBtn.x, menuBtn.y, menuBtn.width, menuBtn.height, menuH, false);
    } else if (state->campaignPhase == CAMPAIGN_PHASE_FAILED) {
        DrawText("FAILED! Try again.", panelX + 20, y, 20, RED); y += 40;
        Rectangle retryBtn = { panelX + 20, y, 120, 35 };
        Rectangle menuBtn = { panelX + 150, y, 120, 35 };
        y += 50;
        bool retryH = CheckCollisionPointRec(mouse, retryBtn);
        bool menuH = CheckCollisionPointRec(mouse, menuBtn);
        draw_button("Retry", retryBtn.x, retryBtn.y, retryBtn.width, retryBtn.height, retryH, false);
        draw_button("Menu", menuBtn.x, menuBtn.y, menuBtn.width, menuBtn.height, menuH, false);
    }
}

void handle_campaign_input(GameState *state, Vector2 mouse) {
    if (!state->currentLevel) return;
    int w = state->currentLevel->width, h = state->currentLevel->height;
    int panelX = w * state->cellSize;
    int y = 20;
    // Для всех фаз кнопки расположены по-разному, вычислим их динамически
    if (state->campaignPhase == CAMPAIGN_PHASE_PLACING) {
        y += 30+25*4+35; // после заголовка и статов
        Rectangle startBtn = { panelX + 20, y, 120, 35 };
        Rectangle resetBtn = { panelX + 150, y, 120, 35 };
        y += 45;
        Rectangle autoBtn = { panelX + 20, y, 260, 35 };
        y += 50;
        Rectangle menuBtn = { panelX + 20, y, 260, 40 };
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mouse, startBtn)) {
                if (state->startCellsPlaced <= state->currentLevel->max_start_cells && state->startCellsPlaced > 0) {
                    state->campaignPhase = CAMPAIGN_PHASE_SIMULATING;
                    state->paused = false;
                }
            }
            if (CheckCollisionPointRec(mouse, resetBtn)) {
                for (int i=0;i<h;i++) for (int j=0;j<w;j++) state->campaignWorld[i][j] = 0;
                state->startCellsPlaced = 0;
                state->campaignGeneration = 0;
            }
            if (CheckCollisionPointRec(mouse, autoBtn)) {
                auto_solve_campaign(state);
            }
            if (CheckCollisionPointRec(mouse, menuBtn)) {
                free_campaign(state);
                state->showMenu = true;
            }
        }
        // Размещение клеток мышью
        int gameWidth = w * state->cellSize, gameHeight = h * state->cellSize;
        if (mouse.x < gameWidth && mouse.y < gameHeight && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int x = mouse.x / state->cellSize, y = mouse.y / state->cellSize;
            if (x>=0 && x<w && y>=0 && y<h && state->campaignWorld[y][x]==0 && state->startCellsPlaced < state->currentLevel->max_start_cells) {
                state->campaignWorld[y][x] = 1;
                state->startCellsPlaced++;
            }
        }
    } else if (state->campaignPhase == CAMPAIGN_PHASE_SIMULATING) {
        y += 30+25*4+35;
        Rectangle pauseBtn = { panelX + 20, y, 120, 35 };
        y += 45;
        Rectangle menuBtn = { panelX + 20, y, 260, 40 };
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mouse, pauseBtn)) state->paused = !state->paused;
            if (CheckCollisionPointRec(mouse, menuBtn)) {
                free_campaign(state);
                state->showMenu = true;
            }
        }
        // Клавиатурные команды в симуляции
        if (IsKeyPressed(KEY_SPACE)) state->paused = !state->paused;
        if (IsKeyPressed(KEY_ENTER) && state->paused) {
            next_generation_with_barriers(state->campaignWorld, state->currentLevel->barriers, w, h, &state->campaignGeneration);
        }
        if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) state->speed = fmin(30, state->speed+1);
        if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) state->speed = fmax(1, state->speed-1);
        if (IsKeyPressed(KEY_G)) state->showGrid = !state->showGrid;
    } else if (state->campaignPhase == CAMPAIGN_PHASE_COMPLETE) {
        y += 30+25*4+35+40;
        Rectangle nextBtn = { panelX + 20, y, 120, 35 };
        Rectangle menuBtn = { panelX + 150, y, 120, 35 };
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mouse, nextBtn)) {
                int nextLevel = state->currentLevelId + 1;
                if (nextLevel <= get_total_levels()) {
                    init_campaign(state, nextLevel);
                } else {
                    // Все уровни пройдены
                    state->showMenu = true;
                }
            }
            if (CheckCollisionPointRec(mouse, menuBtn)) {
                free_campaign(state);
                state->showMenu = true;
            }
        }
    } else if (state->campaignPhase == CAMPAIGN_PHASE_FAILED) {
        y += 30+25*4+35+40;
        Rectangle retryBtn = { panelX + 20, y, 120, 35 };
        Rectangle menuBtn = { panelX + 150, y, 120, 35 };
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mouse, retryBtn)) {
                init_campaign(state, state->currentLevelId); // рестарт уровня
            }
            if (CheckCollisionPointRec(mouse, menuBtn)) {
                free_campaign(state);
                state->showMenu = true;
            }
        }
    }
}

void auto_solve_campaign(GameState *state) {
    if (state->campaignPhase != CAMPAIGN_PHASE_PLACING) return;
    int w = state->currentLevel->width, h = state->currentLevel->height;
    int maxCells = state->currentLevel->max_start_cells;
    // Очистить поле
    for (int i=0;i<h;i++) for (int j=0;j<w;j++) state->campaignWorld[i][j] = 0;
    state->startCellsPlaced = 0;
    // Простая стратегия: выбрать случайные клетки, не слишком близко
    int attempts = 0;
    while (state->startCellsPlaced < maxCells && attempts < 1000) {
        int x = rand() % w, y = rand() % h;
        if (state->campaignWorld[y][x] == 0) {
            state->campaignWorld[y][x] = 1;
            state->startCellsPlaced++;
        }
        attempts++;
    }
}

bool check_campaign_win(GameState *state) {
    int w = state->currentLevel->width, h = state->currentLevel->height;
    for (int y=0; y<h; y++)
        for (int x=0; x<w; x++)
            if (state->campaignWorld[y][x] == 0) return false;
    return true;
}

// ========== MAIN ==========
int main() {
    srand(time(NULL));
    int screenWidth = 1300, screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Game of Life - Adventure");
    SetTargetFPS(60);
    
    GameState state = {0};
    state.showMenu = true;
    state.gameMode = GAME_MODE_CLASSIC;
    
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        Vector2 mouse = GetMousePosition();
        
        if (state.showMenu) {
            draw_main_menu(&state, mouse);
            handle_main_menu_input(&state, mouse);
        } else {
            if (state.gameMode == GAME_MODE_CLASSIC) {
                // Обновление классического режима
                if (!state.paused) {
                    state.timeAccumulator += dt;
                    float step = 1.0f / state.speed;
                    while (state.timeAccumulator >= step) {
                        next_generation(state.classicWorld);
                        state.classicGeneration++;
                        state.timeAccumulator -= step;
                    }
                }
                // Отрисовка поля
                int gameWidth = WIDTH * state.cellSize, gameHeight = HEIGHT * state.cellSize;
                for (int y=0; y<HEIGHT; y++)
                    for (int x=0; x<WIDTH; x++) {
                        Color col = state.classicWorld[y][x] ? COLOR_ALIVE : COLOR_DEAD;
                        DrawRectangle(x*state.cellSize, y*state.cellSize, state.cellSize, state.cellSize, col);
                    }
                if (state.showGrid) {
                    for (int i=0;i<=WIDTH;i++) DrawLine(i*state.cellSize,0,i*state.cellSize, gameHeight, COLOR_GRID);
                    for (int i=0;i<=HEIGHT;i++) DrawLine(0,i*state.cellSize, gameWidth, i*state.cellSize, COLOR_GRID);
                }
                draw_classic_ui(&state, mouse);
                handle_classic_input(&state, mouse);
            } else if (state.gameMode == GAME_MODE_CAMPAIGN && state.currentLevel) {
                int w = state.currentLevel->width, h = state.currentLevel->height;
                int gameWidth = w * state.cellSize, gameHeight = h * state.cellSize;
                // Отрисовка поля
                for (int y=0; y<h; y++)
                    for (int x=0; x<w; x++) {
                        Color col = state.campaignWorld[y][x] ? COLOR_ALIVE : COLOR_DEAD;
                        DrawRectangle(x*state.cellSize, y*state.cellSize, state.cellSize, state.cellSize, col);
                    }
                if (state.showGrid) {
                    for (int i=0;i<=w;i++) DrawLine(i*state.cellSize,0,i*state.cellSize, gameHeight, COLOR_GRID);
                    for (int i=0;i<=h;i++) DrawLine(0,i*state.cellSize, gameWidth, i*state.cellSize, COLOR_GRID);
                }
                draw_barriers(state.currentLevel->barriers, state.cellSize, 0, 0);
                
                draw_campaign_ui(&state, mouse);
                handle_campaign_input(&state, mouse);
                
                // Симуляция
                if (state.campaignPhase == CAMPAIGN_PHASE_SIMULATING && !state.paused) {
                    state.timeAccumulator += dt;
                    float step = 1.0f / state.speed;
                    while (state.timeAccumulator >= step) {
                        next_generation_with_barriers(state.campaignWorld, state.currentLevel->barriers, w, h, &state.campaignGeneration);
                        state.timeAccumulator -= step;
                        // Проверка победы
                        if (check_campaign_win(&state)) {
                            state.campaignPhase = CAMPAIGN_PHASE_COMPLETE;
                            state.paused = true;
                            break;
                        }
                        if (state.campaignGeneration >= state.currentLevel->target_generations) {
                            state.campaignPhase = CAMPAIGN_PHASE_FAILED;
                            state.paused = true;
                            break;
                        }
                    }
                }
            }
        }
        
        BeginDrawing();
        ClearBackground(BLACK);
        // Всё уже нарисовано внутри веток
        EndDrawing();
    }
    
    free_campaign(&state);
    CloseWindow();
    return 0;
}