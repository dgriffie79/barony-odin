/*-------------------------------------------------------------------------------

BARONY
File: mod_tools.hpp
Desc: misc modding tools

Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
See LICENSE for details.

-------------------------------------------------------------------------------*/

#pragma once
#include "../odin/containers/dynamic_map.hpp"
#include "main.hpp"
#include "stat.hpp"
#include "json.hpp"
#include "files.hpp"
#include "prng.hpp"
#include "items.hpp"
#include "../odin/json_shim/json_shim.hpp"
#include "net.hpp"
#include "scores.hpp"
#include "entity.hpp"
#include "ui/Widget.hpp"

#ifdef USE_LIBCURL
#include <curl/curl.h>
#endif

class CustomHelpers
{
public:
	static void addMemberToSubkey(JsonNode d, std::string subkey, std::string name, JsonNode value);
	static void addMemberToRoot(JsonNode d, std::string name, JsonNode value);
	static void addArrayMemberToSubkey(JsonNode d, std::string subkey, JsonNode value);
	static bool isLevelPartOfSet(int level, bool secret, std::pair<DynamicSetI32, DynamicSetI32>& pairOfSets)
	{
		if ( !secret )
		{
			if ( pairOfSets.first.find(level) == pairOfSets.first.end() )
			{
				return false;
			}
		}
		else
		{
			if ( pairOfSets.second.find(level) == pairOfSets.second.end() )
			{
				return false;
			}
		}
		return true;
	}
};

class MonsterStatCustomManager
{
public:
	static const DynamicArrayStr itemStatusStrings;
	static const DynamicArrayStr shopkeeperTypeStrings;
	MonsterStatCustomManager() = default;
	static BaronyRNG monster_stat_rng;

	int getSlotFromKeyName(std::string keyName);

	class ItemEntry
	{
	public:
		ItemType type = WOODEN_SHIELD;
		Status status = DECREPIT;
		Sint16 beatitude = 0;
		Sint16 count = 1;
		Uint32 appearance = 0;
		bool identified = 0;
		int percentChance = 100;
		int weightedChance = 1;
		int dropChance = 100;
		bool emptyItemEntry = false;
		bool dropItemOnDeath = true;
		ItemEntry() {};
		ItemEntry(const Item& itemToRead)
		{
			readFromItem(itemToRead);
		}
		void readFromItem(const Item& itemToRead);
		void setValueFromAttributes(JsonNode d, JsonNode outObject);

		const char* getRandomArrayStr(const JsonNode arr, const char* invalidEntry);
		int getRandomArrayInt(const JsonNode arr, int invalidEntry);

		bool readKeyToItemEntry(JsonMemberIt itr);
	};

	class StatEntry
	{
	public:
		char name[128];
		int type = NOTHING;
		sex_t sex = sex_t::MALE;
		Uint32 appearance = 0;
		Sint32 HP = 10;
		Sint32 MAXHP = 10;
		Sint32 OLDHP = 10;
		Sint32 MP = 10;
		Sint32 MAXMP = 10;
		Sint32 STR = 0;
		Sint32 DEX = 0;
		Sint32 CON = 0;
		Sint32 INT = 0;
		Sint32 PER = 0;
		Sint32 CHR = 0;
		Sint32 EXP = 0;
		Sint32 LVL = 0;
		Sint32 GOLD = 0;
		Sint32 HUNGER = 0;
		Sint32 RANDOM_STR = 0;
		Sint32 RANDOM_DEX = 0;
		Sint32 RANDOM_CON = 0;
		Sint32 RANDOM_INT = 0;
		Sint32 RANDOM_PER = 0;
		Sint32 RANDOM_CHR = 0;
		Sint32 RANDOM_MAXHP = 0;
		Sint32 RANDOM_HP = 0;
		Sint32 RANDOM_MAXMP = 0;
		Sint32 RANDOM_MP = 0;
		Sint32 RANDOM_LVL = 0;
		Sint32 RANDOM_GOLD = 0;

		Sint32 PROFICIENCIES[NUMPROFICIENCIES];

		DynamicArray equipped_items;  // vector<pair<ItemEntry,int>>
		DynamicArray inventory_items;  // vector<ItemEntry> (POD)
		struct VariantPair_t
		{
			DynamicString name = "";
			int chance = 0;
		};
		DynamicArrayT<VariantPair_t> followerVariants;
		DynamicArrayT<VariantPair_t> shopkeeperStoreTypes;
		int chosenShopkeeperStore = -1;
		int shopkeeperMinItems = -1;
		int shopkeeperMaxItems = -1;
		int shopkeeperMaxGeneratedBlessing = -1;
		bool shopkeeperGenDefaultItems = true;
		enum ShopkeeperCustomFlags : int
		{
			ENABLE_GEN_ITEMS = 1,
			DISABLE_GEN_ITEMS
		};

		int numFollowers = 0;
		bool isMonsterNameGeneric = false;
		bool useDefaultEquipment = true;
		bool useDefaultInventoryItems = true;
		bool disableMiniboss = true;
		bool forceFriendlyToPlayer = false;
		bool forceEnemyToPlayer = false;
		bool forceRecruitableToPlayer = false;
		bool disableItemDrops = false;
		int xpAwardPercent = 100;
		bool castSpellbooksFromInventory = false;
		int spellbookCastCooldown = 250;

		StatEntry(const Stat* myStats)
		{
			readFromStats(myStats);
			strcpy(name, "");
		}
		StatEntry()
		{
			for ( int i = 0; i < NUMPROFICIENCIES; ++i )
			{
				PROFICIENCIES[i] = 0;
			}
			strcpy(name, "");
		};

		std::string getFollowerVariant();

		void readFromStats(const Stat* myStats);

		void setStats(Stat* myStats);

		void setItems(Stat* myStats);

		void setStatsAndEquipmentToMonster(Stat* myStats);

		void setStatsAndEquipmentToPlayer(Stat* myStats, int player);
	};

	void writeAllFromStats(Stat* myStats);

	void readItemsFromStats(Stat* myStats, JsonNode d);

	void readAttributesFromStats(Stat* myStats, JsonNode d);

	bool readKeyToStatEntry(StatEntry& statEntry, JsonMemberIt itr);

	void addArrayMemberFromItem(JsonNode d, std::string rootKey, Item* item);
	void addMemberFromItem(JsonNode d, std::string rootKey, std::string key, Item* item);

	void writeToFile(JsonNode d, std::string monsterFileName);

	StatEntry* readFromFile(std::string monsterFileName);
};
extern MonsterStatCustomManager monsterStatCustomManager;

class MonsterCurveCustomManager
{
	bool usingCustomManager = false;
public:
	MonsterCurveCustomManager() = default;
	BaronyRNG monster_curve_rng;

	class MonsterCurveEntry
	{
	public:
		int monsterType = NOTHING;
		int levelmin = 0;
		int levelmax = 99;
		int chance = 1;
		int fallbackMonsterType = NOTHING;
		struct MonsterVariant_t
		{
			DynamicString name = "";
			int chance = 0;
		};
		DynamicArrayT<MonsterVariant_t> variants;
		MonsterCurveEntry(std::string monsterStr, int levelNumMin, int levelNumMax, int chanceNum, std::string fallbackMonsterStr)
		{
			monsterType = getMonsterTypeFromString(monsterStr);
			fallbackMonsterType = getMonsterTypeFromString(fallbackMonsterStr);
			levelmin = levelNumMin;
			levelmax = levelNumMax;
			chance = chanceNum;
		};
		void addVariant(std::string variantName, int chance);
	};

	class LevelCurve
	{
	public:
		DynamicString mapName = "";
		DynamicArrayT<MonsterCurveEntry> monsterCurve;
		DynamicArrayT<MonsterCurveEntry> fixedSpawns;
	};

	DynamicArrayT<LevelCurve> allLevelCurves;

	struct FollowerGenerateDetails_t
	{
		real_t x = 0.0;
		real_t y = 0.0;
		int leaderType = NOTHING;
		Uint32 uid = 0;
		DynamicString followerName = "";
	};
	DynamicArrayT<FollowerGenerateDetails_t> followersToGenerateForLeaders;
	inline bool inUse() { return usingCustomManager; };

	void readFromFile(Uint32 seed);

	static int getMonsterTypeFromString(std::string monsterStr);
	void printCurve(DynamicArrayT<LevelCurve> toPrint)
	{
		return;
		for ( LevelCurve curve : toPrint )
		{
			printlog("Map Name: %s", curve.mapName.c_str());
			for ( MonsterCurveEntry monsters : curve.monsterCurve )
			{
				printlog("[MonsterCurveCustomManager]: Monster: %s | lvl: %d-%d | chance: %d | fallback type: %s", monstertypename[monsters.monsterType],
					monsters.levelmin, monsters.levelmax, monsters.chance, monstertypename[monsters.fallbackMonsterType]);
			}
		}
	}
	bool curveExistsForCurrentMapName(std::string currentMap);
	int rollMonsterFromCurve(std::string currentMap);
	std::string rollMonsterVariant(DynamicString currentMap, int monsterType);
	std::string rollFixedMonsterVariant(DynamicString currentMap, int monsterType);

	void createMonsterFromFile(Entity* entity, Stat* myStats, const std::string& filename, Monster& outMonsterType);

	void generateFollowersForLeaders();

	void writeSampleToDocument();

	void writeToFile(JsonNode d);
};
extern MonsterCurveCustomManager monsterCurveCustomManager;

class GameplayCustomManager
{
public:
	bool usingCustomManager = false;
	int xpShareRange = XPSHARERANGE;
	std::pair<DynamicSetI32, DynamicSetI32> minotaurForceEnableFloors;
	std::pair<DynamicSetI32, DynamicSetI32> minotaurForceDisableFloors;
	std::pair<DynamicSetI32, DynamicSetI32> hungerDisableFloors;
	std::pair<DynamicSetI32, DynamicSetI32> herxChatterDisableFloors;
	std::pair<DynamicSetI32, DynamicSetI32> minimapDisableFloors;
	int globalXPPercent = 100;
	int globalGoldPercent = 100;
	bool minimapShareProgress = false;
	int playerWeightPercent = 100;
	double playerSpeedMax = 12.5;
	inline bool inUse() { return usingCustomManager; };
	void resetValues();

	class MapGeneration
	{
	public:
		MapGeneration(std::string name) { mapName = name; };
		DynamicString mapName = "";
		DynamicArrayStr trapTypes;
		DynamicSetI32 minoFloors;
		DynamicSetI32 darkFloors;
		DynamicSetI32 shopFloors;
		DynamicSetI32 npcSpawnFloors;
		bool usingTrapTypes = false;
		int minoPercent = -1;
		int shopPercent = -1;
		int darkPercent = -1;
		int npcSpawnPercent = -1;
	};

	DynamicArrayT<MapGeneration> allMapGenerations;
	bool mapGenerationExistsForMapName(std::string name);
	MapGeneration* getMapGenerationForMapName(std::string name);

