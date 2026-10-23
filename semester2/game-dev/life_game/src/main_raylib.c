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

#define PANEL_WIDTH 300
#define MIN_CELL_SIZE 5
#define MAX_CELL_SIZE 35

#define COLOR_DEAD   (Color){ 40, 40, 40, 255 }
#define COLOR_ALIVE  (Color){ 100, 200, 100, 255 }
#define COLOR_GRID   (Color){ 60, 60, 60, 255 }
#define COLOR_PANEL  (Color){ 30, 30, 40, 255 }
#define COLOR_BUTTON (Color){ 60, 60, 80, 255 }
#define COLOR_BUTTON_HOVER (Color){ 90, 90, 120, 255 }
#define COLOR_BUTTON_ACTIVE (Color){ 120, 120, 160, 255 }
#define COLOR_BARRIER (Color){ 255, 80, 80, 255 }

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

typedef struct {
    Rectangle classic, campaign, about, help, exit;
} MainMenuButtons;

typedef struct {
    Rectangle random, glider, blinker, clear, menu;
} ClassicButtons;

typedef struct {
    Rectangle prevLevel, nextLevel;
    Rectangle start, reset, autoSolve, menu;
    Rectangle pause, stepSim, resetSim, menuSim;
    Rectangle next, retryComplete, menuComplete;
    Rectangle retry, autoSolveFailed, menuFailed;
} CampaignButtons;

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
    int campaignWorldHeight;
    int campaignGeneration;
    int startCellsPlaced;
    CampaignPhase campaignPhase;
    int currentLevelId;
    int autoSolveCursor;

    HistoryList campaignHistory;

    bool showMenu;
    bool showAbout;
    bool showHelp;
    bool showDifficultyMenu;

    MainMenuButtons mainBtns;
    ClassicButtons classicBtns;
    CampaignButtons campaignBtns;
} GameState;

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
void step_campaign_simulation(GameState *state);
void history_init(HistoryList *list, int width, int height);
void history_push(HistoryList *list, int **world, int generation);
void history_undo(HistoryList *list, int **world, int *generation);
void history_reset(HistoryList *list);
void history_free(HistoryList *list);
void log_event(const char *format, ...);
void save_progress(int level);
int load_progress();

void log_event(const char *format, ...) {
    FILE *log = fopen("game.log", "a");
    if (log) {
        time_t now = time(NULL);
        char *timestr = ctime(&now);
        if (timestr && strlen(timestr) > 0)
            timestr[strlen(timestr) - 1] = '\0';
        fprintf(log, "[%s] ", timestr ? timestr : "time");
        va_list args;
        va_start(args, format);
        vfprintf(log, format, args);
        va_end(args);
        fprintf(log, "\n");
        fclose(log);
    }
}

void save_progress(int level) {
    FILE *f = fopen("progress.dat", "w");
    if (f) {
        fprintf(f, "%d\n", level);
        fclose(f);
        log_event("Progress saved (level %d)", level);
    }
}

int load_progress() {
    int level = 1;
    FILE *f = fopen("progress.dat", "r");
    if (f) {
        if (fscanf(f, "%d", &level) != 1) level = 1;
        fclose(f);
    }
    if (level < 1 || level > get_total_levels()) level = 1;
    log_event("Progress loaded (level %d)", level);
    return level;
}

void history_init(HistoryList *list, int width, int height) {
    list->head = list->current = NULL;
    list->width = width;
    list->height = height;
}

void history_push(HistoryList *list, int **world, int generation) {
    if (list->current && list->current->next) {
        HistoryNode *tmp = list->current->next;
        while (tmp) {
            HistoryNode *next = tmp->next;
            for (int i = 0; i < list->height; i++) free(tmp->world[i]);
            free(tmp->world);
            free(tmp);
            tmp = next;
        }
        list->current->next = NULL;
    }
    HistoryNode *node = malloc(sizeof(HistoryNode));
    if (!node) return;
    node->world = malloc(list->height * sizeof(int*));
    if (!node->world) { free(node); return; }
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
        free(node->world);
        free(node);
        node = next;
    }
    list->head = list->current = NULL;
}

void history_free(HistoryList *list) {
    history_reset(list);
}

static int count_live_cells(int **world, int w, int h) {
    int live = 0;
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            live += world[y][x];
    return live;
}

void init_classic(GameState *state) {
    pattern_random(state->classicWorld, state->classicDensity);
    state->classicGeneration = 0;
    state->paused = true;
    if (state->speed < 1.0f) state->speed = 5.0f;
    state->timeAccumulator = 0.0f;
    state->showGrid = true;
    int maxCellW = (GetScreenWidth() - PANEL_WIDTH) / WIDTH;
    int maxCellH = GetScreenHeight() / HEIGHT;
    state->cellSize = (int)fmin(maxCellW, maxCellH);
    state->cellSize = (int)fmax(MIN_CELL_SIZE, fmin(state->cellSize, MAX_CELL_SIZE));
    log_event("Classic mode initialized with density %d", state->classicDensity);
}

