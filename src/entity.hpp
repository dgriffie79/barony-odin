/*-------------------------------------------------------------------------------

	BARONY
	File: entity.hpp
	Desc: contains entity related declarations

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#pragma once

#include "main.hpp"
#include "game.hpp"
#include "stat.hpp"
#include "light.hpp"
#include "monster.hpp"
#include "interface/consolecommand.hpp"

// entity flags
#define BRIGHT 1
#define INVISIBLE 2
#define NOUPDATE 3
#define UPDATENEEDED 4
#define GENIUS 5
#define OVERDRAW 6
#define SPRITE 7
#define BLOCKSIGHT 8
#define BURNING 9
#define BURNABLE 10
#define UNCLICKABLE 11
#define PASSABLE 12
#define USERFLAG1 14
#define USERFLAG2 15
#define INVISIBLE_DITHER 16
#define NOCLIP_WALLS 17
#define NOCLIP_CREATURES 18
#define ENTITY_SKIP_CULLING 19
#define STASIS_DITHER 20

// number of entity skills and fskills
static const int NUMENTITYSKILLS = 60;
static const int NUMENTITYFSKILLS = 30;
extern CvarInt cvar_entity_bodypart_sync_tick;
struct spell_t;

// entity class
class Entity
{
	inline int& circuit_status() { return skill[28]; }
	inline const int& circuit_status() const { return skill[28]; }	// Use CIRCUIT_OFF and CIRCUIT_ON.
	inline int& switch_power() { return skill[0]; }
	inline const int& switch_power() const { return skill[0]; }	// Switch/mechanism power status.
	inline int& chanceToPutOutFire() { return skill[37]; }
	inline const int& chanceToPutOutFire() const { return skill[37]; } // skill[37] - Value between 5 and 10, with 10 being the default starting chance, and 5 being absolute minimum

	// Power crystal skills
	inline int& crystalInitialised() { return skill[1]; }
	inline const int& crystalInitialised() const { return skill[1]; } // 1 if init, else 0 skill[1]
	inline int& crystalTurning() { return skill[3]; }
	inline const int& crystalTurning() const { return skill[3]; } // 1 if currently rotating, else 0 skill[3]
	inline int& crystalTurnStartDir() { return skill[4]; }
	inline const int& crystalTurnStartDir() const { return skill[4]; } // when rotating, the previous facing direction stored here 0-3 skill[4]
	inline int& crystalGeneratedElectricityNodes() { return skill[5]; }
	inline const int& crystalGeneratedElectricityNodes() const { return skill[5]; } // 1 if electricity nodes generated previously, else 0 skill[5]
	inline int& crystalHoverDirection() { return skill[7]; }
	inline const int& crystalHoverDirection() const { return skill[7]; } // animation, waiting/up/down floating state skill[7]
	inline int& crystalHoverWaitTimer() { return skill[8]; }
	inline const int& crystalHoverWaitTimer() const { return skill[8]; } // animation, if waiting state, then wait this many ticks before moving to next state skill[8]

	// Pedestal Orb skills
	inline int& orbInitialised() { return skill[1]; }
	inline const int& orbInitialised() const { return skill[1]; } // 1 if init, else 0 skill[1]
	inline int& orbHoverDirection() { return skill[7]; }
	inline const int& orbHoverDirection() const { return skill[7]; } // animation, waiting/up/down floating state skill[7]
	inline int& orbHoverWaitTimer() { return skill[8]; }
	inline const int& orbHoverWaitTimer() const { return skill[8]; } // animation, if waiting state, then wait this many ticks before moving to next state skill[8]
	inline int& entityShowOnMap() { return skill[59]; }
	inline const int& entityShowOnMap() const { return skill[59]; } //skill[59]

	//### Begin - Private Entity Constants for BURNING Status Effect
	static const Sint32 MIN_TICKS_ON_FIRE		= TICKS_TO_PROCESS_FIRE *  4; // Minimum time an Entity can be on fire is  4 cycles (120 ticks)
	static const Sint32 MAX_TICKS_ON_FIRE		= TICKS_TO_PROCESS_FIRE * 20; // Maximum time an Entity can be on fire is 20 cycles (600 ticks)
	static const Sint32 MIN_CHANCE_STOP_FIRE	= 5;	// Minimum chance an Entity has to stop being on fire is 1 in  5
	static const Sint32 MAX_CHANCE_STOP_FIRE	= 10;	// Maximum chance an Entity has to stop being on fire is 1 in 10

	// Maximum level of CON needed to get MIN_CHANCE_STOP_FIRE. Every 5 points = 1 increase in chance up to MIN_CHANCE_STOP_FIRE
	static const Sint32 MAX_CON_FOR_STOP_FIRE = 5 * MIN_CHANCE_STOP_FIRE;	
	// Maximum level of CON needed to get MIN_TICKS_ON_FIRE. Every 2 points = 1 second decrease in time up to MIN_TICKS_ON_FIRE
	static const Sint32 MAX_CON_FOR_FIRE_TIME = (2 * (MAX_TICKS_ON_FIRE - MIN_TICKS_ON_FIRE)) / TICKS_TO_PROCESS_FIRE;
	//### End   - Private Entity Constants for BURNING Status Effect

	static const int CRYSTAL_HOVER_UP = 0;
	static const int CRYSTAL_HOVER_UP_WAIT = 1;
	static const int CRYSTAL_HOVER_DOWN = 2;
	static const int CRYSTAL_HOVER_DOWN_WAIT = 3;

	//--- Mechanism defines ---
	static const int CIRCUIT_OFF = 1;
	static const int CIRCUIT_ON = 2;

	static const int SWITCH_UNPOWERED = 0;
	static const int SWITCH_POWERED = 1;
	Uint32 uid;                    // entity uid
public:
	Entity(Sint32 in_sprite, Uint32 pos, list_t* entlist, list_t* creaturelist);
	~Entity();
    
    bool ditheringDisabled = false;
	int ditheringOverride = -1;
    struct Dither {
        int value = 0;
        Uint32 lastUpdateTick = 0;
        static constexpr int MAX = 10;
    };
    DynamicMapPtrT<Dither_t> dithering;
	vec4_t lightBonus;

	void* entity_sound = nullptr;

	void stopEntitySound();

	void setEntityString(const char* str);

	bool entityHasString(const char* str);

	Uint32 getUID() const {return uid;}
	void setUID(Uint32 new_uid);
	Uint32 ticks;                  // duration of the entity's existence
	real_t x, y, z;                // world coordinates
	real_t yaw, pitch, roll;       // rotation
	real_t focalx, focaly, focalz; // focal point for rotation, movement, etc.
	real_t scalex, scaley, scalez; // stretches/squashes the entity visually
	Sint32 sizex, sizey;           // entity bounding box size
	Sint32 sprite;                 // the entity's sprite index

	// network stuff
	Uint32 lastupdate;                   // last time since the entity was updated
	Uint32 lastupdateserver;             // used to sort out old packets
	real_t vel_x, vel_y, vel_z;          // entity velocity vector
	real_t new_x, new_y, new_z;          // world coordinates
	real_t new_yaw, new_pitch, new_roll; // rotation

	// entity attributes
	real_t fskill[NUMENTITYFSKILLS]; // floating point general purpose variables
	Sint32 skill[NUMENTITYSKILLS];  // general purpose variables
	bool flags[24];    // engine flags
	char* string;      // general purpose string
	light_t* light;    // every entity has a specialized light pointer
	list_t children;   // every entity has a list of child objects
	Uint32 parent;     // id of the entity's "parent" entity

	TimerExperiments::EntityStates lerpPreviousState;
	TimerExperiments::EntityStates lerpCurrentState;
	TimerExperiments::EntityStates lerpRenderState;
	real_t lerp_ox;
	real_t lerp_oy;
	bool bNeedsRenderPositionInit = true;
	bool bUseRenderInterpolation = false;
	int mapGenerationRoomX = 0; // captures the x/y of the 'room' this spawned in on generate dungeon
	int mapGenerationRoomY = 0; // captures the x/y of the 'room' this spawned in on generate dungeon

	BaronyRNG* entity_rng = nullptr;
	void seedEntityRNG(Uint32 seed);

	//--PUBLIC CHEST SKILLS--

	//Chest skills.
	//skill[0]
	inline int& chestInit() { return skill[0]; }
	inline const int& chestInit() const { return skill[0]; }
	//skill[1]
	//0 = closed. 1 = open.
	//0 = closed. 1 = open.
	inline int& chestStatus() { return skill[1]; }
	inline const int& chestStatus() const { return skill[1]; }
	//skill[2] is reserved for all entities.
	//skill[3]
	inline int& chestHealth() { return skill[3]; }
	inline const int& chestHealth() const { return skill[3]; }
	//skill[5]
	//Index of the player the chest was opened by.
	inline int& chestOpener() { return skill[5]; }
	inline const int& chestOpener() const { return skill[5]; }
	//skill[6]
	inline int& chestLidClicked() { return skill[6]; }
	inline const int& chestLidClicked() const { return skill[6]; }
	//skill[7]
	inline int& chestAmbience() { return skill[7]; }
	inline const int& chestAmbience() const { return skill[7]; }
	//skill[8]
	inline int& chestMaxHealth() { return skill[8]; }
	inline const int& chestMaxHealth() const { return skill[8]; }
	//skill[9]
	//field to be set if the chest sprite is 75-81 in the editor, otherwise should stay at value 0
	inline int& chestType() { return skill[9]; }
	inline const int& chestType() const { return skill[9]; }

	//skill[4]
	//0 = unlocked. 1 = locked.
	inline int& chestLocked() { return skill[4]; }
	inline const int& chestLocked() const { return skill[4]; }
	/*
	 * skill[10]
	 * 1 = chest already has been unlocked, or spawned in unlocked (prevent spell exploit)
	 * 0 = chest spawned in locked and is still ripe for harvest.
	 * Purpose: To prevent exploits with repeatedly locking and unlocking a chest.
	 * Also doesn't spawn gold for chests that didn't spawn locked
	 * (e.g. you locked a chest with a spell...sorry, no gold for you)
	 */
	inline int& chestPreventLockpickCapstoneExploit() { return skill[10]; }
	inline const int& chestPreventLockpickCapstoneExploit() const { return skill[10]; }
	inline int& chestHasVampireBook() { return skill[11]; }
	inline const int& chestHasVampireBook() const { return skill[11]; } // skill[11]
	inline int& chestLockpickHealth() { return skill[12]; }
	inline const int& chestLockpickHealth() const { return skill[12]; } // skill[12]
	inline int& chestOldHealth() { return skill[15]; }
	inline const int& chestOldHealth() const { return skill[15]; } //skill[15]
	inline int& chestMimicChance() { return skill[16]; }
	inline const int& chestMimicChance() const { return skill[16]; } //skill[16]
	inline int& chestVoidState() { return skill[17]; }
	inline const int& chestVoidState() const { return skill[17]; }
	inline int& char_gonnavomit() { return skill[26]; }
	inline const int& char_gonnavomit() const { return skill[26]; } // skill[26]
	inline int& char_heal() { return skill[22]; }
	inline const int& char_heal() const { return skill[22]; } // skill[22]
	inline int& char_energize() { return skill[23]; }
	inline const int& char_energize() const { return skill[23]; } // skill[23]
	inline int& char_drunk() { return skill[24]; }
	inline const int& char_drunk() const { return skill[24]; } // skill[24]
	inline int& char_torchtime() { return skill[25]; }
	inline const int& char_torchtime() const { return skill[25]; } // skill[25]
	inline int& char_poison() { return skill[21]; }
	inline const int& char_poison() const { return skill[21]; } // skill[21]
	inline int& char_fire() { return skill[36]; }
	inline const int& char_fire() const { return skill[36]; }		// skill[36] - Counter for how many ticks Entity will be on fire

	//--PUBLIC MONSTER SKILLS--
	inline int& monsterState() { return skill[0]; }
	inline const int& monsterState() const { return skill[0]; } //skill[0]
	inline int& monsterTarget() { return skill[1]; }
	inline const int& monsterTarget() const { return skill[1]; } //skill[1]
	inline real_t& monsterTargetX() { return fskill[2]; }
	inline const real_t& monsterTargetX() const { return fskill[2]; } //fskill[2]
	inline real_t& monsterTargetY() { return fskill[3]; }
	inline const real_t& monsterTargetY() const { return fskill[3]; } //fskill[3]
	inline int& monsterSpecialTimer() { return skill[29]; }
	inline const int& monsterSpecialTimer() const { return skill[29]; } //skill[29]
	//Only used by goatman.
	inline int& monsterSpecialState() { return skill[33]; }
	inline const int& monsterSpecialState() const { return skill[33]; } //skill[33]
	inline int& monsterSpellAnimation() { return skill[31]; }
	inline const int& monsterSpellAnimation() const { return skill[31]; } //skill[31]
	inline int& monsterFootstepType() { return skill[32]; }
	inline const int& monsterFootstepType() const { return skill[32]; } //skill[32]
	inline int& monsterLookTime() { return skill[4]; }
	inline const int& monsterLookTime() const { return skill[4]; } //skill[4]
	inline int& monsterAttack() { return skill[8]; }
	inline const int& monsterAttack() const { return skill[8]; } //skill[8]
	inline int& monsterAttackTime() { return skill[9]; }
	inline const int& monsterAttackTime() const { return skill[9]; } //skill[9]
	inline int& monsterArmbended() { return skill[10]; }
	inline const int& monsterArmbended() const { return skill[10]; } //skill[10]
	inline real_t& monsterWeaponYaw() { return fskill[5]; }
	inline const real_t& monsterWeaponYaw() const { return fskill[5]; } //fskill[5]
	inline int& monsterMoveTime() { return skill[6]; }
	inline const int& monsterMoveTime() const { return skill[6]; } //skill[6]
	inline int& monsterHitTime() { return skill[7]; }
	inline const int& monsterHitTime() const { return skill[7]; } //skill[7]
	inline int& monsterPathBoundaryXStart() { return skill[14]; }
	inline const int& monsterPathBoundaryXStart() const { return skill[14]; } //skill[14]
	inline int& monsterPathBoundaryYStart() { return skill[15]; }
	inline const int& monsterPathBoundaryYStart() const { return skill[15]; } //skill[15]
	inline int& monsterPathBoundaryXEnd() { return skill[16]; }
	inline const int& monsterPathBoundaryXEnd() const { return skill[16]; } //skill[16]
	inline int& monsterPathBoundaryYEnd() { return skill[17]; }
	inline const int& monsterPathBoundaryYEnd() const { return skill[17]; } //skill[17]
	inline int& monsterStoreType() { return skill[18]; }
	inline const int& monsterStoreType() const { return skill[18]; } //skill[18]
	inline int& monsterDevilNumSummons() { return skill[18]; }
	inline const int& monsterDevilNumSummons() const { return skill[18]; } //skill[18]
	inline int& monsterStrafeDirection() { return skill[39]; }
	inline const int& monsterStrafeDirection() const { return skill[39]; } //skill[39]
	inline int& monsterPathCount() { return skill[38]; }
	inline const int& monsterPathCount() const { return skill[38]; } //skill[38]
	inline real_t& monsterLookDir() { return fskill[4]; }
	inline const real_t& monsterLookDir() const { return fskill[4]; } //fskill[4]
	inline int& monsterEntityRenderAsTelepath() { return skill[41]; }
	inline const int& monsterEntityRenderAsTelepath() const { return skill[41]; } //skill[41]
	inline int& monsterAllyIndex() { return skill[42]; }
	inline const int& monsterAllyIndex() const { return skill[42]; } //skill[42] If monster is an ally of a player, assign number 0-3 to it for the players to track on the map.
	inline int& monsterAllyState() { return skill[43]; }
	inline const int& monsterAllyState() const { return skill[43]; } //skill[43]
	inline int& monsterAllyPickupItems() { return skill[44]; }
	inline const int& monsterAllyPickupItems() const { return skill[44]; } //skill[44]
	inline int& monsterAllyInteractTarget() { return skill[45]; }
	inline const int& monsterAllyInteractTarget() const { return skill[45]; } //skill[45]
	inline int& monsterAllyClass() { return skill[46]; }
	inline const int& monsterAllyClass() const { return skill[46]; } //skill[46]
	inline int& monsterDefend() { return skill[47]; }
	inline const int& monsterDefend() const { return skill[47]; } //skill[47]
	inline int& monsterAllySpecial() { return skill[48]; }
	inline const int& monsterAllySpecial() const { return skill[48]; } //skill[48]
	inline int& monsterAllySpecialCooldown() { return skill[49]; }
	inline const int& monsterAllySpecialCooldown() const { return skill[49]; } //skill[49]
	inline int& monsterAllySummonRank() { return skill[50]; }
	inline const int& monsterAllySummonRank() const { return skill[50]; } //skill[50]
	inline real_t& monsterKnockbackVelocity() { return fskill[9]; }
	inline const real_t& monsterKnockbackVelocity() const { return fskill[9]; } //fskill[9]
	inline int& monsterKnockbackUID() { return skill[51]; }
	inline const int& monsterKnockbackUID() const { return skill[51]; } //skill[51]
	inline int& creatureWebbedSlowCount() { return skill[52]; }
	inline const int& creatureWebbedSlowCount() const { return skill[52]; } //skill[52]
	inline int& monsterFearfulOfUid() { return skill[53]; }
	inline const int& monsterFearfulOfUid() const { return skill[53]; } //skill[53]
	inline int& creatureShadowTaggedThisUid() { return skill[54]; }
	inline const int& creatureShadowTaggedThisUid() const { return skill[54]; } //skill[54]
	inline int& monsterIllusionTauntingThisUid() { return skill[55]; }
	inline const int& monsterIllusionTauntingThisUid() const { return skill[55]; } //skill[55]
	inline int& monsterLastDistractedByNoisemaker() { return skill[55]; }
	inline const int& monsterLastDistractedByNoisemaker() const { return skill[55]; }//skill[55] shared with above as above only is for inner demons.
	inline int& monsterExtraReflexTick() { return skill[56]; }
	inline const int& monsterExtraReflexTick() const { return skill[56]; } //skill[56]
	inline real_t& monsterSentrybotLookDir() { return fskill[10]; }
	inline const real_t& monsterSentrybotLookDir() const { return fskill[10]; } //fskill[10]
	inline real_t& monsterKnockbackTangentDir() { return fskill[11]; }
	inline const real_t& monsterKnockbackTangentDir() const { return fskill[11]; } //fskill[11]
	inline real_t& playerStrafeVelocity() { return fskill[12]; }
	inline const real_t& playerStrafeVelocity() const { return fskill[12]; } //fskill[12]
	inline real_t& playerStrafeDir() { return fskill[13]; }
	inline const real_t& playerStrafeDir() const { return fskill[13]; } //fskill[13]
	inline real_t& monsterSpecialAttackUnequipSafeguard() { return fskill[14]; }
	inline const real_t& monsterSpecialAttackUnequipSafeguard() const { return fskill[14]; } //fskill[14]
	inline real_t& creatureWindDir() { return fskill[15]; }
	inline const real_t& creatureWindDir() const { return fskill[15]; } //fskill[15]
	inline real_t& creatureWindVelocity() { return fskill[16]; }
	inline const real_t& creatureWindVelocity() const { return fskill[16]; } //fskill[16]
	inline real_t& creatureHoverZ() { return fskill[17]; }
	inline const real_t& creatureHoverZ() const { return fskill[17]; } //fskill[17]

	//--EFFECTS--
	inline int& effectPolymorph() { return skill[50]; }
	inline const int& effectPolymorph() const { return skill[50]; } // skill[50]
	inline int& effectShapeshift() { return skill[53]; }
	inline const int& effectShapeshift() const { return skill[53]; } // skill[53]

	//--PUBLIC GENERAL ENTITY STUFF--
	inline int& interactedByMonster() { return skill[47]; }
	inline const int& interactedByMonster() const { return skill[47]; } //skill[47] for use with monsterAllyInteractTarget
	inline real_t& highlightForUI() { return fskill[29]; }
	inline const real_t& highlightForUI() const { return fskill[29]; } //fskill[29] for highlighting interactibles
	inline real_t& highlightForUIGlow() { return fskill[28]; }
	inline const real_t& highlightForUIGlow() const { return fskill[28]; } //fskill[28] for highlighting animation
	inline real_t& grayscaleGLRender() { return fskill[27]; }
	inline const real_t& grayscaleGLRender() const { return fskill[27]; } //fskill[27] for grayscale rendering
	inline real_t& noColorChangeAllyLimb() { return fskill[26]; }
	inline const real_t& noColorChangeAllyLimb() const { return fskill[26]; } // fskill[26] for ignoring recolor of follower limbs
	inline real_t& mistformGLRender() { return fskill[22]; }
	inline const real_t& mistformGLRender() const { return fskill[22]; }

	//--PUBLIC PLAYER SKILLS--
	inline int& playerLevelEntrySpeech() { return skill[18]; }
	inline const int& playerLevelEntrySpeech() const { return skill[18]; } //skill[18]
	inline int& playerAliveTime() { return skill[12]; }
	inline const int& playerAliveTime() const { return skill[12]; } //skill[12]
	inline int& playerVampireCurse() { return skill[51]; }
	inline const int& playerVampireCurse() const { return skill[51]; } //skill[51]
	inline int& playerAutomatonDeathCounter() { return skill[15]; }
	inline const int& playerAutomatonDeathCounter() const { return skill[15]; } //skill[15] - 0 if unused, > 0 if counting to death
	inline int& playerCreatedDeathCam() { return skill[16]; }
	inline const int& playerCreatedDeathCam() const { return skill[16]; } //skill[16] - if we triggered actDeathCam already.
	inline int& playerCastTimeAnim() { return skill[17]; }
	inline const int& playerCastTimeAnim() const { return skill[17]; } // how many ticks we're casting for in the current animation

	//--PUBLIC MONSTER ANIMATION SKILLS--
	inline int& monsterAnimationLimbDirection() { return skill[20]; }
	inline const int& monsterAnimationLimbDirection() const { return skill[20]; }  //skill[20]
	inline int& monsterAnimationLimbOvershoot() { return skill[30]; }
	inline const int& monsterAnimationLimbOvershoot() const { return skill[30]; } //skill[30]

	//--PUBLIC MONSTER SHADOW SKILLS--
	inline int& monsterShadowInitialMimic() { return skill[34]; }
	inline const int& monsterShadowInitialMimic() const { return skill[34]; } //skill[34]. 0 = false, 1 = true.
	inline int& monsterShadowDontChangeName() { return skill[35]; }
	inline const int& monsterShadowDontChangeName() const { return skill[35]; } //skill[35]. 0 = false, 1 = true. Doesn't change name in its mimic if = 1.

	//--PUBLIC MONSTER SLIME SKILLS--
	inline int& monsterSlimeLastAttack() { return skill[34]; }
	inline const int& monsterSlimeLastAttack() const { return skill[34]; } // skill[34]

	//--PUBLIC MONSTER LICH SKILLS--
	inline int& monsterLichFireMeleeSeq() { return skill[34]; }
	inline const int& monsterLichFireMeleeSeq() const { return skill[34]; } //skill[34]
	inline int& monsterLichFireMeleePrev() { return skill[35]; }
	inline const int& monsterLichFireMeleePrev() const { return skill[35]; } //skill[35]
	inline int& monsterLichIceCastSeq() { return skill[34]; }
	inline const int& monsterLichIceCastSeq() const { return skill[34]; } //skill[34]
	inline int& monsterLichIceCastPrev() { return skill[35]; }
	inline const int& monsterLichIceCastPrev() const { return skill[35]; } //skill[35]
	inline int& monsterLichMagicCastCount() { return skill[37]; }
	inline const int& monsterLichMagicCastCount() const { return skill[37]; } //skill[37] count the basic spell attacks in the seq and switch things up if too many in a row.
	inline int& monsterLichMeleeSwingCount() { return skill[38]; }
	inline const int& monsterLichMeleeSwingCount() const { return skill[38]; } //skill[38] count the 'regular' attacks in the seq and switch things up if too many in a row.
	inline int& monsterLichBattleState() { return skill[27]; }
	inline const int& monsterLichBattleState() const { return skill[27]; } //skill[27] used to track hp/battle progress
	inline int& monsterLichTeleportTimer() { return skill[40]; }
	inline const int& monsterLichTeleportTimer() const { return skill[40]; } //skill[40] used to track conditions to teleport away.
	inline int& monsterLichAllyStatus() { return skill[18]; }
	inline const int& monsterLichAllyStatus() const { return skill[18]; } //skill[18] used to track if allies are alive.
	inline int& monsterLichAllyUID() { return skill[17]; }
	inline const int& monsterLichAllyUID() const { return skill[17]; } //skill[17] used to track lich ally uid.

	//--PUBLIC POWER CRYSTAL SKILLS--
	inline int& crystalTurnReverse() { return skill[9]; }
	inline const int& crystalTurnReverse() const { return skill[9]; } // skill[9] 0 Clockwise, 1 Anti-Clockwise
	inline int& crystalNumElectricityNodes() { return skill[6]; }
	inline const int& crystalNumElectricityNodes() const { return skill[6]; } // skill[6] how many nodes to spawn in the facing dir
	inline int& crystalSpellToActivate() { return skill[10]; }
	inline const int& crystalSpellToActivate() const { return skill[10]; } // skill[10] If 1, must be hit by unlocking spell to start generating electricity.
	inline real_t& crystalStartZ() { return fskill[0]; }
	inline const real_t& crystalStartZ() const { return fskill[0]; } // fskill[0] mid point of animation, starting height.
	inline real_t& crystalMaxZVelocity() { return fskill[1]; }
	inline const real_t& crystalMaxZVelocity() const { return fskill[1]; } // fskill[1] 
	inline real_t& crystalMinZVelocity() { return fskill[2]; }
	inline const real_t& crystalMinZVelocity() const { return fskill[2]; } // fskill[2] 
	inline real_t& crystalTurnVelocity() { return fskill[3]; }
	inline const real_t& crystalTurnVelocity() const { return fskill[3]; } // fskill[3] how fast to turn on click.

	//--PUBLIC GATE SKILLS--
	inline int& gateInit() { return skill[1]; }
	inline const int& gateInit() const { return skill[1]; } //skill[1]
	inline int& gateStatus() { return skill[3]; }
	inline const int& gateStatus() const { return skill[3]; } //skill[3]
	inline int& gateRattle() { return skill[4]; }
	inline const int& gateRattle() const { return skill[4]; } //skill[4]
	inline real_t& gateStartHeight() { return fskill[0]; }
	inline const real_t& gateStartHeight() const { return fskill[0]; } //fskill[0]
	inline real_t& gateVelZ() { return vel_z; }
	inline const real_t& gateVelZ() const { return vel_z; } //vel_z
	inline int& gateInverted() { return skill[5]; }
	inline const int& gateInverted() const { return skill[5]; } //skill[5]
	inline int& gateDisableOpening() { return skill[6]; }
	inline const int& gateDisableOpening() const { return skill[6]; } //skill[6]

	//--PUBLIC LEVER SKILLS--
	inline int& leverTimerTicks() { return skill[3]; }
	inline const int& leverTimerTicks() const { return skill[3]; }//skill[1]
	inline int& leverStatus() { return skill[1]; }
	inline const int& leverStatus() const { return skill[1]; }//skill[3]

	//--PUBLIC BOULDER TRAP SKILLS--
	inline int& boulderTrapRefireAmount() { return skill[1]; }
	inline const int& boulderTrapRefireAmount() const { return skill[1]; } //skill[1]
	inline int& boulderTrapRefireDelay() { return skill[3]; }
	inline const int& boulderTrapRefireDelay() const { return skill[3]; } //skill[3]
	inline int& boulderTrapAmbience() { return skill[6]; }
	inline const int& boulderTrapAmbience() const { return skill[6]; } //skill[6]
	inline int& boulderTrapFired() { return skill[0]; }
	inline const int& boulderTrapFired() const { return skill[0]; } //skill[0]
	inline int& boulderTrapRefireCounter() { return skill[4]; }
	inline const int& boulderTrapRefireCounter() const { return skill[4]; } //skill[4]
	inline int& boulderTrapPreDelay() { return skill[5]; }
	inline const int& boulderTrapPreDelay() const { return skill[5]; } //skill[5]
	inline int& boulderTrapRocksToSpawn() { return skill[7]; }
	inline const int& boulderTrapRocksToSpawn() const { return skill[7]; } //skill[7] bitwise storage. 
	inline int& boulderShatterEarthSpell() { return skill[16]; }
	inline const int& boulderShatterEarthSpell() const { return skill[16]; }
	inline int& boulderShatterEarthDamage() { return skill[17]; }
	inline const int& boulderShatterEarthDamage() const { return skill[17]; }

	//--PUBLIC AMBIENT PARTICLE EFFECT SKILLS--
	inline int& particleDuration() { return skill[0]; }
	inline const int& particleDuration() const { return skill[0]; } //skill[0]
	inline int& particleShrink() { return skill[1]; }
	inline const int& particleShrink() const { return skill[1]; } //skill[1]

	//--PUBLIC PARTICLE TIMER EFFECT SKILLS--
	inline int& particleTimerDuration() { return skill[0]; }
	inline const int& particleTimerDuration() const { return skill[0]; } //skill[0]
	inline int& particleTimerEndAction() { return skill[1]; }
	inline const int& particleTimerEndAction() const { return skill[1]; } //skill[1]
	inline int& particleTimerEndSprite() { return skill[3]; }
	inline const int& particleTimerEndSprite() const { return skill[3]; } //skill[3]
	inline int& particleTimerCountdownAction() { return skill[4]; }
	inline const int& particleTimerCountdownAction() const { return skill[4]; } //skill[4]
	inline int& particleTimerCountdownSprite() { return skill[5]; }
	inline const int& particleTimerCountdownSprite() const { return skill[5]; } //skill[5]
	inline int& particleTimerTarget() { return skill[6]; }
	inline const int& particleTimerTarget() const { return skill[6]; } //skill[6]
	inline int& particleTimerPreDelay() { return skill[7]; }
	inline const int& particleTimerPreDelay() const { return skill[7]; } //skill[7]
	inline int& particleTimerVariable1() { return skill[8]; }
	inline const int& particleTimerVariable1() const { return skill[8]; } //skill[8]
	inline int& particleTimerVariable2() { return skill[9]; }
	inline const int& particleTimerVariable2() const { return skill[9]; } //skill[9]
	inline int& particleTimerEffectLifetime() { return skill[10]; }
	inline const int& particleTimerEffectLifetime() const { return skill[10]; }
	inline int& particleTimerVariable3() { return skill[11]; }
	inline const int& particleTimerVariable3() const { return skill[11]; }
	inline int& particleTimerVariable4() { return skill[12]; }
	inline const int& particleTimerVariable4() const { return skill[12]; }

	//--PUBLIC DOOR SKILLS--
	inline int& doorDir() { return skill[0]; }
	inline const int& doorDir() const { return skill[0]; } //skill[0]
	inline int& doorInit() { return skill[1]; }
	inline const int& doorInit() const { return skill[1]; } //skill[1]
	inline int& doorStatus() { return skill[3]; }
	inline const int& doorStatus() const { return skill[3]; } //skill[3]
	inline int& doorHealth() { return skill[4]; }
	inline const int& doorHealth() const { return skill[4]; } //skill[4]
	inline int& doorLocked() { return skill[5]; }
	inline const int& doorLocked() const { return skill[5]; } //skill[5]
	inline int& doorSmacked() { return skill[6]; }
	inline const int& doorSmacked() const { return skill[6]; } //skill[6]
	inline int& doorTimer() { return skill[7]; }
	inline const int& doorTimer() const { return skill[7]; } //skill[7]
	inline int& doorOldStatus() { return skill[8]; }
	inline const int& doorOldStatus() const { return skill[8]; } //skill[8]
	inline int& doorMaxHealth() { return skill[9]; }
	inline const int& doorMaxHealth() const { return skill[9]; } //skill[9]
	inline real_t& doorStartAng() { return fskill[0]; }
	inline const real_t& doorStartAng() const { return fskill[0]; } //fskill[0]
	inline int& doorPreventLockpickExploit() { return skill[10]; }
	inline const int& doorPreventLockpickExploit() const { return skill[10]; } //skill[10]
	inline int& doorForceLockedUnlocked() { return skill[11]; }
	inline const int& doorForceLockedUnlocked() const { return skill[11]; } //skill[11]
	inline int& doorDisableLockpicks() { return skill[12]; }
	inline const int& doorDisableLockpicks() const { return skill[12]; } //skill[12]
	inline int& doorDisableOpening() { return skill[13]; }
	inline const int& doorDisableOpening() const { return skill[13]; } //skill[13]
	inline int& doorLockpickHealth() { return skill[14]; }
	inline const int& doorLockpickHealth() const { return skill[14]; } //skill[14]
	inline int& doorOldHealth() { return skill[15]; }
	inline const int& doorOldHealth() const { return skill[15]; } //skill[15]
	inline int& doorUnlockWhenPowered() { return skill[16]; }
	inline const int& doorUnlockWhenPowered() const { return skill[16]; } //skill[16]

	//--PUBLIC PEDESTAL SKILLS--
	inline int& pedestalHasOrb() { return skill[0]; }
	inline const int& pedestalHasOrb() const { return skill[0]; } //skill[0]
	inline int& pedestalOrbType() { return skill[1]; }
	inline const int& pedestalOrbType() const { return skill[1]; }  //skill[1]
	inline int& pedestalInvertedPower() { return skill[3]; }
	inline const int& pedestalInvertedPower() const { return skill[3]; } //skill[3]
	inline int& pedestalInGround() { return skill[4]; }
	inline const int& pedestalInGround() const { return skill[4]; } //skill[4]
	inline int& pedestalInit() { return skill[5]; }
	inline const int& pedestalInit() const { return skill[5]; } //skill[5]
	inline int& pedestalAmbience() { return skill[6]; }
	inline const int& pedestalAmbience() const { return skill[6]; } //skill[6]
	inline int& pedestalLockOrb() { return skill[7]; }
	inline const int& pedestalLockOrb() const { return skill[7]; } //skill[7]
	inline int& pedestalPowerStatus() { return skill[8]; }
	inline const int& pedestalPowerStatus() const { return skill[8]; } //skill[8]
	inline real_t& orbStartZ() { return fskill[0]; }
	inline const real_t& orbStartZ() const { return fskill[0]; } // fskill[0] mid point of animation, starting height.
	inline real_t& orbMaxZVelocity() { return fskill[1]; }
	inline const real_t& orbMaxZVelocity() const { return fskill[1]; } //fskill[1]
	inline real_t& orbMinZVelocity() { return fskill[2]; }
	inline const real_t& orbMinZVelocity() const { return fskill[2]; } //fskill[2]
	inline real_t& orbTurnVelocity() { return fskill[3]; }
	inline const real_t& orbTurnVelocity() const { return fskill[3]; } //fskill[3] how fast to turn.

	//--PUBLIC PORTAL SKILLS--
	inline int& portalAmbience() { return skill[0]; }
	inline const int& portalAmbience() const { return skill[0]; } //skill[0]
	inline int& portalInit() { return skill[1]; }
	inline const int& portalInit() const { return skill[1]; } //skill[1]
	inline int& portalNotSecret() { return skill[3]; }
	inline const int& portalNotSecret() const { return skill[3]; } //skill[3]
	inline int& portalVictoryType() { return skill[4]; }
	inline const int& portalVictoryType() const { return skill[4]; } //skill[4]
	inline int& portalFireAnimation() { return skill[5]; }
	inline const int& portalFireAnimation() const { return skill[5]; } //skill[5]
	inline int& portalCustomLevelsToJump() { return skill[6]; }
	inline const int& portalCustomLevelsToJump() const { return skill[6]; } //skill[6]
	inline int& portalCustomRequiresPower() { return skill[7]; }
	inline const int& portalCustomRequiresPower() const { return skill[7]; } //skill[7]
	inline int& portalCustomSprite() { return skill[8]; }
	inline const int& portalCustomSprite() const { return skill[8]; } //skill[8]
	inline int& portalCustomSpriteAnimationFrames() { return skill[9]; }
	inline const int& portalCustomSpriteAnimationFrames() const { return skill[9]; } //skill[9]
	inline int& portalCustomZOffset() { return skill[10]; }
	inline const int& portalCustomZOffset() const { return skill[10]; } //skill[10]
	inline int& portalCustomLevelText1() { return skill[11]; }
	inline const int& portalCustomLevelText1() const { return skill[11]; } //skill[11]
	inline int& portalCustomLevelText2() { return skill[12]; }
	inline const int& portalCustomLevelText2() const { return skill[12]; } //skill[12]
	inline int& portalCustomLevelText3() { return skill[13]; }
	inline const int& portalCustomLevelText3() const { return skill[13]; } //skill[13]
	inline int& portalCustomLevelText4() { return skill[14]; }
	inline const int& portalCustomLevelText4() const { return skill[14]; } //skill[14]
	inline int& portalCustomLevelText5() { return skill[15]; }
	inline const int& portalCustomLevelText5() const { return skill[15]; } //skill[15]
	inline int& portalCustomLevelText6() { return skill[16]; }
	inline const int& portalCustomLevelText6() const { return skill[16]; } //skill[16]
	inline int& portalCustomLevelText7() { return skill[17]; }
	inline const int& portalCustomLevelText7() const { return skill[17]; } //skill[17]
	inline int& portalCustomLevelText8() { return skill[18]; }
	inline const int& portalCustomLevelText8() const { return skill[18]; } //skill[18]

	//--PUBLIC TELEPORTER SKILLS--
	inline int& teleporterX() { return skill[0]; }
	inline const int& teleporterX() const { return skill[0]; } //skill[0]
	inline int& teleporterY() { return skill[1]; }
	inline const int& teleporterY() const { return skill[1]; } //skill[1]
	inline int& teleporterType() { return skill[3]; }
	inline const int& teleporterType() const { return skill[3]; } //skill[3]
	inline int& teleporterAmbience() { return skill[4]; }
	inline const int& teleporterAmbience() const { return skill[4]; } //skill[4]
	inline int& teleporterStartFrame() { return skill[5]; }
	inline const int& teleporterStartFrame() const { return skill[5]; }
	inline int& teleporterCurrentFrame() { return skill[6]; }
	inline const int& teleporterCurrentFrame() const { return skill[6]; }
	inline int& teleporterNumFrames() { return skill[7]; }
	inline const int& teleporterNumFrames() const { return skill[7]; }
	inline int& teleporterDuration() { return skill[8]; }
	inline const int& teleporterDuration() const { return skill[8]; }

	//--PUBLIC CEILING TILE SKILLS--
	inline int& ceilingTileModel() { return skill[0]; }
	inline const int& ceilingTileModel() const { return skill[0]; } //skill[0]
	inline int& ceilingTileDir() { return skill[1]; }
	inline const int& ceilingTileDir() const { return skill[1]; } //skill[1]
	inline int& ceilingTileAllowTrap() { return skill[3]; }
	inline const int& ceilingTileAllowTrap() const { return skill[3]; } //skill[3]
	inline int& ceilingTileBreakable() { return skill[4]; }
	inline const int& ceilingTileBreakable() const { return skill[4]; } //skill[4]

	//--PUBLIC FLOOR DECORATION MODELS--
	inline int& floorDecorationModel() { return skill[0]; }
	inline const int& floorDecorationModel() const { return skill[0]; } //skill[0]
	inline int& floorDecorationRotation() { return skill[1]; }
	inline const int& floorDecorationRotation() const { return skill[1]; } //skill[1]
	inline int& floorDecorationHeightOffset() { return skill[3]; }
	inline const int& floorDecorationHeightOffset() const { return skill[3]; } //skill[3] positive numbers will lift the model higher
	inline int& floorDecorationXOffset() { return skill[4]; }
	inline const int& floorDecorationXOffset() const { return skill[4]; } //skill[4]
	inline int& floorDecorationYOffset() { return skill[5]; }
	inline const int& floorDecorationYOffset() const { return skill[5]; } //skill[5]
	inline int& floorDecorationDestroyIfNoWall() { return skill[6]; }
	inline const int& floorDecorationDestroyIfNoWall() const { return skill[6]; } //skill[6]
	inline int& floorDecorationDialogueProgress() { return skill[7]; }
	inline const int& floorDecorationDialogueProgress() const { return skill[7]; } // for players interacting with a dialogue bubble progress on clicking, unused
	inline int& floorDecorationInteractText1() { return skill[8]; }
	inline const int& floorDecorationInteractText1() const { return skill[8]; } //skill[8]
	inline int& floorDecorationInteractText2() { return skill[9]; }
	inline const int& floorDecorationInteractText2() const { return skill[9]; } //skill[9]
	inline int& floorDecorationInteractText3() { return skill[10]; }
	inline const int& floorDecorationInteractText3() const { return skill[10]; } //skill[10]
	inline int& floorDecorationInteractText4() { return skill[11]; }
	inline const int& floorDecorationInteractText4() const { return skill[11]; } //skill[11]
	inline int& floorDecorationInteractText5() { return skill[12]; }
	inline const int& floorDecorationInteractText5() const { return skill[12]; } //skill[12]
	inline int& floorDecorationInteractText6() { return skill[13]; }
	inline const int& floorDecorationInteractText6() const { return skill[13]; } //skill[13]
	inline int& floorDecorationInteractText7() { return skill[14]; }
	inline const int& floorDecorationInteractText7() const { return skill[14]; } //skill[14]
	inline int& floorDecorationInteractText8() { return skill[15]; }
	inline const int& floorDecorationInteractText8() const { return skill[15]; } //skill[15]

	//--PUBLIC COLLISION DECORATION MODELS--
	inline int& colliderDecorationModel() { return skill[0]; }
	inline const int& colliderDecorationModel() const { return skill[0]; } //skill[0]
	inline int& colliderDecorationRotation() { return skill[1]; }
	inline const int& colliderDecorationRotation() const { return skill[1]; } //skill[1]
	inline int& colliderDecorationHeightOffset() { return skill[3]; }
	inline const int& colliderDecorationHeightOffset() const { return skill[3]; } //skill[3] positive numbers will lift the model higher
	inline int& colliderDecorationXOffset() { return skill[4]; }
	inline const int& colliderDecorationXOffset() const { return skill[4]; } //skill[4]
	inline int& colliderDecorationYOffset() { return skill[5]; }
	inline const int& colliderDecorationYOffset() const { return skill[5]; } //skill[5]
	inline int& colliderHasCollision() { return skill[6]; }
	inline const int& colliderHasCollision() const { return skill[6]; } //skill[6]
	inline int& colliderSizeX() { return skill[7]; }
	inline const int& colliderSizeX() const { return skill[7]; } //skill[7]
	inline int& colliderSizeY() { return skill[8]; }
	inline const int& colliderSizeY() const { return skill[8]; } //skill[8]
	inline int& colliderMaxHP() { return skill[9]; }
	inline const int& colliderMaxHP() const { return skill[9]; } //skill[9]
	inline int& colliderDiggable() { return skill[10]; }
	inline const int& colliderDiggable() const { return skill[10]; } //skill[10]
	inline int& colliderDamageTypes() { return skill[11]; }
	inline const int& colliderDamageTypes() const { return skill[11]; } //skill[11]
	inline int& colliderCurrentHP() { return skill[12]; }
	inline const int& colliderCurrentHP() const { return skill[12]; } //skill[12]
	inline int& colliderOldHP() { return skill[13]; }
	inline const int& colliderOldHP() const { return skill[13]; } //skill[13]
	inline int& colliderInit() { return skill[14]; }
	inline const int& colliderInit() const { return skill[14]; } //skill[14]
	inline int& colliderContainedEntity() { return skill[15]; }
	inline const int& colliderContainedEntity() const { return skill[15]; } //skill[15]
	inline int& colliderHideMonster() { return skill[16]; }
	inline const int& colliderHideMonster() const { return skill[16]; } //skill[16]
	inline int& colliderKillerUid() { return skill[17]; }
	inline const int& colliderKillerUid() const { return skill[17]; } //skill[17]
	inline int& colliderSpellEvent() { return skill[18]; }
	inline const int& colliderSpellEvent() const { return skill[18]; }
	inline int& colliderSpellEventCooldown() { return skill[19]; }
	inline const int& colliderSpellEventCooldown() const { return skill[19]; }
	inline int& colliderCreatedParent() { return skill[20]; }
	inline const int& colliderCreatedParent() const { return skill[20]; }
	inline int& colliderSpellEventTrigger() { return skill[21]; }
	inline const int& colliderSpellEventTrigger() const { return skill[21]; }
	inline int& colliderIsMapGenerated() { return skill[22]; }
	inline const int& colliderIsMapGenerated() const { return skill[22]; }
	inline int& colliderSpellTarget() { return skill[23]; }
	inline const int& colliderSpellTarget() const { return skill[23]; }
	inline int& colliderTelepathy() { return skill[24]; }
	inline const int& colliderTelepathy() const { return skill[24]; }
	inline int& colliderDropVariable() { return skill[25]; }
	inline const int& colliderDropVariable() const { return skill[25]; } // store germinate drop qtys
	static void colliderAssignProperties(Entity* entity, bool mapGeneration, map_t* whichMap);
	static Entity* createBreakableCollider(int colliderDamageType, real_t _x, real_t _y, Entity* parent);
	void colliderSetServerSkillOnSpawned();

	//--PUBLIC SPELL TRAP SKILLS--
	inline int& spellTrapType() { return skill[0]; }
	inline const int& spellTrapType() const { return skill[0]; } //skill[0]
	inline int& spellTrapRefire() { return skill[1]; }
	inline const int& spellTrapRefire() const { return skill[1]; } //skill[1]
	inline int& spellTrapLatchPower() { return skill[3]; }
	inline const int& spellTrapLatchPower() const { return skill[3]; } //skill[3]
	inline int& spellTrapFloorTile() { return skill[4]; }
	inline const int& spellTrapFloorTile() const { return skill[4]; } //skill[4]
	inline int& spellTrapRefireRate() { return skill[5]; }
	inline const int& spellTrapRefireRate() const { return skill[5]; } //skill[5]
	inline int& spellTrapAmbience() { return skill[6]; }
	inline const int& spellTrapAmbience() const { return skill[6]; } //skill[6]
	inline int& spellTrapInit() { return skill[7]; }
	inline const int& spellTrapInit() const { return skill[7]; } //skill[7]
	inline int& spellTrapCounter() { return skill[8]; }
	inline const int& spellTrapCounter() const { return skill[8]; } //skill[8]
	inline int& spellTrapReset() { return skill[9]; }
	inline const int& spellTrapReset() const { return skill[9]; } //skill[9]

	//--PUBLIC SPELL SHRINE SKILLS--
	inline int& shrineSpellEffect() { return skill[0]; }
	inline const int& shrineSpellEffect() const { return skill[0]; } //skill[0]
	inline int& shrineRefire1() { return skill[1]; }
	inline const int& shrineRefire1() const { return skill[1]; } //skill[1]
	inline int& shrineRefire2() { return skill[3]; }
	inline const int& shrineRefire2() const { return skill[3]; } //skill[3]
	inline int& shrineDir() { return skill[4]; }
	inline const int& shrineDir() const { return skill[4]; } //skill[4]
	inline int& shrineAmbience() { return skill[5]; }
	inline const int& shrineAmbience() const { return skill[5]; } //skill[5]
	inline int& shrineInit() { return skill[6]; }
	inline const int& shrineInit() const { return skill[6]; } //skill[6]
	inline int& shrineActivateDelay() { return skill[7]; }
	inline const int& shrineActivateDelay() const { return skill[7]; } //skill[7]
	inline int& shrineZ() { return skill[8]; }
	inline const int& shrineZ() const { return skill[8]; } //skill[8]
	inline int& shrineDestXOffset() { return skill[9]; }
	inline const int& shrineDestXOffset() const { return skill[9]; } //skill[9]
	inline int& shrineDestYOffset() { return skill[10]; }
	inline const int& shrineDestYOffset() const { return skill[10]; } //skill[10]
	inline int& shrineDaedalusState() { return skill[11]; }
	inline const int& shrineDaedalusState() const { return skill[11]; } // skill[11]
	
	//--PUBLIC FURNITURE SKILLS--
	inline int& furnitureType() { return skill[0]; }
	inline const int& furnitureType() const { return skill[0]; } //skill[0]
	inline int& furnitureInit() { return skill[1]; }
	inline const int& furnitureInit() const { return skill[1]; } //skill[1]
	inline int& furnitureDir() { return skill[3]; }
	inline const int& furnitureDir() const { return skill[3]; } //skill[3]
	inline int& furnitureHealth() { return skill[4]; }
	inline const int& furnitureHealth() const { return skill[4]; } //skill[4]
	inline int& furnitureMaxHealth() { return skill[9]; }
	inline const int& furnitureMaxHealth() const { return skill[9]; } //skill[9]
	inline int& furnitureTableRandomItemChance() { return skill[10]; }
	inline const int& furnitureTableRandomItemChance() const { return skill[10]; } //skill[10]
	inline int& furnitureTableSpawnChairs() { return skill[11]; }
	inline const int& furnitureTableSpawnChairs() const { return skill[11]; } //skill[11]
	inline int& furnitureOldHealth() { return skill[15]; }
	inline const int& furnitureOldHealth() const { return skill[15]; } //skill[15]

	//--PUBLIC PISTON SKILLS--
	inline int& pistonCamDir() { return skill[0]; }
	inline const int& pistonCamDir() const { return skill[0]; } //skill[0]
	inline int& pistonCamTimer() { return skill[1]; }
	inline const int& pistonCamTimer() const { return skill[1]; } //skill[1]
	inline real_t& pistonCamRotateSpeed() { return fskill[0]; }
	inline const real_t& pistonCamRotateSpeed() const { return fskill[0]; } //fskill[0]

	//--PUBLIC ARROW/PROJECTILE SKILLS--
	inline int& arrowPower() { return skill[3]; }
	inline const int& arrowPower() const { return skill[3]; } //skill[3]
	inline int& arrowPoisonTime() { return skill[4]; }
	inline const int& arrowPoisonTime() const { return skill[4]; } //skill[4]
	inline int& arrowArmorPierce() { return skill[5]; }
	inline const int& arrowArmorPierce() const { return skill[5]; } //skill[5]
	inline real_t& arrowSpeed() { return fskill[4]; }
	inline const real_t& arrowSpeed() const { return fskill[4]; } //fskill[4]
	inline real_t& arrowFallSpeed() { return fskill[5]; }
	inline const real_t& arrowFallSpeed() const { return fskill[5]; } //fskill[5]
	inline int& arrowBoltDropOffRange() { return skill[6]; }
	inline const int& arrowBoltDropOffRange() const { return skill[6]; } //skill[6]
	inline int& arrowShotByWeapon() { return skill[7]; }
	inline const int& arrowShotByWeapon() const { return skill[7]; } //skill[7]
	inline int& arrowQuiverType() { return skill[8]; }
	inline const int& arrowQuiverType() const { return skill[8]; } //skill[8]
	inline int& arrowShotByParent() { return skill[9]; }
	inline const int& arrowShotByParent() const { return skill[9]; } //skill[9]
	inline int& arrowDropOffEquipmentModifier() { return skill[14]; }
	inline const int& arrowDropOffEquipmentModifier() const { return skill[14]; } //skill[14]
	enum arrowShotBy : int
	{
		ARROW_SHOT_BY_TRAP,
		ARROW_SHOT_BY_PLAYER,
		ARROW_SHOT_BY_MONSTER
	};

	//--PUBLIC ITEM SKILLS--
	inline int& itemNotMoving() { return skill[18]; }
	inline const int& itemNotMoving() const { return skill[18]; } // skill[18]
	inline int& itemNotMovingClient() { return skill[19]; }
	inline const int& itemNotMovingClient() const { return skill[19]; } // skill[19]
	inline int& itemSokobanReward() { return skill[20]; }
	inline const int& itemSokobanReward() const { return skill[20]; } // skill[20]
	inline int& itemOriginalOwner() { return skill[21]; }
	inline const int& itemOriginalOwner() const { return skill[21]; } // skill[21]
	inline int& itemStolen() { return skill[22]; }
	inline const int& itemStolen() const { return skill[22]; } // skill[22]
	inline int& itemShowOnMap() { return skill[23]; }
	inline const int& itemShowOnMap() const { return skill[23]; } //skill[23]
	inline int& itemDelayMonsterPickingUp() { return skill[24]; }
	inline const int& itemDelayMonsterPickingUp() const { return skill[24]; } //skill[24]
	inline int& itemReceivedDetailsFromServer() { return skill[25]; }
	inline const int& itemReceivedDetailsFromServer() const { return skill[25]; } //skill[25]
	inline int& itemAutoSalvageByPlayer() { return skill[26]; }
	inline const int& itemAutoSalvageByPlayer() const { return skill[26]; } //skill[26]
	inline int& itemSplooshed() { return skill[27]; }
	inline const int& itemSplooshed() const { return skill[27]; } //skill[27]
	inline int& itemContainer() { return skill[29]; }
	inline const int& itemContainer() const { return skill[29]; } //skill[29]
	inline int& itemFollowUID() { return skill[30]; }
	inline const int& itemFollowUID() const { return skill[30]; }
	inline int& itemReturnUID() { return skill[31]; }
	inline const int& itemReturnUID() const { return skill[31]; }
	inline int& itemGerminateResult() { return skill[32]; }
	inline const int& itemGerminateResult() const { return skill[32]; }
	inline real_t& itemWaterBob() { return fskill[2]; }
	inline const real_t& itemWaterBob() const { return fskill[2]; } //fskill[2]
	inline real_t& itemLevitate() { return fskill[3]; }
	inline const real_t& itemLevitate() const { return fskill[3]; }
	inline real_t& itemLevitateStartZ() { return fskill[4]; }
	inline const real_t& itemLevitateStartZ() const { return fskill[4]; }

	//--PUBLIC ACTMAGIC SKILLS (Standard projectiles)--
	inline int& actmagicIsVertical() { return skill[6]; }
	inline const int& actmagicIsVertical() const { return skill[6]; } //skill[6]
	inline int& actmagicIsOrbiting() { return skill[7]; }
	inline const int& actmagicIsOrbiting() const { return skill[7]; } //skill[7]
	inline int& actmagicOrbitDist() { return skill[8]; }
	inline const int& actmagicOrbitDist() const { return skill[8]; } //skill[8]
	inline int& actmagicOrbitVerticalDirection() { return skill[9]; }
	inline const int& actmagicOrbitVerticalDirection() const { return skill[9]; } //skill[9]
	inline int& actmagicOrbitLifetime() { return skill[10]; }
	inline const int& actmagicOrbitLifetime() const { return skill[10]; } //skill[10]
	inline int& actmagicMirrorReflected() { return skill[24]; }
	inline const int& actmagicMirrorReflected() const { return skill[24]; } //skill[24] -- skill[11] IS LIGHTBALL_FLICKER!!
	inline int& actmagicMirrorReflectedCaster() { return skill[12]; }
	inline const int& actmagicMirrorReflectedCaster() const { return skill[12]; } //skill[12]
	inline int& actmagicCastByMagicstaff() { return skill[13]; }
	inline const int& actmagicCastByMagicstaff() const { return skill[13]; } //skill[13]
	inline int& actmagicSpellbookBonus() { return skill[21]; }
	inline const int& actmagicSpellbookBonus() const { return skill[21]; } //skill[21]
	inline real_t& actmagicOrbitVerticalSpeed() { return fskill[2]; }
	inline const real_t& actmagicOrbitVerticalSpeed() const { return fskill[2]; } //fskill[2]
	inline real_t& actmagicOrbitStartZ() { return fskill[3]; }
	inline const real_t& actmagicOrbitStartZ() const { return fskill[3]; } //fskill[3]
	inline real_t& actmagicOrbitStationaryX() { return fskill[4]; }
	inline const real_t& actmagicOrbitStationaryX() const { return fskill[4]; } // fskill[4]
	inline real_t& actmagicOrbitStationaryY() { return fskill[5]; }
	inline const real_t& actmagicOrbitStationaryY() const { return fskill[5]; } // fskill[5]
	inline real_t& actmagicOrbitStationaryCurrentDist() { return fskill[6]; }
	inline const real_t& actmagicOrbitStationaryCurrentDist() const { return fskill[6]; } // fskill[6]
	inline real_t& actmagicSprayGravity() { return fskill[7]; }
	inline const real_t& actmagicSprayGravity() const { return fskill[7]; } // fskill[7]
	inline real_t& actmagicVelXStore() { return fskill[8]; }
	inline const real_t& actmagicVelXStore() const { return fskill[8]; } // fskill[8]
	inline real_t& actmagicVelYStore() { return fskill[9]; }
	inline const real_t& actmagicVelYStore() const { return fskill[9]; } // fskill[9]
	inline real_t& actmagicVelZStore() { return fskill[10]; }
	inline const real_t& actmagicVelZStore() const { return fskill[10]; } // fskill[10]
	inline int& actmagicOrbitStationaryHitTarget() { return skill[14]; }
	inline const int& actmagicOrbitStationaryHitTarget() const { return skill[14]; } // skill[14]
	inline int& actmagicOrbitHitTargetUID1() { return skill[15]; }
	inline const int& actmagicOrbitHitTargetUID1() const { return skill[15]; } // skill[15]
	inline int& actmagicOrbitHitTargetUID2() { return skill[16]; }
	inline const int& actmagicOrbitHitTargetUID2() const { return skill[16]; } // skill[16]
	inline int& actmagicOrbitHitTargetUID3() { return skill[17]; }
	inline const int& actmagicOrbitHitTargetUID3() const { return skill[17]; } // skill[17]
	inline int& actmagicOrbitHitTargetUID4() { return skill[18]; }
	inline const int& actmagicOrbitHitTargetUID4() const { return skill[18]; } // skill[18]
	inline int& actmagicProjectileArc() { return skill[19]; }
	inline const int& actmagicProjectileArc() const { return skill[19]; } // skill[19]
	inline int& actmagicOrbitCastFromSpell() { return skill[20]; }
	inline const int& actmagicOrbitCastFromSpell() const { return skill[20]; } // skill[20]
	inline int& actmagicCastByTinkerTrap() { return skill[22]; }
	inline const int& actmagicCastByTinkerTrap() const { return skill[22]; } // skill[22]
	inline int& actmagicTinkerTrapFriendlyFire() { return skill[23]; }
	inline const int& actmagicTinkerTrapFriendlyFire() const { return skill[23]; } // skill[23]
	inline int& actmagicReflectionCount() { return skill[25]; }
	inline const int& actmagicReflectionCount() const { return skill[25]; } // skill[25]
	inline int& actmagicFromSpellbook() { return skill[26]; }
	inline const int& actmagicFromSpellbook() const { return skill[26]; } // skill[26]
	inline int& actmagicSpray() { return skill[27]; }
	inline const int& actmagicSpray() const { return skill[27]; } // skill[27]
	inline int& actmagicEmitter() { return skill[29]; }
	inline const int& actmagicEmitter() const { return skill[29]; } // skill[29]
	inline int& actmagicDelayMove() { return skill[30]; }
	inline const int& actmagicDelayMove() const { return skill[30]; } // skill[30]
	inline int& actmagicNoHitMessage() { return skill[31]; }
	inline const int& actmagicNoHitMessage() const { return skill[31]; } // skill[31]
	inline int& actmagicNoParticle() { return skill[32]; }
	inline const int& actmagicNoParticle() const { return skill[32]; } // skill[32]
	inline int& actmagicNoLight() { return skill[33]; }
	inline const int& actmagicNoLight() const { return skill[33]; } // skill[33]
	inline int& actmagicUpdateOLDHPOnHit() { return skill[34]; }
	inline const int& actmagicUpdateOLDHPOnHit() const { return skill[34]; }
	inline int& actmagicAllowFriendlyFireHit() { return skill[35]; }
	inline const int& actmagicAllowFriendlyFireHit() const { return skill[35]; }
	inline int& actmagicAdditionalDamage() { return skill[38]; }
	inline const int& actmagicAdditionalDamage() const { return skill[38]; } // extra damage bonus from external sources like windgate
	inline int& actfloorMagicType() { return skill[3]; }
	inline const int& actfloorMagicType() const { return skill[3]; }
	inline int& actfloorMagicClientReceived() { return skill[4]; }
	inline const int& actfloorMagicClientReceived() const { return skill[4]; }
	inline int& actRadiusMagicID() { return skill[1]; }
	inline const int& actRadiusMagicID() const { return skill[1]; }
	inline int& actRadiusMagicInit() { return skill[3]; }
	inline const int& actRadiusMagicInit() const { return skill[3]; }
	inline int& actRadiusMagicDist() { return skill[4]; }
	inline const int& actRadiusMagicDist() const { return skill[4]; }
	inline int& actRadiusMagicFollowUID() { return skill[5]; }
	inline const int& actRadiusMagicFollowUID() const { return skill[5]; }
	inline int& actRadiusMagicDoPulseTick() { return skill[6]; }
	inline const int& actRadiusMagicDoPulseTick() const { return skill[6]; }
	inline int& actRadiusMagicAutoPulseTick() { return skill[7]; }
	inline const int& actRadiusMagicAutoPulseTick() const { return skill[7]; }
	inline int& actRadiusMagicEffectPower() { return skill[8]; }
	inline const int& actRadiusMagicEffectPower() const { return skill[8]; }
	inline int& actParticleWaveStartFrame() { return skill[4]; }
	inline const int& actParticleWaveStartFrame() const { return skill[4]; }
	inline int& actParticleWaveLight() { return skill[7]; }
	inline const int& actParticleWaveLight() const { return skill[7]; }
	inline int& actParticleWaveMagicType() { return skill[9]; }
	inline const int& actParticleWaveMagicType() const { return skill[9]; }
	inline int& actParticleWaveClientReceived() { return skill[10]; }
	inline const int& actParticleWaveClientReceived() const { return skill[10]; }
	inline int& actParticleWaveVariable1() { return skill[11]; }
	inline const int& actParticleWaveVariable1() const { return skill[11]; }
	
	//--PUBLIC GOLD SKILLS--
	inline int& goldAmount() { return skill[0]; }
	inline const int& goldAmount() const { return skill[0]; } //skill[0]
	inline int& goldAmbience() { return skill[1]; }
	inline const int& goldAmbience() const { return skill[1]; } //skill[1]
	inline int& goldSokoban() { return skill[2]; }
	inline const int& goldSokoban() const { return skill[2]; } //skill[2]
	inline int& goldBouncing() { return skill[3]; }
	inline const int& goldBouncing() const { return skill[3]; } //skill[3]
	inline int& goldInContainer() { return skill[4]; }
	inline const int& goldInContainer() const { return skill[4]; } //skill[4]
	inline int& goldTelepathy() { return skill[5]; }
	inline const int& goldTelepathy() const { return skill[5]; }
	inline int& goldAmountBonus() { return skill[6]; }
	inline const int& goldAmountBonus() const { return skill[6]; }
	inline int& goldDroppedByPlayer() { return skill[7]; }
	inline const int& goldDroppedByPlayer() const { return skill[7]; }

	//--PUBLIC SOUND SOURCE SKILLS--
	inline int& soundSourceFired() { return skill[0]; }
	inline const int& soundSourceFired() const { return skill[0]; } //skill[0]
	inline int& soundSourceToPlay() { return skill[1]; }
	inline const int& soundSourceToPlay() const { return skill[1]; } //skill[1]
	inline int& soundSourceVolume() { return skill[2]; }
	inline const int& soundSourceVolume() const { return skill[2]; } //skill[2]
	inline int& soundSourceLatchOn() { return skill[3]; }
	inline const int& soundSourceLatchOn() const { return skill[3]; } //skill[3]
	inline int& soundSourceDelay() { return skill[4]; }
	inline const int& soundSourceDelay() const { return skill[4]; } //skill[4]
	inline int& soundSourceDelayCounter() { return skill[5]; }
	inline const int& soundSourceDelayCounter() const { return skill[5]; }//skill[5]
	inline int& soundSourceOrigin() { return skill[6]; }
	inline const int& soundSourceOrigin() const { return skill[6]; }//skill[6]

	//--PUBLIC LIGHT SOURCE SKILLS--
	inline int& lightSourceBrightness() { return skill[0]; }
	inline const int& lightSourceBrightness() const { return skill[0]; } //skill[0]
	inline int& lightSourceAlwaysOn() { return skill[1]; }
	inline const int& lightSourceAlwaysOn() const { return skill[1]; } //skill[1]
	inline int& lightSourceInvertPower() { return skill[2]; }
	inline const int& lightSourceInvertPower() const { return skill[2]; } //skill[2]
	inline int& lightSourceLatchOn() { return skill[3]; }
	inline const int& lightSourceLatchOn() const { return skill[3]; } //skill[3]
	inline int& lightSourceRadius() { return skill[4]; }
	inline const int& lightSourceRadius() const { return skill[4]; } //skill[4]
	inline int& lightSourceFlicker() { return skill[5]; }
	inline const int& lightSourceFlicker() const { return skill[5]; } //skill[5]
	inline int& lightSourceDelay() { return skill[6]; }
	inline const int& lightSourceDelay() const { return skill[6]; } //skill[6]
	inline int& lightSourceDelayCounter() { return skill[7]; }
	inline const int& lightSourceDelayCounter() const { return skill[7]; }//skill[7]
	inline int& lightSourceRGB() { return skill[11]; }
	inline const int& lightSourceRGB() const { return skill[11]; }//skill[11]

	//--PUBLIC TEXT SOURCE SKILLS--
	inline int& textSourceColorRGB() { return skill[0]; }
	inline const int& textSourceColorRGB() const { return skill[0]; } //skill[0]
	inline int& textSourceVariables4W() { return skill[1]; }
	inline const int& textSourceVariables4W() const { return skill[1]; } //skill[1]
	inline int& textSourceDelay() { return skill[2]; }
	inline const int& textSourceDelay() const { return skill[2]; } //skill[2]
	inline int& textSourceIsScript() { return skill[3]; }
	inline const int& textSourceIsScript() const { return skill[3]; } //skill[3]
	inline int& textSourceBegin() { return skill[4]; }
	inline const int& textSourceBegin() const { return skill[4]; } //skill[4]

	//--PUBLIC SIGNAL SKILLS--
	inline int& signalActivateDelay() { return skill[1]; }
	inline const int& signalActivateDelay() const { return skill[1]; } //skill[1]
	inline int& signalTimerInterval() { return skill[2]; }
	inline const int& signalTimerInterval() const { return skill[2]; } //skill[2]
	inline int& signalTimerRepeatCount() { return skill[3]; }
	inline const int& signalTimerRepeatCount() const { return skill[3]; } //skill[3]
	inline int& signalTimerLatchInput() { return skill[4]; }
	inline const int& signalTimerLatchInput() const { return skill[4]; } //skill[4]
	inline int& signalInputDirection() { return skill[5]; }
	inline const int& signalInputDirection() const { return skill[5]; } //skill[5]
	inline int& signalGateANDPowerCount() { return skill[9]; }
	inline const int& signalGateANDPowerCount() const { return skill[9]; } //skill[9]
	inline int& signalInvertOutput() { return skill[10]; }
	inline const int& signalInvertOutput() const { return skill[10]; } //skill[10]

	//--PUBLIC LOCK SKILLS--
	inline int& wallLockState() { return skill[0]; }
	inline const int& wallLockState() const { return skill[0]; } //skill[0]
	inline int& wallLockInvertPower() { return skill[1]; }
	inline const int& wallLockInvertPower() const { return skill[1]; } //skill[1]
	inline int& wallLockTurnable() { return skill[3]; }
	inline const int& wallLockTurnable() const { return skill[3]; } //skill[3]
	inline int& wallLockMaterial() { return skill[4]; }
	inline const int& wallLockMaterial() const { return skill[4]; } //skill[4]
	inline int& wallLockDir() { return skill[5]; }
	inline const int& wallLockDir() const { return skill[5]; } //skill[5]
	inline int& wallLockClientInteractDelay() { return skill[6]; }
	inline const int& wallLockClientInteractDelay() const { return skill[6]; } //skill[6]
	inline int& wallLockPlayerInteracting() { return skill[7]; }
	inline const int& wallLockPlayerInteracting() const { return skill[7]; } //skill[7]
	inline int& wallLockPower() { return skill[8]; }
	inline const int& wallLockPower() const { return skill[8]; } //skill[8]
	inline int& wallLockInit() { return skill[9]; }
	inline const int& wallLockInit() const { return skill[9]; } //skill[9]
	inline int& wallLockTimer() { return skill[10]; }
	inline const int& wallLockTimer() const { return skill[10]; } //skill[10]
	inline int& wallLockPickable() { return skill[11]; }
	inline const int& wallLockPickable() const { return skill[11]; } //skill[11]
	inline int& wallLockPickHealth() { return skill[12]; }
	inline const int& wallLockPickHealth() const { return skill[12]; } //skill[12]
	inline int& wallLockPickableSkeletonKey() { return skill[13]; }
	inline const int& wallLockPickableSkeletonKey() const { return skill[13]; } //skill[13]
	inline int& wallLockPreventLockpickExploit() { return skill[14]; }
	inline const int& wallLockPreventLockpickExploit() const { return skill[14]; } //skill[14]
	inline int& wallLockAutoGenKey() { return skill[15]; }
	inline const int& wallLockAutoGenKey() const { return skill[15]; } //skill[15]

	//--THROWN PROJECTILE--
	inline int& thrownProjectilePower() { return skill[19]; }
	inline const int& thrownProjectilePower() const { return skill[19]; } //skill[19]
	inline int& thrownProjectileCharge() { return skill[20]; }
	inline const int& thrownProjectileCharge() const { return skill[20]; } //skill[20]
	inline int& thrownProjectileParticleTimerUID() { return skill[9]; }
	inline const int& thrownProjectileParticleTimerUID() const { return skill[9]; }

	//--PLAYER SPAWN POINT--
	inline int& playerStartDir() { return skill[1]; }
	inline const int& playerStartDir() const { return skill[1]; } //skill[1]

	//--ACTTRAP/PERMANENT
	inline int& pressurePlateTriggerType() { return skill[3]; }
	inline const int& pressurePlateTriggerType() const { return skill[3]; } //skill[3]

	enum PressurePlateTriggerTypes : int
	{
		PRESSURE_PLATE_DEFAULT_ALL,
		PRESSURE_PLATE_PLAYERS,
		PRESSURE_PLATE_MONSTERS,
		PRESSURE_PLATE_ITEMS,
		PRESSURE_PLATE_BOULDERS,
		PRESSURE_PLATE_PLAYERS_OR_MONSTERS,
		PRESSURE_PLATE_PLAYERS_OR_ALLIES,
		PRESSURE_PLATE_MONSTERS_NON_ALLY,
		PRESSURE_PLATE_ENUM_END
	};

	enum WallLockStates
	{
		LOCK_NO_KEY,
		LOCK_KEY_START,
		LOCK_KEY_ENTER,
		LOCK_KEY_ACTIVE_START,
		LOCK_KEY_ACTIVE,
		LOCK_KEY_INACTIVE_START,
		LOCK_KEY_INACTIVE
	};

	enum EntityShowMapSource
	{
		SHOW_MAP_DEFAULT = 1,
		SHOW_MAP_GYRO = 2,
		SHOW_MAP_SCRY = 3,
		SHOW_MAP_DONATION = 4,
		SHOW_MAP_PINPOINT = 5,
		SHOW_MAP_DETECT_MONSTER = 6
	};
	void setEntityShowOnMap(EntityShowMapSource source, int duration);
	void entityShowOnMapTickDuration();
	int getEntityShowOnMapDuration()
	{
		return (EntityShowMapSource)(entityShowOnMap() & 0xFFFFFF);
	}
	EntityShowMapSource getEntityShowOnMapSource();

	//--WORLDTOOLTIP--
	inline real_t& worldTooltipAlpha() { return fskill[0]; }
	inline const real_t& worldTooltipAlpha() const { return fskill[0]; } //fskill[0]
	inline real_t& worldTooltipZ() { return fskill[1]; }
	inline const real_t& worldTooltipZ() const { return fskill[1]; } //fskill[1]
	inline int& worldTooltipActive() { return skill[0]; }
	inline const int& worldTooltipActive() const { return skill[0]; } //skill[0]
	inline int& worldTooltipPlayer() { return skill[1]; }
	inline const int& worldTooltipPlayer() const { return skill[1]; }  //skill[1]
	inline int& worldTooltipInit() { return skill[3]; }
	inline const int& worldTooltipInit() const { return skill[3]; } //skill[3]
	inline int& worldTooltipFadeDelay() { return skill[4]; }
	inline const int& worldTooltipFadeDelay() const { return skill[4]; } //skill[4]
	inline int& worldTooltipIgnoreDrawing() { return skill[5]; }
	inline const int& worldTooltipIgnoreDrawing() const { return skill[5]; } //skill[5]
	inline int& worldTooltipRequiresButtonHeld() { return skill[6]; }
	inline const int& worldTooltipRequiresButtonHeld() const { return skill[6]; } //skill[6]

	//--STATUES--
	inline int& statueInit() { return skill[0]; }
	inline const int& statueInit() const { return skill[0]; } //skill[0]
	inline int& statueDir() { return skill[1]; }
	inline const int& statueDir() const { return skill[1]; } //skill[1]
	inline int& statueId() { return skill[3]; }
	inline const int& statueId() const { return skill[3]; } //skill[3]

	// new references, just set the skill here

	// actSprite
	inline int& actSpriteUseAlpha() { return skill[6]; }
	inline const int& actSpriteUseAlpha() const { return skill[6]; }
	inline int& actSpriteNoBillboard() { return skill[7]; }
	inline const int& actSpriteNoBillboard() const { return skill[7]; }
	inline int& actSpriteCheckParentExists() { return skill[8]; }
	inline const int& actSpriteCheckParentExists() const { return skill[8]; }
	//Sint32& actSpriteAlwaysDraw = skill[9];
	inline int& actSpriteUseCustomSurface() { return skill[10]; }
	inline const int& actSpriteUseCustomSurface() const { return skill[10]; }
	inline int& actSpriteFollowUID() { return skill[11]; }
	inline const int& actSpriteFollowUID() const { return skill[11]; }
	inline int& actSpriteHasLightInit() { return skill[12]; }
	inline const int& actSpriteHasLightInit() const { return skill[12]; }
	inline int& actSpriteVelXY() { return skill[13]; }
	inline const int& actSpriteVelXY() const { return skill[13]; }
	inline real_t& actSpritePitchRotate() { return fskill[4]; }
	inline const real_t& actSpritePitchRotate() const { return fskill[4]; }

	// actGib
	inline int& actGibHitGroundEvent() { return skill[10]; }
	inline const int& actGibHitGroundEvent() const { return skill[10]; }
	inline int& actGibMagicParticle() { return skill[12]; }
	inline const int& actGibMagicParticle() const { return skill[12]; } // skill[11] is player hud denote
	inline int& actGibDisableDrawForLocalPlayer() { return skill[13]; }
	inline const int& actGibDisableDrawForLocalPlayer() const { return skill[13]; } // set to 1 + playernum, won't draw for that playernum

	// actWind
	inline int& actWindParticleEffect() { return skill[1]; }
	inline const int& actWindParticleEffect() const { return skill[1]; }
	inline int& actWindEffectsProjectiles() { return skill[3]; }
	inline const int& actWindEffectsProjectiles() const { return skill[3]; }
	inline int& actWindLifetime() { return skill[4]; }
	inline const int& actWindLifetime() const { return skill[4]; }
	inline real_t& actWindStrength() { return fskill[0]; }
	inline const real_t& actWindStrength() const { return fskill[0]; }
	inline int& actWindTileBonusLength() { return skill[5]; }
	inline const int& actWindTileBonusLength() const { return skill[5]; }
	inline int& actTrapSabotaged() { return skill[30]; }
	inline const int& actTrapSabotaged() const { return skill[30]; }

	void pedestalOrbInit(); // init orb properties

	// a pointer to the entity's location in a list (ie the map list of entities)
	node_t* mynode;
	node_t* myCreatureListNode;
	node_t* myTileListNode;
	node_t* myWorldUIListNode;

	list_t* path; // pathfinding stuff. Most of the code currently stuffs that into children, but the magic code makes use of this variable instead.

	//Dummy stats to make certain visual features work on clients (such as ambient particles for magic reflection).
	Stat* clientStats;
	bool clientsHaveItsStats;
	void giveClientStats();

	// behavior function pointer
	void (*behavior)(class Entity* my);
	bool ranbehavior;

	void setObituary(const char* obituary);

	void killedByMonsterObituary(Entity* victim, bool fromSpell = false);

	Sint32 getSTR();
	Sint32 getDEX();
	Sint32 getCON();
	Sint32 getINT();
	Sint32 getPER();
	Sint32 getCHR();

	int entityLight(); //NOTE: Name change conflicted with light_t *light
	int entityLightAfterReductions(Stat& myStats, Entity* observer);

	void handleEffects(Stat* myStats);
	static int getHungerTickRate(Stat* myStats, bool isPlayer, bool checkItemsEffects);
	void handleEffectsClient();

	void effectTimes();
	bool increaseSkill(int skill, bool notify = true);

	Stat* getStats() const;

	void setHP(int amount);
	void modHP(int amount); //Adds amount to HP.
	int getHP();

	void setMP(int amount, bool updateClients = true);
	int modMP(int amount, bool updateClients = true); //Adds amount to MP.
	int getMP();

	void drainMP(int amount, bool notifyOverexpend = true); //Removes this much from MP. Anything over the entity's MP is subtracted from their health. Can be very dangerous.
	bool safeConsumeMP(int amount); //A function for the magic code. Attempts to remove mana without overdrawing the player. Returns true if success, returns false if didn't have enough mana.

	static real_t PlayerAttackMeleeStatFactor;
	static real_t PlayerAttackRangedStatFactor;
	static real_t PlayerAttackThrownStatFactor;
	static Sint32 getAttack(Entity* my, Stat* myStats, bool isPlayer, int chargeModifier = -1, int* returnWeaponAttackValue = nullptr);
	static real_t getACEffectiveness(Entity* my, Stat* myStats, bool isPlayer, Entity* attacker, Stat* attackerStats, int& outNumBlessings);
	static void setMeleeDamageSkillModifiers(Entity* my, Stat* myStats, int skill, real_t& baseSkillModifier, real_t& variance, ItemType* itemType);
	Sint32 getBonusAttackOnTarget(Stat& hitstats);
	Sint32 getRangedAttack(int atkFromQuivers);
	Sint32 getThrownAttack();
	bool isBlind();
	bool isWaterWalking() const;
	bool isLavaWalking() const;
	
	bool isInvisible() const;

	bool isMobile();

	void attack(int pose, int charge, Entity* target);

	bool teleport(int x, int y);
	bool teleportRandom(int x1 = 0, int x2 = 0, int y1 = 0, int y2 = 0); // arbitrary map limits variables
	// teleport entity to a target, within a radius dist (range in whole tile lengths)
	bool teleportAroundEntity(Entity* target, int dist, int effectType = 0);
	// teleport entity to fixed position with appropriate sounds, for actTeleporter.
	bool teleporterMove(int x, int y, int type);

	//void entityAwardXP(Entity *dest, Entity *src, bool share, bool root);
	void awardXP(Entity* src, bool share, bool root);

	//--*CheckBetterEquipment functions--
	void checkBetterEquipment(Stat* myStats);
	void checkGroundForItems();
	bool canWieldItem(const Item& item) const;
	bool goblinCanWieldItem(const Item& item) const;
	bool humanCanWieldItem(const Item& item) const;
	bool goatmanCanWieldItem(const Item& item) const;
	bool automatonCanWieldItem(const Item& item) const;
	bool shadowCanWieldItem(const Item& item) const;
	bool insectoidCanWieldItem(const Item& item) const;

	bool monsterWantsItem(const Item& item, Item**& shouldEquip, node_t*& replaceInventoryItem) const;

	void createPathBoundariesNPC(int maxTileDistance = -1);
	void humanSetLimbsClient(int bodypart);

	/*
	 * Check if the goatman can wield the item, and if so, is it something it wants? E.g. does it really want to carry 2 sets of armor?
	 */
	//bool goatmanWantsItem(const Item& item, Item*& shouldWield, node_t*& replaceInventoryItem) const;

	bool shouldMonsterEquipThisWeapon(const Item& itemToEquip) const;//TODO: Look @ proficiencies.
	Item** shouldMonsterEquipThisArmor(const Item& item) const;
	int shouldMonsterDefend(Stat& myStats, const Entity& target, const Stat& targetStats, int targetDist, bool hasrangedweapon);
	bool monsterConsumeFoodEntity(Entity* food, Stat* myStats);
	Entity* monsterAllyGetPlayerLeader() const;
	bool monsterAllyEquipmentInClass(const Item& item) const;
	bool monsterIsTinkeringCreation();
	void monsterHandleKnockbackVelocity(real_t monsterFacingTangent, real_t weightratio);
	int monsterGetDexterityForMovement();
	void monsterGenerateQuiverItem(Stat* myStats, bool lesserMonster = false);
	int getMonsterEffectiveDistanceOfRangedWeapon(Item* weapon);
	bool isFollowerFreeToPathToPlayer(Stat* myStats);
	void removeLightField(); // Removes light field from entity, sets this->light to nullptr.

	//--- Mechanism functions ---
	void circuitPowerOn(); //Called when a nearby circuit or switch powers on.
	void circuitPowerOff(); //Called when a nearby circuit or switch powers off.
	void updateCircuitNeighbors(); //Called when a circuit's powered state changes.
	void mechanismPowerOn(); //Called when a circuit or switch next to a mechanism powers on.
	void mechanismPowerOff(); //Called when a circuit or switch next to a mechanism powers on.
	void toggleSwitch(int skillIndexForPower = -1); //Called when a player flips a switch (lever). skillIndexForPower can use any skill[] to reference for the entity power status (defaults to skill[0] for switches)
	void switchUpdateNeighbors(); //Run each time actSwitch() is called to make sure the network is online if any one switch connected to it is still set to the on position.
	list_t* getPowerableNeighbors(); //Returns a list of all circuits and mechanisms this entity can influence.

	//Chest/container functions.
	void closeChest();
	void closeChestServer(); //Close the chest serverside, silently. Called when the chest is closed somewhere else for that client, but the server end stuff needs to be tied up.
	Item* addItemToChest(Item* item, bool forceNewStack, Item* specificDestinationStack); //Adds an item to the chest. If server, notifies the client. If client, notifies the server.
	static Item* addItemToVoidChest(int player, Item* item, bool forceNewStack, Item* specificDestinationStack); //Adds an item to the chest. If client, notifies the server.
	static Item* addItemToVoidChestServer(int player, Item* item, bool forceNewStack, Item* specificDestinationStack);
	Item* getItemFromChest(Item* item, int amount, bool getInfoOnly = false); //Removes an item from the chest and returns a pointer to it.
	Item* addItemToChestFromInventory(int player, Item* item, int amount, bool forceNewStack, Item* specificDestinationStack);
	Item* addItemToChestServer(Item* item, bool forceNewStack, Item* specificDestinationStack); //Adds an item to the chest. Called when the server receives a notification from the client that an item was added to the chest.
	bool removeItemFromChestServer(Item* item, int count); //Called when the server learns that a client removed an item from the chest.
	static bool removeItemFromVoidChestServer(int player, Item* item, int count); //Called when the server learns that a client removed an item from the chest.
	void unlockChest();
	void lockChest();
	list_t* getChestInventoryList();
	void chestHandleDamageMagic(int damage, Entity &magicProjectile, Entity *caster, bool doSound = true);

	//Power Crystal functions.
	void powerCrystalCreateElectricityNodes();

	//Door functions.
	void doorHandleDamageMagic(int damage, Entity &magicProjectile, Entity *caster, bool messages = true, bool doSound = true);
	void colliderHandleDamageMagic(int damage, Entity &magicProjectile, Entity *caster, bool messages = true, bool doSound = true);

	bool checkEnemy(Entity* your);
	bool checkFriend(Entity* your);
	bool friendlyFireProtection(Entity* your);
	void alertAlliesOnBeingHit(Entity* attacker, DynamicArrayT<Entity*>* skipEntitiesToAlert = nullptr);

	//Act functions.
	void actChest();
	void actPowerCrystal();
	void actGate();
	void actPedestalBase();
	void actPedestalOrb();
	void actMidGamePortal();
	void actExpansionEndGamePortal();
	void actTeleporter();
	void actMagicTrapCeiling();
	void actTeleportShrine();
	void actDaedalusShrine();
	void actAssistShrine();
	bool magicFallingCollision();
	bool magicOrbitingCollision();
	void actFurniture();
	void furnitureHandleDamageMagic(int damage, Entity& magicProjectile, Entity* caster, bool messages = true, bool doSound = true);
	void actPistonCam();
	void actStalagCeiling();
	void actStalagFloor();
	void actStalagColumn();
	void actColumn();
	void actSoundSource();
	void actLightSource();
	void actTextSource();
	void actSignalTimer();
	void actSignalGateAND();
	void actWallLock();
	void actWallButton();
	void actIronDoor();
	void actWind();

	Monster getRace() const;

	bool inline skillCapstoneUnlockedEntity(int proficiency) const;

	/*
	 * Returns -1 if not a player.
	 */
	int isEntityPlayer() const;

	void initMonster(int mySprite);

	//--monster type from sprite
	Monster getMonsterTypeFromSprite() const;
	static Monster getMonsterTypeFromSprite(const int sprite);
	//--monster limb offsets
	void setHelmetLimbOffset(Entity* helm);
	void setTorsoLimbOffset(Entity* torso);
	void setHumanoidLimbOffset(Entity* limb, Monster race, int limbType);
	void actMonsterLimb(bool processLight = false);

	void removeMonsterDeathNodes();

	void spawnBlood(int bloodsprite = 160);

	// reflection is set 1, 2 or 3 depending on the item slot. reflection of 3 does not degrade.
	int getReflection() const;
	// monster attack pose, return the animation to use based on weapon.
	int getAttackPose() const;
	// if monster holding ranged weapon.
	bool hasRangedWeapon(bool ignoreMonsterNPCType = false) const;
	// weapon arm animation attacks
	void handleWeaponArmAttack(Entity* weaponarm);
	// handle walking movement for arms and legs
	void humanoidAnimateWalk(Entity* limb, node_t* bodypartNode, int bodypart, double walkSpeed, double dist, double distForFootstepSound);
	// monster footsteps, needs to be client friendly
	Uint32 getMonsterFootstepSound(int footstepType, int bootSprite);
	// handle humanoid weapon arm animation/sprite offsets
	void handleHumanoidWeaponLimb(Entity* weaponLimb, Entity* weaponArmLimb);
	void handleHumanoidShieldLimb(Entity* shieldLimb, Entity* shieldArmLimb);
	void handleQuiverThirdPersonModel(Stat& myStats, int mySprite = -1);
	// server only function to set boot sprites on monsters.
	bool setBootSprite(Entity* leg, int spriteOffset, bool forceShort = false);
	static bool isBootSpriteShortArmor(Entity* leg);
	// monster special attack handler, returns true if monster should attack after calling this function.
	bool handleMonsterSpecialAttack(Stat* myStats, Entity* target, double dist, bool forceDeinit);
	// monster attack handler
	void handleMonsterAttack(Stat* myStats, Entity* target, double dist);
	void lookAtEntity(Entity& target);
	// automaton specific function
	void automatonRecycleItem();
	// incubus teleport spells
	void incubusTeleportToTarget(const Entity* target);
	void incubusTeleportRandom();
	//Shadow teleport spells.
	void shadowTeleportToTarget(const Entity* target, int range);
	//Lich effects
	void lichFireTeleport();
	void lichIceTeleport();
	void lichIceCreateCannon();
	Entity* lichThrowProjectile(real_t angle);
	void lichIceSummonMonster(Monster creature);
	bool devilSummonMonster(Entity* summonOnEntity, Monster creature, int radiusFromCenter, int playerToTarget = -1);
	int devilGetNumMonstersInArena(Monster creature);
	bool devilBoulderSummonIfPlayerIsHiding(int player);
	void lichFireSummonMonster(Monster creature);
	// check for nearby items to add to monster's inventory, returns true if picked up item
	bool monsterAddNearbyItemToInventory(Stat* myStats, int rangeToFind, int maxInventoryItems, Entity* forcePickupItem = nullptr);
	// degrade chosen armor piece by 1 on entity, update clients.
	bool degradeArmor(Stat& hitstats, Item& armor, int armornum);
	// check stats if monster should "retreat" in actMonster
	bool shouldRetreat(Stat& myStats);
	// check if monster should retreat or stand still when less than given distance
	bool backupWithRangedWeapon(Stat& myStats, int dist, int hasrangedweapon);
	// calc time required for a mana regen tick, uses equipped gear as modifiers.
	static int getManaringFromEquipment(Entity* my, Stat& myStats, bool isPlayer);
	static int getManaringFromEffects(Entity* my, Stat& myStats);
	static int getManaRegenInterval(Entity* my, Stat& myStats, bool isPlayer, bool excludeItemsEffectsBonus = false);
	// calc time required for a hp regen tick, uses equipped gear as modifiers.
	static int getHealringFromEquipment(Entity* my, Stat& myStats, bool isPlayer);
	static int getHealringFromEffects(Entity* my, Stat& myStats);
	static int getHealthRegenInterval(Entity* my, Stat& myStats, bool isPlayer, bool excludeItemsEffectsBonus = false);
	// calc damage/effects for ranged weapons.
	void setRangedProjectileAttack(Entity& marksman, Stat& myStats, int optionalOverrideForArrowType = 0);
	bool setArrowProjectileProperties(int weaponType);
	real_t yawDifferenceFromEntity(Entity* entity); // calc targets yaw compared to an entity, returns 0 - 2 * PI, where > PI is facing towards player.
	spell_t* getActiveMagicEffect(int spellID);

	/*
	 * 1 in @chance chance in spawning a particle with the given sprite and duration.
	 */
	Entity* spawnAmbientParticles(int chance, int particleSprite, int duration, double particleScale, bool shrink);
	Entity* spawnAmbientParticles2(int chance, int particleSprite, int duration, double particleScale, bool shrink);

	//Updates the EFFECTS variable for all clients for this entity.
	void serverUpdateEffectsForEntity(bool guarantee);

	/*
	 * If set on a player, will call serverUpdateEffects() on the player.
	 * @param guarantee: Causes serverUpdateEffectsForEntity() to use sendPacketSafe() rather than just sendPacket().
	 * Returns true on successfully setting value.
	 */
	bool setEffect(int effect, std::variant<bool, Uint8> value, int duration, bool updateClients, bool guarantee = true, bool overrideEffectStrength = false, bool overrideDuration = true);

	/*
	 * @param state: required to let the entity know if it should enter MONSTER_STATE_PATH, MONSTER_STATE_ATTACK, etc.
	 * @param monsterWasHit: monster is retaliating to an attack as opposed to finding an enemy. to set reaction time accordingly in hardcore
	 */
	void monsterAcquireAttackTarget(const Entity& target, Sint32 state, bool monsterWasHit = false);
	bool monsterAlertBeforeHit(Entity* attacker);

	/*
	 * Attempts to set the target to 0.
	 * May refuses to do so and consequently return false in cases such as the shadow, which cannot lose its target until it's dead.
	 * Returns true otherwise, if successfully zero-d out target.
	 */
	bool monsterReleaseAttackTarget(bool force = false);

	//Lets monsters swap out weapons.
	#ifndef EDITOR
	void chooseWeapon(const Entity* target, double dist);
	#endif
	void goatmanChooseWeapon(const Entity* target, double dist);
	void insectoidChooseWeapon(const Entity* target, double dist);
	void incubusChooseWeapon(const Entity* target, double dist);
	void vampireChooseWeapon(const Entity* target, double dist);
	void shadowChooseWeapon(const Entity* target, double dist);
	void succubusChooseWeapon(const Entity* target, double dist);
	void slimeChooseWeapon(const Entity* target, double dist);
	void mothChooseWeapon(const Entity* target, double dist);
	void bugbearChooseWeapon(const Entity* target, double dist);
	void monsterDChooseWeapon(const Entity* target, double dist);
	void monsterMChooseWeapon(const Entity* target, double dist);
	void monsterGChooseWeapon(const Entity* target, double dist);
	void skeletonSummonSetEquipment(Stat* myStats, int rank);
	static void tinkerBotSetStats(Stat* myStats, int rank);
	static void mimicSetStats(Stat* myStats);
	bool monsterInMeleeRange(const Entity* target, double dist) const;

	node_t* addItemToMonsterInventory(Item* item);

	//void returnWeaponarmToNeutral(Entity* weaponarm, Entity* rightbody); //TODO: Need a proper refactor?

	void shadowSpecialAbility(bool initialMimic);

	bool shadowCanMimickSpell(int spellID);

	double monsterRotate();

	//TODO: These two won't work with multiplayer because clients are stubborn little tater tots that refuse to surrender their inventories on demand.
	//Here's the TODO: Fix it.
	Item* getBestMeleeWeaponIHave() const;
	Item* getBestShieldIHave() const;

	void monsterEquipItem(Item& item, Item** slot);

	bool monsterHasSpellbook(int spellbookType);
	//bool monsterKnowsSpell(int spellID); //TODO: Should monsters use the spell item instead of spellbooks?
	node_t* chooseAttackSpellbookFromInventory();

	/* entity.cpp
	 * Attempts to set the Entity on fire. Entities that are not Burnable or are already on fire will return before any processing
	 * Entities that do not have Stats (such as furniture) will return after setting the fire time and chance to stop at max
	 * Entities with Stats will have their fire time (char_fire) and chance to stop being on fire (chanceToPutOutFire) reduced by their CON
	 * Calculations for reductions is outlined in this function
	 */
	bool SetEntityOnFire(Entity* sourceOfFire);

	void addToCreatureList(list_t* list);
	void addToWorldUIList(list_t *list);
	DynamicArray bodyparts;  // vector<Entity*> (non-owning pointers)
	DynamicSetI32 collisionIgnoreTargets;

	bool collisionProjectileMiss(Entity* parent, Entity* projectile);

	// special magic functions/trickery
	void castFallingMagicMissile(int spellID, real_t distFromCaster, real_t angleFromCasterDirection, int heightDelay);
	Entity* castOrbitingMagicMissile(int spellID, real_t distFromCaster, real_t angleFromCasterDirection, int duration);
	void lichFireSetNextAttack(Stat& myStats);
	void lichIceSetNextAttack(Stat& myStats);

	int getEntityInspirationFromAllies();
	int getFollowerBonusDamageResist();
	int getEntityBonusTrapResist();
	bool onEntityTrapHitSacredPath(Entity* trap);
	int getFollowerBonusHPRegen();
	static int getHPRestoreOnLevelUp(Entity* entity, Stat* myStats, int baseHP, bool statCheckOnly = false);
	static int getMPRestoreOnLevelUp(Entity* entity, Stat* myStats, int baseMP, bool statCheckOnly = false);
	void monsterMoveBackwardsAndPath(bool trySidesFirst = false); // monster tries to move backwards in a cross shaped area if stuck against an entity.
	bool monsterHasLeader(); // return true if monsterstats->leader_uid is not 0.
	void monsterAllySendCommand(int command, int destX, int destY, Uint32 uid = 0); // update the behavior of allied NPCs.
	bool monsterAllySetInteract(); // set interact flags for allied NPCs.
	bool isInteractWithMonster(); // is a monster interacting with me? check interact flags for allied NPCs.
	void clearMonsterInteract(); // tidy up flags after interaction.
	bool monsterSetPathToLocation(int destX, int destY, int adjacentTilesToCheck, int pathingType, bool tryRandomSpot = false, bool shortByShortest = true); // monster create path to destination, search adjacent tiles if specified target is inaccessible.
	bool gyrobotSetPathToReturnLocation(int destX, int destY, int adjacentTilesToCheck, bool tryRandomSpot = false); // gyrobot create path to destination to land safely.
	static int getMagicResistance(Stat* myStats); // returns the value of magic resistance of a monster.
	static real_t magicResistancePerPoint;
	void playerLevelEntrySpeechSecond(); // handle secondary voice lines for post-herx content
	bool isPlayerHeadSprite() const; // determines if model of entity is a human head.
	static bool isPlayerHeadSprite(const int sprite);
	void setDefaultPlayerModel(int playernum, Monster playerRace, int limbType, int headSprite); // sets correct base color/model of limbs for player characters.
	Monster getMonsterFromPlayerRace(int playerRace); // convert playerRace into the relevant monster type
	void setHardcoreStats(Stat& stats); // set monster stats for hardcore mode.
	void handleNPCInteractDialogue(Stat& myStats, AllyNPCChatter event); // monster text for interactions.
	void playerStatIncrease(int playerClass, int chosenStats[3]);
	bool isBossMonster(); // return true if boss map (hell boss, boss etc or shopkeeper/shadow/other boss
	bool isSmiteWeakMonster();
	void handleKnockbackDamage(Stat& myStats, Entity* knockedInto); // handle knockback damage from getting hit into other things.
	void setHelmetLimbOffsetWithMask(Entity* helm, Entity* mask);
	bool entityCheckIfTriggeredBomb(bool triggerBomb);
	bool entityCheckIfTriggeredWallButton();
	Sint32 playerInsectoidExpectedManaFromHunger(Stat& myStats);
	Sint32 playerInsectoidHungerValueOfManaPoint(Stat& myStats);
	void playerInsectoidIncrementHungerToMP(int mpAmount);
	static real_t getDamageTableMultiplier(Entity* my, Stat& myStats, DamageTableType damageType, int* magicResistance = nullptr, int* outNumSources = nullptr);
	static real_t getDamageTableEquipmentMod(Stat& myStats, Item& item, real_t base, real_t mod);
	bool isBoulderSprite();
	void createWorldUITooltip();
	bool bEntityTooltipRequiresButtonHeld() const;
	bool bEntityHighlightedForPlayer(const int player) const;
	void updateEntityOnHit(Entity* attacker, bool alertTarget);
	bool isDamageableCollider() const;
	bool isColliderDamageableByMelee() const;
	bool isColliderWeakToSkill(const int proficiency) const;
	bool isColliderResistToSkill(const int proficiency) const;
	bool isColliderWeakToBoulders() const;
	bool isColliderShownAsWallOnMinimap() const;
	bool isColliderDamageableByMagic() const;
	bool isColliderPathableMonster(Monster type) const;
	bool isColliderAttachableToBombs() const;
	bool isColliderWall() const;
	bool isColliderBreakableContainer() const;
	void colliderOnDestroy();
	int getColliderOnHitLangEntry() const;
	int getColliderOnBreakLangEntry() const;
	int getColliderOnJumpLangEntry() const;
	int getColliderSfxOnHit() const;
	int getColliderSfxOnBreak() const;
	int getColliderLangName() const;
	static void monsterRollLevelUpStats(int increasestat[3]);
	bool disturbMimic(Entity* touched, bool takenDamage, bool doMessage);
	bool disturbBat(Entity* touched, bool takenDamage, bool doMessage);
	bool isInertMimic() const;
	bool isUntargetableBat(real_t* outDist = nullptr) const;
	bool entityCanVomit() const;
	bool doSilkenBowOnAttack(Entity* attacker);
	void setBugbearStrafeDir(bool forceDirection);
	void processEntityWind();
	bool windEffectsEntity(Entity* entity);
	real_t monsterGetWeightRatio();
	bool spellEffectPreserveItem(Item* item);
	bool mistFormDodge(bool checkEffectActiveOnly, Entity* attacker);
	bool defyFleshProc(Entity* attacker);
	bool pinpointDamageProc(Entity* attacker, int damage);
	static bool modifyDamageMultipliersFromEffects(Entity* hitentity, Entity* attacker, 
		real_t& damageMultiplier, DamageTableType damageTableType, Entity* projectile = nullptr, int spellID = -1);
	real_t getHealingSpellPotionModifierFromEffects(bool processLevelup);
	void attractItem(Entity& itemEntity);
	void creatureHandleLiftZ();
	bool monsterIsTargetable(bool targetInertMimics = false) const;
	bool monsterCanTradeWith(int player) const;
	bool degradeAmuletProc(Stat* myStats, ItemType type);
	bool myconidReboundOnHit(Entity* attacker);
	void playerShakeGrowthHelmet();
};