	void writeAllToDocument();

	void writeToFile(JsonNode d);

	void readFromFile();

	bool readKeyToGameplayProperty(JsonMemberIt itr);

	bool readKeyToMapGenerationProperty(MapGeneration& m, JsonMemberIt itr);

	bool processedMinotaurSpawn(int level, bool secret, std::string mapName);

	bool processedDarkFloor(int level, bool secret, std::string mapName);

	bool processedShopFloor(int level, bool secret, std::string mapName, bool& shoplevel);

	enum PropertyTypes : int
	{
		PROPERTY_NPC
	};

	bool processedPropertyForFloor(int level, bool secret, std::string mapName, PropertyTypes propertyType, bool& bOut);
};
template <> struct DynamicArrayKindOf<MonsterCurveCustomManager::FollowerGenerateDetails_t> { static constexpr int value = Kind_FollowerDetails; };
template <> struct DynamicArrayKindOf<MonsterStatCustomManager::StatEntry::VariantPair_t> { static constexpr int value = Kind_VariantPair; };
template <> struct DynamicArrayKindOf<MonsterCurveCustomManager::MonsterCurveEntry::MonsterVariant_t> { static constexpr int value = Kind_VariantPair; };
template <> struct DynamicArrayKindOf<MonsterCurveCustomManager::MonsterCurveEntry> { static constexpr int value = Kind_MonsterCurveEntry; };
template <> struct DynamicArrayKindOf<MonsterCurveCustomManager::LevelCurve> { static constexpr int value = Kind_LevelCurve; };
template <> struct DynamicArrayKindOf<GameplayCustomManager::MapGeneration> { static constexpr int value = Kind_MapGeneration; };




extern GameplayCustomManager gameplayCustomManager;

class GameModeManager_t
{
public:
	enum GameModes : int
	{
		GAME_MODE_DEFAULT,
		GAME_MODE_TUTORIAL_INIT,
		GAME_MODE_TUTORIAL,
		GAME_MODE_CUSTOM_RUN_ONESHOT,
		GAME_MODE_CUSTOM_RUN
	};
	GameModes currentMode = GAME_MODE_DEFAULT;
	GameModes getMode() const { return currentMode; };
	void setMode(const GameModes mode);
	bool allowsSaves();
	bool isFastDeathGrave();
	bool allowsBoulderBreak();
	bool allowsHiscores();
	bool allowsStatisticsOrAchievements(const char* achName, int statIndex);
	bool allowsGlobalHiscores();

	class CurrentSession_t
	{
	public:
		Uint32 serverFlags = 0;
		bool bHasSavedServerFlags = false;
		void restoreSavedServerFlags();
		void saveServerFlags();

		class SeededRun_t
		{
		public:
			DynamicString seedString;
			Uint32 seed = 0;
			void setup(DynamicString _seedString);
			void reset();

			static DynamicArrayStr prefixes;
			static DynamicArrayStr suffixes;
			static void readSeedNamesFromFile();
		} seededRun;

		class ChallengeRun_t
		{
			bool inUse = false;
		public:
			enum ChallengeEvents_t
			{
				CHEVENT_XP_250,
				CHEVENT_NOXP_LVL_20,
				CHEVENT_SHOPPING_SPREE,
				CHEVENT_BFG,
				CHEVENT_KILLS_FURNITURE,
				CHEVENT_KILLS_MONSTERS,
				CHEVENT_NOSKILLS,
				CHEVENT_STRONG_TRAPS,
				CHEVENT_ENUM_END
			};

			bool isActive() { return inUse; }
			bool isActive(ChallengeEvents_t _eventType);
			DynamicString scenarioStr = "";
			DynamicString lid = "";
			int lid_version = -1;
			Uint32 seed = 0;
			DynamicString seed_word = "";
			Uint32 lockedFlags = 0;
			Uint32 setFlags = 0;
			int classnum = -1;
			int race = -1;
			bool customBaseStats = false;
			bool customAddStats = false;
			Stat* baseStats = nullptr;
			Stat* addStats = nullptr;
			int eventType = -1;
			int winLevel = -1;
			int startLevel = -1;
			int winCondition = -1;
			int globalXPPercent = 100;
			int globalGoldPercent = 100;
			int playerWeightPercent = 100;
			double playerSpeedMax = 12.5;
			int numKills = -1;

			void setup(std::string _scenario);
			void reset();
			bool loadScenario();
			void applySettings();
			void updateKillEvent(Entity* entity);
		} challengeRun;
	} currentSession;

	bool isServerflagDisabledForCurrentMode(int i);

	class Tutorial_t
	{
		DynamicString currentMap = "";
		const Uint32 kNumTutorialLevels = 10;
	public:
		void init();
		int dungeonLevel = -1;
		bool showFirstTutorialCompletedPrompt = false;
		bool firstTutorialCompleted = false;
		void createFirstTutorialCompletedPrompt();
		void setTutorialMap(std::string& mapname);
		void launchHub();
		void startTutorial(std::string mapToSet);
		static void buttonReturnToTutorialHub(button_t* my);
		static void buttonRestartTrial(button_t* my);
		const Uint32 getNumTutorialLevels() { return kNumTutorialLevels; }
		void openGameoverWindow();
		void onMapRestart(int levelNum);

		class Menu_t
		{
			bool bWindowOpen = false;
		public:
			bool isOpen() { return bWindowOpen; }
			void open();
			void close();
			void onClickEntry();
			int windowScroll = 0;
			int selectedMenuItem = -1;
			DynamicString windowTitle = "";
			DynamicString defaultHoverText = "";
		} Menu;

		class FirstTimePrompt_t
		{
			bool bWindowOpen = false;
		public:
			void createPrompt();
			void drawDialogue();
			bool isOpen() { return bWindowOpen; }
			void close();
			bool doButtonSkipPrompt = false;
			bool showFirstTimePrompt = false;
			static void buttonSkipPrompt(button_t* my);
			static void buttonPromptEnterTutorialHub(button_t* my);
		} FirstTimePrompt;

		class Level_t
		{
		public:
			Level_t()
			{
				filename = "";
				title = "";
				description = "";
				completionTime = 0;
			};
			DynamicString filename;
			DynamicString title;
			DynamicString description;
			Uint32 completionTime;
		};
		DynamicArrayT<Level_t> levels;

		void readFromFile();
		void writeToDocument();

#if defined(LINUX)
		const DynamicString tutorialScoresFilename = "/savegames/tutorial_scores_2.json";
#else
		const DynamicString tutorialScoresFilename = "/savegames/tutorial_scores.json";
#endif
		void writeToFile(JsonNode d);
	} Tutorial;
};
extern GameModeManager_t gameModeManager;
template <> struct DynamicArrayKindOf<GameModeManager_t::Tutorial_t::Level_t> { static constexpr int value = Kind_LevelT; };

class IRCHandler_t
{
	IPaddress ip;
	TCPsocket net_ircsocket = nullptr;
	TCPsocket net_ircsocket_from_server = nullptr;
	SDLNet_SocketSet net_ircsocketset = nullptr;
	bool bSocketConnected = false;
	const unsigned int MAX_BUFFER_LEN = 1024;
	DynamicArray recvBuffer;
	struct Auth_t
	{
		DynamicString oauth = "";
		DynamicString chatroom = "";
		DynamicString username = "";
	} auth;
public:
	IRCHandler_t()
	{
		ip.host = 0;
		ip.port = 0;
		barony_dynamic_array_resize(&recvBuffer, 1, (int32_t)MAX_BUFFER_LEN);
		if (recvBuffer.data) memset(recvBuffer.data, 0, recvBuffer.len);
	}
	int packetSend(std::string data);
	int packetReceive();
	void handleMessage(std::string& msg);
	void run();
	bool connect();
	void disconnect();
	bool readFromFile();
};
extern IRCHandler_t IRCHandler;

class ItemTooltips_t
{
	struct tmpItem_t
	{
		DynamicString internalName = "nothing";
		Sint32 itemId = -1;
		Sint32 fpIndex = -1;
		Sint32 tpIndex = -1;
		Sint32 tpShortIndex = -1;
		Sint32 gold = 0;
		Sint32 weight = 0;
		Sint32 itemLevel = -1;
		DynamicString category = "nothing";
		DynamicString equipSlot = "nothing";
		DynamicArrayStr imagePaths;
		DynamicMapI32 attributes;
		DynamicString tooltip = "tooltip_default";
		DynamicString iconLabelPath = "";
	};

public:
	enum SpellItemTypes : int
	{
		SPELL_TYPE_DEFAULT,
		SPELL_TYPE_PROJECTILE,
		SPELL_TYPE_PROJECTILE_SHORT_X3,
		SPELL_TYPE_SELF,
		SPELL_TYPE_AREA,
		SPELL_TYPE_SELF_SUSTAIN,
		SPELL_TYPE_TOUCH_FLOOR,
		SPELL_TYPE_TOUCH_ALLY,
		SPELL_TYPE_TOUCH_ENEMY,
		SPELL_TYPE_TOUCH_ENTITY,
		SPELL_TYPE_TOUCH_WALL,
		SPELL_TYPE_DIVINE_TARGET
	};
	enum SpellTagTypes : int
	{
		SPELL_TAG_DAMAGE,
		SPELL_TAG_UTILITY,
		SPELL_TAG_STATUS_EFFECT,
		SPELL_TAG_HEALING,
		SPELL_TAG_CURE,
		SPELL_TAG_BASIC_HIT_MESSAGE,
		SPELL_TAG_TRACK_HITS,
		SPELL_TAG_BUFF,
		SPELL_TAG_SPELLBOOK_SCALING,
		SPELL_TAG_BONUS_AS_EFFECT_POWER
	};
private:
	struct spellItem_t
	{
		Sint32 id;
		DynamicString internalName;
		DynamicString name;
		DynamicString name_lowercase;
		DynamicString spellTypeStr;
		SpellItemTypes spellType;
		DynamicString spellbookInternalName;
		DynamicString magicstaffInternalName;
		DynamicString fociInternalName;
		Sint32 spellbookId = -1;
		Sint32 magicstaffId = -1;
		Sint32 fociId = -1;
		DynamicArrayStr spellTagsStr;
		DynamicSetI32 spellTags;
		DynamicArrayStr spellFormatTags;
		DynamicArrayS32 spellbookItemIconPaddingLines;
		DynamicSetI32 spellLevelTags;

		bool hasExpandedJSON = false;
		int damage = 0;
		int damage2 = 0;
		real_t damage_mult = 1.0;
		real_t damage2_mult = 1.0;
		int duration = 0;
		real_t duration_mult = 1.0;
		int duration2 = 0;
		real_t duration2_mult = 1.0;
		int mana = 1;
		real_t distance = 0.0;
		real_t distance_mult = 1.0;
		int life_time = 0;
		real_t life_mult = 1.0;
		real_t cast_time = 1.0;
		real_t cast_time_mult = 1.0;
		int skillID = PRO_SORCERY;
		int difficulty = 100;
		int sustain_mana = 0;
		int sustain_duration = 0;
		real_t sustain_mult = 1.0;
		real_t radius = 0;
		real_t radius_mult = 0.0;
		int drop_table = -1;
	};

