#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../raylibIncludes/raylib.h"
#include "../raylibIncludes/raymath.h"

/* DEFINES */
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
EM_JS(int, CanvasGetWidth, (), {
    return document.getElementById('canvas').clientWidth;
});
EM_JS(int, CanvasGetHeight, (), {
    return document.getElementById('canvas').clientHeight;
});
#endif

#define DEBUG 1
#define MAX_INVENTORY_ITEMS 25
#define DEFAULT_MAP_SIZE 5
#define DEFAULT_BATTLE_SCENE_RECTS_COUNT 2

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
  float fontSize;
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

typedef struct GameMapTile
{
  int type;
  Rectangle tileRect;
  Color tileColor;
} GameMapTile;

typedef struct GameState
{
  bool running;
  Vector2i screenSize;

  Vector2 previousMousePosition;
  Vector2 mousePosition;
  
  MainMenu mainMenu;
  OptionsMenu optionsMenu;
  ControlsMenu controlsMenu;
  GameSettings gameSettings;
  GameMapTile** gameMap;
  
  bool mainMenuActive;
  bool optionsMenuActive;
  bool controlsMenuActive;
  bool craftingInventoryActive;
  bool inventoryActive;
  bool gameActive;
  
  const char* gameText[9];
} GameState;

/* TYPES */
typedef enum TextNames
{
  START_GAME,
  OPTIONS,
  EXIT_GAME,
  SOUND,
  CONTROLS,
  MAIN_MENU,
  INVENTORY,
  CRAFTING,
  MAP,
} TextNames;

typedef struct Item
{
  //const char* name;
  Rectangle rect;
  Texture2D texture;
} Item;

typedef struct Inventory
{
  Rectangle rect;
  Rectangle dragRect;
  Texture2D texture;
  Item* items;
  bool dragging;
} Inventory;

typedef struct Player
{
  Vector2 size;
  Texture2D texture;
  Inventory* inventory;
  Inventory* craftingInventory;
  int recentInventoryOpened;
} Player;


/* OBJECTS */
GameState* gameState;
Player* player;


/* INITIALIZATION */
void AllocateGame();
void InitGame(bool resettingSizes);
void InitGameMap(bool resettingSizes);
void CreatePlayer(bool resettingSize);

/* GENERAL FUNCIONS THAT CONTROL THE FLOW OF THE GAME */
void UnloadGame();
void RenderGame();
void UpdateGame();

/* UTILITY */
void LoadCSVGameMap(const char *path, GameMapTile** mapBuffer);

/* UPDATE FUNCTIONS */
void UpdateScreenSize();
void UpdateMainMenu();
void UpdateOptionsMenu();
void UpdateControlsMenu();
void UpdateCraftingScene();
void UpdateInventory();
void UpdateGameMap();
  
/* RENDER FUNCTIONS */
void RenderMainMenu();
void RenderOptionsMenu();
void RenderControlsMenu();
void RenderCraftingScene();
void RenderInventory();
void RenderGameMap();

int
main()
{
  AllocateGame();
  /*
    Get Screen Size from browser first, this is necessary to scale game
    on browser properly
  */
#if defined (PLATFORM_WEB)
  gameState->screenSize.x = CanvasGetWidth();
  gameState->screenSize.y = CanvasGetHeight();
#else
  gameState->screenSize.x = 1920;
  gameState->screenSize.y = 1080;
#endif
  
  /* Initialize Game Data - Raylib First! */
  InitWindow(gameState->screenSize.x, gameState->screenSize.y, "Game Jam");
  SetTargetFPS(60);

  InitGame(false);
  InitGameMap(false);
  CreatePlayer(false);
  
  while(!WindowShouldClose() && gameState->running) {
    UpdateGame();
    RenderGame();
  }

  UnloadGame();
  CloseWindow();
  
  return 0;
}

