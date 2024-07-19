#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../raylibIncludes/raylib.h"

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
  bool craftingSceneActive;
  bool inventoryActive;
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
  const char* name;
  Rectangle rect;
  Texture2D texture;
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
} Player;


/* OBJECTS */
GameState* gameState;
Player* player;
const char* gameText[9]; // not sure what final size of this will be

/* INITIALIZATION */
void AllocateGame();
void InitGame(bool resettingSizes);
void CreatePlayer();

/* GENERAL FUNCIONS THAT CONTROL THE FLOW OF THE GAME */
void UnloadGame();
void RenderGame();
void UpdateGame();

/* UPDATE FUNCTIONS */
void UpdateScreenSize();
void UpdateMainMenu();
void UpdateOptionsMenu();
void UpdateControlsMenu();
void UpdateCraftingScene();
void UpdateInventory();
  
/* RENDER FUNCTIONS */
void RenderMainMenu();
void RenderOptionsMenu();
void RenderControlsMenu();
void RenderCraftingScene();
void RenderInventory();

int
main()
{
  printf("Game Start/n");

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
AllocateGame()
{
  gameText[START_GAME] = "Start Game";
  gameText[OPTIONS] = "Options";
  gameText[EXIT_GAME] = "Exit Game";
  gameText[SOUND] = "Sound";
  gameText[CONTROLS] = "Controls";
  gameText[MAIN_MENU] = "Main Menu";
  gameText[INVENTORY] = "Inventory";
  gameText[CRAFTING] = "Crafting";
  gameText[MAP] = "Map";

  /* ALLOCATTE MEMORY FOR ALL POINTERS FIRST */
  gameState = malloc(sizeof(GameState));
  if (!gameState) {
    // failure - exit game
#ifdef DEBUG
    printf("Failed to allocate GameState memory.\n");
#endif
    exit(1);
  }
  
  gameState->running = true;
  gameState->screenSize = (Vector2i){1920,1080};
  gameState->mainMenuActive = true;
  gameState->battleSceneActive = false;
  gameState->mapSceneActive = false;
  gameState->optionsMenuActive = false;
  gameState->controlsMenuActive = false;

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
  gameState->battleScene.defaultRectsBuffer = malloc(sizeof(Rectangle)*DEFAULT_BATTLE_SCENE_RECTS_COUNT);
  if (!gameState->battleScene.defaultRectsBuffer) {
#ifdef DEBUG
    printf("Failed to allocate battle scene default buffer.\n");
    exit(1);
#endif
  }
  gameState->battleScene.extraRectsBuffer = malloc(sizeof(Rectangle));
  if (!gameState->battleScene.extraRectsBuffer) {
#ifdef DEBUG
    printf("Failed to allocate battle scene extra buffer.\n");
    exit(1);
#endif
  }
}

void
InitGame(bool resettingSizes)
{
  /* Minimum Screen Size */
  if (gameState->screenSize.x < 800) {
    return;
  }
  if (gameState->screenSize.y < 800) {
    return;
  }
  
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
  
  Vector2 size1 = MeasureTextEx(GetFontDefault(), gameText[START_GAME], 40.f, 1.f);
  Vector2 size2 = MeasureTextEx(GetFontDefault(), gameText[OPTIONS],    40.f, 1.f);
  Vector2 size3 = MeasureTextEx(GetFontDefault(), gameText[EXIT_GAME],  40.f, 1.f);
  
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
  
  size1 = MeasureTextEx(GetFontDefault(), gameText[SOUND], 40.f, 1.f);
  size2 = MeasureTextEx(GetFontDefault(), gameText[CONTROLS], 40.f, 1.f);
  size3 = MeasureTextEx(GetFontDefault(), gameText[MAIN_MENU], 40.f, 1.f);
  
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
  if (resettingSizes) {
    CreateGameMap(true);
  } else {
    CreateGameMap(false);
  }

  if (resettingSizes) {
    InitBattleScene(true);
  } else {
    InitBattleScene(false);
  }
}

void
InitBattleScene(bool resettingSizes)
{
  gameState->battleScene.fontSize = 40.f;
  /* Minimum Screen Size */
  if (gameState->screenSize.x < 800) {
    return;
  }
  if (gameState->screenSize.y < 800) {
    return;
  }
  
  // x scale percentage = 19.6
  // y scale percentage = 5.4
  /* health bar(s) positions */
  float healthBarWidth = (float)gameState->screenSize.x / 3.f;
  float healthBarHeight = (float)gameState->screenSize.y / 14.f;
  float startXleft = gameState->screenSize.x / 19.6f;
  float startYleft = (float)gameState->screenSize.y - 200.f;
  float startXright = (float)gameState->screenSize.x - healthBarWidth - 100.f;
  float startYright = gameState->screenSize.y / 5.4f;
  
  Rectangle healthBarOne = (Rectangle){startXleft, startYleft, healthBarWidth, healthBarHeight};
  Rectangle healthBarTwo = (Rectangle){startXright, startYright, healthBarWidth, healthBarHeight};

  gameState->battleScene.defaultRectsBuffer[0] = healthBarOne;
  gameState->battleScene.defaultRectsBuffer[1] = healthBarTwo;

  if (!resettingSizes) {
    gameState->battleScene.gotoInventoryRectColor = RAYWHITE;
    gameState->battleScene.gotoCraftingSceneColor = RAYWHITE;
    gameState->battleScene.gotoMapSceneRectColor =  RAYWHITE;
  }

  Vector2 size1 = MeasureTextEx(GetFontDefault(), gameText[INVENTORY], 40.f, 1.f);
  Vector2 size2 = MeasureTextEx(GetFontDefault(), gameText[CRAFTING],  40.f, 1.f);
  Vector2 size3 = MeasureTextEx(GetFontDefault(), gameText[MAP],       40.f, 1.f);

  gameState->battleScene.gotoInventoryRect =     (Rectangle){100, 10, size1.x, size1.y};
  gameState->battleScene.gotoCraftingSceneRect = (Rectangle){gameState->battleScene.gotoInventoryRect.x + gameState->battleScene.gotoInventoryRect.width + 10.f,
							     10, size2.x, size2.y};
  gameState->battleScene.gotoMapSceneRect =      (Rectangle){gameState->battleScene.gotoCraftingSceneRect.x + gameState->battleScene.gotoCraftingSceneRect.width + 10.f,
							     10, size3.x, size3.y};

  gameState->battleScene.gotoInventoryTextPosition =     (Vector2){gameState->battleScene.gotoInventoryRect.x,     gameState->battleScene.gotoInventoryRect.y};
  gameState->battleScene.gotoCraftingSceneTextPosition = (Vector2){gameState->battleScene.gotoCraftingSceneRect.x, gameState->battleScene.gotoCraftingSceneRect.y};
  gameState->battleScene.gotoMapSceneTextPosition =      (Vector2){gameState->battleScene.gotoMapSceneRect.x,      gameState->battleScene.gotoMapSceneRect.y};

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
CreateGameMap(bool resettingSizes)
{
  // x scale percentage = 9.6
  // y scale percentage = 5.4
  float startX = gameState->screenSize.x/2.f;
  float currentX = startX;
  float offsetX = gameState->screenSize.x / 9.6f;
  float startY =  (float)gameState->screenSize.y - 180.f;
  float offsetY = gameState->screenSize.y / 5.4f;
  
  int i = 0;
  for (i = 0; i < DEFAULT_MAP_SIZE; i++) {  
    currentX = startX;
    if ((i+1) % 2 == 0) {
      currentX = startX + offsetX;
      offsetX *= -1.f;
    }
    
    gameMap->gameMapNodes[i].nodePositionRect = (Rectangle){currentX - 50.f, startY, 100.f, 100.f};
    startY -= offsetY;
    
    if (!resettingSizes) {
      gameMap->gameMapNodes[i].data=0;
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
  }
  
  if (!resettingSizes) {
    gameMap->gameMapSize = DEFAULT_MAP_SIZE;
    gameMap->playerPositionInGameMap = 0;
    /* Set to -1 to start */
    gameMap->mouseHoverIndex = -1;
  }
}

void
UnloadGame()
{
#ifdef DEBUG
  printf("Freeing all memory.\n");
#endif
  
  free(player);

  free(gameMap->gameMapNodes);
  free(gameMap);

  free(gameState->battleScene.defaultRectsBuffer);
  free(gameState->battleScene.extraRectsBuffer);

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
    InitBattleScene(true); // true - were resetting the size of all the rects
  }
}

void
UpdateGame()
{
  UpdateScreenSize();
  
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

  /*
    I want these to be able to run no matter what
     except for when main,options,controls menus are open
     not sure how to handle that yet
  */
  if (gameState->craftingSceneActive) {
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

    /* These are able to run no matter what, same as in UpdateGame function */
    if (gameState->craftingSceneActive) {
      RenderCraftingScene();
    }
    if (gameState->inventoryActive) {
      RenderInventory();
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
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      gameState->mainMenuActive = false;
      gameState->mapSceneActive = true;
    }
  }
  else if (CheckCollisionPointRec(mousePosition, gameState->mainMenu.gotoOptionsMenuRect)) {
    gameState->mainMenu.gotoOptionsMenuRectColor = BLACK;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      gameState->mainMenuActive = false;
      gameState->optionsMenuActive = true;
    }
  }
  else if (CheckCollisionPointRec(mousePosition, gameState->mainMenu.exitGameRect)) {
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
  DrawTextEx(GetFontDefault(), gameText[START_GAME], gameState->mainMenu.startGameTextPosition,    gameState->mainMenu.fontSize, 1.f, BLACK);
  DrawTextEx(GetFontDefault(), gameText[OPTIONS], gameState->mainMenu.gotoOptionsMenuTextPosition, gameState->mainMenu.fontSize, 1.f, BLACK);
  DrawTextEx(GetFontDefault(), gameText[EXIT_GAME], gameState->mainMenu.exitGameTextPosition,      gameState->mainMenu.fontSize, 1.f, BLACK);
}

void
UpdateOptionsMenu()
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

void
RenderOptionsMenu()
{
  /* ABSOLUTELY CRUCIAL THINGS ARE DRAWN IN CORRECT ORDER */
  DrawRectangleLinesEx(gameState->optionsMenu.soundToggleRect,      1.f,  gameState->optionsMenu.soundToggleRectColor);
  DrawRectangleLinesEx(gameState->optionsMenu.controlsMenuRect,     1.f,  gameState->optionsMenu.controlsMenuRectColor);
  DrawRectangleLinesEx(gameState->optionsMenu.goBackToMainMenuRect, 1.f,  gameState->optionsMenu.goBackToMainMenuRectColor);
  DrawTextEx(GetFontDefault(), gameText[SOUND],     gameState->optionsMenu.soundToggleTextPosition,      gameState->optionsMenu.fontSize, 1.f, BLACK);
  DrawTextEx(GetFontDefault(), gameText[CONTROLS],  gameState->optionsMenu.controlsMenuTextPosition,     gameState->optionsMenu.fontSize, 1.f, BLACK);
  DrawTextEx(GetFontDefault(), gameText[MAIN_MENU], gameState->optionsMenu.goBackToMainMenuTextPosition, gameState->optionsMenu.fontSize, 1.f, BLACK);
}

void UpdateControlsMenu() {}
void RenderControlsMenu() {}

void UpdateBattleScene()
{
  Vector2 mousePosition = GetMousePosition();

  gameState->battleScene.gotoInventoryRectColor = RAYWHITE;
  gameState->battleScene.gotoCraftingSceneColor = RAYWHITE;
  gameState->battleScene.gotoMapSceneRectColor =  RAYWHITE;
  
  if (CheckCollisionPointRec(mousePosition, gameState->battleScene.gotoInventoryRect)) {
    gameState->battleScene.gotoInventoryRectColor = BLACK;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      gameState->inventoryActive = true;
    }
  }
  else if (CheckCollisionPointRec(mousePosition, gameState->battleScene.gotoCraftingSceneRect)) {
    gameState->battleScene.gotoCraftingSceneColor = BLACK;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      gameState->craftingSceneActive = true;
    }
  }
  else if (CheckCollisionPointRec(mousePosition, gameState->battleScene.gotoMapSceneRect)) {
    gameState->battleScene.gotoMapSceneRectColor = BLACK;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      gameState->battleSceneActive = false;
      gameState->mapSceneActive = true;
    }
  }
}