	Uint32 defaultHeadingTextColor = 0xFFFFFFFF;
	Uint32 defaultIconTextColor = 0xFFFFFFFF;
	Uint32 defaultDescriptionTextColor = 0xFFFFFFFF;
	Uint32 defaultDetailsTextColor = 0xFFFFFFFF;
	Uint32 defaultPositiveTextColor = 0xFFFFFFFF;
	Uint32 defaultNegativeTextColor = 0xFFFFFFFF;
	Uint32 defaultStatusEffectTextColor = 0xFFFFFFFF;
	Uint32 defaultFaintTextColor = 0xFFFFFFFF;

public:
		typedef ItemTooltipIcons_tMirror ItemTooltipIcons_t;

	struct ItemTooltip_t
	{
		Uint32 headingTextColor = 0;
		Uint32 descriptionTextColor = 0;
		Uint32 detailsTextColor = 0;
		Uint32 positiveTextColor = 0;
		Uint32 negativeTextColor = 0;
		Uint32 statusEffectTextColor = 0;
		Uint32 faintTextColor = 0;
		DynamicArrayIcon icons;
		DynamicArrayStr descriptionText;
		DynamicMapStrArrStr detailsText;
		DynamicArrayStr detailsTextInsertOrder;
		DynamicMapI32 minWidths;
		DynamicMapI32 maxWidths;
		DynamicMapI32 headerMaxWidths;
		void setColorHeading(Uint32 color);
		void setColorDescription(Uint32 color);
		void setColorDetails(Uint32 color);
		void setColorPositive(Uint32 color);
		void setColorNegative(Uint32 color);
		void setColorStatus(Uint32 color);
		void setColorFaintText(Uint32 color);
	};
	void setSpellValueIfKeyPresent(spellItem_t& t, JsonMemberIt item_itr, Uint32& hash, Uint32& hashShift, const char* key, int& toSet);
	void setSpellValueIfKeyPresent(spellItem_t& t, JsonMemberIt item_itr, Uint32& hash, Uint32& hashShift, const char* key, real_t& toSet);
	void readItemsFromFile();
	static const Uint32 kItemsJsonHash;
	static Uint32 itemsJsonHashRead;
	void readItemLocalizationsFromFile(bool forceLoadBaseDirectory = false);
	void readTooltipsFromFile(bool forceLoadBaseDirectory = false);
	void readBookLocalizationsFromFile(bool forceLoadBaseDirectory = false);
	DynamicArrayT<tmpItem_t> tmpItems;
	DynamicMapI32T<spellItem_t> spellItems;
	DynamicMapStrT<ItemTooltip_t> tooltips;
	DynamicMapStrT<DynamicMapStr> adjectives;
	DynamicMapStrArrStr templates;
	//std::vector<std::pair<int, Sint32>> itemValueTable;
	//std::map<int, std::vector<std::pair<int, Sint32>>> itemValueTableByCategory;
	typedef ItemLocalization_tMirror ItemLocalization_t;
	DynamicMapItemLoc itemNameLocalizations;
	DynamicMapStr bookNameLocalizations;
	DynamicMapStr spellNameLocalizations;
	DynamicMapI32 itemNameStringToItemID;
	DynamicMapI32 spellNameStringToSpellID;
	DynamicString defaultString = "";
	char buf[2048];
	bool autoReload = false;
	bool itemDebug = false;
	DynamicString getItemStatusAdjective(Uint32 itemType, Status status);
	DynamicString getItemBeatitudeAdjective(Sint16 beatitude);
	DynamicString getItemPotionAlchemyAdjective(const int player, Uint32 itemType);
	DynamicString getItemPotionHarmAllyAdjective(Item& item);
	DynamicString getItemProficiencyName(int proficiency);
	DynamicString getItemSlotName(ItemEquippableSlot slotname);
	DynamicString getItemStatShortName(const char* attribute);
	DynamicString getItemStatFullName(const char* attribute);
	DynamicString getItemEquipmentEffectsForIconText(std::string& attribute);
	DynamicString getItemEquipmentEffectsForAttributesText(std::string& attribute);
	DynamicString getProficiencyLevelName(Sint32 proficiencyLevel);
	const char* getIconLabel(Item& item);
	std::string getSpellIconText(const int player, Item& item, const bool excludePlayerStats);
	std::string getSpellIconFormatText(const int player, Item& item, std::string& format, const spell_t* spell, const int iconIndex, const bool compendiumTooltipIntro);
	std::string getSpellDescriptionText(const int player, Item& item);
	std::string getSpellIconPath(const int player, Item& item, int spellID);
	std::string getCostOfSpellString(const int player, Item& item);
	DynamicString getSpellTypeString(const int player, Item& item);
	node_t* getSpellNodeFromSpellID(int spellID);
	real_t getSpellSustainCostPerSecond(int spellID);
	int getSpellDamageOrHealAmount(const int player, spell_t* spell, Item* spellbook, const bool excludePlayerStats);
	bool bIsSpellDamageOrHealingType(spell_t* spell);
	bool bSpellHasBasicHitMessage(const int spellID);

	void formatItemIcon(const int player, std::string tooltipType, Item& item, std::string& str, int iconIndex, std::string& conditionalAttribute, Frame* parentFrame = nullptr);
	void formatItemIcon(const int player, std::string tooltipType, Item& item, DynamicString& str, int iconIndex, DynamicString& conditionalAttribute, Frame* parentFrame = nullptr);
	void formatItemDescription(const int player, std::string tooltipType, Item& item, std::string& str);
	void formatItemDescription(const int player, std::string tooltipType, Item& item, DynamicString& str);
	void formatItemDetails(const int player, std::string tooltipType, Item& item, std::string& str, std::string detailTag, Frame* parentFrame = nullptr);
	void formatItemDetails(const int player, std::string tooltipType, Item& item, DynamicString& str, DynamicString detailTag, Frame* parentFrame = nullptr);
	void stripOutPositiveNegativeItemDetails(std::string& str, std::string& positiveValues, std::string& negativeValues);
	void stripOutPositiveNegativeItemDetails(DynamicString& str, DynamicString& positiveValues, DynamicString& negativeValues);
	void stripOutHighlightBracketText(std::string& str, std::string& bracketText);
	void stripOutHighlightBracketText(DynamicString& str, DynamicString& bracketText);
	void getWordIndexesItemDetails(void* field, std::string& str, std::string& highlightValues, std::string& positiveValues, std::string& negativeValues,
		DynamicMapI32T<Uint32>& highlightIndexes, DynamicMapI32T<Uint32>& positiveIndexes, DynamicMapI32T<Uint32>& negativeIndexes, ItemTooltip_t& tooltip);
	void getWordIndexesItemDetails(void* field, DynamicString& str, DynamicString& highlightValues, DynamicString& positiveValues, DynamicString& negativeValues,
		DynamicMapI32T<Uint32>& highlightIndexes, DynamicMapI32T<Uint32>& positiveIndexes, DynamicMapI32T<Uint32>& negativeIndexes, ItemTooltip_t& tooltip);
};
template <> struct DynamicArrayKindOf<ItemTooltips_t::tmpItem_t> { static constexpr int value = Kind_TmpItem; };
template <> struct MapValueKindOf<ItemTooltips_t::spellItem_t> { static constexpr int value = MK_SpellItem; };
template <> struct MapValueKindOf<ItemTooltips_t::ItemTooltip_t> { static constexpr int value = MK_ItemTooltip; };
template <> struct MapValueKindOf<DynamicMapStr> { static constexpr int value = MK_StrMapStr; };

extern ItemTooltips_t ItemTooltips;

class StatueManager_t
{
public:
	StatueManager_t() {};
	~StatueManager_t() {};
	int processStatueExport();

	bool activeEditing = false;
	Uint32 lastEntityUnderMouse = 0;
	Uint32 editingPlayerUid = 0;
	real_t statueEditorHeightOffset = 0.0;
	bool drawGreyscale = false;
	void readStatueFromFile(int index, std::string filename);
	void readAllStatues();
	void refreshAllStatues();
	void resetStatueEditor();
	static Uint32 statueId;
	DynamicString exportFileName = "";
	int exportRotations = 0;
	bool exportActive = false;
	JsonNode exportDocument;

	class Statue_t
	{
		Uint32 id;
	public:
		struct StatueLimb_t
		{
			real_t x;
			real_t y;
			real_t z;
			real_t pitch;
			real_t roll;
			real_t yaw;
			real_t focalx;
			real_t focaly;
			real_t focalz;
			Sint32 sprite;
			bool visible = true;
		};
		DynamicMapStrT<DynamicArrayT<StatueLimb_t>> limbs;
		Statue_t() {
			id = statueId; 
			++statueId;
		}
		real_t heightOffset = 0.0;
	};

	const DynamicArrayStr directionKeys{ "east", "south", "west", "north" };
	DynamicMapI32T<Statue_t> allStatues;
};  // StatueManager_t
template <> struct MapValueKindOf<StatueManager_t::Statue_t> { static constexpr int value = MK_Statue; };

// MapValueKindOf for StatueLimb_t POD arrays (kind 20 = MK_StatueLimbArray)
template <> struct MapValueKindOf<DynamicArrayT<StatueManager_t::Statue_t::StatueLimb_t>> {
    static constexpr int value = MK_StatueLimbArray;
};
extern StatueManager_t StatueManager;

class DebugTimers_t
{
	std::map<std::string, std::vector<std::pair<std::string, std::chrono::high_resolution_clock::time_point>>> timepoints;
public:
	void addTimePoint(std::string key, DynamicString desc = "");
	void printTimepoints(std::string key, int& posy);
	void clearTimepoints(std::string key);
	void clearAllTimepoints();
	void printAllTimepoints();
};
extern DebugTimers_t DebugTimers;

class GlyphRenderer_t
{
	DynamicString baseSourceFolder = "";
	DynamicString renderedGlyphFolder = "";
	DynamicString basePressedGlyphPath = "";
	DynamicString baseUnpressedGlyphPath = "";
	DynamicString pressedRenderedPrefix = "";
	DynamicString unpressedRenderedPrefix = "";
	DynamicString defaultstring = "";
public:
	struct GlyphData_t
	{
		DynamicString keyname = "";
		DynamicString folder = "";
		DynamicString fullpath = "";
		DynamicString pressedRenderedFullpath = "";
		DynamicString unpressedRenderedFullpath = "";
		DynamicString filename = "";
		DynamicString unpressedGlyphPath = "";
		DynamicString pressedGlyphPath = "";
		int render_offsetx = 0;
		int render_offsety = 0;
		int keycode = SDLK_UNKNOWN;
	};
	DynamicMapI32T<GlyphData_t> allGlyphs;