void
AllocateGame()
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
  
  gameState->gameText[START_GAME] = "Start Game";
  gameState->gameText[OPTIONS] = "Options";
  gameState->gameText[EXIT_GAME] = "Exit Game";
  gameState->gameText[SOUND] = "Sound";
  gameState->gameText[CONTROLS] = "Controls";
  gameState->gameText[MAIN_MENU] = "Main Menu";
  gameState->gameText[INVENTORY] = "Inventory";
  gameState->gameText[CRAFTING] = "Crafting";
  gameState->gameText[MAP] = "Map";
  
  gameState->running = true;
  gameState->screenSize = (Vector2i){1920,1080};
  gameState->mainMenuActive = true;
  gameState->optionsMenuActive = false;
  gameState->controlsMenuActive = false;
  gameState->gameActive = false;
  gameState->inventoryActive = false;
  gameState->craftingInventoryActive = false;
  
  /* Game Settings */
  gameState->gameSettings.soundOn = true;
  
  player = malloc(sizeof(Player));
  if (!player) {
    // failure - exit game
#ifdef DEBUG    
    printf("Failed to allocated Player memory.\n");
#endif
    exit(1);
  }
  player->inventory = malloc(sizeof(Inventory));
  if (!player->inventory) {
#ifdef DEBUG
    printf("Failed to allocated player inventory memory.\n");
    exit(1);
#endif
  }
  player->inventory->items = NULL;
  player->craftingInventory = malloc(sizeof(Inventory));
  if (!player->craftingInventory) {
#ifdef DEBUG
    printf("Failed to allocate player crafting inventory memory.\n");
    exit(1);
#endif
  }
  player->craftingInventory->items = NULL;
  player->inventory->items = malloc(sizeof(Item)*MAX_INVENTORY_ITEMS);
  if (!player->inventory->items) {
#ifdef DEBUG
    printf("Failed to allocated player inventory items memory.\n");
    exit(1);
#endif
  }
  player->craftingInventory->items = malloc(sizeof(Item)*MAX_INVENTORY_ITEMS);
  if (!player->craftingInventory->items) {
#ifdef DEBUG
    printf("Failed to allocate player crafting inventory items memory.\n");
    exit(1);
#endif
  }
  
  gameState->gameMap = malloc(sizeof(GameMapTile*)*10);
  if (!gameState->gameMap) {
#ifdef DEBUG
    printf("Failed to allocate inital game map memory.\n");
    exit(1);
#endif
  }
  for (int i = 0; i < 10; i++) {
    gameState->gameMap[i] = malloc(sizeof(GameMapTile) * 10);
    if (!gameState->gameMap[i]) {
#ifdef DEBUG
      printf("Failed to allocate map row memory.\n");
      exit(1);
#endif
    }
  }
}

void
InitGame(bool resettingSizes)
{
  /* Minimum Screen Size */
  /* if (gameState->screenSize.x < 800) { */
  /*   return; */
  /* } */
  /* if (gameState->screenSize.y < 800) { */
  /*   return; */
  /* } */
  
  /* where all rects start from */
  float startX = gameState->screenSize.x/2.f;
  float startY = gameState->screenSize.y/4.f;
  
  /* Main Menu Stuff */
  if (!resettingSizes) {
    gameState->mainMenu.fontSize = 40.f;
    gameState->mainMenu.startGameRectColor =       RAYWHITE;
    gameState->mainMenu.gotoOptionsMenuRectColor = RAYWHITE;
    gameState->mainMenu.exitGameRectColor =        RAYWHITE;
  }
  
  Vector2 size1 = MeasureTextEx(GetFontDefault(), gameState->gameText[START_GAME], 40.f, 1.f);
  Vector2 size2 = MeasureTextEx(GetFontDefault(), gameState->gameText[OPTIONS],    40.f, 1.f);
  Vector2 size3 = MeasureTextEx(GetFontDefault(), gameState->gameText[EXIT_GAME],  40.f, 1.f);
  
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
  if (!resettingSizes) {
    gameState->optionsMenu.fontSize = 40.f;
    gameState->optionsMenu.soundToggleRectColor =      RAYWHITE;
    gameState->optionsMenu.controlsMenuRectColor =     RAYWHITE;
    gameState->optionsMenu.goBackToMainMenuRectColor = RAYWHITE;
  }
  
  size1 = MeasureTextEx(GetFontDefault(), gameState->gameText[SOUND], 40.f, 1.f);
  size2 = MeasureTextEx(GetFontDefault(), gameState->gameText[CONTROLS], 40.f, 1.f);
  size3 = MeasureTextEx(GetFontDefault(), gameState->gameText[MAIN_MENU], 40.f, 1.f);
  
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


  /* Initial Mouse Position */
  gameState->mousePosition = GetMousePosition();
}

