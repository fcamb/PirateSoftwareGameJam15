#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/raylib.h"

/* DEFINES */
#define DEBUG 1
#define MAX_INVENTORY_ITEMS 25
#define DEFAULT_MAP_SIZE 5

typedef struct GameMapNode
{
  const char* name;
  int data; // placeholder for game map node data
  int type; // also placeholder
  Rectangle nodePositionRect;
  Color nodePositionRectColor;
} GameMapNode;

typedef struct GameMap
{
  GameMapNode* gameMapNodes;
  int playerPositionInGameMap;
  int mouseHoverIndex;
  int gameMapSize;
} GameMap;

typedef struct Vector2i
{
    int x;
    int y;
} Vector2i;

typedef struct GameSettings
{
  bool soundOn;
} GameSettings;

typedef struct ControlsMenu
{
  bool control; // placeholder, has no meaning right now
} ControlsMenu;

typedef struct MainMenu
{
  float fontSize;
  
  Rectangle startGameRect;
  Color startGameRectColor;
  Vector2 startGameTextPosition;
  
  Rectangle gotoOptionsMenuRect;
  Color gotoOptionsMenuRectColor;
  Vector2 gotoOptionsMenuTextPosition;

  Rectangle exitGameRect;
  Color exitGameRectColor;
  Vector2 exitGameTextPosition;
} MainMenu;

typedef struct OptionsMenu
{
  float fontSize;
  
  Rectangle soundToggleRect;
  Color soundToggleRectColor;
  Vector2 soundToggleTextPosition;

  Rectangle controlsMenuRect;
  Color controlsMenuRectColor;
  Vector2 controlsMenuTextPosition;

  Rectangle goBackToMainMenuRect;
  Color goBackToMainMenuRectColor;
  Vector2 goBackToMainMenuTextPosition;
} OptionsMenu;

typedef struct GameState
{
  bool running;
  Vector2i screenSize;
  
  MainMenu mainMenu;
  OptionsMenu optionsMenu;
  ControlsMenu controlsMenu;
  GameSettings gameSettings;
  
  bool mainMenuActive;
  bool optionsMenuActive;
  bool controlsMenuActive;
  bool battleSceneActive;
  bool mapSceneActive;
} GameState;

/* TYPES */
typedef struct Stats
{
  float health;
  float mana;
  float attackPower;
  float spellPower;
  float attackDefense;
  float spellDefense;
  float speed;
} Stats;

typedef enum ItemType
{
  NONE,
  // WEARABLE ITEMS (weapons, armor) - Gear basically
  HAT,
  CHEST_PIECE,
  GLOVES,
  BOOTS,
  RING_ONE,
  RING_TWO,
  NECKLACE,
  RELIC,
  
  // CONSUMABLES + OTHER ITEMS
  HEALTH_POTION,
  MANA_POTION,
  ATTACK_BOOST_POTION,
  SPELL_BOOST_POTION,
  ATTACK_DEFENSE_POTION,
  SPELL_DEFENSE_POTION,
  SPEED_POTION,
  ATTACK_DAMAGE_POT,
  SPELL_DAMAGE_POT,
  SLOW_POT,
  REDUCE_DEFENSE_POT,
  REDUCE_SPELL_DEFENSE_POT,
  
  // SPECIAL ITEMS ?
} ItemType;
    
typedef struct Item
{
  const char* name;
  Rectangle rect;
  Texture2D texture;
  Stats stats;
} Item;

typedef struct Inventory
{
  Rectangle rect;
  Texture2D texture;
  Item items[MAX_INVENTORY_ITEMS];
} Inventory;

typedef struct Player
{
  Vector2 size;
  Texture2D texture;
  Inventory inventory;
  Stats stats;
} Player;

typedef struct Enemy
{
  const char *name;
  Vector2 size;
  Texture2D texture;
  Inventory inventory;
  Stats stats;
} Enemy;

/* OBJECTS */
GameState* gameState;
GameMap* gameMap;
Player *player;
const char* gameText[6]; // not sure what final size of this will be

/* INITIALIZATION */
void InitGame();
void CreatePlayer();
void CreateGameMap();