	GlyphRenderer_t() {};
	~GlyphRenderer_t() {};
	bool readFromFile();
	void renderGlyphsToPNGs();
	DynamicString& getGlyphPath(int scancode, bool pressed = false);
};
// MapValueKindOf for GlyphRenderer_t::GlyphData_t (owns 8 DynamicStrings) — kind 27 = MK_GlyphData
template <> struct MapValueKindOf<GlyphRenderer_t::GlyphData_t> { static constexpr int value = MK_GlyphData; };
extern GlyphRenderer_t GlyphHelper;

bool charIsWordSeparator(char c);

class ScriptTextParser_t
{
public:
	ScriptTextParser_t() {};
	~ScriptTextParser_t() {};
	void readAllScripts();
	bool readFromFile(const std::string& filename);
	void writeWorldSignsToFile();
	enum ObjectType_t : int {
		OBJ_SIGN,
		OBJ_MESSAGE,
		OBJ_SCRIPT,
		OBJ_BUBBLE_SIGN,
		OBJ_BUBBLE_GRAVE,
		OBJ_BUBBLE_DIALOGUE
	};
	enum VariableTypes : int {
		TEXT,
		GLYPH,
		IMG,
		SCRIPT,
		COLOR_R,
		COLOR_G,
		COLOR_B
	};

	struct Entry_t
	{
		DynamicString name = "";
		DynamicArrayStr rawText;
		typedef EntryVariable_tMirror Variable_t;
		DynamicArrayEntryVar variables;
		DynamicString formattedText = "";
		ObjectType_t objectType = OBJ_MESSAGE;
		int hjustify = 4;
		int vjustify = 0;
		DynamicArrayS32 padPerLine;
		int padTopY = 0;
		DynamicString font = "";
		Uint32 fontColor = 0xFFFFFFFF;
		Uint32 fontOutlineColor = 0xFFFFFFFF;
		Uint32 fontHighlightColor = 0xFFFFFFFF;
		Uint32 fontHighlight2Color = 0xFFFFFFFF;
		DynamicArrayS32 wordHighlights;
		DynamicArrayS32 wordHighlights2;
		int imageInlineTextAdjustX = 0; // when img is placed inbetween text, move it by this adjustment to center
		struct AdditionalContentProperties_t
		{
			SDL_Rect pos{0, 0, 0, 0};
			DynamicString path = "";
			DynamicString bgPath = "";
			int imgBorder = 0;
		};
		AdditionalContentProperties_t signVideoContent;
	};
	DynamicMapStrT<Entry_t> allEntries;
};
extern ScriptTextParser_t ScriptTextParser;
template <> struct MapValueKindOf<ScriptTextParser_t::Entry_t> { static constexpr int value = MK_Entry; };

#ifndef EDITOR
//#define USE_THEORA_VIDEO
#endif // !EDITOR
#ifdef USE_THEORA_VIDEO
#include "../third_party/theoraplay/theoraplay.h"
class VideoManager_t
{
	THEORAPLAY_Decoder* decoder = nullptr;
	static bool isInit;
	bool started = false;
	bool whichTexture = false;
	GLuint textureId1 = 0;
	GLuint textureId2 = 0;
	Uint32 lastTime = 0;
	Uint32 currentPlayTime = 0;
	bool loopVideo = false;
	const THEORAPLAY_VideoFrame* pendingFrame = nullptr;
	int videoWidth = 0;
	int videoHeight = 0;
	void drawTexturedQuad(unsigned int texID, int tw, int th, const SDL_Rect& src, const SDL_Rect& dest, float alpha);
	GLuint createTexture(int w, int h, unsigned int format);
	int potCeil(int value)
	{
		--value;
		value |= value >> 1;
		value |= value >> 2;
		value |= value >> 4;
		value |= value >> 8;
		value |= value >> 16;
		++value;
		return value;
	}
	void destroyClip();
	void updateCurrentClip(float timeDelta);
	static void destroy();
	void draw();
	static void init();
	DynamicString currentfile = "";
	DynamicString currentfilePath = "";
public:
	VideoManager_t() {};
	~VideoManager_t() { destroyClip(); };
	void drawAsFrameCallback(const Widget& widget, SDL_Rect frameSize, SDL_Rect offset, float alpha);
	void update();
	void loadfile(const char* filename);
	bool isPlaying(const char* filename) { return currentfile == filename && (decoder != nullptr); }
	void stop() { destroyClip(); }
	static void deinitManager();
};
extern VideoManager_t VideoManager[MAXPLAYERS];
#endif

#ifndef EDITOR
#endif

#ifndef EDITOR
struct ShopkeeperConsumables_t
{
	struct ItemEntry
	{
		DynamicArrayS32 type;
		DynamicArrayS32 status;
		DynamicArrayS32 beatitude;
		DynamicArrayS32 count;
		DynamicArrayU32 appearance;
		DynamicArrayS32 identified;
		
		int percentChance = 100;
		int weightedChance = 1;
		int dropChance = 0;
		bool emptyItemEntry = false;
		bool dropItemOnDeath = false;
	};
	struct StoreSlots_t
	{
		int slotTradingReq = 0;
		DynamicArrayT<ItemEntry> itemEntries;
	};
	static int consumableBuyValueMult;
	static DynamicMapI32T<DynamicArrayT<StoreSlots_t>> entries; // shop type as key
	static void readFromFile();
};
template <> struct DynamicArrayKindOf<ShopkeeperConsumables_t::ItemEntry> { static constexpr int value = Kind_ShopkeeperItem; };
template <> struct DynamicArrayKindOf<ShopkeeperConsumables_t::StoreSlots_t> { static constexpr int value = Kind_StoreSlots; };
template <> struct MapValueKindOf<DynamicArrayT<ShopkeeperConsumables_t::StoreSlots_t>> { static constexpr int value = MK_StoreSlotsArray; };


struct ClassHotbarConfig_t
{
	struct HotbarEntry_t
	{
		DynamicArrayS32 itemTypes;
		DynamicArrayS32 itemCategories;
		int slotnum = -1;
		HotbarEntry_t(int _slotnum)
		{
			slotnum = _slotnum;
		};
	};
	struct ClassHotbar_t
	{
		struct ClassHotbarLayout_t
		{
			DynamicArrayT<HotbarEntry_t> hotbar;
			DynamicArrayT<DynamicArrayT<HotbarEntry_t>> hotbar_alternates;
			void init();
			bool hasData = false;
		};
		ClassHotbarLayout_t layoutClassic;
		ClassHotbarLayout_t layoutModern;
	};
	static ClassHotbar_t ClassHotbarsDefault[NUMCLASSES];
	static ClassHotbar_t ClassHotbars[NUMCLASSES];
	static void assignHotbarSlots(const int player);
	enum HotbarConfigType : int
	{
		HOTBAR_LAYOUT_DEFAULT_CONFIG,
		HOTBAR_LAYOUT_CUSTOM_CONFIG
	};
	enum HotbarConfigWriteMode : int
	{
		HOTBAR_CONFIG_WRITE,
		HOTBAR_CONFIG_DELETE
	};
	static void readFromFile(HotbarConfigType fileReadType);
	static void writeToFile(HotbarConfigType fileWriteType, HotbarConfigWriteMode writeMode);
	static void init();
};
template <> struct DynamicArrayKindOf<ClassHotbarConfig_t::HotbarEntry_t> { static constexpr int value = Kind_HotbarEntry; };
template <> struct DynamicArrayKindOf<DynamicArrayT<ClassHotbarConfig_t::HotbarEntry_t>> { static constexpr int value = Kind_HotbarEntryArray; };


struct LocalAchievements_t
{
	typedef Achievement_tMirror Achievement_t;
	struct Statistic_t
	{
		DynamicString name;
		int value = 0;
	};
	DynamicMapAchievement achievements;
	DynamicMapI32T<Statistic_t> statistics;
	static void readFromFile();
	static void writeToFile();
	static void init();
	void updateAchievement(const char* name, const bool unlocked);
	void updateStatistic(const int stat_num, const int value);
};
// MapValueKindOf for LocalAchievements_t::Statistic_t (owns 1 DynamicString) — kind 28 = MK_Statistic
template <> struct MapValueKindOf<LocalAchievements_t::Statistic_t> { static constexpr int value = MK_Statistic; };
extern LocalAchievements_t LocalAchievements;

class GameplayPreferences_t
{
	//Player& player;
	int player = -1;
public:
	enum GameplayerPrefIndexes : int
	{
		GPREF_ARACHNOPHOBIA = 0,
		GPREF_COLORBLIND,
		GPREF_VOICE_NO_SEND,
		GPREF_VOICE_NO_RECV,
		GPREF_VOICE_PTT,
		GPREF_ENUM_END
	};
	struct GameplayPreference_t
	{
		int value = 0;
		bool needsUpdate = true;
		void set(const int _value);
		void reset();
	};
	GameplayPreference_t preferences[GPREF_ENUM_END];
	bool isInit = false;
	/*GameplayPreferences_t(Player& p) : player(p)
	{};*/
	GameplayPreferences_t() {};
	~GameplayPreferences_t() {};
	Uint32 lastUpdateTick = 0;
	void requestUpdateFromClient();
	void sendToClients(const int targetPlayer);
	void process();
	void sendToServer();
	static void receivePacket();

	enum GameConfigIndexes : int
	{
		GOPT_ARACHNOPHOBIA = 0,
		GOPT_COLORBLIND,
		GOPT_VOICE_NO_SEND,
		GOPT_VOICE_NO_RECV,
		GOPT_VOICE_PTT,
		GOPT_ENUM_END
	};
	static GameplayPreference_t gameConfig[GOPT_ENUM_END];
	static int getGameConfigValue(GameConfigIndexes index);
	static void serverProcessGameConfig();
	static void serverUpdateGameConfig();
	static void receiveGameConfig();
	static Uint32 lastGameConfigUpdateTick;
	static void reset();
};

extern GameplayPreferences_t gameplayPreferences[MAXPLAYERS];
#endif

struct EditorEntityData_t
{
	struct EntityColliderData_t
	{
		int gib = 0;
		DynamicArrayS32 gib_hit;
		DynamicArrayS32 sfxBreak;
		int sfxHit = 0;
		DynamicString damageCalculationType = "default";
		DynamicString name = "";
		DynamicString hpbarLookupName = "object";
		int entityLangEntry = 4335;
		int hitMessageLangEntry = 2509;
		int breakMessageLangEntry = 2510;
		DynamicMapStrT<DynamicArrayS32> hideMonsters;
		DynamicArrayS32 spellTriggers;
		DynamicSetI32 pathableMonsters;
		int colliderJumpLangEntry = 6234;
		DynamicMapI32 overrideProperties;
		bool hasOverride(std::string key) const;
		int getOverride(std::string key) const;
	};
	typedef ColliderDmgProperties_tMirror ColliderDmgProperties_t;
	static const int COLLIDER_COLLISION_FLAG_MINO = 2;
	static const int COLLIDER_COLLISION_FLAG_NPC = 4;
	static DynamicMapColliderDmg colliderDmgTypes;
	static DynamicMapI32T<EntityColliderData_t> colliderData;
	static DynamicMapStrI32Map colliderRandomGenPool;
	static DynamicMapI32 colliderNameIndexes;
	static int getColliderIndexFromName(std::string name);
	static void readFromFile();
};
extern EditorEntityData_t editorEntityData;
template <> struct MapValueKindOf<EditorEntityData_t::EntityColliderData_t> { static constexpr int value = MK_EntityColliderData; };