void RenderBattleScene()
{
  for (int i = 0; i < DEFAULT_BATTLE_SCENE_RECTS_COUNT; i++) {
    DrawRectangleRec(gameState->battleScene.defaultRectsBuffer[i], GREEN);  
  }

  DrawRectangleLinesEx(gameState->battleScene.gotoInventoryRect,     1.f, gameState->battleScene.gotoInventoryRectColor);
  DrawRectangleLinesEx(gameState->battleScene.gotoCraftingSceneRect, 1.f, gameState->battleScene.gotoCraftingSceneColor);
  DrawRectangleLinesEx(gameState->battleScene.gotoMapSceneRect,      1.f, gameState->battleScene.gotoMapSceneRectColor);
  DrawTextEx(GetFontDefault(), gameText[INVENTORY], gameState->battleScene.gotoInventoryTextPosition,     gameState->battleScene.fontSize, 1.f, BLACK);
  DrawTextEx(GetFontDefault(), gameText[CRAFTING],  gameState->battleScene.gotoCraftingSceneTextPosition, gameState->battleScene.fontSize, 1.f, BLACK);
  DrawTextEx(GetFontDefault(), gameText[MAP],       gameState->battleScene.gotoMapSceneTextPosition,      gameState->battleScene.fontSize, 1.f, BLACK);
}

void UpdateCraftingScene() {}
void RenderCraftingScene() {}

void UpdateInventory() {}
void RenderInventory() {}

void
UpdateMapScene()
{
  Vector2 mousePosition = GetMousePosition();

  gameMap->mouseHoverIndex = -1;

  for (int i = 0; i < gameMap->gameMapSize; i++) {

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
	gameState->mapSceneActive = false;
	gameState->battleSceneActive = true;
      }
    }
  }
}

void
RenderMapScene()
{
  for (int i = 0; i < gameMap->gameMapSize; i++) {
    DrawRectangleRec(gameMap->gameMapNodes[i].nodePositionRect, gameMap->gameMapNodes[i].nodePositionRectColor);
  }
  
  if (gameMap->mouseHoverIndex >= 0 && gameMap->mouseHoverIndex < gameMap->gameMapSize) {
    DrawRectangleLinesEx(gameMap->gameMapNodes[gameMap->mouseHoverIndex].nodePositionRect, 5.f, YELLOW);
  }
}