Monster getMonsterFromPlayerRace(int playerRace); // convert playerRace into the relevant monster type
Sint32 statGetSTR(Stat* entitystats, Entity* my);
Sint32 statGetDEX(Stat* entitystats, Entity* my);
Sint32 statGetCON(Stat* entitystats, Entity* my);
Sint32 statGetINT(Stat* entitystats, Entity* my);
Sint32 statGetPER(Stat* entitystats, Entity* my);
Sint32 statGetCHR(Stat* entitystats, Entity* my);
extern Uint32 entity_uids, lastEntityUIDs;
//extern Entity *players[4];
extern Uint32 nummonsters;

class Item;

extern bool swornenemies[NUMMONSTERS][NUMMONSTERS];
extern bool monsterally[NUMMONSTERS][NUMMONSTERS];

int AC(Stat* stat);

Entity* uidToEntity(Sint32 uidnum);
list_t* checkTileForEntity(int x, int y); //Don't forget to free the list returned when you're done with it. Also, provide x and y in map, not entity, units.
/*
 * Don't forget to free the list returned when you're done with it.
 * Provide x and y in map, not entity, units.
 * The list parameter is a pointer to the list all the items found will be appended to.
 */
void getItemsOnTile(int x, int y, list_t** list);

// get mana regen from stats and proficiencies only.
int getBaseManaRegen(Entity* my, Stat& myStats, bool excludeItemsEffectsBonus = false);