struct Mods
{
	static DynamicArrayS32 modelsListModifiedIndexes;
	static DynamicArrayS32 soundsListModifiedIndexes;
	static DynamicArraySurfacePtrStringPair systemResourceImagesToReload;
	static DynamicArrayStringPair mountedFilepaths;
	static DynamicArrayStringPair mountedFilepathsSaved; // saved from config file
	static DynamicSetStr mods_loaded_local;
	static DynamicSetStr mods_loaded_workshop;
	static DynamicArrayStr localModFoldernames;
	static int numCurrentModsLoaded;
	static bool modelsListRequiresReloadUnmodded;
	static bool soundListRequiresReloadUnmodded;
	static bool tileListRequireReloadUnmodded;
	static bool spriteImagesRequireReloadUnmodded;
	static bool booksRequireReloadUnmodded;
	static bool musicRequireReloadUnmodded;
	static bool langRequireReloadUnmodded;
	static bool monsterLimbsRequireReloadUnmodded;
	static bool systemImagesReloadUnmodded;
	static bool customContentLoadedFirstTime;
	static bool disableSteamAchievements;
	static bool lobbyDisableSteamAchievements;
	static bool isLoading;
	static void updateModCounts();
	static bool mountAllExistingPaths();
	static bool clearAllMountedPaths();
	static bool removePathFromMountedFiles(std::string findStr);
	static bool isPathInMountedFiles(std::string findStr);
	static void unloadMods(bool force = false);
	static void loadMods();
	static void loadModels(int start, int end);
	static void verifyAchievements(const char* fullpath, bool ignoreBaseFolder);
	static bool verifyMapFiles(const char* file, bool ignoreBaseFolder);
	static int createBlankModDirectory(std::string foldername);
	static void writeLevelsTxtAndPreview(std::string modFolder);
};

#ifdef USE_LIBCURL
struct LibCURL_t
{
	bool bInit = false;
	CURL* handle = nullptr;
	void init()
	{
		curl_global_init(CURL_GLOBAL_DEFAULT);
		if ( handle = curl_easy_init() )
		{
			bInit = true;
		}
	}
	void download(std::string filename, std::string url);

	~LibCURL_t()
	{
		curl_easy_cleanup(handle);
		handle = nullptr;
	}

	static size_t write_data_fp(void* ptr, size_t size, size_t nmemb, File* stream);
	static size_t write_data_string(void* ptr, size_t size, size_t nmemb, std::string* s);
};
extern LibCURL_t LibCURL;
#endif

struct EquipmentModelOffsets_t
{
	struct ModelOffset_t
	{
		real_t focalx = 0.0;
		real_t focaly = 0.0;
		real_t focalz = 0.0;
		real_t scalex = 0.0;
		real_t scaley = 0.0;
		real_t scalez = 0.0;
		real_t rotation = 0.0;
		real_t pitch = 0.0;
		real_t x = 0.0;
		real_t y = 0.0;
		real_t z = 0.0;
		int limbsIndex = 0;
		bool oversizedMask = false;
		bool expandToFitMask = false;

		struct AdditionalOffset_t
		{
			real_t focalx = 0.0;
			real_t focaly = 0.0;
			real_t focalz = 0.0;
			real_t scalex = 0.0;
			real_t scaley = 0.0;
			real_t scalez = 0.0;
		};
		DynamicMapI32T<AdditionalOffset_t> adjustToOversizeMask;
		DynamicMapI32T<AdditionalOffset_t> adjustToExpandedHelm;
	};
	DynamicMapI32T<DynamicMapI32T<ModelOffset_t>> monsterModelsMap;
	DynamicMapI32T<ModelOffset_t> miscItemsBaseOffsets;
	void readBaseItemsFromFile();
	void readFromFile(std::string monsterName, int monsterType = NOTHING);
	int modelOffsetExists(int monster, int sprite, int monsterSprite);
	int expandHelmToFitMask(int monster, int helmSprite, int maskSprite, int monsterSprite);
	int maskHasAdjustmentForExpandedHelm(int monster, int helmSprite, int maskSprite, int monsterSprite);
	ModelOffset_t::AdditionalOffset_t getExpandHelmOffset(int monster, int helmSprite, int maskSprite);
	ModelOffset_t::AdditionalOffset_t getMaskOffsetForExpandHelm(int monster, int helmSprite, int maskSprite);
	ModelOffset_t& getModelOffset(int monster, int sprite);
};
extern EquipmentModelOffsets_t EquipmentModelOffsets;
template <> struct MapValueKindOf<EquipmentModelOffsets_t::ModelOffset_t> { static constexpr int value = MK_ModelOffset; };
template <> struct MapValueKindOf<EquipmentModelOffsets_t::ModelOffset_t::AdditionalOffset_t> { static constexpr int value = MK_AdditionalOffset; };
template <> struct MapValueKindOf<DynamicMapI32T<EquipmentModelOffsets_t::ModelOffset_t>> { static constexpr int value = MK_I32MapModelOffset; };

struct Compendium_t
{
	struct CompendiumView_t
	{
		real_t ang = 0.0;
		real_t vang = 0.0;
		real_t pan = 0.0;
		real_t zoom = 0.0;
		real_t height = 0.0;
		real_t rotate = 0.0;
		int rotateState = 0;
		real_t rotateLimitMin = 0.0;
		real_t rotateLimitMax = 0.0;
		real_t rotateSpeed = 1.0;
		bool rotateLimit = true;
		bool inUse = false;
	};

	struct PointsAnim_t
	{
		static real_t anim;
		static Uint32 startTicks;
		static Sint32 pointsCurrent;
		static Sint32 pointsChange;
		static Sint32 txtCurrentPoints;
		static Sint32 txtChangePoints;
		static real_t animNoFunds;
		static Uint32 noFundsTick;
		static bool firstLoad;
		static bool noFundsAnimate;
		static bool showChanged;
		static void reset();
		static void tickAnimate();
		static void noFundsEvent();
		static bool mainMenuAlert;
		static Uint32 countUnreadLastTicks;
		static void countUnreadNotifs();
		static void pointsChangeEvent(Sint32 amount);
	};

	static void readContentsLang(std::string name, DynamicMapStrArrayStringPair& contents,
		DynamicMapStr& contentsMap);
	enum CompendiumUnlockStatus : int {
		LOCKED_UNKNOWN,
		LOCKED_REVEALED_UNVISITED,
		LOCKED_REVEALED_VISITED,
		UNLOCKED_UNVISITED,
		UNLOCKED_VISITED,
		COMPENDIUMUNLOCKSTATUS_MAX
	};

	class AchievementData_t
	{
	public:
		static int compendiumAchievementPoints;
		enum AchievementDLCType
		{
			ACH_TYPE_NORMAL,
			ACH_TYPE_DLC1,
			ACH_TYPE_DLC2,
			ACH_TYPE_DLC1_DLC2,
			ACH_TYPE_DLC3,
			ACH_TYPE_DLC1_DLC2_DLC3
		};
		DynamicString name;
		DynamicString desc;
		DynamicString desc_formatted;
		bool hidden = false;
		AchievementDLCType dlcType = ACH_TYPE_NORMAL;
		DynamicString category = "";
		int lorePoints = 0;
		int64_t unlockTime = 0;
		bool unlocked = false;
		int achievementProgress = -1; // ->second is the associated achievement stat index

		static bool achievementsNeedResort;
		static bool achievementsNeedFirstData;
		typedef std::function<bool(std::pair<std::string, std::string>, std::pair<std::string, std::string>)> Comparator;
		static std::set<std::pair<std::string, std::string>, Comparator> achievementNamesSorted;
		static DynamicMapStrArrayStringPair achievementCategories;
		static DynamicSetStr achievementUnlockedLookup;
		static void onAchievementUnlock(const char* ach);
		static DynamicMapStrArrayStringPair contents;
		static DynamicMapStr contentsMap;
		static DynamicMapI32 unlocks;
		static int completionPercent;
		static int numUnread;
		static void readContentsLang();

		struct CompendiumAchievementsDisplay
		{
			DynamicArrayT<DynamicArrayStr> pages;
			int currentPage = 0;
			int numHidden = 0;
		};
		static DynamicMapStrT<CompendiumAchievementsDisplay> achievementsBookDisplay;
		static bool sortAlphabetical;
	};
	static DynamicMapAchievementData achievements;
	static DynamicString compendium_sorting;
	static bool compendium_sorting_hide_undiscovered;
	static bool compendium_sorting_hide_ach_unlocked;