void
InitGameMap(bool resettingSize)
{
  /* Minimum Screen Size */
  /* if (gameState->screenSize.x < 800) { */
  /*   return; */
  /* } */
  /* if (gameState->screenSize.y < 800) { */
  /*   return; */
  /* } */
  
  if (!resettingSize) {
#if defined (PLATFORM_WEB)
    LoadCSVGameMap("gameMap.csv", gameState->gameMap);
#else
    LoadCSVGameMap("src/gameMap.csv", gameState->gameMap);
#endif	
  }

  // x scale factor 60
  // y scale factor = 33.47
  float tileWidth = gameState->screenSize.x / 10.f;
  float tileHeight = gameState->screenSize.y / 10.f;
  for (int y = 0; y < 10; y++) {
    for (int x = 0 ; x < 10; x++) {
      gameState->gameMap[y][x].tileRect = (Rectangle){x * tileWidth, y*tileHeight,tileWidth, tileHeight};
    }
  }
}

void
CreatePlayer(bool resettingSize)
{
  if (!resettingSize) {
    memset(player->inventory->items, 0, MAX_INVENTORY_ITEMS);
    memset(player->craftingInventory->items, 0, MAX_INVENTORY_ITEMS);
    player->size = (Vector2){32.f, 32.f};
    memset(&player->texture, 0, sizeof(Texture2D));
    player->recentInventoryOpened = 0;
  }

  player->inventory->rect = (Rectangle){0.f, 10.f, 800.f, 800.f};
  player->inventory->dragRect = (Rectangle){0.f, 10.f, 800.f, 10.f};
  player->craftingInventory->rect = (Rectangle){0.f, 10.f, 800.f, 800.f};
  player->craftingInventory->dragRect = (Rectangle){0.f, 10.f, 800.f, 10.f};
  player->inventory->dragging = false;
  player->craftingInventory->dragging = false;
}


void
UnloadGame()
{
#ifdef DEBUG
  printf("Freeing all memory.\n");
#endif
  
  free(player);
  for (int i = 0; i < 10; i++) {
    free(gameState->gameMap[i]);
  }
  free(gameState->gameMap);
  free(gameState);
}

void
UpdateScreenSize()
{
#if defined (PLATFORM_WEB)
  gameState->screenSize.x = CanvasGetWidth();
  gameState->screenSize.y = CanvasGetHeight();
#else
  gameState->screenSize.x = GetScreenWidth();
  gameState->screenSize.y = GetScreenHeight();
#endif
  if (gameState->screenSize.x != GetScreenWidth() ||
      gameState->screenSize.y != GetScreenHeight())
  {
    SetWindowSize(gameState->screenSize.x, gameState->screenSize.y);
    InitGame(true); // true - were resetting the size of all the rects
    InitGameMap(true); // true - were resetting the size of all the rects
  }
}

void
UpdateGame()
{
  UpdateScreenSize();

  gameState->previousMousePosition = gameState->mousePosition;
  gameState->mousePosition = GetMousePosition();
  
  if (gameState->gameActive) {
    UpdateGameMap();
  }
  else if (gameState->mainMenuActive) {
    UpdateMainMenu();
  }
  else if (gameState->optionsMenuActive) {
    UpdateOptionsMenu();
  }
  else if (gameState->controlsMenuActive) {
    UpdateControlsMenu();
  }

  /* OPEN CRAFTING */
  if (IsKeyPressed(KEY_I)) {
    if (gameState->inventoryActive) {
      gameState->inventoryActive = false;
    } else {
      player->recentInventoryOpened = 1;
      gameState->inventoryActive = true;
    }
  }
  /* OPEN CRAFTING */
  if (IsKeyPressed(KEY_C)) {
    if (gameState->craftingInventoryActive) {
      gameState->craftingInventoryActive = false;
    } else {
      player->recentInventoryOpened = 1;
      gameState->craftingInventoryActive = true;
    }
  }
  /*
    I want these to be able to run no matter what
     except for when main,options,controls menus are open
     not sure how to handle that yet
  */
  if (gameState->craftingInventoryActive) {
    UpdateCraftingScene();
  }
  if (gameState->inventoryActive) {
    UpdateInventory();
  }
}

