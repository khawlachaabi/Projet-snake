#include "raylib.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

// Colors
#define fond (Color){40, 45, 50, 255}
#define snake (Color){46, 204, 113, 255}
#define fruit (Color){231, 76, 60, 255}
#define grid_color (Color){60, 60, 60, 255}
#define HIGHLIGHT_COLOR (Color){100, 200, 255, 255}
#define texte WHITE

#define cellsize 30
#define largeur 20
#define hauteur 15
#define MAX_OBSTACLES 50

typedef enum { menu, playing, gameover, help } GameScreen;
typedef enum { easy, medium, hard } difficulty;
typedef enum { up, down, left, right, none } Direction;

typedef struct SNAKE {
    int x, y;
    struct SNAKE* next;
} SNAKE;

typedef struct OBSTACLE {
    int x, y;
} OBSTACLE;

typedef struct {
    GameScreen currentScreen;
    difficulty niveau;
    SNAKE* tete;
    Direction direction;
    int fruitX, fruitY;
    int score;
    float timer;
    float vitesse;
    OBSTACLE obstacles[MAX_OBSTACLES];
    int nb_obstacles;
    bool bordure;
    int menuSelection;
    Sound gameover_sound;
} GameState;

SNAKE* CreerSegment(int x, int y) {
    SNAKE* segment = (SNAKE*)malloc(sizeof(SNAKE));
    segment->x = x;
    segment->y = y;
    segment->next = NULL;
    return segment;//returns the memory adredess of a case in the snake
}

void LibererSerpent(SNAKE* tete) {//when the snake moves
    // Loop through the snake's linked list
    while (tete) {
        SNAKE* temp = tete;   // Store current segment
        tete = tete->next;    // Move to next segment
        free(temp);           // Free the current segment
    }
}


void InitJeu(GameState* jeu) {
    // 1. MEMORY CLEANUP PHASE
    LibererSerpent(jeu->tete);  // Critical safety step

    // 2. SNAKE CREATION
    jeu->tete = CreerSegment(largeur / 2, hauteur / 2);  // Center position

    // 3. GAME STATE RESET
    jeu->direction = none;      // No initial movement
    jeu->score = 0;             // Reset score
    jeu->timer = 0;             // Reset movement timer

    // 4. DIFFICULTY-BASED SPEED
    jeu->vitesse = (jeu->niveau == easy) ? 0.2f :
                  (jeu->niveau == medium) ? 0.1f :
                  0.07f;

    // 5. FRUIT SPAWNING
    jeu->fruitX = GetRandomValue(0, largeur - 1);
    jeu->fruitY = GetRandomValue(0, hauteur - 1);
}

// Function to check if the snake's head has collided with its body
bool CheckSelfCollision(SNAKE* head) {
    // If the snake has no segments or only one segment, collision is not possible
    if (!head || !head->next) return false;

    // Store the position of the head
    int headX = head->x;
    int headY = head->y;

    // Start checking from the second segment
    SNAKE* current = head->next;
    while (current) {
        // If any segment matches the head's position, there's a collision
        if (current->x == headX && current->y == headY) return true;
        current = current->next;
    }

    // No collision detected
    return false;
}