//--- Entity act* functions ---
void actMonster(Entity* my);
int playerHeadSprite(Monster race, sex_t sex, int appearance, int frame = 0, int player = -1);
void actPlayer(Entity* my);
void actPlayerXP(Entity* my);
void spawnPlayerXP(real_t x, real_t y, int player, int xpAmount);
void playerAnimateRat(Entity* my);
void playerAnimateSpider(Entity* my);

/*
 * NOTE: Potion effects
 * value 0 = POTION_WATER
 * value 1 = POTION_BOOZE
 * value 2 = POTION_JUICE
 * value 3 = POTION_SICKNESS
 * value 4 = POTION_CONFUSION
 * value 5 = POTION_EXTRAHEALING
 * value 6 = POTION_HEALING
 * value 7 = POTION_RESTORABILITY
 * value 8 = POTION_BLINDNESS
 * value 9 = POTION_RESTOREMAGIC
 * value 10 = POTION_INVISIBILITY
 * value 11 = POTION_LEVITATION
 * value 12 = POTION_SPEED
 * value 13 = POTION_ACID
 * value 14 = POTION_PARALYSIS
 */
//TODO: Allow for cursed fountains. Any fountain that has a negative effect has, say, skill[4] set to 1 to indicate cursed. Used for monster behavior and for effects of things like healing potions.
void actFountain(Entity* my);
void actSink(Entity* my);

