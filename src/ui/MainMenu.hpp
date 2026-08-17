#pragma once

#include <unordered_map>
#include <string>
#include "../main.hpp"
#include "../game.hpp"
#include "../json.hpp"
#include "../ui/Frame.hpp"
#include "../interface/consolecommand.hpp"

#define MAX_SPLITSCREEN 4

namespace MainMenu {
    extern int pause_menu_owner; // which player is driving the pause menu
	extern bool cursor_delete_mode; // if true, mouse cursor displays an extra glyph to denote delete mode (used to delete save games)
	extern Frame* main_menu_frame; // root main menu node
	extern Uint32 main_menu_ticks;
	// Here be new menu options:
	extern DynamicString current_audio_device; // guid of the audio device currently in use
	extern DynamicString current_recording_audio_device; // guid of the recording audio device currently in use
	extern float master_volume; // range is [0 - 100]
	extern bool arachnophobia_filter; // if true, all spiders are crabs'
	extern CvarBool vertical_splitscreen; // if true, 2-player splitscreen has a vertical rather than horizontal layout
	extern CvarBool staggered_splitscreen; // if true, viewport sizes are reduced to preserve aspect ratio
	extern CvarBool clipped_splitscreen; // if true, viewports rest in a corner rather than centered
    extern CvarBool cvar_fastRestart;
	extern CvarFloat cvar_worldtooltip_scale;
	extern CvarFloat cvar_worldtooltip_scale_splitscreen;
	extern CvarBool cvar_hold_to_activate;
	extern CvarFloat cvar_enemybar_scale;
    extern CvarInt cvar_desiredFps;
    extern CvarInt cvar_displayHz;
	extern CvarBool cvar_hdrEnabled;
	
	static constexpr const char* emptyBinding = "[unbound]"; // string appended to default empty bindings
	static constexpr const char* hiddenBinding = "[hidden]"; // string appended to hidden bindings on the UI

	enum class FadeDestination : Uint8 {
		None,           // don't fade anywhere (???)
		TitleScreen,    // fade to the title screen
		RootMainMenu,   // return to main menu, save no score if ingame
		RootMainMenuNoEndGame,   // return to main menu, doEndGame already done
        Endgame,        // save a highscore and return to main menu
		Victory,        // save a highscore and roll credits

		// Story scenes:

		IntroStoryScreen,
		IntroStoryScreenNoMusicFade,

		HerxMidpointHuman,
		HerxMidpointAutomaton,
		HerxMidpointBeast,
		HerxMidpointEvil,

		BaphometMidpointHuman,
		BaphometMidpointAutomaton,
		BaphometMidpointBeast,
		BaphometMidpointEvil,

		EndingHuman,
		EndingAutomaton,
		EndingBeast,
		EndingEvil,
        
        // Classic endings:

		ClassicEndingHuman,
		ClassicEndingAutomaton,
		ClassicEndingBeast,
		ClassicEndingEvil,

		ClassicBaphometEndingHuman,
		ClassicBaphometEndingAutomaton,
		ClassicBaphometEndingBeast,
		ClassicBaphometEndingEvil,

        // Game starts:

		GameStart,          // used by servers and local games
		GameStartDummy,     // used by clients to fade without really launching
		HallOfTrials,       // used to launch a hall-of-trials map
	};

extern "C"     const char* getUsername();              // get local account name
extern "C"     const char* getHostname();              // get local host name
extern "C"     void setUsername(const char* name);     // set local account name
extern "C"     void setHostname(const char* name);     // set local hostname
extern "C" 	void randomizeUsername();				// randomize the username
extern "C" 	void randomizeHostname();				// randomize the hostname
extern "C" 	void logoutOfEpic();					// log out of epic online services

extern "C" 	int getMenuOwner();					// get current pause menu owner
extern "C"     bool isPlayerSignedIn(int index);   // checks whether a player is signed into a given slot
extern "C" 	bool isPlayerSlotLocked(int index);	// checks whether a player slot has been locked out for joining
extern "C"     bool isCutsceneActive();            // checks whether we are playing a cutscene
extern "C" 	bool isMenuOpen();					// checks whether the menu is open
extern "C" 	void beginFade(FadeDestination);    // begins a fade transition to a specific destination

extern "C" 	void settingsApply();					// write settings to global variables (true if video mode changed)
extern "C" 	void settingsMount(bool video = true);	// read settings from global variables
extern "C" 	bool settingsSave();					// write settings to disk (true if succeeded)
extern "C" 	bool settingsLoad();					// read settings from disk (true if succeeded)
extern "C" 	void settingsReset();					// default settings

extern "C" 	void doMainMenu(bool ingame);           // call in a loop to update the menu
extern "C" 	void createTitleScreen();               // creates a fresh title screen
extern "C" 	void createMainMenu(bool ingame);       // creates a fresh main menu
extern "C" 	void destroyMainMenu();                 // destroys the main menu tree
extern "C" 	void createDummyMainMenu();             // creates a main menu devoid of widgets
extern "C" 	void closeMainMenu();                   // closes the menu and unpauses the game

	// special events:

extern "C"     void controllerDisconnected(int player);                        // controller disconnect prompt, eg if a player unplugs a controller
extern "C" 	void tutorialFirstTimeCompleted();								// tutorial first level completed event
extern "C"     void openGameoverWindow(int player, bool tutorial = false);     // opens gameover window, used when player dies
extern "C" 	void openCompendium();
extern "C" 	void disconnectedFromServer(const char* text);                  // called when the player is disconnected from the server, prompts them to end the game
extern "C" 	void receivedInvite(void*);                                     // called when a player receives an invite to a lobby (EOS or Steam)
extern "C" 	void setupSplitscreen();                                        // used to resize player game views, for example if a player drops or we change the aspect ratio
extern "C" 	void crossplayPrompt();                                         // user chose to activate crossplay
extern "C" 	void timedOut();												// special disconnection event that may display a system error message

	struct ClassDescriptions
	{
		struct DescData_t
		{
			DynamicString text;
			DynamicString internal_name;
			DynamicArrayT<SurvivalComplexityEntry_t> survivalComplexity;
			DynamicArrayU32 statRatings;
			DynamicArrayStr statRatingsStrings;
			Sint32 hp = DEFAULT_HP;
			Sint32 mp = DEFAULT_MP;
			DynamicArrayS32 linePaddings;
		};
		static DynamicMapI32T<DescData_t> data;
		static void readFromFile();
		static bool init;
		static void update_stat_growths(Frame& card, int classnum, int shapeshiftedType);
	};

	struct RaceDescriptions
	{
		struct DescData_t
		{
			DynamicString textLeft;
			DynamicString textRight;
			DynamicSetI32 traitLines;
			DynamicSetI32 proLines;
			DynamicArrayS32 linePaddings;
			DynamicString title;
			DynamicString traitsBasedOnPlayerRace;
			DynamicString traitsBasedOnMonsterType;
			DynamicString resistances;
			DynamicString weaknesses;
			DynamicString friendlyWith;
			DynamicString racialSpells;
		};
		static DynamicMapStrT<DescData_t> data;
		static void readFromFile();
		static std::string getRaceKey(int race);
		static DescData_t& getRaceDescriptionData(int race) { return data[getRaceKey(race)]; }
		static DescData_t& getMonsterDescriptionData(int type);
		static bool init;
		static void update_details_text(Frame& card);
		static void update_details_text(Frame& card, void* stats);
		static void update_details_text(Frame& card, int race, int modified_race);
	};