void init_campaign(GameState *state, int levelId) {
    if (levelId < 1) levelId = 1;
    if (levelId > get_total_levels()) levelId = get_total_levels();

    Level *newLevel = load_level(levelId);
    if (!newLevel) {
        log_event("Failed to load level %d", levelId);
        return;
    }

    if (state->campaignWorld) {
        for (int i = 0; i < state->campaignWorldHeight; i++)
            free(state->campaignWorld[i]);
        free(state->campaignWorld);
        state->campaignWorld = NULL;
        state->campaignWorldHeight = 0;
    }

    if (state->currentLevel) {
        free_level(state->currentLevel);
        state->currentLevel = NULL;
    }

    history_free(&state->campaignHistory);

    state->currentLevel = newLevel;
    int h = newLevel->height;
    int w = newLevel->width;

    state->campaignWorld = (int **)calloc((size_t)h, sizeof(int *));
    if (!state->campaignWorld) {
        free_level(state->currentLevel);
        state->currentLevel = NULL;
        log_event("Failed to allocate campaign world pointers");
        return;
    }

    for (int i = 0; i < h; i++) {
        state->campaignWorld[i] = (int *)calloc((size_t)w, sizeof(int));
        if (!state->campaignWorld[i]) {
            for (int j = 0; j < i; j++)
                free(state->campaignWorld[j]);
            free(state->campaignWorld);
            state->campaignWorld = NULL;
            free_level(state->currentLevel);
            state->currentLevel = NULL;
            log_event("Failed to allocate campaign row %d", i);
            return;
        }
    }

    state->campaignWorldHeight = h;
    state->campaignGeneration = 0;
    state->startCellsPlaced = 0;
    state->campaignPhase = CAMPAIGN_PHASE_PLACING;
    state->currentLevelId = levelId;
    state->paused = true;
    if (state->speed < 1.0f) state->speed = 5.0f;
    state->showGrid = true;
    state->timeAccumulator = 0.0f;
    state->autoSolveCursor = 0;

    int maxCellW = (GetScreenWidth() - PANEL_WIDTH) / w;
    int maxCellH = GetScreenHeight() / h;
    state->cellSize = (int)fmin(maxCellW, maxCellH);
    state->cellSize = (int)fmax(MIN_CELL_SIZE, fmin(state->cellSize, MAX_CELL_SIZE));

    history_init(&state->campaignHistory, w, h);
    history_push(&state->campaignHistory, state->campaignWorld, state->campaignGeneration);
    log_event("Campaign level %d started", levelId);
}

void free_campaign(GameState *state) {
    if (!state) return;

    if (state->campaignWorld) {
        for (int i = 0; i < state->campaignWorldHeight; i++)
            free(state->campaignWorld[i]);
        free(state->campaignWorld);
        state->campaignWorld = NULL;
    }
    state->campaignWorldHeight = 0;

    if (state->currentLevel) {
        free_level(state->currentLevel);
        state->currentLevel = NULL;
    }

    history_free(&state->campaignHistory);
}

