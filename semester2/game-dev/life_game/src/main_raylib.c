#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdarg.h>
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

// ========== ДИНАМИЧЕСКАЯ СТРУКТУРА ДЛЯ ИСТОРИИ ==========
typedef struct HistoryNode {
    int **world;
    int generation;
    struct HistoryNode *prev;
    struct HistoryNode *next;
} HistoryNode;

typedef struct {
    HistoryNode *head;
    HistoryNode *current;
    int width, height;
} HistoryList;

// ========== КНОПКИ ==========
typedef struct {
    Rectangle classic, campaign, about, help, exit;
} MainMenuButtons;

typedef struct {
    Rectangle random, glider, blinker, clear, menu;
} ClassicButtons;

typedef struct {
    Rectangle start, reset, autoSolve, menu;
    Rectangle pause, menuSim;
    Rectangle next, menuComplete;
    Rectangle retry, menuFailed;
} CampaignButtons;

// ========== СОСТОЯНИЕ ==========
typedef struct {
    GameMode gameMode;
    int cellSize;
    bool paused;
    bool showGrid;
    float speed;
    float timeAccumulator;
    
    int classicWorld[HEIGHT][WIDTH];
    int classicGeneration;
    int classicDensity;
    
    Level *currentLevel;
    int **campaignWorld;
    int campaignGeneration;
    int startCellsPlaced;
    CampaignPhase campaignPhase;
    int currentLevelId;
    
    HistoryList campaignHistory;
    
    bool showMenu;
    bool showAbout;
    bool showHelp;
    bool showDifficultyMenu;
    
    MainMenuButtons mainBtns;
    ClassicButtons classicBtns;
    CampaignButtons campaignBtns;
} GameState;

// ========== ПРОТОТИПЫ ==========
void init_classic(GameState *state);
void init_campaign(GameState *state, int levelId);
void free_campaign(GameState *state);
void draw_button(const char *text, Rectangle rect, bool hover, bool active);
void draw_main_menu(GameState *state, Vector2 mouse);
void draw_classic_ui(GameState *state, Vector2 mouse);
void draw_campaign_ui(GameState *state, Vector2 mouse);
void draw_barriers(Barriers *b, int cellSize, int offsetX, int offsetY);
void handle_main_menu_input(GameState *state, Vector2 mouse);
void handle_classic_input(GameState *state, Vector2 mouse);
void handle_campaign_input(GameState *state, Vector2 mouse);
void auto_solve_campaign(GameState *state);
bool check_campaign_win(GameState *state);
void history_init(HistoryList *list, int width, int height);
void history_push(HistoryList *list, int **world, int generation);
void history_undo(HistoryList *list, int **world, int *generation);
void history_reset(HistoryList *list);
void history_free(HistoryList *list);
void log_event(const char *format, ...);
void save_progress(int level);
int load_progress();

// ========== ЛОГИРОВАНИЕ ==========
void log_event(const char *format, ...) {
    FILE *log = fopen("game.log", "a");
    if (log) {
        time_t now = time(NULL);
        char *timestr = ctime(&now);
        timestr[strlen(timestr)-1] = '\0';
        fprintf(log, "[%s] ", timestr);
        va_list args;
        va_start(args, format);
        vfprintf(log, format, args);
        va_end(args);
        fprintf(log, "\n");
        fclose(log);
    }
}

// ========== ПРОГРЕСС ==========
void save_progress(int level) {
    FILE *f = fopen("progress.dat", "w");
    if (f) { fprintf(f, "%d\n", level); fclose(f); log_event("Progress saved (level %d)", level); }
}
int load_progress() {
    int level = 1;
    FILE *f = fopen("progress.dat", "r");
    if (f) { fscanf(f, "%d", &level); fclose(f); }
    if (level < 1) level = 1;
    if (level > get_total_levels()) level = get_total_levels();
    log_event("Progress loaded (level %d)", level);
    return level;
}

