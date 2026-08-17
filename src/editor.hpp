/*-------------------------------------------------------------------------------

	BARONY
	File: editor.hpp
	Desc: header file for the editor

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#pragma once

#include <vector>
#include <string>

#include "../odin/containers/dynamic_map.hpp"

#include "draw.hpp"

static const unsigned int MAXWIDTH = 2000;
static const unsigned int MAXHEIGHT = 2000;
static const unsigned int MINWIDTH = 1;
static const unsigned int MINHEIGHT =  1;

extern DynamicMapI32Str modelFileNames;
extern int drawlayer, drawx, drawy, odrawx, odrawy;
extern int alllayers;
extern int scroll;
extern char layerstatus[20];
extern int menuVisible;
extern int subwindow;
extern int subx1, subx2, suby1, suby2;
extern char subtext[1024];
extern int toolbox;
extern int statusbar;
extern int viewsprites;
extern int showgrid;
extern int hovertext;
extern int selectedTile;
extern int tilepalette;
extern int spritepalette;
extern int mclick;
extern int selectedTool; // 0: Pencil 1: Point 2: Brush 3: Select 4: Fill
extern int allowediting; // only turned on when the mouse is over paintable screen region
extern int openwindow, savewindow, newwindow;
extern int slidery, slidersize;
extern int menuDisappear;
extern int selectedFile;
extern DynamicArrayStr mapNames;
extern DynamicArrayStr modFolderNames;
extern std::string physfs_saveDirectory;
extern std::string physfs_openDirectory;
extern char filename[128];
extern char foldername[128];
extern char oldfilename[128];
extern int messagetime;
extern char widthtext[4], heighttext[4], nametext[32], authortext[32], skyboxtext[4];
extern int editproperty;
extern SDL_Cursor* cursorArrow, *cursorPencil, *cursorPoint, *cursorBrush, *cursorSelect, *cursorFill;
extern int* palette;

// button definitions
extern button_t* butX;
extern button_t* but_;
extern button_t* butTilePalette;
extern button_t* butSprite;
extern button_t* butPencil;
extern button_t* butPoint;
extern button_t* butBrush;
extern button_t* butSelect;
extern button_t* butFill;
extern button_t* butFile;
extern button_t* butNew;
extern button_t* butOpen;
extern button_t* butDir;
extern button_t* butSave;
extern button_t* butSaveAs;
extern button_t* butExit;
extern button_t* butEdit;
extern button_t* butCut;
extern button_t* butCopy;
extern button_t* butPaste;
extern button_t* butDelete;
extern button_t* butSelectAll;
extern button_t* butUndo;
extern button_t* butRedo;
extern button_t* butView;
extern button_t* butToolbox;
extern button_t* butStatusBar;
extern button_t* butAllLayers;
extern button_t* butHoverText;
extern button_t* butViewSprites;
extern button_t* butGrid;
extern button_t* butFullscreen;
extern button_t* butMap;
extern button_t* butAttributes;
extern button_t* butClearMap;
extern button_t* butHelp;
extern button_t* butAbout;
extern button_t* butEditorControls;
extern button_t* but3DMode;

extern int menuVisible;
extern int toolbox;
extern int statusbar;
extern int viewsprites;
extern int alllayers;
extern int showgrid;
extern int hovertext;
extern int selectedTile;
extern int tilepalette;
extern int spritepalette;
extern int selectedTool;
extern int openwindow, savewindow, newwindow;
extern int slidery;
extern int slidersize;
extern int selectedFile;
extern int messagetime;
extern char message[256];
extern Uint32 cursorflash;
extern char widthtext[4], heighttext[4], nametext[32], authortext[32], skyboxtext[4];
extern char mapflagtext[MAPFLAGTEXTS][32];
extern char spriteProperties[32][128];
extern char tmpSpriteProperties[32][128];
extern int editproperty;
extern bool mode3d;
extern bool selectingspace;
extern int selectedarea_x1, selectedarea_x2;
extern int selectedarea_y1, selectedarea_y2;
extern bool selectedarea;
extern bool pasting;
extern map_t copymap;
extern list_t undolist;

// fps
extern bool showfps;
extern real_t t, ot, frameval[AVERAGEFRAMES];
extern Uint32 cycles, pingtime;
extern Uint32 timesync;
extern real_t fps;

extern Sint32 ocamx, ocamy;

// undo/redo funcs
extern "C" void makeUndo();
extern "C" void clearUndos();
extern "C" void undo();
extern "C" void redo();

// function prototypes for buttons.c:
void buttonExit(button_t* my);
extern "C" void buttonExitConfirm(button_t* my);
extern "C" void buttonIconify(button_t* my);
extern "C" void buttonTilePalette(button_t* my);
extern "C" void buttonSprite(button_t* my);
extern "C" void buttonPencil(button_t* my);
extern "C" void buttonPoint(button_t* my);
extern "C" void buttonBrush(button_t* my);
extern "C" void buttonSelect(button_t* my);
extern "C" void buttonFill(button_t* my);
extern "C" void buttonFile(button_t* my);
extern "C" void buttonNewConfirm(button_t* my);
extern "C" void buttonNew(button_t* my);
extern "C" void buttonOpenConfirm(button_t* my);
extern "C" void buttonOpen(button_t* my);
extern "C" void buttonSaveConfirm(button_t* my);
extern "C" void buttonSave(button_t* my);
extern "C" void buttonSaveAs(button_t* my);
void buttonExit(button_t* my);
extern "C" void buttonEdit(button_t* my);
extern "C" void buttonCut(button_t* my);
extern "C" void buttonCopy(button_t* my);
extern "C" void buttonPaste(button_t* my);
extern "C" void buttonDelete(button_t* my);
extern "C" void buttonCycleSprites(button_t* my);
extern "C" void buttonSelectAll(button_t* my);
extern "C" void buttonUndo(button_t* my);
extern "C" void buttonRedo(button_t* my);
extern "C" void buttonView(button_t* my);
extern "C" void buttonToolbox(button_t* my);
extern "C" void buttonStatusBar(button_t* my);
extern "C" void buttonAllLayers(button_t* my);
extern "C" void buttonViewSprites(button_t* my);
extern "C" void buttonGrid(button_t* my);
extern "C" void button3DMode(button_t* my);
extern "C" void buttonHoverText(button_t* my);
extern "C" void buttonMap(button_t* my);
extern "C" void buttonAttributes(button_t* my);
extern "C" void buttonAttributesConfirm(button_t* my);
extern "C" void buttonClearMap(button_t* my);
extern "C" void buttonClearMapConfirm(button_t* my);
extern "C" void buttonHelp(button_t* my);
extern "C" void buttonAbout(button_t* my);
extern "C" void buttonEditorToolsHelp(button_t* my);
extern "C" void buttonEditorControls(button_t* my);
extern "C" void buttonCloseSubwindow(button_t* my);
extern "C" void buttonSpriteProperties(button_t* my);
extern "C" void buttonSpritePropertiesConfirm(button_t* my);
extern "C" void buttonCloseSpriteSubwindow(button_t* my);
extern "C" void buttonMonsterItems(button_t* my);
extern "C" void initMonsterPropertiesWindow();
extern "C" void buttonOpenDirectory(button_t* my);
extern "C" void buttonOpenPrevMap(button_t* my);
extern "C" void buttonOpenNextMap(button_t* my);

extern char itemName[128];
extern int itemSelect;
extern int itemSlotSelected;
extern "C" int loadItems();
extern "C" void buttonStartSingleplayer(button_t* my);
extern "C" void steamStatisticUpdate(int statisticNum, ESteamStatTypes type, int value);
class AchievementObserver
{
public:
	void updateGlobalStat(int index, int value); // dummy :<
};
extern AchievementObserver achievementObserver;
extern "C" void propertyPageTextAndInput(int numProperties, int width);
extern "C" void propertyPageError(int rowIndex, int resetValue);
extern "C" void propertyPageCursorFlash(int rowSpacing);
extern "C" void reselectEntityGroup(); // selects group of entities within current selection
#define TICKS_PER_SECOND 50
extern view_t camera;