void draw_button(const char *text, Rectangle rect, bool hover, bool active) {
    Color col = active ? COLOR_BUTTON_ACTIVE : (hover ? COLOR_BUTTON_HOVER : COLOR_BUTTON);
    DrawRectangle((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, col);
    DrawRectangleLines((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, LIGHTGRAY);
    int tw = MeasureText(text, 16);
    DrawText(text, (int)(rect.x + (rect.width - tw) / 2), (int)(rect.y + (rect.height - 16) / 2), 16, WHITE);
}

void draw_main_menu(GameState *state, Vector2 mouse) {
    ClearBackground(BLACK);
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    DrawText("CONWAY'S GAME OF LIFE", sw / 2 - 200, 80, 30, WHITE);

    state->mainBtns.classic  = (Rectangle){ sw / 2 - 100, 180, 200, 50 };
    state->mainBtns.campaign = (Rectangle){ sw / 2 - 100, 260, 200, 50 };
    state->mainBtns.about    = (Rectangle){ sw / 2 - 100, 340, 200, 50 };
    state->mainBtns.help     = (Rectangle){ sw / 2 - 100, 420, 200, 50 };
    state->mainBtns.exit     = (Rectangle){ sw / 2 - 100, 500, 200, 50 };

    draw_button("Classic Mode", state->mainBtns.classic,  CheckCollisionPointRec(mouse, state->mainBtns.classic), false);
    draw_button("Campaign",     state->mainBtns.campaign, CheckCollisionPointRec(mouse, state->mainBtns.campaign), false);
    draw_button("About",        state->mainBtns.about,    CheckCollisionPointRec(mouse, state->mainBtns.about), false);
    draw_button("Help",         state->mainBtns.help,     CheckCollisionPointRec(mouse, state->mainBtns.help), false);
    draw_button("Exit",         state->mainBtns.exit,     CheckCollisionPointRec(mouse, state->mainBtns.exit), false);

    if (state->showAbout) {
        DrawRectangle(sw / 2 - 250, sh / 2 - 150, 500, 300, DARKGRAY);
        DrawRectangleLines(sw / 2 - 250, sh / 2 - 150, 500, 300, WHITE);
        DrawText("ABOUT", sw / 2 - 40, sh / 2 - 130, 24, YELLOW);
        DrawText("Game of Life - Conway's Game with Barriers", sw / 2 - 220, sh / 2 - 90, 16, WHITE);
        DrawText("Authors: Yuzhalkin P.A., Dubitsky D.A.", sw / 2 - 220, sh / 2 - 60, 16, WHITE);
        DrawText("Group: 5131001/50601, Year: 2026", sw / 2 - 220, sh / 2 - 30, 16, WHITE);
        DrawText("University: SPbPU, Institute: ICCS", sw / 2 - 220, sh / 2, 16, WHITE);
        DrawText("Department: Computer Science", sw / 2 - 220, sh / 2 + 30, 16, WHITE);
        DrawText("Click anywhere to close", sw / 2 - 100, sh / 2 + 90, 14, GRAY);
    }
    if (state->showHelp) {
        DrawRectangle(sw / 2 - 300, sh / 2 - 200, 600, 400, DARKGRAY);
        DrawRectangleLines(sw / 2 - 300, sh / 2 - 200, 600, 400, WHITE);
        DrawText("HELP", sw / 2 - 30, sh / 2 - 170, 24, YELLOW);
        DrawText("Classic mode:", sw / 2 - 270, sh / 2 - 130, 16, WHITE);
        DrawText("Space - Pause/Play", sw / 2 - 260, sh / 2 - 100, 14, GRAY);
        DrawText("Enter - Step (when paused)", sw / 2 - 260, sh / 2 - 80, 14, GRAY);
        DrawText("+/- - Speed", sw / 2 - 260, sh / 2 - 60, 14, GRAY);
        DrawText("G - Toggle grid", sw / 2 - 260, sh / 2 - 40, 14, GRAY);
        DrawText("F1-F3 - Patterns", sw / 2 - 260, sh / 2 - 20, 14, GRAY);
        DrawText("Left/Right mouse - draw/erase cells", sw / 2 - 260, sh / 2, 14, GRAY);
        DrawText("Campaign mode:", sw / 2 - 270, sh / 2 + 30, 16, WHITE);
        DrawText("Left/Right click - place/remove start cells", sw / 2 - 260, sh / 2 + 60, 14, GRAY);
        DrawText("Auto Solve - cycle valid solutions", sw / 2 - 260, sh / 2 + 80, 14, GRAY);
        DrawText("Goal: keep >= 4 cells alive until target generation", sw / 2 - 260, sh / 2 + 100, 14, GRAY);
        DrawText("Undo - Ctrl+Z", sw / 2 - 260, sh / 2 + 120, 14, GRAY);
        DrawText("Click anywhere to close", sw / 2 - 100, sh / 2 + 180, 14, GRAY);
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
        } else if (CheckCollisionPointRec(mouse, state->mainBtns.campaign)) {
            state->gameMode = GAME_MODE_CAMPAIGN;
            int startLevel = load_progress();
            init_campaign(state, startLevel);
            state->showMenu = false;
        } else if (CheckCollisionPointRec(mouse, state->mainBtns.about)) {
            state->showAbout = true;
        } else if (CheckCollisionPointRec(mouse, state->mainBtns.help)) {
            state->showHelp = true;
        } else if (CheckCollisionPointRec(mouse, state->mainBtns.exit)) {
            CloseWindow();
        }
    }
}

void draw_classic_ui(GameState *state, Vector2 mouse) {
    int panelX = WIDTH * state->cellSize;
    DrawRectangle(panelX, 0, GetScreenWidth() - panelX, GetScreenHeight(), COLOR_PANEL);
    DrawLine(panelX, 0, panelX, GetScreenHeight(), LIGHTGRAY);

    int y = 20;
    DrawText("CLASSIC MODE", panelX + 20, y, 20, WHITE); y += 35;
    DrawText(TextFormat("Gen: %d", state->classicGeneration), panelX + 20, y, 16, YELLOW); y += 25;
    int live = 0;
    for (int i = 0; i < HEIGHT; i++)
        for (int j = 0; j < WIDTH; j++)
            live += state->classicWorld[i][j];
    DrawText(TextFormat("Live: %d", live), panelX + 20, y, 16, YELLOW); y += 25;
    DrawText(TextFormat("Speed: %.1f", state->speed), panelX + 20, y, 16, YELLOW); y += 40;

    DrawText("CONTROLS:", panelX + 20, y, 16, WHITE); y += 25;
    DrawText("Space - Pause", panelX + 30, y, 14, GRAY); y += 20;
    DrawText("Enter - Step", panelX + 30, y, 14, GRAY); y += 20;
    DrawText("+/- - Speed", panelX + 30, y, 14, GRAY); y += 20;
    DrawText("G - Grid", panelX + 30, y, 14, GRAY); y += 20;
    DrawText("F1-F3 - Patterns", panelX + 30, y, 14, GRAY); y += 20;
    DrawText("LMB/RMB - Draw/Erase", panelX + 30, y, 14, GRAY); y += 30;

    state->classicBtns.random  = (Rectangle){ panelX + 20, y, 120, 30 };
    state->classicBtns.glider  = (Rectangle){ panelX + 150, y, 120, 30 };
    y += 40;
    state->classicBtns.blinker = (Rectangle){ panelX + 20, y, 120, 30 };
    state->classicBtns.clear   = (Rectangle){ panelX + 150, y, 120, 30 };
    y += 50;
    state->classicBtns.menu    = (Rectangle){ panelX + 20, y, 250, 40 };

    draw_button("Random",       state->classicBtns.random,  CheckCollisionPointRec(mouse, state->classicBtns.random), false);
    draw_button("Glider",       state->classicBtns.glider,  CheckCollisionPointRec(mouse, state->classicBtns.glider), false);
    draw_button("Blinker",      state->classicBtns.blinker, CheckCollisionPointRec(mouse, state->classicBtns.blinker), false);
    draw_button("Clear",        state->classicBtns.clear,   CheckCollisionPointRec(mouse, state->classicBtns.clear), false);
    draw_button("Back to Menu", state->classicBtns.menu,    CheckCollisionPointRec(mouse, state->classicBtns.menu), false);
}

void handle_classic_input(GameState *state, Vector2 mouse) {
    int gameW = WIDTH * state->cellSize;
    int gameH = HEIGHT * state->cellSize;
    if (mouse.x >= 0 && mouse.x < gameW && mouse.y >= 0 && mouse.y < gameH) {
        int x = (int)(mouse.x / state->cellSize);
        int y = (int)(mouse.y / state->cellSize);
        if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state->classicWorld[y][x] = 1;
            if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) state->classicWorld[y][x] = 0;
        }
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mouse, state->classicBtns.random)) {
            pattern_random(state->classicWorld, state->classicDensity);
            state->classicGeneration = 0;
            log_event("Random pattern in classic mode");
        } else if (CheckCollisionPointRec(mouse, state->classicBtns.glider)) {
            memset(state->classicWorld, 0, sizeof(state->classicWorld));
            pattern_glider(state->classicWorld, HEIGHT / 2, WIDTH / 2);
            state->classicGeneration = 0;
            log_event("Glider pattern");
        } else if (CheckCollisionPointRec(mouse, state->classicBtns.blinker)) {
            memset(state->classicWorld, 0, sizeof(state->classicWorld));
            pattern_blinker(state->classicWorld, HEIGHT / 2, WIDTH / 2 - 1);
            state->classicGeneration = 0;
            log_event("Blinker pattern");
        } else if (CheckCollisionPointRec(mouse, state->classicBtns.clear)) {
            memset(state->classicWorld, 0, sizeof(state->classicWorld));
            state->classicGeneration = 0;
            log_event("Cleared world");
        } else if (CheckCollisionPointRec(mouse, state->classicBtns.menu)) {
            state->showMenu = true;
            log_event("Return to menu from classic");
        }
    }

    if (IsKeyPressed(KEY_SPACE)) state->paused = !state->paused;
    if (IsKeyPressed(KEY_ENTER) && state->paused) {
        next_generation(state->classicWorld);
        state->classicGeneration++;
    }
    if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) state->speed = fminf(30.0f, state->speed + 1.0f);
    if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) state->speed = fmaxf(1.0f, state->speed - 1.0f);
    if (IsKeyPressed(KEY_G)) state->showGrid = !state->showGrid;
    if (IsKeyPressed(KEY_C)) {
        memset(state->classicWorld, 0, sizeof(state->classicWorld));
        state->classicGeneration = 0;
    }
    if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_F1)) pattern_random(state->classicWorld, state->classicDensity);
    if (IsKeyPressed(KEY_F2)) {
        memset(state->classicWorld, 0, sizeof(state->classicWorld));
        pattern_glider(state->classicWorld, HEIGHT / 2, WIDTH / 2);
        state->classicGeneration = 0;
    }
    if (IsKeyPressed(KEY_F3)) {
        memset(state->classicWorld, 0, sizeof(state->classicWorld));
        pattern_blinker(state->classicWorld, HEIGHT / 2, WIDTH / 2 - 1);
        state->classicGeneration = 0;
    }
}