// ========== ИСТОРИЯ ==========
void history_init(HistoryList *list, int width, int height) {
    list->head = list->current = NULL;
    list->width = width; list->height = height;
}
void history_push(HistoryList *list, int **world, int generation) {
    if (list->current && list->current->next) {
        HistoryNode *tmp = list->current->next;
        while (tmp) {
            HistoryNode *next = tmp->next;
            for (int i = 0; i < list->height; i++) free(tmp->world[i]);
            free(tmp->world); free(tmp); tmp = next;
        }
        list->current->next = NULL;
    }
    HistoryNode *node = malloc(sizeof(HistoryNode));
    node->world = malloc(list->height * sizeof(int*));
    for (int i = 0; i < list->height; i++) {
        node->world[i] = malloc(list->width * sizeof(int));
        memcpy(node->world[i], world[i], list->width * sizeof(int));
    }
    node->generation = generation;
    node->prev = list->current;
    node->next = NULL;
    if (!list->head) list->head = node;
    if (list->current) list->current->next = node;
    list->current = node;
}
void history_undo(HistoryList *list, int **world, int *generation) {
    printf("Undo called, current=%p, prev=%p\n", (void*)list->current, (void*)(list->current ? list->current->prev : NULL));
    if (list->current && list->current->prev) {
        list->current = list->current->prev;
        for (int i = 0; i < list->height; i++) {
            memcpy(world[i], list->current->world[i], list->width * sizeof(int));
        }
        *generation = list->current->generation;
        log_event("Undo performed");
    } else {
        log_event("Undo failed: no previous state");
    }
}
void history_reset(HistoryList *list) {
    HistoryNode *node = list->head;
    while (node) {
        HistoryNode *next = node->next;
        for (int i = 0; i < list->height; i++) free(node->world[i]);
        free(node->world); free(node); node = next;
    }
    list->head = list->current = NULL;
}
void history_free(HistoryList *list) { history_reset(list); }

// ========== ИНИЦИАЛИЗАЦИЯ ==========
void init_classic(GameState *state) {
    pattern_random(state->classicWorld, state->classicDensity);
    state->classicGeneration = 0;
    state->paused = true;
    state->speed = 5.0f;
    state->timeAccumulator = 0.0f;
    state->showGrid = true;
    int maxCellW = (GetScreenWidth() - PANEL_WIDTH) / WIDTH;
    int maxCellH = GetScreenHeight() / HEIGHT;
    state->cellSize = fmin(maxCellW, maxCellH);
    state->cellSize = fmax(MIN_CELL_SIZE, fmin(state->cellSize, MAX_CELL_SIZE));
    log_event("Classic mode initialized with density %d", state->classicDensity);
}

void init_campaign(GameState *state, int levelId) {
    if (state->currentLevel) free_level(state->currentLevel);
    state->currentLevel = load_level(levelId);
    if (!state->currentLevel) {
        log_event("Failed to load level %d", levelId);
        state->showMenu = true;
        return;
    }
    if (state->campaignWorld) {
        for (int i = 0; i < state->currentLevel->height; i++) free(state->campaignWorld[i]);
        free(state->campaignWorld);
    }
    int h = state->currentLevel->height, w = state->currentLevel->width;
    state->campaignWorld = malloc(h * sizeof(int*));
    for (int i = 0; i < h; i++) state->campaignWorld[i] = calloc(w, sizeof(int));
    state->campaignGeneration = 0;
    state->startCellsPlaced = 0;
    state->campaignPhase = CAMPAIGN_PHASE_PLACING;
    state->currentLevelId = levelId;
    int maxCellW = (GetScreenWidth() - PANEL_WIDTH) / w;
    int maxCellH = GetScreenHeight() / h;
    state->cellSize = fmin(maxCellW, maxCellH);
    state->cellSize = fmax(MIN_CELL_SIZE, fmin(state->cellSize, MAX_CELL_SIZE));
    history_init(&state->campaignHistory, w, h);
    history_push(&state->campaignHistory, state->campaignWorld, state->campaignGeneration);
    log_event("Campaign level %d started", levelId);
}

void free_campaign(GameState *state) {
    if (state->campaignWorld) {
        for (int i = 0; i < state->currentLevel->height; i++) free(state->campaignWorld[i]);
        free(state->campaignWorld); state->campaignWorld = NULL;
    }
    if (state->currentLevel) { free_level(state->currentLevel); state->currentLevel = NULL; }
    history_free(&state->campaignHistory);
}

// ========== ОТРИСОВКА КНОПКИ ==========
void draw_button(const char *text, Rectangle rect, bool hover, bool active) {
    Color col = active ? COLOR_BUTTON_ACTIVE : (hover ? COLOR_BUTTON_HOVER : COLOR_BUTTON);
    DrawRectangle(rect.x, rect.y, rect.width, rect.height, col);
    DrawRectangleLines(rect.x, rect.y, rect.width, rect.height, LIGHTGRAY);
    int tw = MeasureText(text, 16);
    DrawText(text, rect.x + (rect.width - tw)/2, rect.y + (rect.height - 16)/2, 16, WHITE);
}