void
RenderGame()
{
  BeginDrawing();
  {
    ClearBackground(RAYWHITE);
    //DrawRectangle(100, 100, player->size.x, player->size.y, PURPLE);
    if (gameState->gameActive) {
      RenderGameMap();
    }
    else if (gameState->mainMenuActive) {
      RenderMainMenu();
    }
    else if (gameState->optionsMenuActive) {
      RenderOptionsMenu();
    }
    else if (gameState->controlsMenuActive) {
      RenderControlsMenu();
    }
    /* These are able to run no matter what, same as in UpdateGame function */
    if (player->recentInventoryOpened == 1) {// crafting inventroy is open
      if (gameState->inventoryActive) {
	RenderInventory();
      }
      if (gameState->craftingInventoryActive) {
	RenderCraftingScene();
      }
    }
    else if (player->recentInventoryOpened == 0) { // basic inventory is open
      if (gameState->craftingInventoryActive) {
	RenderCraftingScene();
      }
      if (gameState->inventoryActive) {
        RenderInventory();
      }
    }
  }
  EndDrawing();
}

void
UpdateMainMenu()
{
  gameState->mainMenu.startGameRectColor =       RAYWHITE;
  gameState->mainMenu.gotoOptionsMenuRectColor = RAYWHITE;
  gameState->mainMenu.exitGameRectColor =        RAYWHITE;
  
  if (CheckCollisionPointRec(gameState->mousePosition, gameState->mainMenu.startGameRect)) {
    gameState->mainMenu.startGameRectColor = BLACK;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      gameState->mainMenuActive = false;
      gameState->gameActive = true;
    }
  }
  else if (CheckCollisionPointRec(gameState->mousePosition, gameState->mainMenu.gotoOptionsMenuRect)) {
    gameState->mainMenu.gotoOptionsMenuRectColor = BLACK;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      gameState->mainMenuActive = false;
      gameState->optionsMenuActive = true;
    }
  }
  else if (CheckCollisionPointRec(gameState->mousePosition, gameState->mainMenu.exitGameRect)) {
    gameState->mainMenu.exitGameRectColor = BLACK;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
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
  DrawTextEx(GetFontDefault(), gameState->gameText[START_GAME], gameState->mainMenu.startGameTextPosition,    gameState->mainMenu.fontSize, 1.f, BLACK);
  DrawTextEx(GetFontDefault(), gameState->gameText[OPTIONS], gameState->mainMenu.gotoOptionsMenuTextPosition, gameState->mainMenu.fontSize, 1.f, BLACK);
  DrawTextEx(GetFontDefault(), gameState->gameText[EXIT_GAME], gameState->mainMenu.exitGameTextPosition,      gameState->mainMenu.fontSize, 1.f, BLACK);
}

void
UpdateOptionsMenu()
{
  
  gameState->optionsMenu.controlsMenuRectColor =     RAYWHITE;
  gameState->optionsMenu.soundToggleRectColor =      RAYWHITE;
  gameState->optionsMenu.goBackToMainMenuRectColor = RAYWHITE;

  if (CheckCollisionPointRec(gameState->mousePosition, gameState->optionsMenu.soundToggleRect)) {
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
  else if (CheckCollisionPointRec(gameState->mousePosition, gameState->optionsMenu.controlsMenuRect)) {
    gameState->optionsMenu.controlsMenuRectColor = BLACK;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      gameState->optionsMenuActive = false;
      gameState->controlsMenuActive = true;
    }
  }
  else if (CheckCollisionPointRec(gameState->mousePosition, gameState->optionsMenu.goBackToMainMenuRect)) {
    gameState->optionsMenu.goBackToMainMenuRectColor = BLACK;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      gameState->optionsMenuActive = false;
      gameState->mainMenuActive = true;
    }
  }
}

void
RenderOptionsMenu()
{
  /* ABSOLUTELY CRUCIAL THINGS ARE DRAWN IN CORRECT ORDER */
  DrawRectangleLinesEx(gameState->optionsMenu.soundToggleRect,      1.f,  gameState->optionsMenu.soundToggleRectColor);
  DrawRectangleLinesEx(gameState->optionsMenu.controlsMenuRect,     1.f,  gameState->optionsMenu.controlsMenuRectColor);
  DrawRectangleLinesEx(gameState->optionsMenu.goBackToMainMenuRect, 1.f,  gameState->optionsMenu.goBackToMainMenuRectColor);
  DrawTextEx(GetFontDefault(), gameState->gameText[SOUND],     gameState->optionsMenu.soundToggleTextPosition,      gameState->optionsMenu.fontSize, 1.f, BLACK);
  DrawTextEx(GetFontDefault(), gameState->gameText[CONTROLS],  gameState->optionsMenu.controlsMenuTextPosition,     gameState->optionsMenu.fontSize, 1.f, BLACK);
  DrawTextEx(GetFontDefault(), gameState->gameText[MAIN_MENU], gameState->optionsMenu.goBackToMainMenuTextPosition, gameState->optionsMenu.fontSize, 1.f, BLACK);
}