void draw_campaign_ui(GameState *state, Vector2 mouse) {
    if (!state->currentLevel) return;
    int w = state->currentLevel->width, h = state->currentLevel->height;
    int panelX = w * state->cellSize;
    DrawRectangle(panelX, 0, GetScreenWidth() - panelX, GetScreenHeight(), COLOR_PANEL);
    DrawLine(panelX, 0, panelX, GetScreenHeight(), LIGHTGRAY);

    int y = 20;
    DrawText("CAMPAIGN", panelX + 20, y, 20, WHITE); y += 28;
    DrawText(TextFormat("Level %d / %d", state->currentLevelId, get_total_levels()), panelX + 20, y, 16, YELLOW); y += 22;
    DrawText(state->currentLevel->description, panelX + 20, y, 13, LIGHTGRAY); y += 25;

    state->campaignBtns.prevLevel = (Rectangle){ panelX + 20, y, 120, 28 };
    state->campaignBtns.nextLevel = (Rectangle){ panelX + 150, y, 120, 28 };
    draw_button("< Prev Lv", state->campaignBtns.prevLevel, CheckCollisionPointRec(mouse, state->campaignBtns.prevLevel), false);
    draw_button("Next Lv >", state->campaignBtns.nextLevel, CheckCollisionPointRec(mouse, state->campaignBtns.nextLevel), false);
    y += 38;

    DrawText(TextFormat("Gen: %d / %d", state->campaignGeneration, state->currentLevel->target_generations), panelX + 20, y, 16, YELLOW); y += 24;
    int live = count_live_cells(state->campaignWorld, w, h);
    DrawText(TextFormat("Live cells: %d (Goal >= 4)", live), panelX + 20, y, 16, live >= 4 ? GREEN : YELLOW); y += 24;
    DrawText(TextFormat("Start cells: %d / %d", state->startCellsPlaced, state->currentLevel->max_start_cells), panelX + 20, y, 16, YELLOW); y += 24;
    DrawText(TextFormat("Speed: %.1f gen/s", state->speed), panelX + 20, y, 15, LIGHTGRAY); y += 30;

    if (state->campaignPhase == CAMPAIGN_PHASE_PLACING) {
        DrawText("PHASE: PLACE CELLS", panelX + 20, y, 16, WHITE); y += 28;
        state->campaignBtns.start     = (Rectangle){ panelX + 20, y, 120, 35 };
        state->campaignBtns.reset     = (Rectangle){ panelX + 150, y, 120, 35 };
        y += 45;
        state->campaignBtns.autoSolve = (Rectangle){ panelX + 20, y, 250, 35 };
        y += 45;
        state->campaignBtns.menu      = (Rectangle){ panelX + 20, y, 250, 40 };

        draw_button("Start",        state->campaignBtns.start,     CheckCollisionPointRec(mouse, state->campaignBtns.start), false);
        draw_button("Reset",        state->campaignBtns.reset,     CheckCollisionPointRec(mouse, state->campaignBtns.reset), false);
        draw_button("Auto Solve",   state->campaignBtns.autoSolve, CheckCollisionPointRec(mouse, state->campaignBtns.autoSolve), false);
        draw_button("Back to Menu", state->campaignBtns.menu,      CheckCollisionPointRec(mouse, state->campaignBtns.menu), false);
    } else if (state->campaignPhase == CAMPAIGN_PHASE_SIMULATING) {
        DrawText("PHASE: SIMULATING", panelX + 20, y, 16, WHITE); y += 28;
        state->campaignBtns.pause    = (Rectangle){ panelX + 20, y, 120, 35 };
        state->campaignBtns.stepSim  = (Rectangle){ panelX + 150, y, 120, 35 };
        y += 45;
        state->campaignBtns.resetSim = (Rectangle){ panelX + 20, y, 250, 35 };
        y += 45;
        state->campaignBtns.menuSim  = (Rectangle){ panelX + 20, y, 250, 40 };

        draw_button(state->paused ? "Run" : "Pause", state->campaignBtns.pause, CheckCollisionPointRec(mouse, state->campaignBtns.pause), false);
        draw_button("Step",         state->campaignBtns.stepSim,  CheckCollisionPointRec(mouse, state->campaignBtns.stepSim), false);
        draw_button("Reset Level",  state->campaignBtns.resetSim, CheckCollisionPointRec(mouse, state->campaignBtns.resetSim), false);
        draw_button("Back to Menu", state->campaignBtns.menuSim,  CheckCollisionPointRec(mouse, state->campaignBtns.menuSim), false);
    } else if (state->campaignPhase == CAMPAIGN_PHASE_COMPLETE) {
        DrawText("LEVEL COMPLETE!", panelX + 20, y, 20, GREEN); y += 35;
        state->campaignBtns.next          = (Rectangle){ panelX + 20, y, 120, 35 };
        state->campaignBtns.retryComplete = (Rectangle){ panelX + 150, y, 120, 35 };
        y += 45;
        state->campaignBtns.menuComplete  = (Rectangle){ panelX + 20, y, 250, 40 };

        draw_button("Next Level",   state->campaignBtns.next,          CheckCollisionPointRec(mouse, state->campaignBtns.next), false);
        draw_button("Replay",       state->campaignBtns.retryComplete, CheckCollisionPointRec(mouse, state->campaignBtns.retryComplete), false);
        draw_button("Back to Menu", state->campaignBtns.menuComplete,  CheckCollisionPointRec(mouse, state->campaignBtns.menuComplete), false);
    } else if (state->campaignPhase == CAMPAIGN_PHASE_FAILED) {
        DrawText("FAILED! Try again.", panelX + 20, y, 20, RED); y += 35;
        state->campaignBtns.retry           = (Rectangle){ panelX + 20, y, 120, 35 };
        state->campaignBtns.autoSolveFailed = (Rectangle){ panelX + 150, y, 120, 35 };
        y += 45;
        state->campaignBtns.menuFailed      = (Rectangle){ panelX + 20, y, 250, 40 };

        draw_button("Retry",        state->campaignBtns.retry,           CheckCollisionPointRec(mouse, state->campaignBtns.retry), false);
        draw_button("Auto Solve",   state->campaignBtns.autoSolveFailed, CheckCollisionPointRec(mouse, state->campaignBtns.autoSolveFailed), false);
        draw_button("Back to Menu", state->campaignBtns.menuFailed,      CheckCollisionPointRec(mouse, state->campaignBtns.menuFailed), false);
    }
}