// ========== ГЛАВНОЕ МЕНЮ ==========
void draw_main_menu(GameState *state, Vector2 mouse) {
    ClearBackground(BLACK);
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    DrawText("CONWAY'S GAME OF LIFE", sw/2 - 200, 80, 30, WHITE);
    
    state->mainBtns.classic  = (Rectangle){ sw/2 - 100, 180, 200, 50 };
    state->mainBtns.campaign = (Rectangle){ sw/2 - 100, 260, 200, 50 };
    state->mainBtns.about    = (Rectangle){ sw/2 - 100, 340, 200, 50 };
    state->mainBtns.help     = (Rectangle){ sw/2 - 100, 420, 200, 50 };
    state->mainBtns.exit     = (Rectangle){ sw/2 - 100, 500, 200, 50 };
    
    draw_button("Classic Mode", state->mainBtns.classic,  CheckCollisionPointRec(mouse, state->mainBtns.classic), false);
    draw_button("Campaign",    state->mainBtns.campaign, CheckCollisionPointRec(mouse, state->mainBtns.campaign), false);
    draw_button("About",       state->mainBtns.about,    CheckCollisionPointRec(mouse, state->mainBtns.about), false);
    draw_button("Help",        state->mainBtns.help,     CheckCollisionPointRec(mouse, state->mainBtns.help), false);
    draw_button("Exit",        state->mainBtns.exit,     CheckCollisionPointRec(mouse, state->mainBtns.exit), false);
    
    if (state->showAbout) {
        DrawRectangle(sw/2 - 250, sh/2 - 150, 500, 300, DARKGRAY);
        DrawRectangleLines(sw/2 - 250, sh/2 - 150, 500, 300, WHITE);
        DrawText("ABOUT", sw/2 - 40, sh/2 - 130, 24, YELLOW);
        DrawText("Game of Life - Conway's Game with Barriers", sw/2 - 220, sh/2 - 90, 16, WHITE);
        DrawText("Authors: Yuzhalkin P.A., Dubitsky D.A.", sw/2 - 220, sh/2 - 60, 16, WHITE);
        DrawText("Group: 5131001/50601, Year: 2026", sw/2 - 220, sh/2 - 30, 16, WHITE);
        DrawText("University: SPbPU, Institute: ICCS", sw/2 - 220, sh/2, 16, WHITE);
        DrawText("Department: Computer Science", sw/2 - 220, sh/2 + 30, 16, WHITE);
        DrawText("Click anywhere to close", sw/2 - 100, sh/2 + 90, 14, GRAY);
    }
    if (state->showHelp) {
        DrawRectangle(sw/2 - 300, sh/2 - 200, 600, 400, DARKGRAY);
        DrawRectangleLines(sw/2 - 300, sh/2 - 200, 600, 400, WHITE);
        DrawText("HELP", sw/2 - 30, sh/2 - 170, 24, YELLOW);
        DrawText("Classic mode:", sw/2 - 270, sh/2 - 130, 16, WHITE);
        DrawText("Space - Pause/Play", sw/2 - 260, sh/2 - 100, 14, GRAY);
        DrawText("Enter - Step (when paused)", sw/2 - 260, sh/2 - 80, 14, GRAY);
        DrawText("+/- - Speed", sw/2 - 260, sh/2 - 60, 14, GRAY);
        DrawText("G - Toggle grid", sw/2 - 260, sh/2 - 40, 14, GRAY);
        DrawText("F1-F4 - Patterns", sw/2 - 260, sh/2 - 20, 14, GRAY);
        DrawText("Left/Right mouse - draw/erase cells", sw/2 - 260, sh/2, 14, GRAY);
        DrawText("Campaign mode:", sw/2 - 270, sh/2 + 30, 16, WHITE);
        DrawText("Left click - place start cells", sw/2 - 260, sh/2 + 60, 14, GRAY);
        DrawText("Right click - remove start cells", sw/2 - 260, sh/2 + 80, 14, GRAY);
        DrawText("Start/Reset/AutoSolve - buttons on panel", sw/2 - 260, sh/2 + 100, 14, GRAY);
        DrawText("Undo - Ctrl+Z", sw/2 - 260, sh/2 + 120, 14, GRAY);
        DrawText("Click anywhere to close", sw/2 - 100, sh/2 + 180, 14, GRAY);
    }
}