	enum EventTags
	{
		CPDM_BLOCKED_ATTACKS,
		CPDM_BROKEN_BY_BLOCKING,
		CPDM_BROKEN,
		CPDM_BLESSED_MAX,
		CPDM_ATTACKS,
		CPDM_THROWN,
		CPDM_SHOTS_FIRED,
		CPDM_AMMO_FIRED,
		CPDM_CONSUMED,
		CPDM_TOWEL_USES,
		CPDM_PICKAXE_WALLS_DUG,
		CPDM_SINKS_TAPPED,
		CPDM_ALEMBIC_BREWED,
		CPDM_TINKERKIT_CRAFTS,
		CPDM_TORCH_WALLS,
		CPDM_SHIELD_REFLECT,
		CPDM_BLESSED_TOTAL,
		CPDM_DMG_MAX,
		CPDM_TRADING_GOLD_EARNED,
		CPDM_FOUNTAINS_TAPPED,
		CPDM_ALEMBIC_DECANTED,
		CPDM_TINKERKIT_REPAIRS,
		CPDM_MIRROR_TELEPORTS,
		CPDM_BLOCKED_HIGHEST_DMG,
		CPDM_TRADING_SOLD,
		CPDM_DMG_0,
		CPDM_BOTTLE_FROM_BREWING,
		CPDM_ALEMBIC_DUPLICATED,
		CPDM_TINKERKIT_METAL_SCRAPPED,
		CPDM_INSPIRATION_XP,
		CPDM_CLOAK_BURNED,
		CPDM_BOTTLES_FROM_CONSUME,
		CPDM_ALEMBIC_BROKEN,
		CPDM_TINKERKIT_MAGIC_SCRAPPED,
		CPDM_TINKERKIT_SALVAGED,
		CPDM_APPRAISED,
		CPDM_TOWEL_BLEEDING,
		CPDM_TOWEL_MESSY,
		CPDM_TOWEL_GREASY,
		CPDM_RUNS_COLLECTED,
		CPDM_ATTACKS_MISSES,
		CPDM_THROWN_HITS,
		CPDM_SHOTS_HIT,
		CPDM_AMMO_HIT,
		CPDM_MAGICSTAFF_RECHARGED,
		CPDM_MAGICSTAFF_CASTS,
		CPDM_FEATHER_ENSCRIBED,
		CPDM_FEATHER_CHARGE_USED,
		CPDM_FEATHER_SPELLBOOKS,
		CPDM_CONSUMED_UNIDENTIFIED,
		CPDM_SPELL_CASTS,
		CPDM_SPELL_FAILURES,
		CPDM_SPELLBOOK_CASTS,
		CPDM_SPELLBOOK_LEARNT,
		CPDM_KILLED_SOLO,
		CPDM_KILLED_PARTY,
		CPDM_KILLED_BY,
		CPDM_GHOST_SPAWNED,
		CPDM_GHOST_TELEPORTS,
		CPDM_GHOST_PINGS,
		CPDM_GHOST_PUSHES,
		CPDM_MINEHEAD_ENTER,
		CPDM_MINEHEAD_RETURN,
		CPDM_GATE_OPENED_SPELL,
		CPDM_GATE_MINOTAUR,
		CPDM_LEVER_PULLED,
		CPDM_LEVER_FOLLOWER_PULLED,
		CPDM_DOOR_BROKEN,
		CPDM_DOOR_OPENED,
		CPDM_DOOR_UNLOCKED,
		CPDM_LEVELS_ENTERED,
		CPDM_LEVELS_EXITED,
		CPDM_LEVELS_MAX_LVL,
		CPDM_LEVELS_MAX_GOLD,
		CPDM_SINKS_USED,
		CPDM_SINKS_RINGS,
		CPDM_SINKS_SLIMES,
		CPDM_FOUNTAIN_FOOCUBI,
		CPDM_FOUNTAIN_DRUNK,
		CPDM_FOUNTAIN_BLESS,
		CPDM_FOUNTAIN_BLESS_ALL,
		CPDM_CHESTS_OPENED,
		CPDM_CHESTS_MIMICS_AWAKENED,
		CPDM_CHESTS_UNLOCKED,
		CPDM_CHESTS_DESTROYED,
		CPDM_BARRIER_DESTROYED,
		CPDM_GRAVE_GHOULS,
		CPDM_GRAVE_EPITAPHS_READ,
		CPDM_GRAVE_EPITAPHS_PERCENT,
		CPDM_GRAVE_GHOULS_ENSLAVED,
		CPDM_SHOP_BOUGHT,
		CPDM_SHOP_SOLD,
		CPDM_SHOP_GOLD_EARNED,
		CPDM_TRAP_KILLED_BY,
		CPDM_TRAP_DAMAGE,
		CPDM_TRAP_FOLLOWERS_KILLED,
		CPDM_BOULDERS_PUSHED,
		CPDM_ARROWS_PILFERED,
		CPDM_SWIM_TIME,
		CPDM_SWIM_KILLED_WHILE,
		CPDM_SWIM_BURN_CURED,
		CPDM_LAVA_DAMAGE,
		CPDM_LAVA_ITEMS_BURNT,
		CPDM_SOKOBAN_SOLVES,
		CPDM_SOKOBAN_FASTEST_SOLVE,
		CPDM_TRAP_MAGIC_STATUSED,
		CPDM_OBELISK_USES,
		CPDM_OBELISK_FOLLOWER_USES,
		CPDM_TRIALS_ATTEMPTS,
		CPDM_TRIALS_PASSED,
		CPDM_TRIALS_DEATHS,
		CPDM_LEVELS_BIOME_CLEAR,
		CPDM_DOOR_CLOSED,
		CPDM_SINKS_HEALTH_RESTORED,
		CPDM_FOUNTAIN_USED,
		CPDM_CHESTS_MIMICS_AWAKENED1ST,
		CPDM_SHOP_SPENT,
		CPDM_SOKOBAN_PERFECT_SOLVES,
		CPDM_PITS_ITEMS_LOST,
		CPDM_PITS_LEVITATED,
		CPDM_PITS_DEATHS,
		CPDM_PITS_ITEMS_VALUE_LOST,
		CPDM_KILLED_MULTIPLAYER,
		CPDM_RECRUITED,
		CPDM_RACE_GAMES_STARTED,
		CPDM_RACE_RECRUITS,
		CPDM_RACE_GAMES_WON,
		CPDM_DISTANCE_TRAVELLED,
		CPDM_DISTANCE_MAX_RUN,
		CPDM_XP_KILLS,
		CPDM_XP_SKILLS,
		CPDM_XP_MAX_IN_FLOOR,
		CPDM_XP_MAX_INSTANCE,
		CPDM_CLASS_LVL_MAX,
		CPDM_CLASS_LVL_GAINED,
		CPDM_CLASS_GAMES_STARTED,
		CPDM_CLASS_GAMES_WON,
		CPDM_CLASS_GAMES_SOLO,
		CPDM_CLASS_GAMES_MULTI,
		CPDM_STAT_MAX,
		CPDM_CLASS_STAT_STR_MAX,
		CPDM_STAT_INCREASES,
		CPDM_STAT_DOUBLED,
		CPDM_HP_MAX,
		CPDM_CLASS_HP_MAX,
		CPDM_MP_MAX,
		CPDM_CLASS_MP_MAX,
		CPDM_CLASS_SKILL_UPS,
		CPDM_CLASS_SKILL_MAX,
		CPDM_CLASS_SKILL_UPS_RUN_MAX,
		CPDM_CLASS_STAT_DEX_MAX,
		CPDM_CLASS_STAT_CON_MAX,
		CPDM_CLASS_STAT_INT_MAX,
		CPDM_CLASS_STAT_PER_MAX,
		CPDM_CLASS_STAT_CHR_MAX,
		CPDM_RES_MAX,
		CPDM_CLASS_RES_MAX,
		CPDM_RES_DMG_RESISTED,
		CPDM_RES_DMG_RESISTED_RUN,
		CPDM_AC_MAX,
		CPDM_CLASS_AC_MAX,
		CPDM_AC_MAX_FROM_BLESS,
		CPDM_HP_LOST_RUN,
		CPDM_MP_SPENT_RUN,
		CPDM_MP_SPENT_TOTAL,
		CPDM_CLASS_LVL_WON_MAX,
		CPDM_CLASS_LVL_WON_MIN,
		CPDM_CLASS_GAMES_WON_CLASSIC,
		CPDM_CLASS_GAMES_WON_HELL,
		CPDM_CLASS_LVL_WON_CLASSIC_MAX,
		CPDM_CLASS_LVL_WON_HELL_MIN,
		CPDM_RACE_GAMES_WON_CLASSIC,
		CPDM_RACE_GAMES_WON_HELL,
		CPDM_CLASS_LVL_WON_CLASSIC_MIN,
		CPDM_CLASS_LVL_WON_HELL_MAX,
		CPDM_DISTANCE_MAX_FLOOR,
		CPDM_PWR_MAX,
		CPDM_CLASS_PWR_MAX,
		CPDM_RGN_HP_SUM,
		CPDM_RGN_HP_RUN,
		CPDM_RGN_MP_SUM,
		CPDM_RGN_MP_RUN,
		CPDM_RGN_HP_RATE_MAX,
		CPDM_RGN_MP_RATE_MAX,
		CPDM_CLASS_WGT_MAX,
		CPDM_CLASS_WGT_MAX_MOVE_100,
		CPDM_MELEE_HITS,
		CPDM_MELEE_DMG_TOTAL,
		CPDM_CLASS_MELEE_HITS_RUN,
		CPDM_MELEE_KILLS,
		CPDM_CRIT_HITS,
		CPDM_CRITS_DMG_TOTAL,
		CPDM_CLASS_CRITS_HITS_RUN,
		CPDM_CRIT_KILLS,
		CPDM_CLASS_WGT_SLOWEST,
		CPDM_CLASS_SKILL_LEGENDS,
		CPDM_SKILL_LEGENDARY_PROCS,
		CPDM_DEGRADED,
		CPDM_REPAIRS,
		CPDM_RANGED_HITS,
		CPDM_RANGED_DMG_TOTAL,
		CPDM_CLASS_RANGED_HITS_RUN,
		CPDM_RANGED_KILLS,
		CPDM_THROWN_TOTAL_HITS,
		CPDM_THROWN_DMG_TOTAL,
		CPDM_CLASS_THROWN_HITS_RUN,
		CPDM_THROWN_KILLS,
		CPDM_FLANK_HITS,
		CPDM_FLANK_DMG,
		CPDM_CLASS_FLANK_HITS_RUN,
		CPDM_CLASS_FLANK_DMG_RUN,
		CPDM_BACKSTAB_HITS,
		CPDM_CLASS_BACKSTAB_KILLS_RUN,
		CPDM_CLASS_BACKSTAB_HITS_RUN,
		CPDM_BACKSTAB_KILLS,
		CPDM_CLASS_BACKSTAB_DMG_RUN,
		CPDM_CLASS_BLOCK_DEFENDED,
		CPDM_CLASS_BLOCK_UNDEFENDED,
		CPDM_CLASS_BLOCK_DEFENDED_RUN,
		CPDM_CLASS_BLOCK_UNDEFENDED_RUN,
		CPDM_CLASS_SPELL_CASTS_RUN,
		CPDM_CLASS_SPELL_FIZZLES_RUN,
		CPDM_CLASS_SNEAK_TIME,
		CPDM_CLASS_SNEAK_SKILLUP_FLOOR,
		CPDM_WANTED_RUNS,
		CPDM_WANTED_TIMES_RUN,
		CPDM_WANTED_INFLUENCE,
		CPDM_WANTED_CRIMES_RUN,
		CPDM_CUSTOM_TAG,
		CPDM_SPELLBOOK_CAST_DEGRADES,
		CPDM_CLASS_SPELLBOOK_CASTS_RUN,
		CPDM_CLASS_SPELLBOOK_FIZZLES_RUN,
		CPDM_CLASS_MOVING_TIME,
		CPDM_CLASS_IDLING_TIME,
		CPDM_CLASS_SKILL_UPS_ALL_RUN,
		CPDM_CLASS_WGT_EQUIPPED_MAX,
		CPDM_CLASS_PWR_MAX_CASTED,
		CPDM_PWR_MAX_EQUIP,
		CPDM_PWR_MAX_SPELLBOOK,
		CPDM_RES_DMG_TAKEN,
		CPDM_RES_SPELLS_HIT,
		CPDM_HP_MOST_DMG_LOST_ONE_HIT,
		CPDM_HP_LOST_TOTAL,
		CPDM_AC_EFFECTIVENESS_MAX,
		CPDM_AC_EQUIPMENT_MAX,
		CPDM_LEVELS_MIN_COMPLETION,
		CPDM_LEVELS_MAX_COMPLETION,
		CPDM_LEVELS_DEATHS,
		CPDM_LEVELS_DEATHS_FASTEST,
		CPDM_LEVELS_DEATHS_SLOWEST,
		CPDM_LEVELS_MIN_LVL,
		CPDM_BIOMES_MIN_COMPLETION,
		CPDM_BIOMES_MAX_COMPLETION,
		CPDM_LEVELS_TIME_SPENT,
		CPDM_MINEHEAD_ENTER_SOLO,
		CPDM_MINEHEAD_ENTER_ONLINE_MP,
		CPDM_MINEHEAD_ENTER_LAN_MP,
		CPDM_MINEHEAD_TOTAL_PLAYTIME,
		CPDM_MINEHEAD_ENTER_SPLIT_MP,
		CPDM_TOTAL_TIME_SPENT,
		CPDM_TRAP_SUMMONED_MONSTERS,
		CPDM_LORE_READ,
		CPDM_LORE_BURNT,
		CPDM_LORE_PERCENT_READ,
		CPDM_LORE_PERCENT_READ_2,
		CPDM_MERCHANT_ORBS,
		CPDM_SPELL_DMG,
		CPDM_SPELL_HEAL,
		CPDM_TIME_WORN,
		CPDM_LOCKPICK_DOOR_UNLOCK,
		CPDM_LOCKPICK_DOOR_LOCK,
		CPDM_LOCKPICK_ARROWTRAPS,
		CPDM_LOCKPICK_TINKERTRAPS,
		CPDM_LOCKPICK_CHESTS_UNLOCK,
		CPDM_LOCKPICK_MIMICS_LOCKED,
		CPDM_LOCKPICK_CHESTS_LOCK,
		CPDM_ALEMBIC_DUPLICATION_FAIL,
		CPDM_ALEMBIC_EXPLOSIONS,
		CPDM_BEARTRAP_DEPLOYED,
		CPDM_BEARTRAP_TRAPPED,
		CPDM_BEARTRAP_DMG,
		CPDM_GADGET_CRAFTED,
		CPDM_GADGET_DEPLOYED,
		CPDM_NOISEMAKER_LURED,
		CPDM_NOISEMAKER_MOST_LURED,
		CPDM_DUMMY_HITS_TAKEN,
		CPDM_DUMMY_DMG_TAKEN,
		CPDM_SENTRY_DEPLOY_KILLS,
		CPDM_SENTRY_DEPLOY_DMG,
		CPDM_GYROBOT_BOULDERS,
		CPDM_GYROBOT_TIME_SPENT,
		CPDM_BOMB_DMG,
		CPDM_BOMB_DETONATED,
		CPDM_BOMB_DETONATED_ALLY,
		CPDM_DETONATOR_SCRAPPED,
		CPDM_DETONATOR_SCRAPPED_METAL,
		CPDM_DETONATOR_SCRAPPED_MAGIC,
		CPDM_DEATHBOX_OPEN_OWN,
		CPDM_DEATHBOX_OPEN_OTHERS,
		CPDM_DEATHBOX_TO_EXIT,
		CPDM_DEATHBOX_MOST_CARRIED,
		CPDM_PICKAXE_BOULDERS_DUG,
		CPDM_TIN_GREASY,
		CPDM_TIN_REGEN_HP,
		CPDM_TIN_REGEN_MP,
		CPDM_SPELL_TARGETS,
		CPDM_GYROBOT_FLIPS,
		CPDM_KILL_XP,
		CPDM_CONTAINER_BROKEN,
		CPDM_CONTAINER_ITEMS,
		CPDM_CONTAINER_GOLD,
		CPDM_CONTAINER_MONSTERS,
		CPDM_DAED_USES,
		CPDM_DAED_EXIT_REVEALS,
		CPDM_DAED_SPEED_BUFFS,
		CPDM_DAED_KILLED_MINO,
		CPDM_BELL_RUNG_TIMES,
		CPDM_BELL_LOOT_ITEMS,
		CPDM_BELL_BROKEN,
		CPDM_BELL_BUFFS_AGILITY,
		CPDM_BELL_BUFFS_STAMINA,
		CPDM_BELL_BUFFS_STRENGTH,
		CPDM_BELL_BUFFS_MENTALITY,
		CPDM_BELL_BUFFS_HEALS,
		CPDM_BELL_LOOT_GOLD,
		CPDM_BELL_LOOT_BATS,
		CPDM_BELL_CLAPPER_BROKEN,
		CPDM_CLASS_SKILL_NOVICES,
		CPDM_FOLLOWER_KILLS,
		CPDM_COMBAT_MASONRY_BOULDERS,
		CPDM_MERLINS,
		CPDM_RITUALS_COMPLETED,
		CPDM_HUMANS_SAVED,
		CPDM_SPELLS_LEARNED,
		CPDM_ALLIES_FED,
		CPDM_COMBAT_MASONRY_GEMS,
		CPDM_COMBAT_MASONRY_ROCKS,
		CPDM_GOLD_LEFT_BEHIND,
		MONSTERS_LEFT_BEHIND,
		ITEMS_LEFT_BEHIND,
		ITEM_VALUE_LEFT_BEHIND,
		CPDM_OFFHAND_CASTING_MP,
		CPDM_OFFHAND_CHARGING_TIME,
		CPDM_OFFHAND_CHARGING_TIME_RUN,
		CPDM_OFFHAND_CHR_MAX,
		CPDM_GOLD_COLLECTED,
		CPDM_GOLD_COLLECTED_RUN,
		CPDM_GOLD_CASTED,
		CPDM_GOLD_CASTED_RUN,
		CPDM_BUTTON_PRESSED,
		CPDM_BUTTON_FOLLOWER_PRESSED,
		CPDM_BUTTON_SHOT,
		CPDM_KEYLOCK_UNLOCKED_KEY,
		CPDM_KEYLOCK_PICKED,
		CPDM_KEYLOCK_SKELETON_KEY,
		CPDM_KEYLOCK_UNLOCKED_KEY_IRON,
		CPDM_KEYLOCK_UNLOCKED_KEY_BRONZE,
		CPDM_KEYLOCK_UNLOCKED_KEY_SILVER,
		CPDM_KEYLOCK_UNLOCKED_KEY_GOLD,
		CPDM_ASSIST_CLOAKS,
		CPDM_ASSIST_RINGS,
		CPDM_ASSIST_AMULETS,
		CPDM_ASSIST_MASKS,
		CPDM_ASSIST_INTERACTS,
		CPDM_CAULDRON_INTERACTS,
		CPDM_WORKBENCH_INTERACTS,
		CPDM_WORKBENCH_SALVAGE,
		CPDM_WORKBENCH_CRAFTS,
		CPDM_WORKBENCH_REPAIRED,
		CPDM_WORKBENCH_SKILLUPS,
		CPDM_COOK_MEALS,
		CPDM_COOK_FLAVORED_MEALS,
		CPDM_CAULDRON_SKILLUPS,
		CPDM_COOK_SLOP_BALLS,
		CPDM_COOK_GREASE_BALLS,
		CPDM_PARRIES,
		CPDM_PARRIES_DMG,
		CPDM_ARCHON_SPELLS_FORGOTTEN,
		CPDM_MULTI_HITS,
		CPDM_MAGIC_KILLS,
		CPDM_EFFECT_DURATION,
		CPDM_JEWEL_RECRUIT_DECREPIT,
		CPDM_JEWEL_RECRUIT_WORN,
		CPDM_JEWEL_RECRUIT_SERVICABLE,
		CPDM_JEWEL_RECRUIT_EXCELLENT,
		CPDM_FORGED,
		CPDM_UPGRADED,
		CPDM_SHILLELAGH_DEBUFFS_MAX,
		CPDM_DUCK_DODGE,
		CPDM_DUCK_CAUGHT,
		CPDM_EVENT_TAGS_MAX
	};