bool check_campaign_win(GameState *state) {
    if (!state || !state->currentLevel || !state->campaignWorld)
        return false;
    if (state->campaignGeneration < state->currentLevel->target_generations)
        return false;
    int live = count_live_cells(state->campaignWorld, state->currentLevel->width, state->currentLevel->height);
    return live >= 4;
}

void step_campaign_simulation(GameState *state) {
    if (!state || !state->currentLevel || !state->campaignWorld) return;
    int w = state->currentLevel->width;
    int h = state->currentLevel->height;

    next_generation_with_barriers(state->campaignWorld, state->currentLevel->barriers, w, h, &state->campaignGeneration);
    history_push(&state->campaignHistory, state->campaignWorld, state->campaignGeneration);

    int live = count_live_cells(state->campaignWorld, w, h);
    if (check_campaign_win(state)) {
        state->campaignPhase = CAMPAIGN_PHASE_COMPLETE;
        state->paused = true;
        int next = state->currentLevelId + 1;
        if (next <= get_total_levels()) save_progress(next);
        log_event("Level %d completed", state->currentLevelId);
    } else if (live == 0 || state->campaignGeneration >= state->currentLevel->target_generations) {
        state->campaignPhase = CAMPAIGN_PHASE_FAILED;
        state->paused = true;
        log_event("Level %d failed at gen %d (live=%d)", state->currentLevelId, state->campaignGeneration, live);
    }
}