void handle_main_menu_input(GameState *state, Vector2 mouse) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (state->showAbout || state->showHelp) {
            state->showAbout = false;
            state->showHelp = false;
            return;
        }
        if (CheckCollisionPointRec(mouse, state->mainBtns.classic)) {
            state->gameMode = GAME_MODE_CLASSIC;
            state->showDifficultyMenu = true;
            state->showMenu = false;
        }
        else if (CheckCollisionPointRec(mouse, state->mainBtns.campaign)) {
            state->gameMode = GAME_MODE_CAMPAIGN;
            int startLevel = load_progress();
            init_campaign(state, startLevel);
            state->showMenu = false;
        }
        else if (CheckCollisionPointRec(mouse, state->mainBtns.about)) state->showAbout = true;
        else if (CheckCollisionPointRec(mouse, state->mainBtns.help)) state->showHelp = true;
        else if (CheckCollisionPointRec(mouse, state->mainBtns.exit)) CloseWindow();
    }
}

// ========== КЛАССИЧЕСКИЙ РЕЖИМ ==========
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
    DrawText("F1-F4 - Patterns", panelX + 30, y, 14, GRAY); y += 20;
    DrawText("LMB/RMB - Draw/Erase", panelX + 30, y, 14, GRAY); y += 20;
    y += 10;
    
    state->classicBtns.random  = (Rectangle){ panelX + 20, y, 120, 30 };
    state->classicBtns.glider  = (Rectangle){ panelX + 150, y, 120, 30 };
    y += 40;
    state->classicBtns.blinker = (Rectangle){ panelX + 20, y, 120, 30 };
    state->classicBtns.clear   = (Rectangle){ panelX + 150, y, 120, 30 };
    y += 50;
    state->classicBtns.menu    = (Rectangle){ panelX + 20, y, 260, 40 };
    
    draw_button("Random",  state->classicBtns.random,  CheckCollisionPointRec(mouse, state->classicBtns.random), false);
    draw_button("Glider",  state->classicBtns.glider,  CheckCollisionPointRec(mouse, state->classicBtns.glider), false);
    draw_button("Blinker", state->classicBtns.blinker, CheckCollisionPointRec(mouse, state->classicBtns.blinker), false);
    draw_button("Clear",   state->classicBtns.clear,   CheckCollisionPointRec(mouse, state->classicBtns.clear), false);
    draw_button("Back to Menu", state->classicBtns.menu, CheckCollisionPointRec(mouse, state->classicBtns.menu), false);
}

void handle_classic_input(GameState *state, Vector2 mouse) {
    // Мышь для рисования на поле
    int gameW = WIDTH * state->cellSize;
    int gameH = HEIGHT * state->cellSize;
    if (mouse.x >= 0 && mouse.x < gameW && mouse.y >= 0 && mouse.y < gameH) {
        int x = mouse.x / state->cellSize;
        int y = mouse.y / state->cellSize;
        if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                state->classicWorld[y][x] = 1;
                // не сохраняем в историю для классики (просто рисуем)
            }
            if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
                state->classicWorld[y][x] = 0;
            }
        }
    }
    
    // Кнопки
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mouse, state->classicBtns.random)) {
            pattern_random(state->classicWorld, state->classicDensity);
            state->classicGeneration = 0;
            log_event("Random pattern in classic mode");
        }
        else if (CheckCollisionPointRec(mouse, state->classicBtns.glider)) {
            memset(state->classicWorld, 0, sizeof(state->classicWorld));
            pattern_glider(state->classicWorld, HEIGHT/2, WIDTH/2);
            state->classicGeneration = 0;
            log_event("Glider pattern");
        }
        else if (CheckCollisionPointRec(mouse, state->classicBtns.blinker)) {
            memset(state->classicWorld, 0, sizeof(state->classicWorld));
            pattern_blinker(state->classicWorld, HEIGHT/2, WIDTH/2-1);
            state->classicGeneration = 0;
            log_event("Blinker pattern");
        }
        else if (CheckCollisionPointRec(mouse, state->classicBtns.clear)) {
            memset(state->classicWorld, 0, sizeof(state->classicWorld));
            state->classicGeneration = 0;
            log_event("Cleared world");
        }
        else if (CheckCollisionPointRec(mouse, state->classicBtns.menu)) {
            state->showMenu = true;
            log_event("Return to menu from classic");
        }
    }
    // Клавиатура
    if (IsKeyPressed(KEY_SPACE)) state->paused = !state->paused;
    if (IsKeyPressed(KEY_ENTER) && state->paused) {
        next_generation(state->classicWorld);
        state->classicGeneration++;
    }
    if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) state->speed = fmin(30, state->speed+1);
    if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) state->speed = fmax(1, state->speed-1);
    if (IsKeyPressed(KEY_G)) state->showGrid = !state->showGrid;
    if (IsKeyPressed(KEY_C)) {
        memset(state->classicWorld, 0, sizeof(state->classicWorld));
        state->classicGeneration = 0;
    }
    if (IsKeyPressed(KEY_R)) pattern_random(state->classicWorld, state->classicDensity);
    if (IsKeyPressed(KEY_F1)) pattern_random(state->classicWorld, state->classicDensity);
    if (IsKeyPressed(KEY_F2)) {
        memset(state->classicWorld, 0, sizeof(state->classicWorld));
        pattern_glider(state->classicWorld, HEIGHT/2, WIDTH/2);
        state->classicGeneration = 0;
    }
    if (IsKeyPressed(KEY_F3)) {
        memset(state->classicWorld, 0, sizeof(state->classicWorld));
        pattern_blinker(state->classicWorld, HEIGHT/2, WIDTH/2-1);
        state->classicGeneration = 0;
    }
}

