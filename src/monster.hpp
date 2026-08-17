/*-------------------------------------------------------------------------------

	BARONY
	File: monster.hpp
	Desc: header file for monsters

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#pragma once

#include "../odin/containers/dynamic_map.hpp"
#include "stat.hpp"
#include "json.hpp"

#ifndef EDITOR
#include "interface/consolecommand.hpp"
extern CvarBool cvar_summonBosses;
#endif

enum Monster : int
{
	NOTHING,
	HUMAN,
	RAT,
	GOBLIN,
	SLIME,
	TROLL,
	BAT_SMALL,
	SPIDER,
	GHOUL,
	SKELETON,
	SCORPION,
	CREATURE_IMP, //Because Apple so unkindly is already using the IMP keyword.
	CRAB,
	GNOME,
	DEMON,
	SUCCUBUS,
	MIMIC,
	LICH,
	MINOTAUR,
	DEVIL,
	SHOPKEEPER,
	KOBOLD,
	SCARAB,
	CRYSTALGOLEM,
	INCUBUS,
	VAMPIRE,
	SHADOW,
	COCKATRICE,
	INSECTOID,
	GOATMAN,
	AUTOMATON,
	LICH_ICE,
	LICH_FIRE,
	SENTRYBOT,
	SPELLBOT,
	GYROBOT,
	DUMMYBOT,
	BUGBEAR,
	DRYAD,
	MYCONID,
	SALAMANDER,
	GREMLIN,
	REVENANT_SKULL,
	MINIMIMIC,
	MONSTER_ADORCISED_WEAPON,
	FLAME_ELEMENTAL,
	HOLOGRAM,
	MOTH_SMALL,
	EARTH_ELEMENTAL,
	DUCK_SMALL,
	MONSTER_UNUSED_6,
	MONSTER_UNUSED_7,
	MONSTER_UNUSED_8,
	MAX_MONSTER
};
const int NUMMONSTERS = MAX_MONSTER;
extern int kills[NUMMONSTERS];

static const DynamicArrayS32 monsterSprites[NUMMONSTERS] = {
    // NOTHING
    {
        0,
    },

    // HUMAN
    {
        113, 114, 115, 116, 117,
        125, 126, 127, 128, 129,
        332, 333,
        341, 342, 343, 344, 345, 346,
        354, 355, 356, 357, 358, 359,
        367, 368, 369, 370, 371, 372,
        380, 381, 382, 383, 384, 385,
    },

    // RAT
    {
        131, 265,                       // normal rat walk cycle
        814,                            // player rat head
        1043,                           // player rat head attack 1
        1044,                           // player rat head attack 2
        1063, 1064, 1065, 1066, 1067,   // normal rat attack frames
        1068, 1069,                     // algernon walk cycle
        1070, 1071, 1072, 1073, 1074,   // algernon attack frames
    },

    // GOBLIN
    {
        180, 694, 752, 1035, 1039,
    },

    // SLIME
    {
        189, 1108, 1109, 1110, 1111, 1112, // blue
        210, 1113, 1114, 1115, 1116, 1117, // green
		1380, 1383, 1384, 1385, 1386, 1387, // red
		1381, 1388, 1389, 1390, 1391, 1392, // tar
		1382, 1393, 1394, 1395, 1396, 1397, // metal
    },

    // TROLL
    {
        204,    // normal troll
        817,    // player troll
        1132,   // thumpus
    },

    // BAT_SMALL
    {
		1408
    },

    // SPIDER
    {
        267,    // normal spider body
        997,    // normal crab body
        823,    // player spider body
        1001,   // player crab body
        1118,   // shelob body
		1189,	// bubbles body
    },

    // GHOUL
    {
        246,
        1017, // coral grimes
    },

    // SKELETON
    {
        229,        // normal
        686,        // player
        1049,       // player (mouldy, female)
        1103,       // mouldy (female)
        1107,       // funny bones
    },

    // SCORPION
    {
        196, 266,           // normal body and tail
        1080, 1081, 1082    // skrabblag body1, body2, tail
    },

    // CREATURE_IMP
    {
        289, 827
    },

    // CRAB
    {
    },

    // GNOME
    {
        295, 1426, 1430, 2213, 2214
    },

    // DEMON
    {
        258,    // normal demon
        1008,   // deu-debreau
    },

    // SUCCUBUS
    {
        190,        // normal succubus
        710,        // player succubus
        1126,       // lilith
    },

    // MIMIC
    {
		1247,
		1792
    },

    // LICH
    {
        274,
    },

    // MINOTAUR
    {
        239,
    },

    // DEVIL
    {
        304,
    },

    // SHOPKEEPER
    {
        217,
    },

    // KOBOLD
    {
        421,
    },

    // SCARAB
    {
        429, 430,   // scarab bodies
        1075,       // scarab attack
        1078, 1079  // xyggi 1, xyggi 2
    },

    // CRYSTALGOLEM
    {
        475,
    },

    // INCUBUS
    {
        445, 702, 1826
    },

    // VAMPIRE
    {
        437,    // normal
        718,    // player male
        756,    // player female
        1136,   // bram
        1137,   // female
    },

    // SHADOW
    {
        481,    // normal shadow head
        1087,   // artemisia head
        1095,   // baratheon head
    },

    // COCKATRICE
    {
        413,
    },

    // INSECTOID
    {
        455, 726, 760, 1057,
    },

    // GOATMAN
    {
        463, 734, 768, 1025, 1029,
    },

    // AUTOMATON
    {
        467, 742, 770, 1007,
    },

    // LICH_ICE
    {
        650,
    },

    // LICH_FIRE
    {
        646,
    },

    // SENTRYBOT
    {
        872,
    },

    // SPELLBOT
    {
        885,
    },

    // GYROBOT
    {
        886,
    },

    // DUMMYBOT
    {
        889,
    },

	// BUGBEAR
	{
		1412,
	},

	//DRYAD
	{
		1485, 1486, 1514, 1515,
		1963, 1964, 1992, 1993
	},
	//MYCONID
	{
		1519, 1520,
		1997, 1998
	},
	//SALAMANDER
	{
		1536, 1538, 1540, 1537, 1539, 1541,
		2014, 2015, 2016, 2017, 2018, 2019
	},
	//GREMLIN
	{
		1569, 1570, 2047, 2048
	},
	//REVENANT_SKULL
	{
		1796
	},
	//MINIMIMIC
	{
		1794
	},
	//MONSTER_ADORCISED_WEAPON
	{
		1797
	},
	//FLAME_ELEMENTAL
	{
		1804
	},
	//HOLOGRAM
	{
		1803
	},
	//MOTH__SMALL
	{
		1819, 1822
	},
	//EARTH_ELEMENTAL
	{
		1871, 1876
	},
	// DUCK_SMALL
	{
		2225, 2226, 2231, 2232, 2237, 2238, 2307, 2308
	},
	//MONSTER_UNUSED_6
	{
	},
	//MONSTER_UNUSED_7
	{
	},
	//MONSTER_UNUSED_8
};

static char monstertypename[][32] =
{
	"nothing",
	"human",
	"rat",
	"goblin",
	"slime",
	"troll",
	"bat",
	"spider",
	"ghoul",
	"skeleton",
	"scorpion",
	"imp",
	"crab",
	"gnome",
	"demon",
	"succubus",
	"mimic",
	"lich",
	"minotaur",
	"devil",
	"shopkeeper",
	"kobold",
	"scarab",
	"crystalgolem",
	"incubus",
	"vampire",
	"shadow",
	"cockatrice",
	"insectoid",
	"goatman",
	"automaton",
	"lichice",
	"lichfire",
	"sentrybot",
	"spellbot",
	"gyrobot",
	"dummybot",
	"bugbear",
	"dryad",
	"myconid",
	"salamander",
	"gremlin",
	"revenant_skull",
	"minimimic",
	"monster_adorcised_weapon",
	"flame_elemental",
	"hologram",
	"moth",
	"earth_elemental",
	"duck_small",
	"monster_unused_6",
	"monster_unused_7",
	"monster_unused_8"
};

// body part focal points
extern float limbs[NUMMONSTERS][30][3];

// 0: nothing
// 1: red blood
// 2: green blood
// 3: slime
static char gibtype[NUMMONSTERS] =
{
	0,	//NOTHING,
	1,	//HUMAN,
	1,	//RAT,
	1,	//GOBLIN,
	3,	//SLIME,
	1,	//TROLL,
	1,	//BAT_SMALL,
	2,	//SPIDER,
	2,	//GHOUL,
	5,	//SKELETON,
	2,	//SCORPION,
	1,	//CREATURE_IMP
	2,	//CRAB,
	1,	//GNOME,
	1,	//DEMON,
	1,	//SUCCUBUS,
	1,	//MIMIC,
	2,	//LICH,
	1,	//MINOTAUR,
	1,	//DEVIL,
	1,	//SHOPKEEPER,
	1,	//KOBOLD,
	2,	//SCARAB,
	0,	//CRYSTALGOLEM,
	1,	//INCUBUS,
	1,	//VAMPIRE,
	4,	//SHADOW,
	1,	//COCKATRICE
	2,	//INSECTOID,
	1,	//GOATMAN,
	0,	//AUTOMATON,
	2,	//LICH_ICE,
	2,	//LICH_FIRE
	0,	//SENTRYBOT
	0,	//SPELLBOT
	0,  //GYROBOT
	0,	//DUMMYBOT
	1,	//BUGBEAR
	1,	//DRYAD
	1,	//MYCONID
	1,	//SALAMANDER
	1,	//GREMLIN
	5,  //REVENANT_SKULL
	1,  //MINIMIMIC
	0,  //MONSTER_ADORCISED_WEAPON
	0,  //FLAME_ELEMENTAL
	0,  //HOLOGRAM
	2,  //MOTH_SMALL
	0,  //EARTH_ELEMENTAL
	0,  //DUCK_SMALL
	1,  //MONSTER_UNUSED_6
	1,  //MONSTER_UNUSED_7
	1   //MONSTER_UNUSED_8
};

// columns go like this:
// sword, mace, axe, polearm, ranged, magic, unarmed
// lower number means less effective, higher number means more effective
static double damagetables[NUMMONSTERS][7] =
{
	{ 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f }, // nothing
	{ 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f }, // human
	{ 1.1, 1.1, 0.9, 0.9, 1.2, 1.f, 1.3 }, // rat
	{ 0.9, 1.f, 1.1, 1.1, 1.1, 1.f, 0.8 }, // goblin
	{ 1.4, 0.5, 1.3, 0.7, 0.5, 1.3, 0.5 }, // slime
	{ 1.1, 0.8, 1.1, 0.8, 0.9, 1.f, 0.8 }, // troll
	{ 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 0.5 }, // bat
	{ 1.f, 1.1, 1.f, 1.2, 1.1, 1.f, 1.1 }, // spider
	{ 1.f, 1.2, 0.8, 1.1, 0.6, 0.8, 1.1 }, // ghoul
	{ 0.5, 1.4, 0.8, 1.3, 0.5, 0.8, 1.1 }, // skeleton
	{ 0.9, 1.1, 1.f, 1.3, 1.f, 1.f, 1.2 }, // scorpion
	{ 1.1, 1.f, 0.8, 1.f, 1.f, 1.2, 1.f }, // imp
	{ 1.f, 1.1, 1.f, 1.2, 1.1, 1.f, 1.1 }, // crab
	{ 0.9, 1.f, 1.f, 0.9, 1.1, 1.1, 1.f }, // gnome
	{ 0.9, 0.8, 1.f, 0.8, 0.9, 1.1, 0.8 }, // demon
	{ 1.2, 1.f, 1.f, 0.9, 1.f, 0.8, 1.f }, // succubus
	{ 0.5, 0.5, 1.0, 0.5, 0.5, 1.3, 0.5 }, // mimic
	{ 2.5, 2.5, 2.5, 2.5, 1.3, 1.f, 1.8 }, // lich
	{ 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f }, // minotaur
	{ 2.f, 2.f, 2.f, 2.f, 1.f, 1.f, 1.f }, // devil
	{ 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5 }, // shopkeeper
	{ 0.9, 1.2, 1.2, 0.9, 1.1, 0.2, 0.8 }, // kobold
	{ 1.5, 1.1, 1.4, 0.7, 1.1, 0.2, 1.4 }, // scarab
	{ 1.f, 1.5, 1.3, 0.8, 0.6, 0.6, 0.6 }, // crystal golem
	{ 1.2, 1.f, 1.f, 0.7, 1.3, 0.8, 1.f }, // incubus
	{ 0.8, 1.2, 0.8, 1.1, 0.5, 0.8, 1.f }, // vampire
	{ 0.5, 0.5, 0.5, 0.5, 0.5, 2.0, 0.5 }, // shadow
	{ 1.6, 1.1, 1.3, 1.8, 0.5, 0.5, 0.8 }, // cockatrice
	{ 1.f, 0.7, 1.3, 1.3, 1.1, 1.f, 0.8 }, // insectoid
	{ 0.9, 1.f, 1.1, 1.1, 1.1, 1.4, 1.f }, // goatman
	{ 1.f, 1.4, 1.3, 1.f, 0.8, 1.2, 0.8 }, // automaton
	{ 1.5, 1.5, 1.5, 1.5, 1.5, 0.7, 1.2 }, // lich ice
	{ 1.8, 1.8, 1.8, 1.8, 1.5, 1.f, 1.4 }, // lich fire
	{ 1.f, 1.f, 1.f, 1.f, 0.5, 0.5, 1.f }, // sentrybot
	{ 1.f, 1.f, 1.f, 1.f, 0.5, 0.5, 1.f }, // sentrybot
	{ 1.f, 1.f, 1.f, 1.f, 0.5, 0.5, 1.f }, // gyrobot
	{ 1.f, 1.f, 1.f, 1.f, 0.5, 1.2, 0.5 }, // dummybot
	{ 1.3, 1.2, 1.2, 0.7, 0.8, 1.f, 0.7 }, // bugbear
	{ 1.3, 1.f, 1.3, 0.9, 0.7, 1.1, 0.8 }, // monster_d
	{ 1.3, 0.7, 1.1, 1.f, 1.f, 0.7, 1.f }, // monster_m
	{ 0.8, 1.f, 1.f, 1.2, 1.1, 1.f, 0.9 }, // monster_s
	{ 1.3, 0.9, 1.1, 0.8, 0.9, 0.8, 0.9 }, // monster_g
	{ 0.7, 1.5, 0.8, 0.8, 0.8, 1.3, 0.7 }, // revenant_skull
	{ 0.5, 0.5, 1.0, 0.5, 0.5, 1.3, 0.5 }, // minimimic
	{ 0.5, 1.f, 0.7, 0.5, 0.5, 1.5, 0.5 }, // monster_adorcised_weapon
	{ 0.5, 0.5, 0.5, 0.5, 0.5, 1.5, 0.5 }, // flame_elemental
	{ 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f }, // hologram
	{ 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f }, // moth_small
	{ 0.7, 1.5, 1.f, 1.f, 0.7, 1.5, 0.7 }, // earth_elemental
	{ 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f }, // duck_small
	{ 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f }, // monster_unused_6
	{ 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f }, // monster_unused_7
	{ 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f }  // monster_unused_8
};

enum DamageTableType : int
{
	DAMAGE_TABLE_SWORD,
	DAMAGE_TABLE_MACE,
	DAMAGE_TABLE_AXE,
	DAMAGE_TABLE_POLEARM,
	DAMAGE_TABLE_RANGED,
	DAMAGE_TABLE_MAGIC,
	DAMAGE_TABLE_UNARMED
};
static const int numDamageTableTypes = 7;

static const unsigned int classStatGrowth[NUMCLASSES][6] =
{
	// stat weightings for classes on level up
	//	STR	DEX	CON	INT	PER	CHR -- sum is approx 24.
	{	6,	5,	5,	2,	4,	2 }, // BARB 0
	{	7,	2,	6,	1,	2,	6 }, // WARRIOR 1
	{	3,	3,	4,	6,	5,	3 }, // HEALER 2
	{	2,	7,	1,	2,	7,	5 }, // ROGUE 3
	{	5,	4,	5,	3,	5,	2 }, // WANDERER 4
	{	4,	2,	5,	5,	4,	4 }, // CLERIC 5
	{	3,	2,	4,	3,	5,	7 }, // MERCHANT 6
	{	1,	3,	2,	7,	6,	5 }, // WIZARD 7
	{	2,	6,	2,	6,	6,	2 }, // ARCANIST 8
	{	4,	4,	4,	4,	4,	4 }, // JOKER 9
	{	3,	4,	2,	4,	2,	3 }, // SEXTON 10
	{	4,	6,	3,	2,	4,	1 }, // NINJA 11
	{	4,	2,	5,	3,	2,	2 }, // MONK 12
	{	3,	2,	4,	6,	4,	4 }, // CONJURER 13
	{	3,	3,	1,	6,	6,	3 }, // ACCURSED 14
	{	3,	3,	1,	6,	4,	7 }, // MESMER 15
	{	4,	5,	5,	2,	3,	5 }, // BREWER 16
	{	2,	5,	2,	4,	7,	4 }, // MACHINIST 17
	{	4,	3,	2,	3,	4,	4 }, // PUNISHER 18
	{	4,	4,	4,	4,	4,	4 }, // SHAMAN 19
	{	1,	7,	1,	4,	7,	4 }, // HUNTER 20
	{	2,	5,	3,	4,	3,	7 },
	{	5,	5,	5,	4,	3,	2 },
	{	2,	3,	2,	8,	4,	5 },
	{	4,	2,	3,	6,	3,	5 },
	{	6,	3,	5,	3,	2,	5 }
};

enum AllyNPCCommand : int
{
	ALLY_CMD_DEFEND,
	ALLY_CMD_CLASS_TOGGLE,
	ALLY_CMD_MOVETO_SELECT,
	ALLY_CMD_PICKUP_TOGGLE,
	ALLY_CMD_MOVEASIDE,
	ALLY_CMD_DROP_EQUIP,
	ALLY_CMD_ATTACK_SELECT,
	ALLY_CMD_SPECIAL,
	ALLY_CMD_FOLLOW,
	ALLY_CMD_MOVETO_CONFIRM,
	ALLY_CMD_CANCEL,
	ALLY_CMD_ATTACK_CONFIRM,
	ALLY_CMD_RETURN_SOUL,
	ALLY_CMD_GYRO_DEPLOY,
	ALLY_CMD_GYRO_PATROL,
	ALLY_CMD_GYRO_LIGHT_TOGGLE,
	ALLY_CMD_GYRO_RETURN,
	ALLY_CMD_GYRO_DETECT_TOGGLE,
	ALLY_CMD_DUMMYBOT_RETURN,
	ALLY_CMD_END
};

static const int AllyNPCSkillRequirements[19] =
{
	SKILL_LEVEL_NOVICE,	// ALLY_CMD_DEFEND,
	SKILL_LEVEL_SKILLED,// ALLY_CMD_CLASS_TOGGLE,
	SKILL_LEVEL_BASIC,// ALLY_CMD_MOVETO_SELECT,
	SKILL_LEVEL_BASIC,	// ALLY_CMD_PICKUP_TOGGLE,
	0,					// ALLY_CMD_MOVEASIDE,
	SKILL_LEVEL_SKILLED,// ALLY_CMD_DROP_EQUIP,
	SKILL_LEVEL_BASIC,	// ALLY_CMD_ATTACK_SELECT,
	SKILL_LEVEL_SKILLED,// ALLY_CMD_SPECIAL,
	SKILL_LEVEL_NOVICE,	// ALLY_CMD_FOLLOW,
	SKILL_LEVEL_BASIC,	// ALLY_CMD_MOVETO_CONFIRM,
	0,					// ALLY_CMD_CANCEL
	SKILL_LEVEL_EXPERT, // ALLY_CMD_ATTACK_CONFIRM
	0,					// ALLY_CMD_RETURN_SOUL
	0,					// ALLY_CMD_GYRO_DEPLOY,
	0,					// ALLY_CMD_GYRO_PATROL,
	0,					// ALLY_CMD_GYRO_LIGHT_TOGGLE,
	0,					// ALLY_CMD_GYRO_RETURN,
	SKILL_LEVEL_SKILLED,// ALLY_CMD_GYRO_DETECT_TOGGLE,
	0					// ALLY_CMD_END
};

enum AllyNPCState : int
{
	ALLY_STATE_DEFAULT,
	ALLY_STATE_MOVETO,
	ALLY_STATE_DEFEND,
	ALLY_STATE_INTERACT
};

enum AllyNPCPickup : int
{
	ALLY_PICKUP_NONPLAYER,
	ALLY_PICKUP_NONE,
	ALLY_PICKUP_ALL
};

enum AllyNPCClass : int
{
	ALLY_CLASS_MIXED,
	ALLY_CLASS_MELEE,
	ALLY_CLASS_RANGED
};

enum AllyNPCGyroLight : int
{
	ALLY_GYRO_LIGHT_NONE,
	ALLY_GYRO_LIGHT_FAINT,
	ALLY_GYRO_LIGHT_BRIGHT,
	ALLY_GYRO_LIGHT_END
};

enum AllyNPCGyroDetection : int
{
	ALLY_GYRO_DETECT_NONE,
	ALLY_GYRO_DETECT_ITEMS_METAL,
	ALLY_GYRO_DETECT_ITEMS_MAGIC,
	ALLY_GYRO_DETECT_TRAPS,
	ALLY_GYRO_DETECT_EXITS,
	ALLY_GYRO_DETECT_MONSTERS,
	ALLY_GYRO_DETECT_ITEMS_VALUABLE,
	ALLY_GYRO_DETECT_END
};

enum AllyNPCChatter : int
{
	ALLY_EVENT_MOVEASIDE,
	ALLY_EVENT_MOVETO_BEGIN,
	ALLY_EVENT_MOVETO_END,
	ALLY_EVENT_MOVETO_FAIL,
	ALLY_EVENT_MOVETO_REPATH,
	ALLY_EVENT_INTERACT_ITEM_UNKNOWN,
	ALLY_EVENT_INTERACT_ITEM_NOUSE,
	ALLY_EVENT_INTERACT_ITEM_FOOD_GOOD,
	ALLY_EVENT_INTERACT_ITEM_FOOD_BAD,
	ALLY_EVENT_INTERACT_ITEM_FOOD_ROTTEN,
	ALLY_EVENT_INTERACT_ITEM_FOOD_FULL,
	ALLY_EVENT_INTERACT_ITEM_CURSED,
	ALLY_EVENT_INTERACT_OTHER,
	ALLY_EVENT_ATTACK,
	ALLY_EVENT_ATTACK_FRIENDLY_FIRE,
	ALLY_EVENT_DROP_WEAPON,
	ALLY_EVENT_DROP_EQUIP,
	ALLY_EVENT_DROP_ALL,
	ALLY_EVENT_DROP_HUMAN_REFUSE,
	ALLY_EVENT_SPOT_ENEMY,
	ALLY_EVENT_WAIT,
	ALLY_EVENT_FOLLOW,
	ALLY_EVENT_REST,
	ALLY_EVENT_DROP_FAILED
};

enum AllyNPCSpecialCmd : int
{
	ALLY_SPECIAL_CMD_NONE,
	ALLY_SPECIAL_CMD_REST
};

enum MonsterDefendType : int
{
	MONSTER_DEFEND_NONE,
	MONSTER_DEFEND_ALLY,
	MONSTER_DEFEND_HOLD
};

#define WAIT_FOLLOWDIST 48
#define HUNT_FOLLOWDIST 64

#define HITRATE 45

#define MONSTER_INIT my->skill[3]
#define MONSTER_NUMBER my->skill[5]
#define MONSTER_HITTIME my->skill[7]
#define MONSTER_ATTACK my->skill[8]
#define MONSTER_ATTACKTIME my->skill[9]
#define MONSTER_ARMBENDED my->skill[10]
#define MONSTER_SPOTSND my->skill[11]
#define MONSTER_SPOTVAR my->skill[12]
#define MONSTER_CLICKED my->skill[13]
#define MONSTER_IDLESND my->skill[19]

#define MONSTER_IDLEVAR myStats->monster_idlevar
#define MONSTER_SOUND myStats->monster_sound
#define MONSTER_VELX my->vel_x
#define MONSTER_VELY my->vel_y
#define MONSTER_VELZ my->vel_z
#define MONSTER_WEAPONYAW my->fskill[5]
#define MONSTER_FLIPPEDANGLE my->fskill[6]
#define MONSTER_SHIELDYAW my->fskill[8]

static const int MONSTER_ALLY_DEXTERITY_SPEED_CAP = 15;

extern "C" void summonMonsterClient(Monster creature, long x, long y, Uint32 uid);
extern "C" Entity* summonMonster(Monster creature, long x, long y, bool forceLocation = false);
extern "C" Entity* summonMonsterNoSmoke(Monster creature, long x, long y, bool forceLocation = false);
extern "C" void summonManyMonster(Monster creature);
extern "C" bool monsterMoveAside(Entity* my, Entity* entity, bool ignoreMonsterState = false);

//--init* functions--
extern "C" void initHuman(Entity* my, Stat* myStats);
extern "C" void initRat(Entity* my, Stat* myStats);
extern "C" void initGoblin(Entity* my, Stat* myStats);
extern "C" void initSlime(Entity* my, Stat* myStats);
extern "C" void initScorpion(Entity* my, Stat* myStats);
extern "C" void initSuccubus(Entity* my, Stat* myStats);
extern "C" void initTroll(Entity* my, Stat* myStats);
extern "C" void initShopkeeper(Entity* my, Stat* myStats);
extern "C" void initSkeleton(Entity* my, Stat* myStats);
extern "C" void initMinotaur(Entity* my, Stat* myStats);
extern "C" void initGhoul(Entity* my, Stat* myStats);
extern "C" void initDemon(Entity* my, Stat* myStats);
extern "C" void initSpider(Entity* my, Stat* myStats);
extern "C" void initLich(Entity* my, Stat* myStats);
extern "C" void initImp(Entity* my, Stat* myStats);
extern "C" void initGnome(Entity* my, Stat* myStats);
extern "C" void initDevil(Entity* my, Stat* myStats);
extern "C" void initAutomaton(Entity* my, Stat* myStats);
extern "C" void initCockatrice(Entity* my, Stat* myStats);
extern "C" void initCrystalgolem(Entity* my, Stat* myStats);
extern "C" void initScarab(Entity* my, Stat* myStats);
extern "C" void initKobold(Entity* my, Stat* myStats);
extern "C" void initShadow(Entity* my, Stat* myStats);
extern "C" void initVampire(Entity* my, Stat* myStats);
extern "C" void initIncubus(Entity* my, Stat* myStats);
extern "C" void initInsectoid(Entity* my, Stat* myStats);
extern "C" void initGoatman(Entity* my, Stat* myStats);
extern "C" void initLichFire(Entity* my, Stat* myStats);
extern "C" void initLichIce(Entity* my, Stat* myStats);
extern "C" void initSentryBot(Entity* my, Stat* myStats);
extern "C" void initGyroBot(Entity* my, Stat* myStats);
extern "C" void initDummyBot(Entity* my, Stat* myStats);
extern "C" void initMimic(Entity* my, Stat* myStats);
extern "C" void initBat(Entity* my, Stat* myStats);
extern "C" void initBugbear(Entity* my, Stat* myStats);
extern "C" void initMonsterD(Entity* my, Stat* myStats);
extern "C" void initMonsterM(Entity* my, Stat* myStats);
extern "C" void initMonsterS(Entity* my, Stat* myStats);
extern "C" void initMonsterG(Entity* my, Stat* myStats);
extern "C" void initRevenantSkull(Entity* my, Stat* myStats);
extern "C" void initAdorcisedWeapon(Entity* my, Stat* myStats);
extern "C" void initMiniMimic(Entity* my, Stat* myStats);
extern "C" void initFlameElemental(Entity* my, Stat* myStats);
extern "C" void initHologram(Entity* my, Stat* myStats);
extern "C" void initMoth(Entity* my, Stat* myStats);
extern "C" void initEarthElemental(Entity* my, Stat* myStats);
extern "C" void initDuck(Entity* my, Stat* myStats);

//--act*Limb functions--
extern "C" void actHumanLimb(Entity* my);
extern "C" void actGoblinLimb(Entity* my);
extern "C" void actScorpionTail(Entity* my);
extern "C" void actSuccubusLimb(Entity* my);
extern "C" void actTrollLimb(Entity* my);
extern "C" void actShopkeeperLimb(Entity* my);
extern "C" void actSkeletonLimb(Entity* my);
extern "C" void actMinotaurLimb(Entity* my);
extern "C" void actGhoulLimb(Entity* my);
extern "C" void actDemonLimb(Entity* my);
extern "C" void actSpiderLimb(Entity* my);
extern "C" void actLichLimb(Entity* my);
extern "C" void actImpLimb(Entity* my);
extern "C" void actGnomeLimb(Entity* my);
extern "C" void actDevilLimb(Entity* my);
extern "C" void actAutomatonLimb(Entity* my);
extern "C" void actCockatriceLimb(Entity* my);
extern "C" void actCrystalgolemLimb(Entity* my);
extern "C" void actKoboldLimb(Entity* my);
extern "C" void actShadowLimb(Entity* my);
extern "C" void actVampireLimb(Entity* my);
extern "C" void actIncubusLimb(Entity* my);
extern "C" void actInsectoidLimb(Entity* my);
extern "C" void actGoatmanLimb(Entity* my);
extern "C" void actScarabLimb(Entity* my);
extern "C" void actLichFireLimb(Entity* my);
extern "C" void actLichIceLimb(Entity* my);
extern "C" void actSentryBotLimb(Entity* my);
extern "C" void actGyroBotLimb(Entity* my);
extern "C" void actDummyBotLimb(Entity* my);
extern "C" void actMimicLimb(Entity* my);
extern "C" void actBatLimb(Entity* my);
extern "C" void actBugbearLimb(Entity* my);
extern "C" void actMonsterDLimb(Entity* my);
extern "C" void actMonsterMLimb(Entity* my);
extern "C" void actMonsterSLimb(Entity* my);
extern "C" void actMonsterGLimb(Entity* my);
extern "C" void actRevenantSkullLimb(Entity* my);
extern "C" void actMiniMimicLimb(Entity* my);
extern "C" void actAdorcisedWeaponLimb(Entity* my);
extern "C" void actFlameElementalLimb(Entity* my);
extern "C" void actHologramLimb(Entity* my);
extern "C" void actMothLimb(Entity* my);
extern "C" void actEarthElementalLimb(Entity* my);
extern "C" void actDuckLimb(Entity* my);

//--*Die functions--
extern "C" void humanDie(Entity* my);
extern "C" void ratDie(Entity* my);
extern "C" void goblinDie(Entity* my);
extern "C" void slimeDie(Entity* my);
extern "C" void scorpionDie(Entity* my);
extern "C" void succubusDie(Entity* my);
extern "C" void trollDie(Entity* my);
extern "C" void shopkeeperDie(Entity* my);
extern "C" void skeletonDie(Entity* my);
extern "C" void minotaurDie(Entity* my);
extern "C" void ghoulDie(Entity* my);
extern "C" void demonDie(Entity* my);
extern "C" void spiderDie(Entity* my);
extern "C" void lichDie(Entity* my);
extern "C" void impDie(Entity* my);
extern "C" void gnomeDie(Entity* my);
extern "C" void devilDie(Entity* my);
extern "C" void automatonDie(Entity* my);
extern "C" void cockatriceDie(Entity* my);
extern "C" void crystalgolemDie(Entity* my);
extern "C" void scarabDie(Entity* my);
extern "C" void koboldDie(Entity* my);
extern "C" void shadowDie(Entity* my);
extern "C" void vampireDie(Entity* my);
extern "C" void incubusDie(Entity* my);
extern "C" void insectoidDie(Entity* my);
extern "C" void goatmanDie(Entity* my);
extern "C" void lichFireDie(Entity* my);
extern "C" void lichIceDie(Entity* my);
extern "C" void sentryBotDie(Entity* my);
extern "C" void gyroBotDie(Entity* my);
extern "C" void dummyBotDie(Entity* my);
extern "C" void mimicDie(Entity* my);
extern "C" void batDie(Entity* my);
extern "C" void bugbearDie(Entity* my);
extern "C" void monsterDDie(Entity* my);
extern "C" void monsterMDie(Entity* my);
extern "C" void monsterSDie(Entity* my);
extern "C" void monsterGDie(Entity* my);
extern "C" void revenantSkullDie(Entity* my);
extern "C" void miniMimicDie(Entity* my);
extern "C" void adorcisedWeaponDie(Entity* my);
extern "C" void flameElementalDie(Entity* my);
extern "C" void hologramDie(Entity* my);
extern "C" void mothDie(Entity* my);
extern "C" void earthElementalDie(Entity* my);
extern "C" void duckDie(Entity* my);

extern "C" void monsterAnimate(Entity* my, Stat* myStats, double dist);
//--*MoveBodyparts functions--
extern "C" void humanMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void ratAnimate(Entity* my, double dist);
extern "C" void goblinMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void slimeSetType(Entity* my, Stat* myStats, bool sink, BaronyRNG* rng);
extern "C" void slimeSprayAttack(Entity* my);
extern "C" void slimeAnimate(Entity* my, Stat* myStats, double dist);
extern "C" void scorpionAnimate(Entity* my, double dist);
extern "C" void succubusMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void trollMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void shopkeeperMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void skeletonMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void minotaurMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void ghoulMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void demonMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void spiderMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void lichAnimate(Entity* my, double dist);
extern "C" void impMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void gnomeMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void devilMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void cockatriceMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void automatonMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void crystalgolemMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void scarabAnimate(Entity* my, Stat* myStats, double dist);
extern "C" void koboldMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void shadowMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void vampireMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void incubusMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void insectoidMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void goatmanMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void lichFireAnimate(Entity* my, Stat* myStats, double dist);
extern "C" void lichIceAnimate(Entity* my, Stat* myStats, double dist);
extern "C" void sentryBotAnimate(Entity* my, Stat* myStats, double dist);
extern "C" void gyroBotAnimate(Entity* my, Stat* myStats, double dist);
extern "C" void dummyBotAnimate(Entity* my, Stat* myStats, double dist);
extern "C" void mimicAnimate(Entity* my, Stat* myStats, double dist);
extern "C" void batAnimate(Entity* my, Stat* myStats, double dist);
extern "C" void bugbearMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void monsterDMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void monsterMMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void monsterSMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void monsterGMoveBodyparts(Entity* my, Stat* myStats, double dist);
extern "C" void revenantSkullAnimate(Entity* my, Stat* myStats, double dist);
extern "C" void hologramAnimate(Entity* my, Stat* myStats, double dist);
extern "C" int mothGetAttackPose(Entity* my, int basePose);
extern "C" void mothAnimate(Entity* my, Stat* myStats, double dist);
extern "C" void earthElementalAnimate(Entity* my, Stat* myStats, double dist);
extern "C" void duckAnimate(Entity* my, Stat* myStats, double dist);
extern "C" void duckSpawnFeather(int sprite, real_t x, real_t y, real_t z, Entity* my);
extern "C" bool duckAreaQuck(Entity* my);

//--misc functions--
extern "C" void actMinotaurTrap(Entity* my);
extern "C" int getMinotaurTimeToArrive();
extern "C" void actMinotaurTimer(Entity* my);
extern "C" void actMinotaurCeilingBuster(Entity* my);
extern "C" void actDemonCeilingBuster(Entity* my);

extern "C" void actDevilTeleport(Entity* my);

extern "C" void createMinotaurTimer(Entity* entity, map_t* map, Uint32 seed);

extern "C" void actSummonTrap(Entity* my);
extern "C" int monsterCurve(int level);

extern "C" bool forceFollower(Entity& leader, Entity& follower);

//--monsterState constants
static const Sint32 MONSTER_STATE_WAIT = 0;
static const Sint32 MONSTER_STATE_ATTACK = 1;
static const Sint32 MONSTER_STATE_PATH = 2;
static const Sint32 MONSTER_STATE_HUNT = 3;
static const Sint32 MONSTER_STATE_TALK = 4;
static const Sint32 MONSTER_STATE_LICH_DODGE = 5;
static const Sint32 MONSTER_STATE_LICH_SUMMON = 6;
static const Sint32 MONSTER_STATE_LICH_DEATH = 7;
static const Sint32 MONSTER_STATE_DEVIL_DEATH = 8;
static const Sint32 MONSTER_STATE_DEVIL_TELEPORT = 9;
static const Sint32 MONSTER_STATE_DEVIL_RISING = 10;
static const Sint32 MONSTER_STATE_DEVIL_SUMMON = 11;
static const Sint32 MONSTER_STATE_DEVIL_BOULDER = 12;
static const Sint32 MONSTER_STATE_LICHFIRE_DODGE = 13;
static const Sint32 MONSTER_STATE_LICH_CASTSPELLS = 14;
static const Sint32 MONSTER_STATE_LICHFIRE_TELEPORT_STATIONARY = 15;
static const Sint32 MONSTER_STATE_LICH_TELEPORT_ROAMING = 16;
static const Sint32 MONSTER_STATE_LICHICE_TELEPORT_STATIONARY = 17;
static const Sint32 MONSTER_STATE_LICHICE_DODGE = 13;
static const Sint32 MONSTER_STATE_LICHFIRE_DIE = 18;
static const Sint32 MONSTER_STATE_LICHICE_DIE = 18;
static const Sint32 MONSTER_STATE_GENERIC_DODGE = 19;
static const Sint32 MONSTER_STATE_GENERIC_CHARGE = 20;

//--special monster attack constants
static const int MONSTER_POSE_MELEE_WINDUP1 = 4;
static const int MONSTER_POSE_MELEE_WINDUP2 = 5;
static const int MONSTER_POSE_MELEE_WINDUP3 = 6;
static const int MONSTER_POSE_RANGED_WINDUP1 = 7;
static const int MONSTER_POSE_RANGED_WINDUP2 = 8;
static const int MONSTER_POSE_RANGED_WINDUP3 = 9;
//TODO: Need potions and thrown.
static const int MONSTER_POSE_MAGIC_WINDUP1 = 10;
static const int MONSTER_POSE_MAGIC_WINDUP2 = 11;
static const int MONSTER_POSE_MAGIC_WINDUP3 = 12;
static const int MONSTER_POSE_SPECIAL_WINDUP1 = 13;
static const int MONSTER_POSE_SPECIAL_WINDUP2 = 14;
static const int MONSTER_POSE_SPECIAL_WINDUP3 = 15;
static const int MONSTER_POSE_RANGED_SHOOT1 = 16;
static const int MONSTER_POSE_RANGED_SHOOT2 = 17;
static const int MONSTER_POSE_RANGED_SHOOT3 = 18;
static const int MONSTER_POSE_MAGIC_CAST1 = 19;
static const int MONSTER_POSE_MAGIC_CAST2 = 20;
static const int MONSTER_POSE_MAGIC_CAST3 = 21;
static const int MONSTER_POSE_GOLEM_SMASH = 22;
static const int MONSTER_POSE_COCKATRICE_DOUBLEATTACK = 23;
static const int MONSTER_POSE_AUTOMATON_RECYCLE = 24;
//static const int MONSTER_POSE_SHADOW_TELEMIMICINVISI_WINDUP = 25;
static const int MONSTER_POSE_INSECTOID_DOUBLETHROW = 25;
static const int MONSTER_POSE_INCUBUS_CONFUSION = 26;
static const int MONSTER_POSE_INCUBUS_TELEPORT = 27;
static const int MONSTER_POSE_VAMPIRE_AURA_CHARGE = 28;
static const int MONSTER_POSE_VAMPIRE_DRAIN = 29;
static const int MONSTER_POSE_VAMPIRE_AURA_CAST = 30;
static const int MONSTER_POSE_AUTOMATON_MALFUNCTION = 31;
static const int MONSTER_POSE_LICH_FIRE_SWORD = 32;
static const int PLAYER_POSE_GOLEM_SMASH = 33;
static const int MONSTER_POSE_INCUBUS_TAUNT = 34;
static const int MONSTER_POSE_MIMIC_DISTURBED = 35;
static const int MONSTER_POSE_MIMIC_DISTURBED2 = 36;
static const int MONSTER_POSE_MIMIC_LOCKED = 37;
static const int MONSTER_POSE_MIMIC_LOCKED2 = 38;
static const int MONSTER_POSE_MIMIC_MAGIC1 = 39;
static const int MONSTER_POSE_MIMIC_MAGIC2 = 40;
static const int MONSTER_POSE_BUGBEAR_SHIELD = 41;
static const int MONSTER_POSE_PARRY = 42;
static const int MONSTER_POSE_EARTH_ELEMENTAL_ROLL = 43;
static const int MONSTER_POSE_FLAIL_SWING = 44;
static const int MONSTER_POSE_FLAIL_SWING_RETURN = 45;
static const int MONSTER_POSE_FLAIL_SWING_WINDUP = 46;
static const int MONSTER_POSE_SWEEP_ATTACK_NO_UPDATE = 47;

//--monster special cooldowns
static const int MONSTER_SPECIAL_COOLDOWN_GOLEM = 150;
static const int MONSTER_SPECIAL_COOLDOWN_KOBOLD = 250;
static const int MONSTER_SPECIAL_COOLDOWN_COCKATRICE_ATK = 100;
static const int MONSTER_SPECIAL_COOLDOWN_COCKATRICE_STONE = 250;
static const int MONSTER_SPECIAL_COOLDOWN_AUTOMATON_RECYCLE = 500;
static const int MONSTER_SPECIAL_COOLDOWN_AUTOMATON_MALFUNCTION = 200;
static const int MONSTER_SPECIAL_COOLDOWN_GOATMAN_THROW = 300;
static const int MONSTER_SPECIAL_COOLDOWN_GOATMAN_DRINK = 350;
static const int MONSTER_SPECIAL_COOLDOWN_SHADOW_TELEMIMICINVISI_ATTACK = 500;
static const int MONSTER_SPECIAL_COOLDOWN_SHADOW_PASIVE_TELEPORT = 250;
static const int MONSTER_SPECIAL_COOLDOWN_SHADOW_SPELLCAST = 250;
static const int MONSTER_SPECIAL_COOLDOWN_SHADOW_TELEPORT = 300;
static const int MONSTER_SPECIAL_COOLDOWN_INSECTOID_THROW = 250;
static const int MONSTER_SPECIAL_COOLDOWN_INSECTOID_ACID = 500;
static const int MONSTER_SPECIAL_COOLDOWN_SPIDER_CAST = 500;
static const int MONSTER_SPECIAL_COOLDOWN_INCUBUS_CONFUSION = 500;
static const int MONSTER_SPECIAL_COOLDOWN_INCUBUS_STEAL = 500;
static const int MONSTER_SPECIAL_COOLDOWN_INCUBUS_CHARM = 300;
static const int MONSTER_SPECIAL_COOLDOWN_INCUBUS_TELEPORT_RANDOM = 400;
static const int MONSTER_SPECIAL_COOLDOWN_INCUBUS_TELEPORT_TARGET = 200;
static const int MONSTER_SPECIAL_COOLDOWN_VAMPIRE_AURA = 500;
static const int MONSTER_SPECIAL_COOLDOWN_VAMPIRE_DRAIN = 300;
static const int MONSTER_SPECIAL_COOLDOWN_SUCCUBUS_CHARM = 400;
static const int MONSTER_SPECIAL_COOLDOWN_MIMIC_EAT = 500;
static const int MONSTER_SPECIAL_COOLDOWN_SLIME_SPRAY = 250;
static const int MONSTER_SPECIAL_COOLDOWN_SKULL_CAST = 250;
static const int MONSTER_SPECIAL_COOLDOWN_MOTH_CAST = 250;
static const int MONSTER_SPECIAL_COOLDOWN_BUGBEAR = 500;
static const int MONSTER_SPECIAL_COOLDOWN_MONSTER_D = 250;
static const int MONSTER_SPECIAL_COOLDOWN_MONSTER_D_PUSH = 75;
static const int MONSTER_SPECIAL_COOLDOWN_MONSTER_M_THROW = 500;
static const int MONSTER_SPECIAL_COOLDOWN_MONSTER_M_CAST_SHORT = 300;
static const int MONSTER_SPECIAL_COOLDOWN_MONSTER_M_CAST_LONG = 550;
static const int MONSTER_SPECIAL_COOLDOWN_MONSTER_G_THROW = 500;
static const int MONSTER_SPECIAL_COOLDOWN_MONSTER_G_CAST = 550;

//--monster target search types
static const int MONSTER_TARGET_ENEMY = 0;
static const int MONSTER_TARGET_FRIEND = 1;
static const int MONSTER_TARGET_PLAYER = 2;
static const int MONSTER_TARGET_ALL = 3;

//--monster animation handler
static const int ANIMATE_YAW = 1;
static const int ANIMATE_PITCH = 2;
static const int ANIMATE_ROLL = 3;
static const int ANIMATE_WEAPON_YAW = 4;
static const int ANIMATE_Z = 5;

static const int ANIMATE_DIR_POSITIVE = 1;
static const int ANIMATE_DIR_NEGATIVE = -1;
static const int ANIMATE_DIR_NONE = 0;

static const int ANIMATE_OVERSHOOT_TO_SETPOINT = 1;
static const int ANIMATE_OVERSHOOT_TO_ENDPOINT = 2;
static const int ANIMATE_OVERSHOOT_NONE = 0;

//--monster limb bodypart IDs
static const int LIMB_HUMANOID_TORSO = 2;
static const int LIMB_HUMANOID_RIGHTLEG = 3;
static const int LIMB_HUMANOID_LEFTLEG = 4;
static const int LIMB_HUMANOID_RIGHTARM = 5;
static const int LIMB_HUMANOID_LEFTARM = 6;
static const int LIMB_HUMANOID_WEAPON = 7;
static const int LIMB_HUMANOID_SHIELD = 8;
static const int LIMB_HUMANOID_CLOAK = 9;
static const int LIMB_HUMANOID_HELMET = 10;
static const int LIMB_HUMANOID_MASK = 11;

//--monster attack windup duration, in ticks, roughly 180ms
static const int ANIMATE_DURATION_WINDUP = 9;
static const int ANIMATE_DURATION_WINDUP_SHADOW_SPECIAL = 50;

//--monster footstep sounds
static const int MONSTER_FOOTSTEP_NONE = 0;
static const int MONSTER_FOOTSTEP_STOMP = 1;
static const int MONSTER_FOOTSTEP_SKELETON = 2;
static const int MONSTER_FOOTSTEP_LEATHER = 3;
static const int MONSTER_FOOTSTEP_USE_BOOTS = 4; // variable dependent on footwear

//--monster spellcasting animation types
static const int MONSTER_SPELLCAST_NONE = 0;
static const int MONSTER_SPELLCAST_SMALL_HUMANOID = 1;
static const int MONSTER_SPELLCAST_HUMANOID = 2;

//--monster NPC language lines
static const int MONSTER_NPC_DIALOGUE_LINES = 10;

//--animates the selected limb to setpoint along the axis, at the given rate.
extern "C" int limbAnimateToLimit(Entity* limb, int axis, double rate, double setpoint, bool shake, double shakerate);
//--animates the selected limb to setpoint, then endpoint along the axis, provided MONSTER_LIMB_OVERSHOOT is set
extern "C" int limbAnimateWithOvershoot(Entity* limb, int axis, double setpointRate, double setpoint, double endpointRate, double endpoint, int dir);
extern "C" int limbAngleWithinRange(real_t angle, double rate, double setpoint);
extern "C" real_t normaliseAngle2PI(real_t angle);
extern "C" void getTargetsAroundEntity(Entity* my, Entity* originalTarget, double distToFind, real_t angleToSearch, int searchType, list_t** list);
extern "C" int numTargetsAroundEntity(Entity* my, double distToFind, real_t angleToSearch, int searchType);
// change animation speeds for debugging, default value 10.
extern int monsterGlobalAnimationMultiplier;
// change attacktime for debugging, default value 1.
extern int monsterGlobalAttackTimeMultiplier;
// monster custom NPC chatter
extern "C" bool handleMonsterChatter(int monsterclicked, bool ringconflict, char namesays[64], Entity* my, Stat* myStats);
// check qty of a certain creature race alive on a map
extern "C" int numMonsterTypeAliveOnMap(Monster creature, Entity*& lastMonster);
// get monster strings from language file
extern "C" DynamicString getMonsterLocalizedName(Monster creature, Stat* optionalStats = nullptr);
extern "C" DynamicString getMonsterLocalizedPlural(Monster creature);
extern "C" DynamicString getMonsterLocalizedInjury(Monster creature);

//-----RACE SPECIFIC CONSTANTS-----

//--Goatman--
static const int GOATMAN_HEALINGPOTION_MOD = 3;
static const int GOATMAN_HEALING_POTION_SPEED_BOOST_DURATION = 1800;
static const int GOATMAN_POTION = 1;
static const int GOATMAN_THROW = 2;

//--Automaton--
static const int AUTOMATON_RECYCLE_ANIMATION_WAITING = 0;
static const int AUTOMATON_RECYCLE_ANIMATION_COMPLETE = 1;
static const int AUTOMATON_MALFUNCTION_START = 2;
static const int AUTOMATON_MALFUNCTION_RUN = 3;

//--Insectoid--
static const int INSECTOID_ACID = 1;
static const int INSECTOID_DOUBLETHROW_FIRST = 2;
static const int INSECTOID_DOUBLETHROW_SECOND = 3;

//--Incubus--
static const int INCUBUS_CONFUSION = 1;
static const int INCUBUS_STEAL = 2;
static const int INCUBUS_TELEPORT_STEAL = 3;
static const int INCUBUS_TELEPORT = 4;
static const int INCUBUS_CHARM = 5;

//--Vampire--
static const int VAMPIRE_CAST_AURA = 1;
static const int VAMPIRE_CAST_DRAIN = 2;

//--Succubus--
static const int SUCCUBUS_CHARM = 1;

//--Spider--
static const int SPIDER_CAST = 1;

//--Slime--
static const int SLIME_CAST = 1;

//--Skull
static const int SKULL_CAST = 1;

//--Moth
static const int MOTH_CAST = 1;

//--Shadow--
static const int SHADOW_SPELLCAST = 1;
static const int SHADOW_TELEPORT_ONLY = 2;

//--Generic
static const int MONSTER_SPELLCAST_GENERIC = 100;
static const int MONSTER_SPELLCAST_GENERIC2 = 101;
static const int MONSTER_SPECIAL_SAFEGUARD_TIMER_BASE = 0;

//--Lich Attacks--
static const int LICH_ATK_VERTICAL_SINGLE = 0;
static const int LICH_ATK_HORIZONTAL_SINGLE = 1;
static const int LICH_ATK_RISING_RAIN = 2;
static const int LICH_ATK_BASICSPELL_SINGLE = 3;
static const int LICH_ATK_RISING_SINGLE = 4;
static const int LICH_ATK_VERTICAL_QUICK = 5;
static const int LICH_ATK_HORIZONTAL_RETURN = 6;
static const int LICH_ATK_HORIZONTAL_QUICK = 7;
static const int LICH_ATK_CHARGE_AOE = 8;
static const int LICH_ATK_FALLING_DIAGONAL = 9;
static const int LICH_ATK_SUMMON = 10;

//--Lich Special States--
static const int LICH_ICE_ATTACK_COMBO = 1;
static const int LICH_ALLY_ALIVE = 0;
static const int LICH_ALLY_DEAD = 1;

//--Lich Battle States--
static const int LICH_BATTLE_IMMOBILE = -1;
static const int LICH_BATTLE_READY = 0;

//--Gyrobot--
static const int GYRO_RETURN_PATHING = 1;
static const int GYRO_RETURN_LANDING = 2;
static const int GYRO_INTERACT_LANDING = 3;
static const int GYRO_START_FLYING = 4;

//--Dummybot--
static const int DUMMYBOT_RETURN_FORM = 1;

//--Mimic--
static const int MIMIC_ACTIVE = 0;
static const int MIMIC_INERT = 1;
static const int MIMIC_MAGIC = 2;
static const int MIMIC_INERT_SECOND = 3;
static const int MIMIC_STATUS_IMMOBILE = 4;

//-Bat--
static const int BAT_REST = 1;
static const int BAT_REST_DISTURBED = 2;

//-Bugbear--
static const int BUGBEAR_DEFENSE = 1;

static const int MONSTER_D_SPECIAL_CAST1 = 1;
static const int MONSTER_D_SPECIAL_CAST2 = 2;
static const int MONSTER_D_SPECIAL_CAST3 = 3;

static const int MONSTER_M_SPECIAL_THROW = 1;
static const int MONSTER_M_SPECIAL_CAST1 = 2;
static const int MONSTER_M_SPECIAL_CAST2 = 3;
static const int MONSTER_M_SPECIAL_CAST3 = 4;

static const int MONSTER_G_SPECIAL_THROW = 1;
static const int MONSTER_G_SPECIAL_CAST1 = 2;

//-Duck--
static const int DUCK_DIVE = 1;
static const int DUCK_INERT = 2;
static const int DUCK_RETURN = 3;

struct MonsterData_t
{
	struct MonsterDataEntry_t
	{
		int monsterType = NOTHING;
		DynamicString defaultIconPath = "";
		struct IconLookup_t
		{
			DynamicString key = "";
			DynamicString iconPath = "";
		};
		DynamicMapI32T<IconLookup_t> iconSpritesAndPaths;
		DynamicMapStrT<DynamicArrayS32> keyToSpriteLookup;
		DynamicSetI32 modelIndexes;
		DynamicSetI32 playerModelIndexes;
		DynamicString defaultShortDisplayName = "";
		typedef SpecialNPCEntry_tMirror SpecialNPCEntry_t;
		DynamicMapSpecialNPC specialNPCs;
		MonsterDataEntry_t(int type)
		{
			monsterType = type;
			defaultShortDisplayName = "";
			defaultIconPath = "";
		};
		MonsterDataEntry_t() = default;
	};
	static DynamicString iconDefaultString;
	static DynamicString keyDefaultString;
	static DynamicMapI32T<MonsterDataEntry_t> monsterDataEntries;
	static DynamicString getAllyIconFromSprite(int sprite, int type = -1);
	static DynamicString getKeyFromSprite(int sprite, int type = -1);
	static int getSpriteFromKey(int sprite, DynamicString key, int type = -1);
	static int getSpecialNPCBaseModel(Stat& myStats);
	static DynamicString getSpecialNPCName(Stat& myStats);
	static bool nameMatchesSpecialNPCName(Stat& myStats, DynamicString npcKey);
	static void loadMonsterDataJSON();
};
extern MonsterData_t monsterData;
template <> struct MapValueKindOf<MonsterData_t::MonsterDataEntry_t::IconLookup_t> { static constexpr int value = MK_IconLookup; };
template <> struct MapValueKindOf<MonsterData_t::MonsterDataEntry_t> { static constexpr int value = MK_MonsterDataEntry; };

class ShopkeeperPlayerHostility_t
{
public:
	enum WantedLevel : int
	{
		NO_WANTED_LEVEL,
		FAILURE_TO_IDENTIFY,
		WANTED_FOR_AGGRESSION_SHOPKEEP_INITIATED,
		WANTED_FOR_ACCESSORY,
		WANTED_FOR_AGGRESSION,
		WANTED_FOR_KILL
	};
	struct PlayerRaceHostility_t
	{
	public:
		int numAggressions = 0;
		int numKills = 0;
		int numAccessories = 0;
		Monster playerRace = NOTHING;
		sex_t sex = sex_t::MALE;
		Uint8 equipment = 0;
		Uint32 type = NOTHING;
		WantedLevel wantedLevel = NO_WANTED_LEVEL;
		int player = -1;
		bool bRequiresNetUpdate = false;
		PlayerRaceHostility_t()
		{
			wantedLevel = NO_WANTED_LEVEL;
			type = NOTHING;
			playerRace = NOTHING;
			sex = sex_t::MALE;
			equipment = 0;
			player = -1;
			numAggressions = 0;
			numKills = 0;
			numAccessories = 0;
		};
		PlayerRaceHostility_t(const Uint32 _type, const WantedLevel _wantedLevel, const int _player) :
			PlayerRaceHostility_t()
		{
			wantedLevel = _wantedLevel;
			type = _type;
			playerRace = (Monster)(_type & 0xFF);
			sex = ((_type >> 8) & 0x1) ? sex_t::MALE : sex_t::FEMALE;
			equipment = ((_type >> 9) & 0x7F);
			player = _player;
		}

		bool serialize(FileInterface* fp);
	};
	bool playerRaceCheckHostility(const int player, const Monster type) const;
	PlayerRaceHostility_t* getPlayerHostility(const int player, Uint32 overrideType = NOTHING);
	void serverSendClientUpdate(const bool force = false);
	void reset();
	void resetPlayerHostility(const int player, bool clearAll = false);
	ShopkeeperPlayerHostility_t();
	bool isPlayerEnemy(const int player);
	void setWantedLevel(PlayerRaceHostility_t& h, WantedLevel wantedLevel, Entity* shopkeeper, bool primaryPlayerCheck);
	WantedLevel getWantedLevel(const int player);
	void onShopkeeperDeath(Entity* my, Stat* myStats, Entity* attacker);
	void onShopkeeperHit(Entity* my, Stat* myStats, Entity* attacker);
	void updateShopkeeperActMonster(Entity& my, Stat& myStats, bool ringconflict);
	DynamicMapI32T<PlayerRaceHostility_t> playerHostility[MAXPLAYERS];
};
extern ShopkeeperPlayerHostility_t ShopkeeperPlayerHostility;
template <> struct MapValueKindOf<ShopkeeperPlayerHostility_t::PlayerRaceHostility_t> { static constexpr int value = MK_PlayerRaceHostility; };

struct MonsterAllyFormation_t
{
	struct MonsterAllies_t
	{
		struct FormationInfo_t
		{
			int x = 0;
			int y = 0;
			int pathingDelay = 0;
			int tryExtendPath = 0;
			bool init = false;
			bool expired = false;
		};
		DynamicMapI32T<FormationInfo_t> meleeUnits;
		DynamicMapI32T<FormationInfo_t> rangedUnits;
		Uint32 updatedOnTick = 0;
	};
	void updateFormation(Uint32 leaderUid, Uint32 monsterUpdateUid = 0);
	bool getFollowLocation(Uint32 uid, Uint32 leaderUid, std::pair<int, int>& outPos);
	void updateOnPathFail(Uint32 uid, Entity* entity);
	void updateOnPathSucceed(Uint32 uid, Entity* entity);
	void updateOnFollowCommand(Uint32 uid, Entity* entity);
	DynamicMapI32T<MonsterAllies_t> units;
	DynamicArrayT<std::pair<int, int>> formationShape;
	MonsterAllyFormation_t()
	{
		formationShape.push_back(std::make_pair(-2, 0));
		formationShape.push_back(std::make_pair(2, 0));
		for ( int i = 1; i < 50; ++i )
		{
			if ( i % 2 == 0 )
			{
				formationShape.push_back(std::make_pair(-2, -i));
				formationShape.push_back(std::make_pair(2, -i));
				formationShape.push_back(std::make_pair(0, -i));
			}
			else
			{
				formationShape.push_back(std::make_pair(-1, -i));
				formationShape.push_back(std::make_pair(1, -i));
			}
		}
	}
	void reset();
	int getFollowerChaseLeaderInterval(Entity& my, Stat& myStats);
	int getFollowerPathingDelay(Entity& my, Stat& myStats);
	int getFollowerTryExtendedPathSearch(Entity& my, Stat& myStats);
};
extern MonsterAllyFormation_t monsterAllyFormations;
template <> struct MapValueKindOf<MonsterAllyFormation_t::MonsterAllies_t::FormationInfo_t> { static constexpr int value = MK_FormationInfo; };
template <> struct MapValueKindOf<MonsterAllyFormation_t::MonsterAllies_t> { static constexpr int value = MK_MonsterAllies; };

struct MimicGenerator
{
	BaronyRNG mimic_rng;
	DynamicSetI32 mimic_floors;
	DynamicSetI32 mimic_secret_floors;
	void init();
	bool bForceSpawnForCurrentFloor();
};
extern MimicGenerator mimic_generator;

extern "C" bool monsterDebugModels(Entity* my, real_t* dist);

extern double sightranges[NUMMONSTERS];