//--- Mechanism functions ---
void actCircuit(Entity* my);
void actSwitch(Entity* my); //Needs to be called periodically to ensure network's powered state is correct.
void getPowerablesOnTile(int x, int y, list_t** list); //Stores a list of all circuits and mechanisms, on the tile (in map coordinates), in list.
void actGate(Entity* my);
void actArrowTrap(Entity* my);
void actTrap(Entity* my);
void actTrapPermanent(Entity* my);
void actSwitchWithTimer(Entity* my);
void actIronDoor(Entity* my);

/*
 * Note: Circuits and mechanisms use skill[28] to signify powered state.
 * * If skill[28] == 0, it's not a mechanism (or circuit).
 * * If skill[28] == 1, it's powered off.
 * * If skill[28] == 2, it's powered on.
 * * Mechanism only: If skill[28] == 3, it's powered on and the entity already processed it. Sort of combining a mechanism->powered and mechanism->powered_last_frame variable into one. Not sure if it's necessary, but I thought it did when I came up with this, so there you have it.
 */

//---Chest/container functions---
void actChest(Entity* my);
void actChestLid(Entity* my);
void closeChestClientside(const int player); //Called by the client to manage all clientside stuff relating to closing a chest.
Item* addItemToChestClientside(const int player, Item* item, bool forceNewStack, Item* specificDestinationStack); //Called by the client to manage all clientside stuff relating to adding an item to a chest.
void createChestInventory(Entity* my, int chestType);