/* GENERAL FUNCIONS THAT CONTROL THE FLOW OF THE GAME */
void UnloadGame();
void RenderGame();
void UpdateGame();

/* UPDATE FUNCTIONS */
void UpdateMainMenu();
void UpdateOptionsMenu();
void UpdateControlsMenu();
void UpdateBattleScene();
void UpdateMapScene();

/* RENDER FUNCTIONS */
void RenderMainMenu();
void RenderOptionsMenu();
void RenderControlsMenu();
void RenderBattleScene();
void RenderMapScene();

int
main()
{  
  /* Initialize Game Data - Raylib First! */
  InitWindow(1920, 1080, "Game Jam");
  SetTargetFPS(60);
  
  InitGame();
  CreatePlayer();
  
  while(!WindowShouldClose() && gameState->running) {
    UpdateGame();
    RenderGame();
  }

  UnloadGame();
  CloseWindow();
  
  return 0;
}

void
InitGame()
{
  /* ALLOCATTE MEMORY FOR ALL POINTERS FIRST */
  gameState = malloc(sizeof(GameState));
  if (!gameState) {
    // failure - exit game
#ifdef DEBUG
    printf("Failed to allocate GameState memory.\n");
#endif
    exit(1);
  }
  player = malloc(sizeof(Player));
  if (!player) {
    // failure - exit game
#ifdef DEBUG    
    printf("Failed to allocated Player memory.\n");
#endif
    exit(1);
  }
  gameMap = malloc(sizeof(GameMap));
  if (!gameMap) {
#ifdef DEBUG
    printf("Failed to allocate game map memory.\n");
    exit(1);
#endif
  }
  gameMap->gameMapNodes = malloc(sizeof(GameMapNode) * DEFAULT_MAP_SIZE);
  if (!gameMap->gameMapNodes) {
#ifdef DEBUG
    printf("Failed to allocate game map nodes memory.\n");
    exit(1);
#endif
  }

  gameText[0] = "Start Game";
  gameText[1] = "Options";
  gameText[2] = "Exit Game";
  gameText[3] = "Sound";
  gameText[4] = "Controls";
  gameText[5] = "Main Menu";

  gameState->running = true;
  gameState->screenSize = (Vector2i){1920,1080};
  gameState->mainMenuActive = true;
  gameState->battleSceneActive = false;
  gameState->mapSceneActive = false;
  gameState->optionsMenuActive = false;
  gameState->controlsMenuActive = false;

  /* Game Settings */
  gameState->gameSettings.soundOn = true;


  /* where all rects start from */
  float startX = 1920.f/2.f;
  float startY = 1080.f/4.f;
  
  /* Main Menu Stuff */  
  gameState->mainMenu.fontSize = 40.f;
  gameState->mainMenu.startGameRectColor =       RAYWHITE;
  gameState->mainMenu.gotoOptionsMenuRectColor = RAYWHITE;
  gameState->mainMenu.exitGameRectColor =        RAYWHITE;
  
  Vector2 size1 = MeasureTextEx(GetFontDefault(), gameText[0], 40.f, 1.f);
  Vector2 size2 = MeasureTextEx(GetFontDefault(), gameText[1], 40.f, 1.f);
  Vector2 size3 = MeasureTextEx(GetFontDefault(), gameText[2], 40.f, 1.f);
  
  gameState->mainMenu.startGameRect =       (Rectangle){startX - (size1.x/2.f) - 10.f,
						        startY - (size1.y/2.f) - 10.f,
						        size1.x + 20.f, size1.y + 20.f};
  gameState->mainMenu.gotoOptionsMenuRect = (Rectangle){startX - (size2.x/2.f) - 10.f,
						       (startY - (size2.y/2.f) - 10.f) + 100.f,
							size2.x + 20.f, size2.y + 20.f};
  gameState->mainMenu.exitGameRect =        (Rectangle){startX - (size3.x/2.f) - 10.f,
						       (startY - (size3.y/2.f) - 10.f) + 200.f,
						        size3.x + 20.f, size3.y + 20.f};
  
  gameState->mainMenu.startGameTextPosition =       (Vector2){gameState->mainMenu.startGameRect.x + 10.f,       gameState->mainMenu.startGameRect.y + 10.f};
  gameState->mainMenu.gotoOptionsMenuTextPosition = (Vector2){gameState->mainMenu.gotoOptionsMenuRect.x + 10.f, gameState->mainMenu.gotoOptionsMenuRect.y + 10.f};
  gameState->mainMenu.exitGameTextPosition =        (Vector2){gameState->mainMenu.exitGameRect.x + 10.f,        gameState->mainMenu.exitGameRect.y + 10.f};
  
  /* Options Menu STuff */
  gameState->optionsMenu.fontSize = 40.f;
  gameState->optionsMenu.soundToggleRectColor =      RAYWHITE;
  gameState->optionsMenu.controlsMenuRectColor =     RAYWHITE;
  gameState->optionsMenu.goBackToMainMenuRectColor = RAYWHITE;
  
  size1 = MeasureTextEx(GetFontDefault(), gameText[3], 40.f, 1.f);
  size2 = MeasureTextEx(GetFontDefault(), gameText[4], 40.f, 1.f);
  size3 = MeasureTextEx(GetFontDefault(), gameText[5], 40.f, 1.f);
  
  gameState->optionsMenu.soundToggleRect =     (Rectangle){startX - (size1.x/2.f) - 10.f,
						 	   startY - (size1.y/2.f) - 10.f,
							   size1.x + 20.f, size1.y + 20.f};
  gameState->optionsMenu.controlsMenuRect =    (Rectangle){startX - (size2.x/2.f) - 10.f,
						           (startY - (size2.y/2.f) - 10.f) + 100.f,
						            size2.x + 20.f, size2.y + 20.f};
  gameState->optionsMenu.goBackToMainMenuRect = (Rectangle){startX - (size3.x/2.f) - 10.f,
						           (startY - (size3.y/2.f) - 10.f) + 200.f,
						            size3.x + 20.f, size3.y + 20.f};

  gameState->optionsMenu.soundToggleTextPosition =      (Vector2){gameState->optionsMenu.soundToggleRect.x + 10.f,      gameState->optionsMenu.soundToggleRect.y + 10.f};
  gameState->optionsMenu.controlsMenuTextPosition =     (Vector2){gameState->optionsMenu.controlsMenuRect.x + 10.f,     gameState->optionsMenu.controlsMenuRect.y + 10.f};
  gameState->optionsMenu.goBackToMainMenuTextPosition = (Vector2){gameState->optionsMenu.goBackToMainMenuRect.x + 10.f, gameState->optionsMenu.goBackToMainMenuRect.y + 10.f};


  /* GAME MAP STUFF */
  CreateGameMap();
}