void handle_campaign_input(GameState *state, Vector2 mouse) {
    if (!state->currentLevel) return;
    int w = state->currentLevel->width, h = state->currentLevel->height;

    if (state->campaignPhase == CAMPAIGN_PHASE_PLACING) {
        int gameW = w * state->cellSize;
        int gameH = h * state->cellSize;
        if (mouse.x >= 0 && mouse.x < gameW && mouse.y >= 0 && mouse.y < gameH) {
            int x = (int)(mouse.x / state->cellSize);
            int y = (int)(mouse.y / state->cellSize);
            if (x >= 0 && x < w && y >= 0 && y < h) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                    state->campaignWorld[y][x] == 0 &&
                    state->startCellsPlaced < state->currentLevel->max_start_cells) {
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

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mouse, state->campaignBtns.prevLevel)) {
            int prev = state->currentLevelId > 1 ? state->currentLevelId - 1 : get_total_levels();
            save_progress(prev);
            init_campaign(state, prev);
            return;
        }
        if (CheckCollisionPointRec(mouse, state->campaignBtns.nextLevel)) {
            int next = state->currentLevelId < get_total_levels() ? state->currentLevelId + 1 : 1;
            save_progress(next);
            init_campaign(state, next);
            return;
        }

        if (state->campaignPhase == CAMPAIGN_PHASE_PLACING) {
            if (CheckCollisionPointRec(mouse, state->campaignBtns.start) && state->startCellsPlaced > 0) {
                state->campaignPhase = CAMPAIGN_PHASE_SIMULATING;
                state->paused = false;
                state->timeAccumulator = 0.0f;
                log_event("Campaign simulation started on level %d", state->currentLevelId);
            } else if (CheckCollisionPointRec(mouse, state->campaignBtns.reset)) {
                init_campaign(state, state->currentLevelId);
                log_event("Campaign level reset");
            } else if (CheckCollisionPointRec(mouse, state->campaignBtns.autoSolve)) {
                auto_solve_campaign(state);
            } else if (CheckCollisionPointRec(mouse, state->campaignBtns.menu)) {
                free_campaign(state);
                state->showMenu = true;
                log_event("Return to menu from campaign");
            }
        } else if (state->campaignPhase == CAMPAIGN_PHASE_SIMULATING) {
            if (CheckCollisionPointRec(mouse, state->campaignBtns.pause)) {
                state->paused = !state->paused;
            } else if (CheckCollisionPointRec(mouse, state->campaignBtns.stepSim)) {
                state->paused = true;
                step_campaign_simulation(state);
            } else if (CheckCollisionPointRec(mouse, state->campaignBtns.resetSim)) {
                init_campaign(state, state->currentLevelId);
            } else if (CheckCollisionPointRec(mouse, state->campaignBtns.menuSim)) {
                free_campaign(state);
                state->showMenu = true;
                log_event("Return to menu during simulation");
            }
        } else if (state->campaignPhase == CAMPAIGN_PHASE_COMPLETE) {
            if (CheckCollisionPointRec(mouse, state->campaignBtns.next)) {
                int nextLevel = state->currentLevelId < get_total_levels() ? state->currentLevelId + 1 : 1;
                save_progress(nextLevel);
                init_campaign(state, nextLevel);
                log_event("Advanced to level %d", nextLevel);
            } else if (CheckCollisionPointRec(mouse, state->campaignBtns.retryComplete)) {
                init_campaign(state, state->currentLevelId);
            } else if (CheckCollisionPointRec(mouse, state->campaignBtns.menuComplete)) {
                free_campaign(state);
                state->showMenu = true;
            }
        } else if (state->campaignPhase == CAMPAIGN_PHASE_FAILED) {
            if (CheckCollisionPointRec(mouse, state->campaignBtns.retry)) {
                init_campaign(state, state->currentLevelId);
                log_event("Retry level");
            } else if (CheckCollisionPointRec(mouse, state->campaignBtns.autoSolveFailed)) {
                init_campaign(state, state->currentLevelId);
                auto_solve_campaign(state);
            } else if (CheckCollisionPointRec(mouse, state->campaignBtns.menuFailed)) {
                free_campaign(state);
                state->showMenu = true;
            }
        }
    }

    if (IsKeyPressed(KEY_SPACE) && state->campaignPhase == CAMPAIGN_PHASE_SIMULATING) {
        state->paused = !state->paused;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        if (state->campaignPhase == CAMPAIGN_PHASE_PLACING && state->startCellsPlaced > 0) {
            state->campaignPhase = CAMPAIGN_PHASE_SIMULATING;
            state->paused = false;
            state->timeAccumulator = 0.0f;
        } else if (state->campaignPhase == CAMPAIGN_PHASE_SIMULATING && state->paused) {
            step_campaign_simulation(state);
        }
    }
    if (IsKeyPressed(KEY_R)) {
        init_campaign(state, state->currentLevelId);
    }
    if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) state->speed = fminf(30.0f, state->speed + 1.0f);
    if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) state->speed = fmaxf(1.0f, state->speed - 1.0f);
    if (IsKeyPressed(KEY_G)) state->showGrid = !state->showGrid;
    if (IsKeyPressed(KEY_Z) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))) {
        history_undo(&state->campaignHistory, state->campaignWorld, &state->campaignGeneration);
        state->startCellsPlaced = count_live_cells(state->campaignWorld, w, h);
    }
}