//---Stalag functions---
void actStalagFloor(Entity* my);
void actStalagCeiling(Entity* my);
void actStalagColumn(Entity* my);

//---Ceiling Tile functions---
void actCeilingTile(Entity* my);

//--Piston functions--
void actPistonBase(Entity* my);
void actPistonCam(Entity* my);

void actColumn(Entity* my);

//--Floor vegetation--
void actFloorDecoration(Entity* my);

//--Collider decoration--
void actColliderDecoration(Entity* my);

//---Magic entity functions---
void actMagiclightBall(Entity* my);
void actMagiclightMoving(Entity* my);

//---Misc act functions---
void actAmbientParticleEffectIdle(Entity* my);

void actTextSource(Entity* my);

//checks if a sprite falls in certain sprite ranges

static const int NUM_ITEM_STRINGS = ITEM_ENUM_MAX + 3;
static const int NUM_ITEM_STRINGS_BY_TYPE = 236;
static const int NUM_EDITOR_TILES = 350;

// furniture types.
static const int FURNITURE_TABLE = 0;
static const int FURNITURE_CHAIR = 1;
static const int FURNITURE_BED = 2;
static const int FURNITURE_BUNKBED = 3;
static const int FURNITURE_PODIUM = 4;

int checkSpriteType(Sint32 sprite);
Monster editorSpriteTypeToMonster(Sint32 sprite);
extern DynamicArray spriteEditorNameStrings;  // vector<const char*> (non-owning)
extern char tileEditorNameStrings[NUM_EDITOR_TILES][44];
extern char monsterEditorNameStrings[NUMMONSTERS][32];
extern char itemStringsByType[10][NUM_ITEM_STRINGS_BY_TYPE][32];
extern char itemNameStrings[NUM_ITEM_STRINGS][32];
int canWearEquip(Entity* entity, int category);
void createMonsterEquipment(Stat* stats, BaronyRNG& rng);
int countCustomItems(Stat* stats);
int countDefaultItems(Stat* stats);
void copyMonsterStatToPropertyStrings(Stat* tmpSpriteStats);
void setRandomMonsterStats(Stat* stats, BaronyRNG& rng);