void UpdateJeu(GameState* jeu) {
    // Only update the game logic if the player is currently playing
    if (jeu->currentScreen != playing) return;

    // Increment timer based on frame time
    jeu->timer += GetFrameTime();
    if (jeu->timer >= jeu->vitesse) {
        jeu->timer = 0;

        // Only move the snake if a direction is selected
        if (jeu->direction != none) {
            int dx = 0, dy = 0;
            if (jeu->direction == up) dy = -1;
            if (jeu->direction == down) dy = 1;
            if (jeu->direction == left) dx = -1;
            if (jeu->direction == right) dx = 1;

            // Calculate new head position
            int newX = jeu->tete->x + dx;
            int newY = jeu->tete->y + dy;

            // Check wall collision
            if (newX < 0 || newY < 0 || newX >= largeur || newY >= hauteur|| CheckSelfCollision(jeu->tete)) {
                 PlaySound(jeu->gameover_sound);
                jeu->currentScreen = gameover;
                return;
            }

            // Move snake by creating a new head segment
            SNAKE* nouveau = CreerSegment(newX, newY);
            nouveau->next = jeu->tete;
            jeu->tete = nouveau;

            // Check for fruit collision
            if (newX == jeu->fruitX && newY == jeu->fruitY) {
                jeu->score += 10;
                jeu->fruitX = GetRandomValue(0, largeur - 1);
                jeu->fruitY = GetRandomValue(0, hauteur - 1);
            } else {
                // Remove the last segment of the snake if no fruit was eaten
                SNAKE* current = jeu->tete;
                while (current->next && current->next->next)
                    current = current->next;
                free(current->next);
                current->next = NULL;
            }

            // Check for self-collision
            if (CheckSelfCollision(jeu->tete)) {
                jeu->currentScreen = gameover;
                return;
            }
        }
    }
}


// Function to draw the current game state on the screen
void DrawJeu(GameState* jeu) {
    BeginDrawing();                 // Start drawing frame
    ClearBackground(fond);         // Clear screen with background color

    switch (jeu->currentScreen) {
        case menu:
            // Draw the game title
            DrawText("SNAKE GAME", GetScreenWidth()/2 - MeasureText("SNAKE GAME", 50)/2, 50, 50, HIGHLIGHT_COLOR);

            // Draw difficulty label
            DrawText("DIFFICULTY:", GetScreenWidth()/2 - MeasureText("DIFFICULTY:", 30)/2, 150, 30,
                     jeu->menuSelection == 0 ? HIGHLIGHT_COLOR : texte);

            // Get the current difficulty text
            const char* diffText = (jeu->niveau == easy) ? "EASY" : (jeu->niveau == medium) ? "MEDIUM" : "HARD";

            // Draw the selected difficulty
            DrawText(diffText, GetScreenWidth()/2 - MeasureText(diffText, 30)/2, 190, 30,
                     jeu->menuSelection == 0 ? HIGHLIGHT_COLOR : texte);

            // Draw PLAY option
            DrawText("PLAY", GetScreenWidth()/2 - MeasureText("PLAY", 30)/2, 250, 30,
                     jeu->menuSelection == 1 ? HIGHLIGHT_COLOR : texte);

            // Draw HELP option
            DrawText("HELP", GetScreenWidth()/2 - MeasureText("HELP", 30)/2, 310, 30,
                     jeu->menuSelection == 2 ? HIGHLIGHT_COLOR : texte);

            // Display controls for navigating the menu
            DrawText("USE ARROWS TO NAVIGATE, ENTER TO SELECT",
                     GetScreenWidth()/2 - MeasureText("USE ARROWS TO NAVIGATE, ENTER TO SELECT", 20)/2,
                     450, 20, texte);
            break;

        case playing: {
            // Draw the fruit as a red square
            DrawRectangle(jeu->fruitX * cellsize, jeu->fruitY * cellsize, cellsize, cellsize, fruit);

            // Draw each segment of the snake
            SNAKE* current = jeu->tete;
            while (current) {
                DrawRectangle(current->x * cellsize, current->y * cellsize, cellsize, cellsize, snake);
                current = current->next;
            }

            // Display the current score
            DrawText(TextFormat("Score: %d", jeu->score), 10, 10, 20, texte);
        } break;

        case gameover:
            // Display game over message
            DrawText("GAME OVER", GetScreenWidth()/2 - MeasureText("GAME OVER", 50)/2, GetScreenHeight()/2 - 30, 50, RED);

            // Prompt player to retry or return to menu
            DrawText("Press SPACE to retry or ESC to return to menu",
                     GetScreenWidth()/2 - 200, GetScreenHeight()/2 + 40, 20, texte);
            break;

        case help:
            // Display help/instructions
            DrawText("HELP: Use arrow keys to move the snake.", 60, 100, 20, texte);
            DrawText("Eat fruits, avoid walls and yourself.", 60, 140, 20, texte);
            DrawText("Press ESC to go back.", 60, 180, 20, texte);
            break;
    }

    EndDrawing(); // End drawing frame and present the result
}