	// Owning map value kinds: the Odin free/copy handlers (MK_ClassDescData /
	// MK_RaceDescData) operate on the de-STL'd DescData_t layouts above.
	template <> struct MapValueKindOf<ClassDescriptions::DescData_t> { static constexpr int value = MK_ClassDescData; };
	template <> struct MapValueKindOf<RaceDescriptions::DescData_t> { static constexpr int value = MK_RaceDescData; };

	struct MainMenuBanners_t
	{
		static DynamicString updateBannerImg;
		static DynamicString updateBannerImgHighlight;
		static DynamicString updateBannerURL;
		static void readFromFile();
	};

	enum class DLC {
		Base,
		MythsAndOutcasts,
		LegendsAndPariahs,
		DesertersAndDisciples
	};

	typedef Class_tMirror Class;

		static DynamicMapClass classes = []() {
		DynamicMapClass m;

				m.put("barbarian", Class_tMirror{ (int)DLC::Base, "ClassSelect_Icon_Barbarian_00.png", "ClassSelect_Icon_BarbarianOn_00.png", "ClassSelect_Icon_BarbarianLocked_00.png" });

				m.put("warrior", Class_tMirror{ (int)DLC::Base, "ClassSelect_Icon_Warrior_00.png", "ClassSelect_Icon_WarriorOn_00.png", "ClassSelect_Icon_WarriorLocked_00.png" });

				m.put("healer", Class_tMirror{ (int)DLC::Base, "ClassSelect_Icon_Healer_00.png", "ClassSelect_Icon_HealerOn_00.png", "ClassSelect_Icon_HealerLocked_00.png" });

				m.put("rogue", Class_tMirror{ (int)DLC::Base, "ClassSelect_Icon_Rogue_00.png", "ClassSelect_Icon_RogueOn_00.png", "ClassSelect_Icon_RogueLocked_00.png" });

				m.put("wanderer", Class_tMirror{ (int)DLC::Base, "ClassSelect_Icon_Wanderer_00.png", "ClassSelect_Icon_WandererOn_00.png", "ClassSelect_Icon_WandererLocked_00.png" });

				m.put("cleric", Class_tMirror{ (int)DLC::Base, "ClassSelect_Icon_Cleric_00.png", "ClassSelect_Icon_ClericOn_00.png", "ClassSelect_Icon_ClericLocked_00.png" });

				m.put("merchant", Class_tMirror{ (int)DLC::Base, "ClassSelect_Icon_Merchant_00.png", "ClassSelect_Icon_MerchantOn_00.png", "ClassSelect_Icon_MerchantLocked_00.png" });

				m.put("wizard", Class_tMirror{ (int)DLC::Base, "ClassSelect_Icon_Wizard_00.png", "ClassSelect_Icon_WizardOn_00.png", "ClassSelect_Icon_WizardLocked_00.png" });

				m.put("arcanist", Class_tMirror{ (int)DLC::Base, "ClassSelect_Icon_Arcanist_00.png", "ClassSelect_Icon_ArcanistOn_00.png", "ClassSelect_Icon_ArcanistLocked_00.png" });

				m.put("joker", Class_tMirror{ (int)DLC::Base, "ClassSelect_Icon_Jester_00.png", "ClassSelect_Icon_JesterOn_00.png", "ClassSelect_Icon_JesterLocked_00.png" });

				m.put("sexton", Class_tMirror{ (int)DLC::Base, "ClassSelect_Icon_Sexton_00.png", "ClassSelect_Icon_SextonOn_00.png", "ClassSelect_Icon_SextonLocked_00.png" });

				m.put("ninja", Class_tMirror{ (int)DLC::Base, "ClassSelect_Icon_Ninja_00.png", "ClassSelect_Icon_NinjaOn_00.png", "ClassSelect_Icon_NinjaLocked_00.png" });

				m.put("monk", Class_tMirror{ (int)DLC::Base, "ClassSelect_Icon_Monk_00.png", "ClassSelect_Icon_MonkOn_00.png", "ClassSelect_Icon_MonkLocked_00.png" });

				m.put("conjurer", Class_tMirror{ (int)DLC::MythsAndOutcasts, "ClassSelect_Icon_Conjurer_00.png", "ClassSelect_Icon_ConjurerOn_00.png", "ClassSelect_Icon_ConjurerLocked_00.png" });

				m.put("accursed", Class_tMirror{ (int)DLC::MythsAndOutcasts, "ClassSelect_Icon_Accursed_00.png", "ClassSelect_Icon_AccursedOn_00.png", "ClassSelect_Icon_AccursedLocked_00.png" });

				m.put("mesmer", Class_tMirror{ (int)DLC::MythsAndOutcasts, "ClassSelect_Icon_Mesmer_00.png", "ClassSelect_Icon_MesmerOn_00.png", "ClassSelect_Icon_MesmerLocked_00.png" });

				m.put("brewer", Class_tMirror{ (int)DLC::MythsAndOutcasts, "ClassSelect_Icon_Brewer_00.png", "ClassSelect_Icon_BrewerOn_00.png", "ClassSelect_Icon_BrewerLocked_00.png" });

				m.put("mechanist", Class_tMirror{ (int)DLC::LegendsAndPariahs, "ClassSelect_Icon_Mechanist_00.png", "ClassSelect_Icon_MechanistOn_00.png", "ClassSelect_Icon_MechanistLocked_00.png" });

				m.put("punisher", Class_tMirror{ (int)DLC::LegendsAndPariahs, "ClassSelect_Icon_Punisher_00.png", "ClassSelect_Icon_PunisherOn_00.png", "ClassSelect_Icon_PunisherLocked_00.png" });

				m.put("shaman", Class_tMirror{ (int)DLC::LegendsAndPariahs, "ClassSelect_Icon_Shaman_00.png", "ClassSelect_Icon_ShamanOn_00.png", "ClassSelect_Icon_ShamanLocked_00.png" });

				m.put("hunter", Class_tMirror{ (int)DLC::LegendsAndPariahs, "ClassSelect_Icon_Hunter_00.png", "ClassSelect_Icon_HunterOn_00.png", "ClassSelect_Icon_HunterLocked_00.png" });

				m.put("bard", Class_tMirror{ (int)DLC::DesertersAndDisciples, "ClassSelect_Icon_Bard_00.png", "ClassSelect_Icon_BardOn_00.png", "ClassSelect_Icon_BardLocked_00.png" });

				m.put("sapper", Class_tMirror{ (int)DLC::DesertersAndDisciples, "ClassSelect_Icon_Sapper_00.png", "ClassSelect_Icon_SapperOn_00.png", "ClassSelect_Icon_SapperLocked_00.png" });

				m.put("scion", Class_tMirror{ (int)DLC::DesertersAndDisciples, "ClassSelect_Icon_Scion_00.png", "ClassSelect_Icon_ScionOn_00.png", "ClassSelect_Icon_ScionLocked_00.png" });

				m.put("hermit", Class_tMirror{ (int)DLC::DesertersAndDisciples, "ClassSelect_Icon_Hermit_00.png", "ClassSelect_Icon_HermitOn_00.png", "ClassSelect_Icon_HermitLocked_00.png" });

				m.put("paladin", Class_tMirror{ (int)DLC::DesertersAndDisciples, "ClassSelect_Icon_Paladin_00.png", "ClassSelect_Icon_PaladinOn_00.png", "ClassSelect_Icon_PaladinLocked_00.png" });

		return m;
	}();
	static const char* classes_in_order[] = {
		"barbarian", "warrior", "healer",
		"rogue", "wanderer", "cleric", "merchant",
		"wizard", "arcanist", "joker", "sexton",
		"ninja", "monk", "conjurer", "accursed",
		"mesmer", "brewer", "mechanist", "punisher",
		"shaman", "hunter", "bard", "sapper", "scion", "hermit", "paladin"
	};

}