// ========== КАМПАНИЯ ==========
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
        state->campaignBtns.start     = (Rectangle){ panelX + 20, y, 120, 35 };
        state->campaignBtns.reset     = (Rectangle){ panelX + 150, y, 120, 35 };
        y += 45;
        state->campaignBtns.autoSolve = (Rectangle){ panelX + 20, y, 260, 35 };
        y += 50;
        state->campaignBtns.menu      = (Rectangle){ panelX + 20, y, 260, 40 };
        
        draw_button("Start",     state->campaignBtns.start,     CheckCollisionPointRec(mouse, state->campaignBtns.start), false);
        draw_button("Reset",     state->campaignBtns.reset,     CheckCollisionPointRec(mouse, state->campaignBtns.reset), false);
        draw_button("Auto Solve",state->campaignBtns.autoSolve, CheckCollisionPointRec(mouse, state->campaignBtns.autoSolve), false);
        draw_button("Back to Menu", state->campaignBtns.menu,   CheckCollisionPointRec(mouse, state->campaignBtns.menu), false);
    }
    else if (state->campaignPhase == CAMPAIGN_PHASE_SIMULATING) {
        DrawText("PHASE: SIMULATING", panelX + 20, y, 16, WHITE); y += 30;
        state->campaignBtns.pause    = (Rectangle){ panelX + 20, y, 120, 35 };
        y += 45;
        state->campaignBtns.menuSim  = (Rectangle){ panelX + 20, y, 260, 40 };
        draw_button(state->paused ? "Run" : "Pause", state->campaignBtns.pause, CheckCollisionPointRec(mouse, state->campaignBtns.pause), false);
        draw_button("Back to Menu", state->campaignBtns.menuSim, CheckCollisionPointRec(mouse, state->campaignBtns.menuSim), false);
    }
    else if (state->campaignPhase == CAMPAIGN_PHASE_COMPLETE) {
        DrawText("LEVEL COMPLETE!", panelX + 20, y, 20, GREEN); y += 40;
        state->campaignBtns.next      = (Rectangle){ panelX + 20, y, 120, 35 };
        state->campaignBtns.menuComplete = (Rectangle){ panelX + 150, y, 120, 35 };
        draw_button("Next Level", state->campaignBtns.next, CheckCollisionPointRec(mouse, state->campaignBtns.next), false);
        draw_button("Menu", state->campaignBtns.menuComplete, CheckCollisionPointRec(mouse, state->campaignBtns.menuComplete), false);
    }
    else if (state->campaignPhase == CAMPAIGN_PHASE_FAILED) {
        DrawText("FAILED! Try again.", panelX + 20, y, 20, RED); y += 40;
        state->campaignBtns.retry     = (Rectangle){ panelX + 20, y, 120, 35 };
        state->campaignBtns.menuFailed = (Rectangle){ panelX + 150, y, 120, 35 };
        draw_button("Retry", state->campaignBtns.retry, CheckCollisionPointRec(mouse, state->campaignBtns.retry), false);
        draw_button("Menu", state->campaignBtns.menuFailed, CheckCollisionPointRec(mouse, state->campaignBtns.menuFailed), false);
    }
}