// Handles user keyboard input depending on the current screen state
void HandleInput(GameState* jeu) {
    switch (jeu->currentScreen) {

        case menu:
            // Navigate through menu options
            if (IsKeyPressed(KEY_UP)) jeu->menuSelection = (jeu->menuSelection - 1 + 4) % 4;
            if (IsKeyPressed(KEY_DOWN)) jeu->menuSelection = (jeu->menuSelection + 1) % 4;

            // Select menu option
            if (IsKeyPressed(KEY_ENTER)) {
                switch (jeu->menuSelection) {
                    case 0: // Change difficulty
                        jeu->niveau = (jeu->niveau + 1) % 3;
                        break;
                    case 1: // Start the game
                        jeu->currentScreen = playing;
                        InitJeu(jeu);
                        break;
                    case 2: // Open help screen
                        jeu->currentScreen = help;
                        break;
                    case 3: // Exit (commented out CloseWindow)
                        // CloseWindow();
                        break;
                }
            }
            break;

        case playing:
            // Change snake direction based on arrow key input, prevent reversing
            if (IsKeyPressed(KEY_UP) && jeu->direction != down) jeu->direction = up;
            else if (IsKeyPressed(KEY_DOWN) && jeu->direction != up) jeu->direction = down;
            else if (IsKeyPressed(KEY_LEFT) && jeu->direction != right) jeu->direction = left;
            else if (IsKeyPressed(KEY_RIGHT) && jeu->direction != left) jeu->direction = right;

            // Pause or return to menu
            if (IsKeyPressed(KEY_ESCAPE)) jeu->currentScreen = menu;
            break;

        case gameover:
            // Restart game or return to menu
            if (IsKeyPressed(KEY_SPACE)) {
                jeu->currentScreen = playing;
                InitJeu(jeu);
            } else if (IsKeyPressed(KEY_ESCAPE)) {
                jeu->currentScreen = menu;
            }
            break;

        case help:
            // Exit help screen
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) jeu->currentScreen = menu;
            break;
    }
}

int main() {
    // 1. Initialize window first
    InitWindow(largeur * cellsize, hauteur * cellsize, "Snake Game");

    // 2. Then initialize audio
    InitAudioDevice();

    // 3. Print debug info
    printf("Current directory: %s\n", GetWorkingDirectory());
    printf("File exists: %d\n", FileExists("assets/gameover.wav"));

    // 4. Initialize game state
    GameState jeu = {0};
    jeu.currentScreen = menu;
    jeu.niveau = easy;
    jeu.menuSelection = 0;
    jeu.gameover_sound = (Sound){0};  // Safe init

    // 5. Load sound - ABSOLUTELY FOOLPROOF METHOD
    const char* sound_paths[] = {
        "assets/gameover.wav",
        "gameover.wav",
        "C:/Windows/Media/chimes.wav"
    };

    bool sound_loaded = false;
    for (int i = 0; i < 3; i++) {
        if (FileExists(sound_paths[i])) {
            jeu.gameover_sound = LoadSound(sound_paths[i]);
            printf("Successfully loaded: %s\n", sound_paths[i]);
            sound_loaded = true;
            break;
        }
    }
    if (!sound_loaded) {
        printf("ERROR: No sound file could be loaded!\n");
    }

    // Main game loop
    while (!WindowShouldClose()) {
        HandleInput(&jeu);
        UpdateJeu(&jeu);
        DrawJeu(&jeu);
    }

    // Cleanup
    if (sound_loaded) {
        UnloadSound(jeu.gameover_sound);
    }
    CloseAudioDevice();
    LibererSerpent(jeu.tete);
    CloseWindow();
    return 0;
}