void auto_solve_campaign(GameState *state) {
    if (!state || !state->currentLevel || !state->campaignWorld)
        return;

    state->campaignPhase = CAMPAIGN_PHASE_PLACING;
    state->paused = true;
    state->campaignGeneration = 0;
    state->timeAccumulator = 0.0f;

    int w = state->currentLevel->width;
    int h = state->currentLevel->height;
    int target = state->currentLevel->target_generations;
    if (w < 2 || h < 2 || target < 1) return;

    int **candidate = (int **)calloc((size_t)h, sizeof(int *));
    if (!candidate) return;
    for (int y = 0; y < h; y++) {
        candidate[y] = (int *)calloc((size_t)w, sizeof(int));
        if (!candidate[y]) {
            for (int j = 0; j < y; j++) free(candidate[j]);
            free(candidate);
            return;
        }
    }

    int totalPositions = (w - 1) * (h - 1);
    bool solved = false;
    int chosenSy = 0, chosenSx = 0;

    for (int offset = 0; offset < totalPositions; offset++) {
        int idx = (state->autoSolveCursor + offset) % totalPositions;
        int sy = idx / (w - 1);
        int sx = idx % (w - 1);

        for (int y = 0; y < h; y++)
            memset(candidate[y], 0, (size_t)w * sizeof(int));

        candidate[sy][sx]         = 1;
        candidate[sy][sx + 1]     = 1;
        candidate[sy + 1][sx]     = 1;
        candidate[sy + 1][sx + 1] = 1;

        int generation = 0;
        bool survives = true;

        while (generation < target) {
            next_generation_with_barriers(candidate, state->currentLevel->barriers, w, h, &generation);
            if (count_live_cells(candidate, w, h) < 4) {
                survives = false;
                break;
            }
        }

        if (survives) {
            chosenSy = sy;
            chosenSx = sx;
            state->autoSolveCursor = (idx + 1) % totalPositions;
            solved = true;
            break;
        }
    }

    for (int y = 0; y < h; y++) free(candidate[y]);
    free(candidate);

    if (!solved) {
        log_event("Auto Solve: no valid configuration found for level %d", state->currentLevelId);
        return;
    }

    for (int y = 0; y < h; y++)
        memset(state->campaignWorld[y], 0, (size_t)w * sizeof(int));

    state->campaignWorld[chosenSy][chosenSx]         = 1;
    state->campaignWorld[chosenSy][chosenSx + 1]     = 1;
    state->campaignWorld[chosenSy + 1][chosenSx]     = 1;
    state->campaignWorld[chosenSy + 1][chosenSx + 1] = 1;

    state->startCellsPlaced = 4;
    state->campaignGeneration = 0;
    history_reset(&state->campaignHistory);
    history_push(&state->campaignHistory, state->campaignWorld, 0);
    log_event("Auto Solve placed solution at (%d, %d) for level %d", chosenSx, chosenSy, state->currentLevelId);
}