int checkEquipType(const Item *ITEM);

static const int SPRITE_GLOVE_RIGHT_OFFSET = 0;
static const int SPRITE_GLOVE_LEFT_OFFSET = 4;
static const int SPRITE_BOOT_RIGHT_OFFSET = 0;
static const int SPRITE_BOOT_LEFT_OFFSET = 2;

int setGloveSprite(Stat * myStats, Entity* ent, int spriteOffset);
bool isLevitating(Stat * myStats);
int getWeaponSkill(const Item* weapon);
int getStatForProficiency(int skill);
void setSpriteAttributes(Entity* entityToSet, Entity* entityToCopy, Entity* entityStatToCopy);
bool monsterIsImmobileTurret(Entity* my, Stat* myStats);
bool monsterChangesColorWhenAlly(Stat* myStats, Entity* entity = nullptr);
int monsterTinkeringConvertHPToAppearance(Stat* myStats);
int monsterTinkeringConvertAppearanceToHP(Stat* myStats, int appearance);

static const int MSG_DESCRIPTION = 0;
static const int MSG_COMBAT = 1;
static const int MSG_OBITUARY = 2;
static const int MSG_GENERIC = 3;
static const int MSG_ATTACKS = 4;
static const int MSG_STEAL_WEAPON = 5;
static const int MSG_TOOL_BOMB = 6;
static const int MSG_COMBAT_BASIC = 7;
void messagePlayerMonsterEvent(int player, Uint32 color, Stat& monsterStats, const char* msgGeneric, const char* msgNamed, int detailType, Entity* optionalEntity = nullptr);
char const * playerClassLangEntry(int classnum, int playernum);