void
CreatePlayer()
{
  memset(&player->inventory.items, 0, sizeof(player->inventory.items));
  player->size = (Vector2){32.f, 32.f};
  memset(&player->texture, 0, sizeof(Texture2D));

  player->stats.health = 10;
  player->stats.mana = 10;
  player->stats.attackPower = 20;
  player->stats.spellPower = 20;
  player->stats.attackDefense = 10;
  player->stats.spellDefense = 10;
  player->stats.speed = 5;
}

void
CreateGameMap()
{
  float startX = 1920/2.f;
  float currentX = startX;
  float offsetX = 200.f;
  float startY = 900;
  int i = 1;

  for (i = 0; i < DEFAULT_MAP_SIZE; i++) {
    gameMap->gameMapNodes[i].data=0;
    currentX = startX;
    if ((i+1) % 2 == 0) {
      currentX = startX + offsetX;
      offsetX *= -1.f;
    }
    
    gameMap->gameMapNodes[i].nodePositionRect = (Rectangle){currentX - 50.f, startY, 100.f, 100.f};
    startY -= 200.f;
	
    if (i == DEFAULT_MAP_SIZE - 1) {
      gameMap->gameMapNodes[i].name = "BOSS";
      gameMap->gameMapNodes[i].type = 1;
      gameMap->gameMapNodes[i].nodePositionRectColor = RED;
    } else {
      gameMap->gameMapNodes[i].name = "BASIC";
      gameMap->gameMapNodes[i].type = 0;
      gameMap->gameMapNodes[i].nodePositionRectColor = BLUE;
    }
  }
  gameMap->gameMapSize = DEFAULT_MAP_SIZE;
  gameMap->playerPositionInGameMap = 0;
  /* Set to -1 to start */
  gameMap->mouseHoverIndex = -1;
}