void draw_barriers(Barriers *b, int cellSize, int offsetX, int offsetY) {
    if (!b) return;
    /* Горизонтальный барьер между (y, x) и (y, x+1) — вертикальная красная грань */
    for (int y = 0; y < b->height; y++) {
        for (int x = 0; x < b->width - 1; x++) {
            if (has_h_barrier(b, y, x)) {
                Vector2 p1 = { (float)(offsetX + (x + 1) * cellSize), (float)(offsetY + y * cellSize) };
                Vector2 p2 = { (float)(offsetX + (x + 1) * cellSize), (float)(offsetY + (y + 1) * cellSize) };
                DrawLineEx(p1, p2, 3.0f, COLOR_BARRIER);
            }
        }
    }
    /* Вертикальный барьер между (y, x) и (y+1, x) — горизонтальная красная грань */
    for (int y = 0; y < b->height - 1; y++) {
        for (int x = 0; x < b->width; x++) {
            if (has_v_barrier(b, y, x)) {
                Vector2 p1 = { (float)(offsetX + x * cellSize), (float)(offsetY + (y + 1) * cellSize) };
                Vector2 p2 = { (float)(offsetX + (x + 1) * cellSize), (float)(offsetY + (y + 1) * cellSize) };
                DrawLineEx(p1, p2, 3.0f, COLOR_BARRIER);
            }
        }
    }
}

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
    srand((unsigned int)time(NULL));
    int screenWidth = 1300, screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Game of Life - Adventure");
    SetTargetFPS(60);

    GameState state = {0};
    state.showMenu = true;
    state.classicDensity = 25;
    state.speed = 5.0f;
    state.showGrid = true;
    state.paused = true;

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
            DrawRectangle(sw / 2 - 200, sh / 2 - 100, 400, 200, DARKGRAY);
            DrawRectangleLines(sw / 2 - 200, sh / 2 - 100, 400, 200, WHITE);
            DrawText("Select Difficulty", sw / 2 - 100, sh / 2 - 70, 20, YELLOW);
            Rectangle easy   = { sw / 2 - 150, sh / 2 - 30, 100, 40 };
            Rectangle medium = { sw / 2 - 50,  sh / 2 - 30, 100, 40 };
            Rectangle hard   = { sw / 2 + 50,  sh / 2 - 30, 100, 40 };
            bool easyH = CheckCollisionPointRec(mouse, easy);
            bool medH  = CheckCollisionPointRec(mouse, medium);
            bool hardH = CheckCollisionPointRec(mouse, hard);
            draw_button("15%", easy,   easyH, state.classicDensity == 15);
            draw_button("25%", medium, medH,  state.classicDensity == 25);
            draw_button("40%", hard,   hardH, state.classicDensity == 40);
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
                    float step = 1.0f / fmaxf(1.0f, state.speed);
                    while (state.timeAccumulator >= step) {
                        next_generation(state.classicWorld);
                        state.classicGeneration++;
                        state.timeAccumulator -= step;
                    }
                }
                int gameW = WIDTH * state.cellSize, gameH = HEIGHT * state.cellSize;
                for (int y = 0; y < HEIGHT; y++)
                    for (int x = 0; x < WIDTH; x++)
                        DrawRectangle(x * state.cellSize, y * state.cellSize, state.cellSize, state.cellSize,
                                      state.classicWorld[y][x] ? COLOR_ALIVE : COLOR_DEAD);
                if (state.showGrid) {
                    for (int i = 0; i <= WIDTH; i++) DrawLine(i * state.cellSize, 0, i * state.cellSize, gameH, COLOR_GRID);
                    for (int i = 0; i <= HEIGHT; i++) DrawLine(0, i * state.cellSize, gameW, i * state.cellSize, COLOR_GRID);
                }
                draw_classic_ui(&state, mouse);
                handle_classic_input(&state, mouse);
            } else if (state.gameMode == GAME_MODE_CAMPAIGN && state.currentLevel) {
                int w = state.currentLevel->width, h = state.currentLevel->height;
                int gameW = w * state.cellSize, gameH = h * state.cellSize;
                for (int y = 0; y < h; y++)
                    for (int x = 0; x < w; x++)
                        DrawRectangle(x * state.cellSize, y * state.cellSize, state.cellSize, state.cellSize,
                                      state.campaignWorld[y][x] ? COLOR_ALIVE : COLOR_DEAD);
                if (state.showGrid) {
                    for (int i = 0; i <= w; i++) DrawLine(i * state.cellSize, 0, i * state.cellSize, gameH, COLOR_GRID);
                    for (int i = 0; i <= h; i++) DrawLine(0, i * state.cellSize, gameW, i * state.cellSize, COLOR_GRID);
                }
                draw_barriers(state.currentLevel->barriers, state.cellSize, 0, 0);
                draw_campaign_ui(&state, mouse);
                handle_campaign_input(&state, mouse);

                if (state.campaignPhase == CAMPAIGN_PHASE_SIMULATING && !state.paused) {
                    state.timeAccumulator += dt;
                    float step = 1.0f / fmaxf(1.0f, state.speed);
                    while (state.timeAccumulator >= step && state.campaignPhase == CAMPAIGN_PHASE_SIMULATING) {
                        state.timeAccumulator -= step;
                        step_campaign_simulation(&state);
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