//Some testing functions/commands.
Entity* summonChest(long x, long y);

//Various settings variables regarding entities.
extern bool flickerLights;

//Boulder functions.
void boulderSokobanOnDestroy(bool pushedOffLedge);
void boulderLavaOrArcaneOnDestroy(Entity* my, int sprite, Entity* boulderHitEntity);

int playerEntityMatchesUid(Uint32 uid); // Returns >= 0 if player uid matches uid.
bool monsterNameIsGeneric(Stat& monsterStats); // returns true if a monster's name is a generic decription rather than a miniboss.
bool shieldSpriteAllowedImpForm(int sprite);
bool weaponSpriteAllowedImpForm(int sprite);

bool playerRequiresBloodToSustain(int player); // vampire type or accursed class
void spawnBloodVialOnMonsterDeath(Entity* entity, Stat* hitstats, Entity* killer);

void shrineDaedalusRevealMap(Entity& my);
void daedalusShrineInteract(Entity* my, Entity* touched);

enum EntityHungerIntervals : int
{
	HUNGER_INTERVAL_OVERSATIATED,
	HUNGER_INTERVAL_HUNGRY,
	HUNGER_INTERVAL_WEAK,
	HUNGER_INTERVAL_STARVING,
	HUNGER_INTERVAL_AUTOMATON_SUPERHEATED,
	HUNGER_INTERVAL_AUTOMATON_CRITICAL
};
int getEntityHungerInterval(int player, Entity* my, Stat* myStats, EntityHungerIntervals hungerInterval);