	struct CompendiumMonsters_t
	{
		enum MonsterSpecies
		{
			SPECIES_NONE,
			SPECIES_HUMANOID,
			SPECIES_BEAST,
			SPECIES_BEASTFOLK,
			SPECIES_UNDEAD,
			SPECIES_DEMONOID,
			SPECIES_CONSTRUCT,
			SPECIES_ELEMENTAL
		};
		struct Monster_t
		{
			int monsterType = NOTHING;
			DynamicString unique_npc = "";
			DynamicArrayStr blurb;
			DynamicArrayS32 hp;
			DynamicArrayS32 spd;
			DynamicArrayS32 ac;
			DynamicArrayS32 atk;
			DynamicArrayS32 rangeatk;
			DynamicArrayS32 pwr;
			DynamicArrayS32 str;
			DynamicArrayS32 con;
			DynamicArrayS32 dex;
			MonsterSpecies species;
			DynamicArrayS32 lvl;
			int resistances[7];
			DynamicArrayStr abilities;
			DynamicArrayStr inventory;
			DynamicString imagePath = "";
			DynamicArrayStr models;
			DynamicSetStr unlockAchievements;
			int lorePoints = 0;
			DynamicArrayS32 getDisplayStat(const char* name);
		};
		static DynamicMapStrArrayStringPair contents;
		static DynamicMapStr contentsMap;
		static DynamicMapStrArrayStringPair contents_unfiltered;
		static void readContentsLang();
		static DynamicMapI32 unlocks;
		static int completionPercent;
		static int numUnread;
	};
	DynamicMapStrT<CompendiumMonsters_t::Monster_t> monsters;
	void readMonstersFromFile(bool forceLoadBaseDirectory = false);
	void readMonstersTranslationsFromFile(bool forceLoadBaseDirectory = false);
	void exportCurrentMonster(Entity* monster);
	void readModelLimbsFromFile(std::string section);
	CompendiumView_t defaultCamera;
	struct ObjectLimbs_t
	{
		CompendiumView_t baseCamera;
		CompendiumView_t currentCamera;
		DynamicArray entities;  // vector<Entity> (byte moves)
	};
	DynamicMapStrT<ObjectLimbs_t> compendiumObjectLimbs;
	CompendiumView_t currentView;
	struct CompendiumMap_t
	{
		Uint32 width = 0;
		Uint32 height = 0;
		Uint32 ceiling = (Uint32)-1;
	};
	struct CompendiumMapTiles_t
	{
		CompendiumMap_t first;
		DynamicArrayS32 second;
	};
	DynamicMapStrT<CompendiumMapTiles_t> compendiumObjectMapTiles;
	map_t compendiumMap;
	struct CompendiumWorld_t
	{
		struct World_t
		{
			int modelIndex = -1;
			DynamicString imagePath = "";
			DynamicArrayStr models;
			DynamicArrayStr blurb;
			DynamicArrayU32 linesToHighlight;
			DynamicArrayStr details;
			DynamicSetStr unlockAchievements;
			DynamicSetI32 unlockTags;
			DynamicString featureImg = "";
			int id = -1;
			int lorePoints = 0;
		};
		static DynamicMapStrArrayStringPair contents;
		static DynamicMapStr contentsMap;
		static void readContentsLang();
		static DynamicMapI32 unlocks;
		static int completionPercent;
		static int numUnread;
	};
	DynamicMapStrT<CompendiumWorld_t::World_t> worldObjects;
	void readWorldFromFile(bool forceLoadBaseDirectory = false);
	void readWorldTranslationsFromFile(bool forceLoadBaseDirectory = false);

