/*-------------------------------------------------------------------------------

	BARONY
	File: menu.hpp
	Desc: definitions and prototypes for menu.c

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#pragma once

#define NUMSUBTITLES 30
extern int subtitleCurrent;
extern bool subtitleVisible;

extern Sint32 gearrot;
extern Sint32 gearsize;
extern Uint16 logoalpha;
extern int credittime;
extern int creditstage;
extern int intromovietime;
extern int intromoviestage;
extern int firstendmovietime;
extern int firstendmoviestage;
extern int secondendmovietime;
extern int secondendmoviestage;
extern int thirdendmoviestage;
extern int thirdendmovietime;
extern int fourthendmoviestage;
extern int fourthendmovietime;
extern int DLCendmovieStageAndTime[8][2];
enum NewMovieStageAndTimeIndex : int
{
	MOVIE_STAGE,
	MOVIE_TIME,
};
enum NewMovieCrawlTypes : int
{
	MOVIE_MIDGAME_HERX_MONSTERS,
	MOVIE_MIDGAME_BAPHOMET_MONSTERS,
	MOVIE_MIDGAME_BAPHOMET_HUMAN_AUTOMATON,
	MOVIE_CLASSIC_WIN_MONSTERS,
	MOVIE_CLASSIC_WIN_BAPHOMET_MONSTERS,
	MOVIE_WIN_AUTOMATON,
	MOVIE_WIN_DEMONS_UNDEAD,
	MOVIE_WIN_BEASTS
};
extern bool losingConnection[MAXPLAYERS];
extern int rebindaction;

// button definitions
extern "C" void buttonQuitConfirm(button_t* my);
extern "C" void buttonQuitNoSaveConfirm(button_t* my);
extern "C" void buttonEndGameConfirm(button_t* my);
extern "C" void buttonCloseAndEndGameConfirm(button_t* my);
extern "C" void buttonCloseSubwindow(button_t* my);
extern "C" void buttonContinue(button_t* my);
extern "C" void buttonBack(button_t* my);
extern "C" void buttonVideoTab(button_t* my);
extern "C" void buttonAudioTab(button_t* my);
extern "C" void buttonKeyboardTab(button_t* my);
extern "C" void buttonMouseTab(button_t* my);
extern "C" void buttonGamepadBindingsTab(button_t* my);
extern "C" void buttonGamepadSettingsTab(button_t* my);
extern "C" void buttonMiscTab(button_t* my);
extern "C" void buttonSettingsAccept(button_t* my);
extern "C" void buttonSettingsOK(button_t* my);
extern "C" void buttonStartSingleplayer(button_t* my);
extern "C" void buttonStartServer(button_t* my);
extern "C" void buttonHostMultiplayer(button_t* my);
extern "C" void buttonJoinMultiplayer(button_t* my);
extern "C" void buttonHostLobby(button_t* my);
extern "C" void buttonJoinLobby(button_t* my);
extern "C" void buttonDisconnect(button_t* my);
extern "C" void buttonScoreNext(button_t* my);
extern "C" void buttonScorePrev(button_t* my);
extern "C" void buttonScoreToggle(button_t* my);
extern "C" void buttonOpenCharacterCreationWindow(button_t* my);
extern "C" void buttonDeleteSavedSoloGame(button_t* my);
extern "C" void buttonDeleteSavedMultiplayerGame(button_t* my);
extern "C" void buttonConfirmDeleteSoloFile(button_t* my);
extern "C" void buttonConfirmDeleteMultiplayerFile(button_t* my);
extern "C" void buttonLoadSingleplayerGame(button_t* my);
extern "C" void buttonLoadMultiplayerGame(button_t* my);
extern "C" void buttonRandomCharacter(button_t* my);
extern "C" bool replayLastCharacter(const int index, int multiplayer);
extern "C" void buttonRandomName(button_t* my);
extern "C" void buttonGamemodsOpenDirectory(button_t* my);
extern "C" void buttonGamemodsPrevDirectory(button_t* my);
extern "C" void buttonGamemodsBaseDirectory(button_t* my);
extern "C" void buttonGamemodsSelectDirectoryForUpload(button_t* my);
extern "C" void buttonGamemodsOpenModifyExistingWindow(button_t* my);
extern "C" void buttonGamemodsStartModdedGame(button_t* my);
extern "C" void buttonInviteFriends(button_t* my);

extern "C" void windowEnterSerialPrompt();
extern "C" void windowSerialResult(int success);
extern "C" size_t serialHash(const std::string& input);
extern char serialInputText[64];

#define SLIDERFONT font12x12_bmp

// achievement window
extern "C" void openAchievementsWindow();
extern "C" void closeAchievementsWindow(button_t*);
extern bool achievements_window;
extern int achievements_window_page;
extern "C" void buttonAchievementsUp(button_t* my);
extern "C" void buttonAchievementsDown(button_t* my);

// misc functions
extern "C" void openSettingsWindow();
extern "C" void openFailedConnectionWindow(int mode);
extern "C" void openSteamLobbyBrowserWindow(button_t* my);
extern "C" void openLoadGameWindow(button_t* my);
extern "C" void openNewLoadGameWindow(button_t* my);
extern "C" void doSlider(int x, int y, int dots, int minvalue, int maxvalue, int increment, int* var, SDL_Surface* slider_font = SLIDERFONT, int slider_font_char_width = 16);
extern "C" void doSliderF(int x, int y, int dots, real_t minvalue, real_t maxvalue, real_t increment, real_t* var);

// menu variables
extern bool settings_window;
extern int charcreation_step;
extern int loadGameSaveShowRectangle;
extern Uint32 charcreation_ticks;
extern bool playing_random_char;
extern int settings_tab;
extern int connect_window;
extern bool lobby_window;
extern int score_window;
extern Uint32 lobbyWindowSvFlags;

// gamemods window stuff
extern int gamemods_window;
extern int gamemods_window_scroll;
extern int gamemods_window_fileSelect;
extern int gamemods_uploadStatus;
//extern int gamemods_numCurrentModsLoaded;
extern DynamicArrayStr currentDirectoryFiles;
extern DynamicString directoryPath;
extern "C" void gamemodsWindowClearVariables();
extern "C" void gamemodsCustomContentInit();
extern "C" bool gamemodsDrawClickableButton(int padx, int pady, int padw, int padh, Uint32 btnColor, std::string btnText, int action);
extern "C" bool gamemodsRemovePathFromMountedFiles(std::string findStr);
extern "C" bool gamemodsIsPathInMountedFiles(std::string findStr);
extern "C" bool gamemodsClearAllMountedPaths();
extern "C" bool gamemodsMountAllExistingPaths();
//extern std::vector<std::pair<std::string, std::string>> gamemods_mountedFilepaths;
//extern bool gamemods_modelsListRequiresReload;
//extern bool gamemods_soundListRequiresReload;
//extern bool gamemods_modPreload;
extern "C" bool drawClickableButton(int padx, int pady, int padw, int padh, Uint32 btnColor);
extern bool scoreDisplayMultiplayer;
struct SaveGameListEntry_t {
	int lastModified = 0;
	int multiplayerType = 0;
	int fileEntry = 0;
	DynamicString description;
};
template <> struct DynamicArrayKindOf<SaveGameListEntry_t> { static constexpr int value = Kind_SaveGameListEntry; };
using DynamicArraySaveGameList = DynamicArrayT<SaveGameListEntry_t>;
extern DynamicArraySaveGameList savegamesList; // last modified, multiplayer type, file entry, description

extern Sint32 slidery, slidersize, oslidery;

// settings window
extern Uint32 settings_fov;
extern Uint32 settings_fps;
extern int settings_xres, settings_yres;
extern bool settings_smoothlighting;
extern int settings_fullscreen, settings_shaking, settings_bobbing;
extern bool settings_borderless;
extern real_t settings_gamma;
extern int settings_sfxvolume, settings_musvolume;
extern int settings_sfxAmbientVolume;
extern int settings_sfxEnvironmentVolume;
extern int settings_impulses[NUMIMPULSES];
extern int settings_reversemouse;
extern bool settings_smoothmouse;
extern bool settings_disablemouserotationlimit;
extern real_t settings_mousespeed;
extern bool settings_broadcast;
extern bool settings_nohud;
extern bool settings_colorblind;
extern bool settings_spawn_blood;
extern bool settings_light_flicker;
extern bool settings_vsync;
extern bool settings_status_effect_icons;
extern bool settings_minimap_ping_mute;
extern bool settings_mute_audio_on_focus_lost;
extern bool settings_mute_player_monster_sounds;
extern int settings_minimap_transparency_foreground;
extern int settings_minimap_transparency_background;
extern int settings_minimap_scale;
extern int settings_minimap_object_zoom;
extern char portnumber_char[6];
extern char connectaddress[64];
extern bool usecamerasmoothing;
extern bool disablemouserotationlimit;
extern bool broadcast;
extern bool nohud;
extern int menuselect;
extern bool colorblind;
extern bool right_click_protect;
extern bool settings_auto_hotbar_new_items;
extern bool settings_auto_hotbar_categories[NUM_HOTBAR_CATEGORIES];
extern int settings_autosort_inventory_categories[NUM_AUTOSORT_CATEGORIES];
extern bool settings_hotbar_numkey_quick_add;
extern bool settings_disable_messages;
extern bool settings_right_click_protect;
extern bool settings_auto_appraise_new_items;
extern bool settings_lock_right_sidebar;
extern bool settings_show_game_timer_always;
extern bool settings_uiscale_charactersheet;
extern bool settings_uiscale_skillspage;
extern real_t settings_uiscale_hotbar;
extern real_t settings_uiscale_playerbars;
extern real_t settings_uiscale_chatlog;
extern real_t settings_uiscale_inventory;
extern bool settings_hide_statusbar;
extern bool settings_hide_playertags;
extern bool settings_show_skill_values;
extern bool settings_disableMultithreadedSteamNetworking;
extern bool settings_disableFPSLimitOnNetworkMessages;

static const int NUM_SETTINGS_TABS = 7;

static const int SETTINGS_VIDEO_TAB = 0;
static const int SETTINGS_AUDIO_TAB = 1;
static const int SETTINGS_KEYBOARD_TAB = 2;
static const int SETTINGS_MOUSE_TAB = 3;
static const int SETTINGS_GAMEPAD_BINDINGS_TAB = 4;
static const int SETTINGS_GAMEPAD_SETTINGS_TAB = 5;
static const int SETTINGS_MISC_TAB = 6;


//Confirm resolution window stuff.
extern bool resolutionChanged;
extern bool confirmResolutionWindow;
extern int resolutionConfirmationTimer;
static const int RESOLUTION_CONFIRMATION_TIME = 10000; //Time in milliseconds before resolution reverts.
extern Sint32 oldXres;
extern Sint32 oldYres;
extern button_t* revertResolutionButton;

extern "C" int getNumDisplays();
struct resolution {
	int x;
	int y;
	int hz;

	bool operator==(const resolution& rhs) const {
		return x == rhs.x && y == rhs.y && hz == rhs.hz;
	}
};
extern "C" void getResolutionList(int device_id, DynamicArrayT<resolution>&);
extern "C" void applySettings();
extern "C" void openConfirmResolutionWindow();
extern "C" void buttonAcceptResolution(button_t* my);
extern "C" void buttonRevertResolution(button_t* my);
extern "C" void revertResolution();

class Stat;
int isCharacterValidFromDLC(Stat& myStats, int characterClass);
int isCharacterValidFromDLCDirect(int player, int characterClass, int race, int appearance);

// handle intro stage stuff
extern "C" void doQuitGame();
extern "C" void doNewGame(bool makeHighscore);
extern "C" void doCredits();
extern "C" void doEndgame(bool saveHighscore, bool onServerDisconnect);
extern "C" void doEndgameOnDisconnect();
extern "C" void doIntro();
extern "C" void doEndgameHerx();
extern "C" void doEndgameDevil();
extern "C" void doMidgame();
extern "C" void doEndgameCitadel();
extern "C" void doEndgameClassicAndExtraMidGame();
extern "C" void doEndgameExpansion();

enum CharacterDLCValidation : int
{
	INVALID_CHARACTER,
	VALID_OK_CHARACTER,
	INVALID_REQUIREDLC1,
	INVALID_REQUIREDLC2,
	INVALID_REQUIRE_ACHIEVEMENT,
	INVALID_REQUIREDLC3
};

struct LastCreatedCharacter {
	static const int NUM_LAST_CHARACTERS = 6;
	static const int LASTCHAR_LAN_PERSONA_INDEX = 4;
	static const int LASTCHAR_ONLINE_PERSONA_INDEX = 5;
	int characterClass[NUM_LAST_CHARACTERS];
	int characterAppearance[NUM_LAST_CHARACTERS];
	int characterSex[NUM_LAST_CHARACTERS];
	int characterRace[NUM_LAST_CHARACTERS];
	DynamicString characterName[NUM_LAST_CHARACTERS];
	LastCreatedCharacter()
	{
		for ( int i = 0; i < NUM_LAST_CHARACTERS; ++i )
		{
			characterClass[i] = -1;
			characterAppearance[i] = -1;
			characterSex[i] = -1;
			characterRace[i] = -1;
			characterName[i] = "";
		}
	}
};
extern LastCreatedCharacter LastCreatedCharacterSettings;

extern "C" bool isAchievementUnlockedForClassUnlock(int race);