void handle_campaign_input(GameState *state, Vector2 mouse) {
    if (!state->currentLevel) return;
    int w = state->currentLevel->width, h = state->currentLevel->height;
    int panelX = w * state->cellSize;
    
    // Рисование мышью (только в фазе размещения)
    if (state->campaignPhase == CAMPAIGN_PHASE_PLACING) {
        int gameW = w * state->cellSize;
        int gameH = h * state->cellSize;
        if (mouse.x >= 0 && mouse.x < gameW && mouse.y >= 0 && mouse.y < gameH) {
            int x = mouse.x / state->cellSize;
            int y = mouse.y / state->cellSize;
            if (x >= 0 && x < w && y >= 0 && y < h) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && state->campaignWorld[y][x] == 0 && state->startCellsPlaced < state->currentLevel->max_start_cells) {
                    state->campaignWorld[y][x] = 1;
                    state->startCellsPlaced++;
                    history_push(&state->campaignHistory, state->campaignWorld, state->campaignGeneration);
                }
                if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && state->campaignWorld[y][x] == 1) {
                    state->campaignWorld[y][x] = 0;
                    state->startCellsPlaced--;
                    history_push(&state->campaignHistory, state->campaignWorld, state->campaignGeneration);
                }
            }
        }
    }
    
    // Кнопки
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (state->campaignPhase == CAMPAIGN_PHASE_PLACING) {
            if (CheckCollisionPointRec(mouse, state->campaignBtns.start) && state->startCellsPlaced > 0) {
                state->campaignPhase = CAMPAIGN_PHASE_SIMULATING;
                state->paused = false;
                log_event("Campaign simulation started");
            }
            else if (CheckCollisionPointRec(mouse, state->campaignBtns.reset)) {
                for (int i=0;i<h;i++) for (int j=0;j<w;j++) state->campaignWorld[i][j] = 0;
                state->startCellsPlaced = 0;
                state->campaignGeneration = 0;
                history_reset(&state->campaignHistory);
                history_push(&state->campaignHistory, state->campaignWorld, state->campaignGeneration);
                log_event("Campaign level reset");
            }
            else if (CheckCollisionPointRec(mouse, state->campaignBtns.autoSolve)) {
                auto_solve_campaign(state);
                log_event("Auto solve used");
            }
            else if (CheckCollisionPointRec(mouse, state->campaignBtns.menu)) {
                free_campaign(state);
                state->showMenu = true;
                log_event("Return to menu from campaign");
            }
        }
        else if (state->campaignPhase == CAMPAIGN_PHASE_SIMULATING) {
            if (CheckCollisionPointRec(mouse, state->campaignBtns.pause)) state->paused = !state->paused;
            else if (CheckCollisionPointRec(mouse, state->campaignBtns.menuSim)) {
                free_campaign(state);
                state->showMenu = true;
                log_event("Return to menu during simulation");
            }
        }
        else if (state->campaignPhase == CAMPAIGN_PHASE_COMPLETE) {
            if (CheckCollisionPointRec(mouse, state->campaignBtns.next)) {
                int nextLevel = state->currentLevelId + 1;
                if (nextLevel <= get_total_levels()) {
                    save_progress(nextLevel);
                    init_campaign(state, nextLevel);
                    log_event("Advanced to next level");
                } else {
                    log_event("Campaign completed");
                    state->showMenu = true;
                }
            }
            else if (CheckCollisionPointRec(mouse, state->campaignBtns.menuComplete)) {
                free_campaign(state);
                state->showMenu = true;
            }
        }
        else if (state->campaignPhase == CAMPAIGN_PHASE_FAILED) {
            if (CheckCollisionPointRec(mouse, state->campaignBtns.retry)) {
                init_campaign(state, state->currentLevelId);
                log_event("Retry level");
            }
            else if (CheckCollisionPointRec(mouse, state->campaignBtns.menuFailed)) {
                free_campaign(state);
                state->showMenu = true;
            }
        }
    }
    
    // Клавиатура для симуляции
    if (state->campaignPhase == CAMPAIGN_PHASE_SIMULATING) {
        if (IsKeyPressed(KEY_SPACE)) state->paused = !state->paused;
        if (IsKeyPressed(KEY_ENTER) && state->paused) {
            next_generation_with_barriers(state->campaignWorld, state->currentLevel->barriers, w, h, &state->campaignGeneration);
            history_push(&state->campaignHistory, state->campaignWorld, state->campaignGeneration);
        }
        if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) state->speed = fmin(30, state->speed+1);
        if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) state->speed = fmax(1, state->speed-1);
        if (IsKeyPressed(KEY_G)) state->showGrid = !state->showGrid;
        if (IsKeyPressed(KEY_Z) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))) {
            history_undo(&state->campaignHistory, state->campaignWorld, &state->campaignGeneration);
        }
    }
}