	struct CompendiumCodex_t
	{
		struct Codex_t
		{
			int modelIndex = -1;
			DynamicString imagePath = "";
			DynamicArrayStr renderedImagePaths;
			DynamicArrayStr blurb;
			DynamicArrayU32 linesToHighlight;
			DynamicArrayStr details;
			DynamicArrayStr models;
			DynamicString featureImg = "";
			int id = -1;
			CompendiumView_t view;
			int lorePoints = 0;
			bool enableTutorial = false;
		};
		static DynamicMapStrArrayStringPair contents;
		static DynamicMapStr contentsMap;
		static void readContentsLang();
		static DynamicMapI32 unlocks;
		static int completionPercent;
		static int numUnread;
	};
	DynamicMapStrT<CompendiumCodex_t::Codex_t> codex;
	bool migrateOldSkillIndexes = false;
	void readCodexFromFile(bool forceLoadBaseDirectory = false);
	void readCodexTranslationsFromFile(bool forceLoadBaseDirectory = false);
	static const char* compendiumCurrentLevelToWorldString(const int currentlevel, const bool secretlevel);

	struct CompendiumItems_t
	{
		struct Codex_t
		{
			struct CodexItem_t
			{
				DynamicString name = "";
				int rotation = 0;
				int spellID = -1;
				int effectID = -1;
				int itemID = -1;
			};
			int modelIndex = -1;
			DynamicString imagePath = "";
			DynamicArrayStr blurb;
			DynamicArrayT<CodexItem_t> items_in_category;
			int lorePoints = 0;
		};
		static DynamicMapStrArrayStringPair contents;
		static DynamicMapStr contentsMap;
		static void readContentsLang();
		static DynamicMapI32 unlocks;
		static DynamicMapI32T<CompendiumUnlockStatus> itemUnlocks;
		static int completionPercent;
		static int numUnread;
	};
	DynamicMapStrT<CompendiumItems_t::Codex_t> items;
	void readItemsFromFile(bool forceLoadBaseDirectory = false);
	void readItemsTranslationsFromFile(bool forceLoadBaseDirectory = false);

	struct CompendiumMagic_t
	{
		static DynamicMapStrArrayStringPair contents;
		static DynamicMapStr contentsMap;
		static void readContentsLang();
		static int completionPercent;
		static int numUnread;
	};
	DynamicMapStrT<CompendiumItems_t::Codex_t> magic;
	void readMagicFromFile(bool forceLoadBaseDirectory = false);
	void readMagicTranslationsFromFile(bool forceLoadBaseDirectory = false);
	static Item compendiumItem;
	static bool tooltipNeedUpdate;
	static void updateTooltip();
	static SDL_Rect tooltipPos;
	static Entity compendiumItemModel;
	static Uint32 lastTickUpdate;
	static int lorePointsFromAchievements;
	static int lorePointsAchievementsTotal;
	static int lorePointsSpent;
	static bool lorePointsFirstLoad;
	static void updateLorePointCounts();
	static void writeUnlocksSaveData();
	static void readUnlocksSaveData();

	static const char* getSkillStringForCompendium(const int skill);

	struct CompendiumEntityCurrent
	{
		DynamicString contentsName = "";
		DynamicString modelName = "";
		int modelIndex = -1;
		Uint32 modelRNG = 0;
		void set(std::string _contentsName, std::string _modelName, int _modelIndex = -1);
	};
	static CompendiumEntityCurrent compendiumEntityCurrent;

	struct Events_t
	{
		enum Type
		{
			SUM,
			MAX,
			AVERAGE_RANGE,
			BITFIELD,
			MIN
		};
		enum ClientUpdateType
		{
			SERVER_ONLY,
			CLIENT_ONLY,
			CLIENT_AND_SERVER,
			CLIENT_UPDATETYPE_MAX
		};
		enum EventTrackingType
		{
			ALWAYS_UPDATE,
			ONCE_PER_RUN,
			UNIQUE_PER_RUN,
			UNIQUE_PER_FLOOR
		};
		struct Event_t
		{
			Type type = SUM;
			EventTrackingType eventTrackingType = ALWAYS_UPDATE;
			ClientUpdateType clienttype = CLIENT_ONLY;
			DynamicString name = "";
			int id = CPDM_EVENT_TAGS_MAX;
			DynamicSetStr attributes;
		};
		struct EventVal_t
		{
			Type type = SUM;
			int id = CPDM_EVENT_TAGS_MAX;
			Sint32 value = 0;
			bool firstValue = true;
			bool applyValue(const Sint32 val);
			EventVal_t() = default;
			EventVal_t(EventTags tag)
			{
				auto& def = events[tag];
				type = def.type;
				id = def.id;
				value = 0;
				firstValue = true;
			}
		};
		static DynamicMapI32T<Event_t> events;
		static DynamicMapI32 eventIdLookup;
		static DynamicMapI32T<DynamicSetI32> itemEventLookup;
		static DynamicMapI32 monsterUniqueIDLookup;
		static DynamicMapI32Str itemIDToString;
		static DynamicMapI32Str monsterIDToString;
		static DynamicMapI32Str codexIDToString;
		static DynamicMapI32Str worldIDToString;
		static DynamicMapI32T<DynamicArrayS32> itemDisplayedEventsList;
		static DynamicMapI32T<DynamicArrayStr> itemDisplayedCustomEventsList;
		static DynamicMapStr customEventsValues;
		static DynamicMapI32T<DynamicSetI32> eventItemLookup;
		static DynamicMapI32T<DynamicSetI32> eventMonsterLookup;
		static DynamicMapI32T<DynamicSetStr> eventWorldLookup;
		static DynamicMapI32T<DynamicSetStr> eventCodexLookup;
		static DynamicMapI32 eventWorldIDLookup;
		static DynamicMapI32 eventCodexIDLookup;
		static DynamicMapI32Map eventClassIds;
		static const int kEventClassesMax = 40;
		static DynamicMapI32T<DynamicMapStr> eventLangEntries;
		static DynamicMapStrT<DynamicMapStr> eventCustomLangEntries;
		static std::vector<std::pair<std::string, Sint32>> getCustomEventValue(std::string key, std::string compendiumSection, std::string compendiumContentsSelected, int specificClass = -1);
		static std::string formatEventRecordText(Sint32 value, const char* formatType, int formatVal, DynamicMapStr& langMap);
		static void readEventsFromFile();
		static void writeItemsSaveData();
		static void loadItemsSaveData();
		static void readEventsTranslations();
		static void createDummyClientData(const int playernum);
		static void eventUpdate(int playernum, const EventTags tag, const ItemType type, Sint32 value, const bool loadingValue = false, const int spellID = -1);
		static void eventUpdateMonster(int playernum, const EventTags tag, const Entity* entity, Sint32 value, const bool loadingValue = false, const int entryID = -1);
		static void eventUpdateWorld(int playernum, const EventTags tag, const char* category, Sint32 value, const bool loadingValue = false, const int entryID = -1, const bool commitUniqueValue = true);
		static void eventUpdateCodex(int playernum, const EventTags tag, const char* category, Sint32 value, const bool loadingValue = false, const int entryID = -1, const bool floorEvent = false);
		static DynamicMapI32T<DynamicMapI32T<EventVal_t>> playerEvents;
		static DynamicMapI32T<DynamicMapI32T<EventVal_t>> serverPlayerEvents[MAXPLAYERS];
		static void onLevelChangeEvent(const int playernum, const int prevlevel, const bool prevsecretfloor, const std::string prevmapname, const bool died);
		static void onEndgameEvent(const int playernum, const bool tutorialend, const bool saveHighscore, const bool died);
		static void sendClientDataOverNet(const int playernum);
		static void updateEventsInMainLoop(const int playernum);
		static DynamicMapI32Str clientDataStrings[MAXPLAYERS];
		static DynamicMapI32Str clientReceiveData[256];
		static Uint8 clientSequence;
		static const int kEventSpellOffset = 10000;
		static const int kEventMonsterOffset = 1000;
		static const int kEventWorldOffset = 2000;
		static const int kEventCodexOffset = 3000;
		static const int kEventCodexClassOffset = 3500;
		static const int kEventCodexOffsetMax = 9999;
		static int previousCurrentLevel;
		static bool previousSecretlevel;
	};
};

template <> struct DynamicArrayKindOf<DynamicArrayT<DynamicArrayStr>> { static constexpr int value = Kind_DynArrayStrArray; };
template <> struct DynamicArrayKindOf<Compendium_t::CompendiumItems_t::Codex_t::CodexItem_t> { static constexpr int value = Kind_CodexItem; };
// MapValueKindOf for Compendium_t::AchievementData_t::CompendiumAchievementsDisplay (owns 1 nested array + 2 ints)
template <> struct MapValueKindOf<Compendium_t::AchievementData_t::CompendiumAchievementsDisplay> { static constexpr int value = MK_CompendiumAchievementsDisplay; };
template <> struct MapValueKindOf<Compendium_t::CompendiumMapTiles_t> { static constexpr int value = MK_CompendiumMapTiles; };
template <> struct MapValueKindOf<Compendium_t::ObjectLimbs_t> { static constexpr int value = MK_ObjectLimbs; };
template <> struct MapValueKindOf<Compendium_t::CompendiumWorld_t::World_t> { static constexpr int value = MK_World; };
template <> struct MapValueKindOf<Compendium_t::CompendiumCodex_t::Codex_t> { static constexpr int value = MK_Codex; };
template <> struct MapValueKindOf<Compendium_t::CompendiumItems_t::Codex_t> { static constexpr int value = MK_ItemsCodex; };
template <> struct MapValueKindOf<Compendium_t::CompendiumMonsters_t::Monster_t> { static constexpr int value = MK_Monster; };
template <> struct MapValueKindOf<Compendium_t::Events_t::Event_t> { static constexpr int value = MK_Event; };
template <> struct MapValueKindOf<Compendium_t::Events_t::EventVal_t> { static constexpr int value = MK_EventVal; };
template <> struct MapValueKindOf<DynamicMapI32T<Compendium_t::Events_t::EventVal_t>> { static constexpr int value = MK_I32MapEventVal; };


extern Compendium_t CompendiumEntries;

struct TreasureRoomGenerator
{
	BaronyRNG treasure_rng;
	DynamicSetI32 treasure_floors;
	DynamicSetI32 treasure_secret_floors;
	DynamicMapI32Str orb_floors;
	DynamicMapI32Str station_floors;
	DynamicMapI32Str station_secret_floors;
	void init();
	bool bForceSpawnForCurrentFloor(int secretlevelexit, bool minotaur, BaronyRNG& mapRNG);
	bool bForceStationSpawnForCurrentFloor(int secretlevelexit);
};
extern TreasureRoomGenerator treasure_room_generator;