void
UnloadGame()
{
#ifdef DEBUG
  printf("Freeing all memory.\n");
#endif
  
  free(player);
  free(gameState);
  free(gameMap->gameMapNodes);
  free(gameMap);
}

void
UpdateGame()
{
  if (gameState->mainMenuActive) {
    UpdateMainMenu();
  }
  else if (gameState->optionsMenuActive) {
    UpdateOptionsMenu();
  }
  else if (gameState->controlsMenuActive) {
    UpdateControlsMenu();
  }
  else if (gameState->battleSceneActive) {
    UpdateBattleScene();
  }
  else if (gameState->mapSceneActive) {
    UpdateMapScene();
  }
}

void
RenderGame()
{
  BeginDrawing();
  {
    ClearBackground(RAYWHITE);
    //DrawRectangle(100, 100, player->size.x, player->size.y, PURPLE);
    if (gameState->mainMenuActive) {
      RenderMainMenu();
    }
    else if (gameState->optionsMenuActive) {
      RenderOptionsMenu();
    }
    else if (gameState->controlsMenuActive) {
      RenderControlsMenu();
    }
    else if (gameState->battleSceneActive) {
      RenderBattleScene();
    }
    else if (gameState->mapSceneActive) {
      RenderMapScene();
    }
  }
  EndDrawing();
}

void
UpdateMainMenu()
{
  Vector2 mousePosition = GetMousePosition();

  gameState->mainMenu.startGameRectColor =       RAYWHITE;
  gameState->mainMenu.gotoOptionsMenuRectColor = RAYWHITE;
  gameState->mainMenu.exitGameRectColor =        RAYWHITE;
  
  if (CheckCollisionPointRec(mousePosition, gameState->mainMenu.startGameRect)) {
    gameState->mainMenu.startGameRectColor = BLACK;
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      gameState->mainMenuActive = false;
      gameState->mapSceneActive = true;
    }
  }
  else if (CheckCollisionPointRec(mousePosition, gameState->mainMenu.gotoOptionsMenuRect)) {
    gameState->mainMenu.gotoOptionsMenuRectColor = BLACK;
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      gameState->mainMenuActive = false;
      gameState->optionsMenuActive = true;
    }
  }
  else if (CheckCollisionPointRec(mousePosition, gameState->mainMenu.exitGameRect)) {
    gameState->mainMenu.exitGameRectColor = BLACK;
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      gameState->running = false;
      gameState->mainMenuActive = false;
    }
  }
}

void
RenderMainMenu()
{
  /* ABSOLUTELY CRUCIAL THINGS ARE DRAWN IN CORRECT ORDER */
  DrawRectangleLinesEx(gameState->mainMenu.startGameRect,       1.f, gameState->mainMenu.startGameRectColor);
  DrawRectangleLinesEx(gameState->mainMenu.gotoOptionsMenuRect, 1.f, gameState->mainMenu.gotoOptionsMenuRectColor);
  DrawRectangleLinesEx(gameState->mainMenu.exitGameRect,        1.f, gameState->mainMenu.exitGameRectColor);
  DrawTextEx(GetFontDefault(), gameText[0], gameState->mainMenu.startGameTextPosition,       gameState->mainMenu.fontSize, 1.f, BLACK);
  DrawTextEx(GetFontDefault(), gameText[1], gameState->mainMenu.gotoOptionsMenuTextPosition, gameState->mainMenu.fontSize, 1.f, BLACK);
  DrawTextEx(GetFontDefault(), gameText[2], gameState->mainMenu.exitGameTextPosition,        gameState->mainMenu.fontSize, 1.f, BLACK);
}