void auto_solve_campaign(GameState *state) {
    if (state->campaignPhase != CAMPAIGN_PHASE_PLACING) return;
    int w = state->currentLevel->width, h = state->currentLevel->height;
    int maxCells = state->currentLevel->max_start_cells;
    for (int i=0;i<h;i++) for (int j=0;j<w;j++) state->campaignWorld[i][j] = 0;
    state->startCellsPlaced = 0;
    int attempts = 0;
    while (state->startCellsPlaced < maxCells && attempts < 1000) {
        int x = rand() % w, y = rand() % h;
        if (state->campaignWorld[y][x] == 0) {
            state->campaignWorld[y][x] = 1;
            state->startCellsPlaced++;
        }
        attempts++;
    }
    history_reset(&state->campaignHistory);
    history_push(&state->campaignHistory, state->campaignWorld, state->campaignGeneration);
}

bool check_campaign_win(GameState *state) {
    int w = state->currentLevel->width, h = state->currentLevel->height;
    for (int y=0; y<h; y++) for (int x=0; x<w; x++) if (state->campaignWorld[y][x] == 0) return false;
    return true;
}

void draw_barriers(Barriers *b, int cellSize, int offsetX, int offsetY) {
    if (!b) return;
    for (int y = 0; y < b->height; y++) {
        for (int x = 0; x < b->width - 1; x++) {
            if (has_h_barrier(b, y, x))
                DrawLine(offsetX + (x+1)*cellSize, offsetY + y*cellSize + cellSize/2,
                         offsetX + (x+1)*cellSize, offsetY + y*cellSize + cellSize/2, COLOR_BARRIER);
        }
    }
    for (int y = 0; y < b->height - 1; y++) {
        for (int x = 0; x < b->width; x++) {
            if (has_v_barrier(b, y, x))
                DrawLine(offsetX + x*cellSize + cellSize/2, offsetY + (y+1)*cellSize,
                         offsetX + x*cellSize + cellSize/2, offsetY + (y+1)*cellSize, COLOR_BARRIER);
        }
    }
}

