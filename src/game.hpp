/*-------------------------------------------------------------------------------

	BARONY
	File: game.hpp
	Desc: header file for the game

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#pragma once

#include <vector>
#include <chrono>
#include "../odin/containers/dynamic_string.hpp"


#include "interface/consolecommand.hpp"

#include "Config.hpp"

// REMEMBER TO CHANGE THIS WITH EVERY NEW OFFICIAL VERSION!!!
static const char VERSION[] = "v5.0.2";
#define GAME_CODE

class Entity;

#define DEBUG 1
#define ENTITY_PACKET_LENGTH 47
#define NET_PACKET_SIZE 512

// impulses (bound keystrokes, mousestrokes, and joystick/game controller strokes) //TODO: Player-by-player basis.
extern Uint32 impulses[NUMIMPULSES];
extern Uint32 joyimpulses[NUM_JOY_IMPULSES]; //Joystick/gamepad only impulses.

extern "C" bool handleEvents(void);
void startMessages();

// net packet send
typedef struct packetsend_t
{
	UDPsocket sock;
	int channel;
	UDPpacket* packet;
	int num;
	int tries;
	int hostnum;
} packetsend_t;
extern list_t safePacketsSent;
extern DynamicMapI32T<Uint32> safePacketsReceivedMap[MAXPLAYERS];
extern bool receivedclientnum;

extern Sint32 numplayers;
extern Sint32 clientnum;
extern bool intro;
extern int introstage;
extern bool gamePaused;
extern bool fadeout, fadefinished;
extern int fadealpha;
extern Entity* client_selected[MAXPLAYERS];
extern bool inrange[MAXPLAYERS];
extern bool deleteallbuttons;
extern Sint32 client_classes[MAXPLAYERS];
extern Uint32 client_keepalive[MAXPLAYERS];
extern Uint16 portnumber;
extern list_t messages;
extern list_t command_history;
extern node_t* chosen_command;
extern bool command;
extern bool noclip, godmode, buddhamode;
extern bool everybodyfriendly;
extern bool combat, combattoggle;
extern bool assailant[MAXPLAYERS];
extern bool oassailant[MAXPLAYERS];
extern int assailantTimer[MAXPLAYERS];
static const int COMBAT_MUSIC_COOLDOWN = 200; // 200 ticks of combat music before it fades away.
extern list_t removedEntities;
extern char maptoload[256], configtoload[256];
extern bool loadingmap, loadingconfig;
extern int startfloor;
extern bool skipintro;
extern Uint32 uniqueGameKey;
extern Uint32 uniqueLobbyKey;
extern bool arachnophobia_filter;
extern bool colorblind_lobby;

// definitions
extern bool showfps;
extern real_t time_diff;
extern real_t t, ot, frameval[AVERAGEFRAMES];
extern Uint32 cycles, pingtime;
extern real_t fps;
static const int NUMCLASSES = 26;
#define NUMRACES 18
#define NUMPLAYABLERACES 14
extern char address[64];
extern bool loadnextlevel;
extern int skipLevelsOnLoad;
extern bool loadingSameLevelAsCurrent;
extern DynamicString loadCustomNextMap;
extern Uint32 forceMapSeed;
extern int currentlevel;
extern bool secretlevel;
extern bool darkmap;
extern int shaking, bobbing;

enum MessageType : Uint32 {
	MESSAGE_COMBAT = 1u << 0, // damage received or given in combat
	MESSAGE_STATUS = 1u << 1, // character status changes and passive effects
	MESSAGE_INVENTORY = 1u << 2, // inventory and item appraisal
	MESSAGE_EQUIPMENT = 1u << 3, // player equipment changes
	MESSAGE_WORLD = 1u << 4, // diegetic messages, such as speech and text
	MESSAGE_CHAT = 1u << 5, // multiplayer chat
	MESSAGE_PROGRESSION = 1u << 6, // player character progression messages (ie level-ups)
	MESSAGE_INTERACTION = 1u << 7, // player interactions with the world
	MESSAGE_INSPECTION = 1u << 8, // player inspections of world objects
	MESSAGE_HINT = 1u << 9, // special text cues and descriptive messages
	MESSAGE_OBITUARY = 1u << 10, // character death announcement
	MESSAGE_CHATTER = 1u << 11, // NPC chatter
	MESSAGE_SPAM_MISC = 1u << 28, // misc spammy messages "dropped item" "it burns!" 
	MESSAGE_COMBAT_BASIC = 1u << 29, // basic combat 'the skeleton hits!' 'you hit the skeleton!'
	MESSAGE_DEBUG = 1u << 30, // debug only messages
	MESSAGE_MISC = 1u << 31, // miscellaneous messages
};
extern Uint32 messagesEnabled;

enum PlayerClasses : int
{
	CLASS_BARBARIAN,
	CLASS_WARRIOR,
	CLASS_HEALER,
	CLASS_ROGUE,
	CLASS_WANDERER,
	CLASS_CLERIC,
	CLASS_MERCHANT,
	CLASS_WIZARD,
	CLASS_ARCANIST,
	CLASS_JOKER,
	CLASS_SEXTON,
	CLASS_NINJA,
	CLASS_MONK,
	CLASS_CONJURER,
	CLASS_ACCURSED,
	CLASS_MESMER,
	CLASS_BREWER,
	CLASS_MACHINIST,
	CLASS_PUNISHER,
	CLASS_SHAMAN,
	CLASS_HUNTER,
	CLASS_BARD,
	CLASS_SAPPER,
	CLASS_SCION,
	CLASS_HERMIT,
	CLASS_PALADIN
};

static DynamicArrayStr playerClassInternalNames = []() {
	DynamicArrayStr m;
		m.push_back("class_barbarian");
		m.push_back("class_warrior");
		m.push_back("class_healer");
		m.push_back("class_rogue");
		m.push_back("class_wanderer");
		m.push_back("class_cleric");
		m.push_back("class_merchant");
		m.push_back("class_wizard");
		m.push_back("class_arcanist");
		m.push_back("class_joker");
		m.push_back("class_sexton");
		m.push_back("class_ninja");
		m.push_back("class_monk");
		m.push_back("class_conjurer");
		m.push_back("class_accursed");
		m.push_back("class_mesmer");
		m.push_back("class_brewer");
		m.push_back("class_machinist");
		m.push_back("class_punisher");
		m.push_back("class_shaman");
		m.push_back("class_hunter");
		m.push_back("class_bard");
		m.push_back("class_sapper");
		m.push_back("class_scion");
		m.push_back("class_hermit");
		m.push_back("class_paladin");
	return m;
}();

static const int CLASS_SHAMAN_NUM_STARTING_SPELLS = 15;

enum PlayerRaces : int
{
	RACE_HUMAN,
	RACE_SKELETON,
	RACE_VAMPIRE,
	RACE_SUCCUBUS,
	RACE_GOATMAN,
	RACE_AUTOMATON,
	RACE_INCUBUS,
	RACE_GOBLIN,
	RACE_INSECTOID,
	RACE_RAT,
	RACE_TROLL,
	RACE_SPIDER,
	RACE_IMP,
	RACE_GNOME,
	RACE_GREMLIN,
	RACE_DRYAD,
	RACE_MYCONID,
	RACE_SALAMANDER,
	RACE_ENUM_END
};

extern "C" bool achievementUnlocked(const char* achName);
extern "C" void steamAchievement(const char* achName);
extern "C" void steamUnsetAchievement(const char* achName);
extern "C" void steamAchievementClient(int player, const char* achName);
extern "C" void steamAchievementEntity(Entity* my, const char* achName); // give steam achievement to an entity, and check for valid player info.
extern "C" void steamStatisticUpdate(int statisticNum, ESteamStatTypes type, int value);
extern "C" void steamStatisticUpdateClient(int player, int statisticNum, ESteamStatTypes type, int value);
extern "C" void steamIndicateStatisticProgress(int statisticNum, ESteamStatTypes type);
extern "C" void pauseGame(int mode, int ignoreplayer);
extern "C" int initGame();
extern "C" void initGameDatafiles(bool moddedReload);
extern "C" void initGameDatafilesAsync(bool moddedReload);
extern "C" void deinitGame();
extern "C" void handleButtons(void);
extern "C" void gameLogic(void);

// behavior function prototypes:
extern "C" void actAnimator(Entity* my);
extern "C" void actRotate(Entity* my);
extern "C" void actLiquid(Entity* my);
extern "C" void actEmpty(Entity* my);
extern "C" void actFurniture(Entity* my);
extern "C" void actMCaxe(Entity* my);
extern "C" void actStatueAnimator(Entity* my);
extern "C" void actStatue(Entity* my);
extern "C" void actDoorFrame(Entity* my);
extern "C" void actDeathCam(Entity* my);
extern "C" void actProjectSpiritCam(Entity* my);
extern "C" void actDeathGhost(Entity* my);
extern "C" void actDeathGhostLimb(Entity* my);
extern "C" void actPlayerLimb(Entity* my);
extern "C" void actTorch(Entity* my);
extern "C" void actCrystalShard(Entity* my);
extern "C" void actDoor(Entity* my);
extern "C" void actHudWeapon(Entity* my);
extern "C" void actHudArm(Entity* my);
extern "C" void actHudShield(Entity* my);
extern "C" void actHudAdditional(Entity* my);
extern "C" void actHudArrowModel(Entity* my);
extern "C" void actHudAdditional2(Entity* my);
extern "C" void actItem(Entity* my);
extern "C" void actGoldBag(Entity* my);
extern "C" void actGib(Entity* my);
extern "C" void actGreasePuddleSpawner(Entity* my);
extern "C" void actGreasePuddle(Entity* my);
extern "C" void actMiscPuddle(Entity* my);
extern "C" void spawnGreasePuddleSpawner(Entity* caster, real_t x, real_t y, int duration);
extern "C" void actDamageGib(Entity* my);
extern "C" void actFociGib(Entity* my);
extern "C" Entity* spawnFociGib(real_t x, real_t y, real_t z, real_t dir, real_t velocityBonus, Uint32 parentUid, int sprite, Uint32 seed);
extern "C" Entity* spawnGib(Entity* parentent, int customGibSprite = -1);
extern "C" Entity* spawnDamageGib(Entity* parentent, Sint32 dmgAmount, int gibDmgType, int displayType = 0, bool updateClients = false);
extern "C" Entity* spawnGibClient(Sint16 x, Sint16 y, Sint16 z, Sint16 sprite);
extern "C" Entity* spawnMiscPuddle(Entity* parentent, real_t x, real_t y, int sprite, bool updateClients = false);
extern "C" void serverSpawnGibForClient(Entity* gib);
extern "C" void actLadder(Entity* my);
extern "C" void actLadderUp(Entity* my);
extern "C" void actPortal(Entity* my);
extern "C" void actWinningPortal(Entity* my);
extern "C" void actFlame(Entity* my);
extern "C" void actCampfire(Entity* my);
extern "C" void actCauldron(Entity* my);
extern "C" void actWorkbench(Entity* my);
extern "C" void actMailbox(Entity* my);
extern "C" Entity* spawnFlame(Entity* parentent, Sint32 sprite);
extern "C" Entity* spawnFlameSprites(Entity* parentent, Sint32 sprite);
extern "C" Entity* castMagic(Entity* parentent);
extern "C" void actSprite(Entity* my);
extern "C" void actSpriteNametag(Entity* my);
extern "C" void actSpriteWorldTooltip(Entity* my);
extern "C" void actSleepZ(Entity* my);
extern "C" Entity* spawnBang(Sint16 x, Sint16 y, Sint16 z);
extern "C" Entity* spawnExplosion(Sint16 x, Sint16 y, Sint16 z);
extern "C" Entity* spawnExplosionFromSprite(Uint16 sprite, Sint16 x, Sint16 y, Sint16 z);
extern "C" Entity* spawnPoof(Sint16 x, Sint16 y, Sint16 z, real_t scale, bool updateClients = false);
extern "C" Entity* spawnSleepZ(Sint16 x, Sint16 y, Sint16 z);
extern "C" Entity* spawnFloatingSpriteMisc(int sprite, Sint16 x, Sint16 y, Sint16 z);
extern "C" void actArrow(Entity* my);
extern "C" void actBoulder(Entity* my);
extern "C" void actBoulderTrap(Entity* my);
extern "C" void actBoulderTrapHole(Entity* my);
extern "C" void actBoulderTrapEast(Entity* my);
extern "C" void actBoulderTrapWest(Entity* my);
extern "C" void actBoulderTrapSouth(Entity* my);
extern "C" void actBoulderTrapNorth(Entity* my);
extern "C" void actHeadstone(Entity* my);
extern "C" void actThrown(Entity* my);
extern "C" void actBeartrap(Entity* my);
extern "C" void actBeartrapLaunched(Entity* my);
extern "C" void actBomb(Entity* my);
extern "C" void actDecoyBox(Entity* my);
extern "C" void actDecoyBoxCrank(Entity* my);
extern "C" void actSpearTrap(Entity* my);
extern "C" void actWallBuster(Entity* my);
extern "C" void actWallBuilder(Entity* my);
extern "C" void actPowerCrystalBase(Entity* my);
extern "C" void actPowerCrystal(Entity* my);
extern "C" void actPowerCrystalParticleIdle(Entity* my);
extern "C" void actPedestalBase(Entity* my);
extern "C" void actPedestalOrb(Entity* my);
extern "C" void actMidGamePortal(Entity* my);
extern "C" void actCustomPortal(Entity* my);
extern "C" void actTeleporter(Entity* my);
extern "C" void actMagicTrapCeiling(Entity* my);
extern "C" void actTeleportShrine(Entity* my);
extern "C" void actDaedalusShrine(Entity* my);
extern "C" void actAssistShrine(Entity* my);
extern "C" void actBell(Entity* my);
extern "C" void bellBreakBulb(Entity* my, bool minotaurBreak);
extern "C" void actSpellShrine(Entity* my);
extern "C" void actExpansionEndGamePortal(Entity* my);
extern "C" void actSoundSource(Entity* my);
extern "C" void actLightSource(Entity* my);
extern "C" void actSignalTimer(Entity* my);
extern "C" void actSignalGateAND(Entity* my);
extern "C" void actWallLock(Entity* my);
extern "C" void actWallButton(Entity* my);
extern "C" void actWind(Entity* my);
extern "C" void createWaterSplash(real_t x, real_t y, int lifetime);

void startMessages();
extern "C" bool frameRateLimit(Uint32 maxFrameRate, bool resetAccumulator = true, bool sleep = false);
extern Uint32 networkTickrate;
extern bool gameloopFreezeEntities;
extern Uint32 serverSchedulePlayerHealthUpdate;

extern "C" void drawAllPlayerCameras();

#define TOUCHRANGE 32
#define STRIKERANGE 24
#define XPSHARERANGE 99999

// function prototypes for charclass.c:
extern "C" void initClass(int player);
extern "C" void initClassStats(const int classnum, void* myStats);
extern "C" void initShapeshiftHotbar(int player);
extern "C" void deinitShapeshiftHotbar(int player);
extern "C" bool playerUnlockedShamanSpell(int player, Item* item);

extern char last_ip[64];
extern char last_port[64];

//TODO: Maybe increase with level or something?
//TODO: Pause health regen during combat?
#define HEAL_TIME 600 //12 seconds. //Original time: 3600 (1 minute)
#define MAGIC_REGEN_TIME 600 // 12 seconds
#define MAGIC_REGEN_AUTOMATON_TIME 300

#define DEFAULT_HP 30
#define DEFAULT_MP 30
#define HP_MOD 5
#define MP_MOD 5

#define SPRITE_FLAME 13
#define SPRITE_CRYSTALFLAME 96

#define MAXCHARGE 30 // charging up weapons

static const int BASE_MELEE_DAMAGE = 8;
static const int BASE_RANGED_DAMAGE = 7;
static const int BASE_THROWN_DAMAGE = 6;
static const int BASE_PLAYER_UNARMED_DAMAGE = 8;

extern bool spawn_blood;
extern bool capture_mouse; //Useful for debugging when the game refuses to release the mouse when it's crashed.

#define LEVELSFILE "maps/levels.txt"
#define SECRETLEVELSFILE "maps/secretlevels.txt"
#define LENGTH_OF_LEVEL_REGION 5

#define TICKS_PER_SECOND 50
static const Uint8 TICKS_TO_PROCESS_FIRE = 30; // The amount of ticks needed until the 'BURNING' Status Effect is processed (char_fire % TICKS_TO_PROCESS_FIRE == 0)
static const int EFFECT_WITHDRAWAL_BASE_TIME = TICKS_PER_SECOND * 60 * 8; // 8 minutes base withdrawal time.

static const DynamicString PLAYERNAMES_MALE_FILE = "playernames-male.txt";
static const DynamicString PLAYERNAMES_FEMALE_FILE = "playernames-female.txt";
static const DynamicString NPCNAMES_MALE_FILE = "npcnames-male.txt";
static const DynamicString NPCNAMES_FEMALE_FILE = "npcnames-female.txt";
extern DynamicArrayStr randomPlayerNamesMale;
extern DynamicArrayStr randomPlayerNamesFemale;
extern DynamicArrayStr randomNPCNamesMale;
extern DynamicArrayStr randomNPCNamesFemale;
extern bool enabledDLCPack1;
extern bool enabledDLCPack2;
extern bool enabledDLCPack3;
extern DynamicArrayStr physFSFilesInDirectory;
extern "C" void loadRandomNames();
extern "C" int mapLevel(int player, int radius, int _x, int _y, bool usingSpell);
extern "C" void mapLevel2(int player);
extern "C" void mapFoodOnLevel(int player);
extern "C" bool mapTileDiggable(const int x, const int y);

class TileEntityListHandler
{
private:
	static const int kMaxMapDimension = 256;
public:
	list_t gridEntities[kMaxMapDimension][kMaxMapDimension];

	void clearTile(int x, int y);
	void emptyGridEntities();
	list_t* getTileList(int x, int y);
	node_t* addEntity(Entity& entity);
	node_t* updateEntity(Entity& entity);
	DynamicArrayT<list_t*> getEntitiesWithinRadius(int u, int v, int radius);
	DynamicArrayT<list_t*> getEntitiesWithinRadiusAroundEntity(Entity* entity, int radius);

	TileEntityListHandler()
	{
		for ( int i = 0; i < kMaxMapDimension; ++i )
		{
			for ( int j = 0; j < kMaxMapDimension; ++j )
			{
				gridEntities[i][j].first = nullptr;
				gridEntities[i][j].last = nullptr;
			}
		}
	};

	~TileEntityListHandler()
	{
		for ( int i = 0; i < kMaxMapDimension; ++i )
		{
			for ( int j = 0; j < kMaxMapDimension; ++j )
			{
				clearTile(i, j);
			}
		}
	};
};
extern TileEntityListHandler TileEntityList;

class DebugStatsClass
{
public:
	std::chrono::high_resolution_clock::time_point t1StartLoop;
	std::chrono::high_resolution_clock::time_point t2PostEvents;
	std::chrono::high_resolution_clock::time_point t21PostHandleMessages;
	std::chrono::high_resolution_clock::time_point t3SteamCallbacks;
	std::chrono::high_resolution_clock::time_point t4Music;
	std::chrono::high_resolution_clock::time_point t5MainDraw;
	std::chrono::high_resolution_clock::time_point t6Messages;
	std::chrono::high_resolution_clock::time_point t7Inputs;
	std::chrono::high_resolution_clock::time_point t8Status;
	std::chrono::high_resolution_clock::time_point t9GUI;
	std::chrono::high_resolution_clock::time_point t10FrameLimiter;
	std::chrono::high_resolution_clock::time_point t11End;

	std::chrono::high_resolution_clock::time_point gui1;
	std::chrono::high_resolution_clock::time_point gui2;
	std::chrono::high_resolution_clock::time_point gui3;
	std::chrono::high_resolution_clock::time_point gui4;
	std::chrono::high_resolution_clock::time_point gui5;
	std::chrono::high_resolution_clock::time_point gui6;
	std::chrono::high_resolution_clock::time_point gui7;
	std::chrono::high_resolution_clock::time_point gui8;
	std::chrono::high_resolution_clock::time_point gui9;
	std::chrono::high_resolution_clock::time_point gui10;
	std::chrono::high_resolution_clock::time_point gui11;
	std::chrono::high_resolution_clock::time_point gui12;

	std::chrono::high_resolution_clock::time_point eventsT1;
	std::chrono::high_resolution_clock::time_point eventsT2;
	std::chrono::high_resolution_clock::time_point eventsT3;
	std::chrono::high_resolution_clock::time_point eventsT4;
	std::chrono::high_resolution_clock::time_point eventsT5;
	std::chrono::high_resolution_clock::time_point eventsT6;

	std::chrono::high_resolution_clock::time_point drawWorldT1;
	std::chrono::high_resolution_clock::time_point drawWorldT2;
	std::chrono::high_resolution_clock::time_point drawWorldT3;
	std::chrono::high_resolution_clock::time_point drawWorldT4;
	std::chrono::high_resolution_clock::time_point drawWorldT5;
	std::chrono::high_resolution_clock::time_point drawWorldT6;

	std::chrono::high_resolution_clock::time_point messagesT1;

	std::chrono::high_resolution_clock::time_point t1Stored;
	std::chrono::high_resolution_clock::time_point t2Stored;
	std::chrono::high_resolution_clock::time_point t21Stored;
	std::chrono::high_resolution_clock::time_point t3Stored;
	std::chrono::high_resolution_clock::time_point t4Stored;
	std::chrono::high_resolution_clock::time_point t5Stored;
	std::chrono::high_resolution_clock::time_point t6Stored;
	std::chrono::high_resolution_clock::time_point t7Stored;
	std::chrono::high_resolution_clock::time_point t8Stored;
	std::chrono::high_resolution_clock::time_point t9Stored;
	std::chrono::high_resolution_clock::time_point t10Stored;
	std::chrono::high_resolution_clock::time_point t11Stored;

	std::chrono::high_resolution_clock::time_point eventsT1stored;
	std::chrono::high_resolution_clock::time_point eventsT2stored;
	std::chrono::high_resolution_clock::time_point eventsT3stored;
	std::chrono::high_resolution_clock::time_point eventsT4stored;
	std::chrono::high_resolution_clock::time_point eventsT5stored;
	std::chrono::high_resolution_clock::time_point eventsT6stored;

	std::chrono::high_resolution_clock::time_point messagesT1stored;

	std::chrono::high_resolution_clock::time_point messagesT2WhileLoop;
	bool handlePacketStartLoop = false;

	DynamicMapI32T<NetworkPacket_t> networkPackets;
	DynamicMapI32T<int> entityUpdatePackets;

	bool displayStats = false;
	char debugOutput[1024];
	char debugEventOutput[1024];

	DebugStatsClass()
	{};

	void inline storeOldTimePoints();;

	void storeStats();

	void storeEventStats();
};

extern CvarBool cvar_enableKeepAlives;
extern CvarBool cvar_map_sequence_rng;

extern DebugStatsClass DebugStats;
//extern CvarBool cvar_useTimerInterpolation;

#include "draw.hpp"

class TimerExperiments
{
public:
    //static constexpr bool& bUseTimerInterpolation = *cvar_useTimerInterpolation;
    static bool bUseTimerInterpolation;
	static bool bIsInit;
	static real_t lerpFactor;
	static int timeDivision;
	static bool bDebug;
	struct Clock
	{
		using duration = std::chrono::milliseconds;
		using rep = duration::rep;
		using period = duration::period;
		using time_point = std::chrono::time_point<Clock>;
		static constexpr bool is_steady = true;

		static time_point now() noexcept;
	};

	struct State
	{
		double acceleration;
		double velocity;
		double position;
		void resetMovement();
		void resetPosition();
		void normalize(real_t min, real_t max);
	};

	struct EntityStates
	{
		State x;
		State y;
		State z;
		State yaw;
		State pitch;
		State roll;
		void resetMovement();
		void resetPosition();
	};

	friend EntityStates operator+(EntityStates lhs, EntityStates rhs)
	{
		return{ lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, 
			lhs.yaw + rhs.yaw, lhs.pitch + rhs.pitch, lhs.roll + rhs.roll };
	}
	friend EntityStates operator*(EntityStates lhs, double rhs)
	{
		return{ lhs.x * rhs, lhs.y * rhs, lhs.z * rhs,
			lhs.yaw * rhs, lhs.pitch * rhs, lhs.roll * rhs };
	}
	friend State operator+(State x, State y)
	{
		return{ x.acceleration + y.acceleration, x.velocity + y.velocity, x.position + y.position };
	}
	friend State operator*(State x, double y)
	{
		return{ x.acceleration * y, x.velocity * y, x.position * y };
	}

	static void
		integrate(State& state,
			std::chrono::time_point<Clock, std::chrono::duration<double>>,
			std::chrono::duration<double> dt);

	static std::chrono::duration<long long, std::ratio<1, 60>> dt;
	using duration = decltype(Clock::duration{} +dt);
	using time_point = std::chrono::time_point<Clock, duration>;

	static time_point timepoint;
	static time_point currentTime;
	static duration accumulator;

	static EntityStates cameraPreviousState[MAXPLAYERS];
	static EntityStates cameraCurrentState[MAXPLAYERS];
	static EntityStates cameraRenderState[MAXPLAYERS];

	static DynamicString render(State state);

	static void reset();
	static void updateClocks();
	static real_t lerpAngle(real_t angle1, real_t angle2, real_t alpha);
	static void renderCameras(view_t& camera, int player);
	static void postRenderRestore(view_t& camera, int player);
	static void updateEntityInterpolationPosition(Entity* entity);
};

extern "C" void loadAchievementData(const char* path);
extern "C" void sortAchievementsForDisplay();

extern "C" real_t getFPSScale(real_t baseFPS);