//Fountain potion drop chance variables.
extern const DynamicArrayU32 fountainPotionDropChances;
extern const DynamicArrayT<std::pair<int, int>> potionStandardAppearanceMap;
std::pair<int, int> fountainGeneratePotionDrop(BaronyRNG& rng);

class TextSourceScript
{
public:
	const int k_ScriptError = -1;
	const int k_ScriptRangeEntireMap = -2;
	enum ClientInformationType : int
	{
		CLIENT_UPDATE_ALL,
		CLIENT_UPDATE_CLASS,
		CLIENT_UPDATE_HUNGER
	};
	enum AttachToEntity : int
	{
		TO_NONE,
		TO_MONSTERS,
		TO_ITEMS,
		TO_PLAYERS,
		TO_NOTHING,
		TO_HUMAN,
		TO_RAT,
		TO_GOBLIN,
		TO_SLIME,
		TO_TROLL,
		TO_BAT_SMALL,
		TO_SPIDER,
		TO_GHOUL,
		TO_SKELETON,
		TO_SCORPION,
		TO_IMP,
		TO_CRAB,
		TO_GNOME,
		TO_DEMON,
		TO_SUCCUBUS,
		TO_MIMIC,
		TO_LICH,
		TO_MINOTAUR,
		TO_DEVIL,
		TO_SHOPKEEPER,
		TO_KOBOLD,
		TO_SCARAB,
		TO_CRYSTALGOLEM,
		TO_INCUBUS,
		TO_VAMPIRE,
		TO_SHADOW,
		TO_COCKATRICE,
		TO_INSECTOID,
		TO_GOATMAN,
		TO_AUTOMATON,
		TO_LICHICE,
		TO_LICHFIRE,
		TO_SENTRYBOT,
		TO_SPELLBOT,
		TO_GYROBOT,
		TO_DUMMYBOT,
		TO_BUGBEAR,
		TO_MONSTER_D,
		TO_MONSTER_M,
		TO_MONSTER_S,
		TO_MONSTER_G,
		TO_REVENANT_SKULL,
		TO_MINIMIMIC,
		TO_ADORCISED_WEAPON,
		TO_FLAME_ELEMENTAL,
		TO_HOLOGRAM,
		TO_MOTH_SMALL,
		TO_EARTH_ELEMENTAL,
		TO_DUCK_SMALL,
		TO_MONSTER_UNUSED_6,
		TO_MONSTER_UNUSED_7,
		TO_MONSTER_UNUSED_8,
		TO_MONSTER_MAX,
		TO_BREAKABLE,
		TO_COLLIDER,
		TO_GOLD,
		TO_BELL
	};
	enum ScriptType : int
	{
		NO_SCRIPT,
		SCRIPT_NORMAL,
		SCRIPT_ATTACHED,
		SCRIPT_ATTACHED_FIRED
	};
	enum ScriptTriggeredBy : int
	{
		TRIGGER_POWER,
		TRIGGER_ATTACHED_ISREMOVED,
		TRIGGER_ATTACHED_EXISTS,
		TRIGGER_ATTACHED_INVIS,
		TRIGGER_ATTACHED_VISIBLE,
		TRIGGER_ATTACHED_ALWAYS,
		TRIGGER_ON_VARIABLE,
		TRIGGER_ATTACHED_INTERACTED
	};
	/*enum TagAvailableToEntity : int
	{
		AVAILABLE_ALL,
		CREATURES_ALL,
		CREATURES_MONSTERS,
		CREATURES_PLAYERS,
		ITEMS
	};*/
	bool containsOperator(char c);
	void eraseTag(std::string& script, std::string& scriptTag, size_t tagIndex);
	void updateClientInformation(int player, bool clearInventory, bool clearStats, ClientInformationType updateType);
	void playerClearInventory(bool clearStats);
	DynamicString getScriptFromEntity(Entity& src);
	void parseScriptInMapGeneration(Entity& src);
	Entity* createScriptEntityInMapGen(int x, int y, const char* text);
	void addScriptToTextSource(Entity& src, const char* text);
	void handleTextSourceScript(Entity& src, DynamicString input);
	int textSourceProcessScriptTag(std::string& input, std::string findTag, Entity& src);
	int textSourceProcessScriptTag(DynamicString& input, std::string findTag, Entity& src);
	bool hasClearedInventory = false;
	int getScriptType(Sint32 skill);
	int getAttachedToEntityType(Sint32 skill);
	int getTriggerType(Sint32 skill);
	void setScriptType(Sint32& skill, int setValue);
	void setAttachedToEntityType(Sint32& skill, int setValue);
	void setTriggerType(Sint32& skill, int setValue);
	DynamicArrayT<Entity*> getScriptAttachedEntities(Entity& script);
	DynamicMapI32 scriptVariables;
};
extern TextSourceScript textSourceScript;