void UpdateControlsMenu() {}
void RenderControlsMenu() {}

void UpdateCraftingScene()
{
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && player->craftingInventory->dragging) {
    Vector2 difference = Vector2Subtract(gameState->mousePosition, gameState->previousMousePosition);
    player->craftingInventory->rect.x += difference.x;
    player->craftingInventory->rect.y += difference.y;
    player->craftingInventory->dragRect.x += difference.x;
    player->craftingInventory->dragRect.y += difference.y;
  }
  
  if (CheckCollisionPointRec(gameState->mousePosition, player->craftingInventory->dragRect)) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      player->recentInventoryOpened = 1; // crafting inventory is open
      player->craftingInventory->dragging = true;
    } 
  }
  
  if (CheckCollisionPointRec(gameState->mousePosition, player->craftingInventory->rect)) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      player->recentInventoryOpened = 1;
    }
  }

  if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    player->craftingInventory->dragging = false;
  }
}

void RenderCraftingScene()
{
  DrawRectangleRec(player->craftingInventory->rect,     BROWN);
  DrawRectangleRec(player->craftingInventory->dragRect, BLACK);
}

void UpdateInventory()
{
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && player->inventory->dragging) {
    Vector2 difference = Vector2Subtract(gameState->mousePosition, gameState->previousMousePosition);
    player->inventory->rect.x += difference.x;
    player->inventory->rect.y += difference.y;
    player->inventory->dragRect.x += difference.x;
    player->inventory->dragRect.y += difference.y;
  }

  if (CheckCollisionPointRec(gameState->mousePosition, player->inventory->dragRect)) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      player->inventory->dragging = true;
      player->recentInventoryOpened = 0;
    }
  }
  
  if (CheckCollisionPointRec(gameState->mousePosition, player->inventory->rect)) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      player->recentInventoryOpened = 0;
    }
  }

  if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    player->inventory->dragging = false;
  }
}
void RenderInventory()
{
  DrawRectangleRec(player->inventory->rect,     PURPLE);
  DrawRectangleRec(player->inventory->dragRect, BLACK);
}

void UpdateGameMap()
{
  Vector2 mousePosition = GetMousePosition();
  for (int y = 0; y < 10; y++) {
    for (int x = 0; x < 10; x++) {
      if (CheckCollisionPointRec(mousePosition, gameState->gameMap[y][x].tileRect)) {
	gameState->gameMap[y][x].tileColor = RED;
      } else {
	gameState->gameMap[y][x].tileColor = BLACK;
      }
    }
  }
}

void
RenderGameMap()
{
  for (int y = 0; y < 10; y++) {
    for (int x = 0; x < 10; x++) {
      DrawRectangleLinesEx(gameState->gameMap[y][x].tileRect, 1.f, gameState->gameMap[y][x].tileColor);
    }
  }
}

void
LoadCSVGameMap(const char* path, GameMapTile** mapBuffer)
{
  FILE* file = fopen(path, "r");
  if (!file) {
#ifdef DEBUG
    printf("Failed to open map csv file.\n");
    exit(1);
#endif
  }

  /* This will be based on size of map*/
  char line[25];
  char* linePtr;
  int x = 0;
  int y = 0;
  int data = 0;
  
  while (fgets(line, sizeof(line), file)) {
    linePtr = &line[0];
    
    while(*linePtr != '\n') {
      
      if (*linePtr != ',' && *linePtr != ' ') {
	data = data * 10 + (*linePtr - 48);
      }
      else if (*linePtr == ',') {
	mapBuffer[y][x++].type = data;
	data = 0;
      }
      
      linePtr++;
    }
    
    mapBuffer[y][x].type = data;
    data = 0;
    y++;
    x=0;
  }

  fclose(file);
  
#ifdef DEBUG
  printf("Map loaded.\n");
#endif
}