// ========== MAIN ==========
int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0) {
            printf("Game of Life - Conway's Game with barriers and campaign\n");
            printf("Usage: game.exe [--help] [--version]\n");
            return 0;
        }
        if (strcmp(argv[1], "--version") == 0) {
            printf("Game of Life v2.0 (c) 2026\n");
            printf("Authors: Yuzhalkin P.A., Dubitsky D.A.\n");
            return 0;
        }
    }
    srand(time(NULL));
    int screenWidth = 1300, screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Game of Life - Adventure");
    SetTargetFPS(60);
    
    GameState state = {0};
    state.showMenu = true;
    state.classicDensity = 25;
    
    log_event("Program started");
    
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        Vector2 mouse = GetMousePosition();
        
        BeginDrawing();
        ClearBackground(BLACK);
        
        if (state.showMenu) {
            draw_main_menu(&state, mouse);
            handle_main_menu_input(&state, mouse);
        } else if (state.showDifficultyMenu) {
            int sw = GetScreenWidth(), sh = GetScreenHeight();
            DrawRectangle(sw/2 - 200, sh/2 - 100, 400, 200, DARKGRAY);
            DrawRectangleLines(sw/2 - 200, sh/2 - 100, 400, 200, WHITE);
            DrawText("Select Difficulty", sw/2 - 100, sh/2 - 70, 20, YELLOW);
            Rectangle easy   = { sw/2 - 150, sh/2 - 30, 100, 40 };
            Rectangle medium = { sw/2 - 50,  sh/2 - 30, 100, 40 };
            Rectangle hard   = { sw/2 + 50,  sh/2 - 30, 100, 40 };
            bool easyH = CheckCollisionPointRec(mouse, easy);
            bool medH  = CheckCollisionPointRec(mouse, medium);
            bool hardH = CheckCollisionPointRec(mouse, hard);
            draw_button("15%", easy,   easyH, state.classicDensity==15);
            draw_button("25%", medium, medH,  state.classicDensity==25);
            draw_button("40%", hard,   hardH, state.classicDensity==40);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (easyH) state.classicDensity = 15;
                else if (medH) state.classicDensity = 25;
                else if (hardH) state.classicDensity = 40;
                if (easyH || medH || hardH) {
                    init_classic(&state);
                    state.showDifficultyMenu = false;
                    state.showMenu = false;
                }
            }
        } else {
            if (state.gameMode == GAME_MODE_CLASSIC) {
                if (!state.paused) {
                    state.timeAccumulator += dt;
                    float step = 1.0f / state.speed;
                    while (state.timeAccumulator >= step) {
                        next_generation(state.classicWorld);
                        state.classicGeneration++;
                        state.timeAccumulator -= step;
                    }
                }
                int gameW = WIDTH * state.cellSize, gameH = HEIGHT * state.cellSize;
                for (int y=0; y<HEIGHT; y++)
                    for (int x=0; x<WIDTH; x++)
                        DrawRectangle(x*state.cellSize, y*state.cellSize, state.cellSize, state.cellSize,
                                      state.classicWorld[y][x] ? COLOR_ALIVE : COLOR_DEAD);
                if (state.showGrid) {
                    for (int i=0;i<=WIDTH;i++) DrawLine(i*state.cellSize, 0, i*state.cellSize, gameH, COLOR_GRID);
                    for (int i=0;i<=HEIGHT;i++) DrawLine(0, i*state.cellSize, gameW, i*state.cellSize, COLOR_GRID);
                }
                draw_classic_ui(&state, mouse);
                handle_classic_input(&state, mouse);
            } else if (state.gameMode == GAME_MODE_CAMPAIGN && state.currentLevel) {
                int w = state.currentLevel->width, h = state.currentLevel->height;
                int gameW = w * state.cellSize, gameH = h * state.cellSize;
                for (int y=0; y<h; y++)
                    for (int x=0; x<w; x++)
                        DrawRectangle(x*state.cellSize, y*state.cellSize, state.cellSize, state.cellSize,
                                      state.campaignWorld[y][x] ? COLOR_ALIVE : COLOR_DEAD);
                if (state.showGrid) {
                    for (int i=0;i<=w;i++) DrawLine(i*state.cellSize, 0, i*state.cellSize, gameH, COLOR_GRID);
                    for (int i=0;i<=h;i++) DrawLine(0, i*state.cellSize, gameW, i*state.cellSize, COLOR_GRID);
                }
                draw_barriers(state.currentLevel->barriers, state.cellSize, 0, 0);
                draw_campaign_ui(&state, mouse);
                handle_campaign_input(&state, mouse);
                
                if (state.campaignPhase == CAMPAIGN_PHASE_SIMULATING && !state.paused) {
                    state.timeAccumulator += dt;
                    float step = 1.0f / state.speed;
                    while (state.timeAccumulator >= step) {
                        next_generation_with_barriers(state.campaignWorld, state.currentLevel->barriers, w, h, &state.campaignGeneration);
                        history_push(&state.campaignHistory, state.campaignWorld, state.campaignGeneration);
                        state.timeAccumulator -= step;
                        if (check_campaign_win(&state)) {
                            state.campaignPhase = CAMPAIGN_PHASE_COMPLETE;
                            state.paused = true;
                            int next = state.currentLevelId + 1;
                            if (next <= get_total_levels()) save_progress(next);
                            log_event("Level %d completed", state.currentLevelId);
                            break;
                        }
                        if (state.campaignGeneration >= state.currentLevel->target_generations) {
                            state.campaignPhase = CAMPAIGN_PHASE_FAILED;
                            state.paused = true;
                            log_event("Level %d failed", state.currentLevelId);
                            break;
                        }
                    }
                }
            }
        }
        EndDrawing();
    }
    
    free_campaign(&state);
    CloseWindow();
    log_event("Program closed");
    return 0;
}