void UpdateOptionsMenu()
{
  Vector2 mousePosition = GetMousePosition();

  gameState->optionsMenu.controlsMenuRectColor =     RAYWHITE;
  gameState->optionsMenu.soundToggleRectColor =      RAYWHITE;
  gameState->optionsMenu.goBackToMainMenuRectColor = RAYWHITE;

  if (CheckCollisionPointRec(mousePosition, gameState->optionsMenu.soundToggleRect)) {
    gameState->optionsMenu.soundToggleRectColor = BLACK;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      if (gameState->gameSettings.soundOn) {
	gameState->gameSettings.soundOn = false;
      } else {
	gameState->gameSettings.soundOn = true;
      }
      printf("Changing sound setting - %d\n", gameState->gameSettings.soundOn);
    }
  }
  else if (CheckCollisionPointRec(mousePosition, gameState->optionsMenu.controlsMenuRect)) {
    gameState->optionsMenu.controlsMenuRectColor = BLACK;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      gameState->optionsMenuActive = false;
      gameState->controlsMenuActive = true;
    }
  }
  else if (CheckCollisionPointRec(mousePosition, gameState->optionsMenu.goBackToMainMenuRect)) {
    gameState->optionsMenu.goBackToMainMenuRectColor = BLACK;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      gameState->optionsMenuActive = false;
      gameState->mainMenuActive = true;
    }
  }
}

void RenderOptionsMenu()
{
  /* ABSOLUTELY CRUCIAL THINGS ARE DRAWN IN CORRECT ORDER */
  DrawRectangleLinesEx(gameState->optionsMenu.soundToggleRect,      1.f,  gameState->optionsMenu.soundToggleRectColor);
  DrawRectangleLinesEx(gameState->optionsMenu.controlsMenuRect,     1.f,  gameState->optionsMenu.controlsMenuRectColor);
  DrawRectangleLinesEx(gameState->optionsMenu.goBackToMainMenuRect, 1.f,  gameState->optionsMenu.goBackToMainMenuRectColor);
  DrawTextEx(GetFontDefault(), gameText[3], gameState->optionsMenu.soundToggleTextPosition,      gameState->optionsMenu.fontSize, 1.f, BLACK);
  DrawTextEx(GetFontDefault(), gameText[4], gameState->optionsMenu.controlsMenuTextPosition,     gameState->optionsMenu.fontSize, 1.f, BLACK);
  DrawTextEx(GetFontDefault(), gameText[5], gameState->optionsMenu.goBackToMainMenuTextPosition, gameState->optionsMenu.fontSize, 1.f, BLACK);
}

void UpdateControlsMenu() {}
void RenderControlsMenu() {}

void UpdateBattleScene() {}
void RenderBattleScene() {}

void UpdateMapScene()
{
  Vector2 mousePosition = GetMousePosition();

  gameMap->mouseHoverIndex = -1;

  for (int i = 0; i < gameMap->gameMapSize; i++) {
    // reset colors
    if (i == gameMap->gameMapSize - 1) { // BOSS NODE - final node
      gameMap->gameMapNodes[i].nodePositionRectColor = RED;
    } else {
      gameMap->gameMapNodes[i].nodePositionRectColor = BLUE;
    }
    
    if (CheckCollisionPointRec(mousePosition, gameMap->gameMapNodes[i].nodePositionRect)) {
      gameMap->mouseHoverIndex = i;
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
	gameMap->playerPositionInGameMap = i;
	gameMap->gameMapNodes[i].nodePositionRectColor = GREEN;
      }
    }
  }
}

void RenderMapScene()
{
  for (int i = 0; i < gameMap->gameMapSize; i++) {
    DrawRectangleRec(gameMap->gameMapNodes[i].nodePositionRect, gameMap->gameMapNodes[i].nodePositionRectColor);
  }
  
  if (gameMap->mouseHoverIndex >= 0 && gameMap->mouseHoverIndex < gameMap->gameMapSize) {
    DrawRectangleLinesEx(gameMap->gameMapNodes[gameMap->mouseHoverIndex].nodePositionRect, 5.f, YELLOW);
  }
}
