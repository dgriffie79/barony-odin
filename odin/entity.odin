// entity.odin — Odin mirrors of entity.hpp.
package main

import "containers"

// struct Dither (entity.hpp, used as DynamicMapPtrT value)
Dither_t :: struct {
	value:          i32,
	last_update_tick: u32,
}
#assert(size_of(Dither_t) == 8)

// struct State (game.hpp) — { double acceleration, velocity, position; } = 24B
State :: struct {
	acceleration: f64,
	velocity:     f64,
	position:     f64,
}
#assert(size_of(State) == 24)

// struct EntityStates (game.hpp) — 6 x State = 144B
Entity_States :: struct {
	x:     State,
	y:     State,
	z:     State,
	yaw:   State,
	pitch: State,
	roll:  State,
}
#assert(size_of(Entity_States) == 144)

// class Entity — 1432 bytes (field order from libclang dump of entity.hpp)
// The 446 former reference members are now getters in C++; in Odin they are
// accessor procs reading skill[]/fskill[] (generated below).
Entity :: struct {
	uid:                  u32,
	dithering_disabled:   bool,
	dithering_override:   i32,
	dithering:            map[rawptr]Dither_t, // DynamicMapPtrT<Dither_t> (ptr-keyed)
	light_bonus:          vec4_t,
	entity_sound:         rawptr, // void* (former FMOD/OpenAL channel)
	ticks:                u32,
	x:                    f64, // real_t
	y:                    f64,
	z:                    f64,
	yaw:                  f64,
	pitch:                f64,
	roll:                 f64,
	focalx:               f64,
	focaly:               f64,
	focalz:               f64,
	scalex:               f64,
	scaley:               f64,
	scalez:               f64,
	sizex:                i32,
	sizey:                i32,
	sprite:               i32,
	lastupdate:           u32,
	lastupdateserver:     u32,
	vel_x:                f64,
	vel_y:                f64,
	vel_z:                f64,
	new_x:                f64,
	new_y:                f64,
	new_z:                f64,
	new_yaw:              f64,
	new_pitch:            f64,
	new_roll:             f64,
	fskill:               [30]f64, // real_t fskill[NUMENTITYFSKILLS]
	skill:                [60]i32, // Sint32 skill[NUMENTITYSKILLS]
	flags:                [24]bool,
	string:               cstring, // char*
	light:                ^light_t,
	children:             list_t,
	parent:               u32,
	lerp_previous_state:  Entity_States, // TimerExperiments::EntityStates
	lerp_current_state:   Entity_States,
	lerp_render_state:    Entity_States,
	lerp_ox:              f64,
	lerp_oy:              f64,
	b_needs_render_position_init: bool,
	b_use_render_interpolation:   bool,
	map_generation_room_x: i32,
	map_generation_room_y: i32,
	entity_rng:           ^Barony_RNG,
	mynode:               ^node_t,
	my_creature_list_node: ^node_t,
	my_tile_list_node:    ^node_t,
	my_world_ui_list_node: ^node_t,
	path:                 ^list_t,
	client_stats:         ^Stat,
	clients_have_its_stats: bool,
	behavior:             proc(^Entity), // void (*behavior)(class Entity*)
	ranbehavior:          bool,
	bodyparts:            [dynamic]^Entity, // vector<Entity*> (non-owning)
	collision_ignore_targets: map[i32]struct{}, // DynamicSetI32 (32B)
}

#assert(size_of(Entity) == 1432)

// ---------------------------------------------------------------------------
// Accessor procs for the 446 former reference members (entity.hpp getters).
// C++: inline T& name() { return skill[N]; } -> Odin: entity_name(e) -> ^T
// ---------------------------------------------------------------------------
entity_actGibDisableDrawForLocalPlayer :: proc(e: ^Entity) -> ^i32 { return &e.skill[13] }
entity_actGibHitGroundEvent :: proc(e: ^Entity) -> ^i32 { return &e.skill[10] }
entity_actGibMagicParticle :: proc(e: ^Entity) -> ^i32 { return &e.skill[12] }
entity_actParticleWaveClientReceived :: proc(e: ^Entity) -> ^i32 { return &e.skill[10] }
entity_actParticleWaveLight :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_actParticleWaveMagicType :: proc(e: ^Entity) -> ^i32 { return &e.skill[9] }
entity_actParticleWaveStartFrame :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_actParticleWaveVariable1 :: proc(e: ^Entity) -> ^i32 { return &e.skill[11] }
entity_actRadiusMagicAutoPulseTick :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_actRadiusMagicDist :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_actRadiusMagicDoPulseTick :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_actRadiusMagicEffectPower :: proc(e: ^Entity) -> ^i32 { return &e.skill[8] }
entity_actRadiusMagicFollowUID :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_actRadiusMagicID :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_actRadiusMagicInit :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_actSpriteCheckParentExists :: proc(e: ^Entity) -> ^i32 { return &e.skill[8] }
entity_actSpriteFollowUID :: proc(e: ^Entity) -> ^i32 { return &e.skill[11] }
entity_actSpriteHasLightInit :: proc(e: ^Entity) -> ^i32 { return &e.skill[12] }
entity_actSpriteNoBillboard :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_actSpritePitchRotate :: proc(e: ^Entity) -> ^f64 { return &e.fskill[4] }
entity_actSpriteUseAlpha :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_actSpriteUseCustomSurface :: proc(e: ^Entity) -> ^i32 { return &e.skill[10] }
entity_actSpriteVelXY :: proc(e: ^Entity) -> ^i32 { return &e.skill[13] }
entity_actTrapSabotaged :: proc(e: ^Entity) -> ^i32 { return &e.skill[30] }
entity_actWindEffectsProjectiles :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_actWindLifetime :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_actWindParticleEffect :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_actWindStrength :: proc(e: ^Entity) -> ^f64 { return &e.fskill[0] }
entity_actWindTileBonusLength :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_actfloorMagicClientReceived :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_actfloorMagicType :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_actmagicAdditionalDamage :: proc(e: ^Entity) -> ^i32 { return &e.skill[38] }
entity_actmagicAllowFriendlyFireHit :: proc(e: ^Entity) -> ^i32 { return &e.skill[35] }
entity_actmagicCastByMagicstaff :: proc(e: ^Entity) -> ^i32 { return &e.skill[13] }
entity_actmagicCastByTinkerTrap :: proc(e: ^Entity) -> ^i32 { return &e.skill[22] }
entity_actmagicDelayMove :: proc(e: ^Entity) -> ^i32 { return &e.skill[30] }
entity_actmagicEmitter :: proc(e: ^Entity) -> ^i32 { return &e.skill[29] }
entity_actmagicFromSpellbook :: proc(e: ^Entity) -> ^i32 { return &e.skill[26] }
entity_actmagicIsOrbiting :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_actmagicIsVertical :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_actmagicMirrorReflected :: proc(e: ^Entity) -> ^i32 { return &e.skill[24] }
entity_actmagicMirrorReflectedCaster :: proc(e: ^Entity) -> ^i32 { return &e.skill[12] }
entity_actmagicNoHitMessage :: proc(e: ^Entity) -> ^i32 { return &e.skill[31] }
entity_actmagicNoLight :: proc(e: ^Entity) -> ^i32 { return &e.skill[33] }
entity_actmagicNoParticle :: proc(e: ^Entity) -> ^i32 { return &e.skill[32] }
entity_actmagicOrbitCastFromSpell :: proc(e: ^Entity) -> ^i32 { return &e.skill[20] }
entity_actmagicOrbitDist :: proc(e: ^Entity) -> ^i32 { return &e.skill[8] }
entity_actmagicOrbitHitTargetUID1 :: proc(e: ^Entity) -> ^i32 { return &e.skill[15] }
entity_actmagicOrbitHitTargetUID2 :: proc(e: ^Entity) -> ^i32 { return &e.skill[16] }
entity_actmagicOrbitHitTargetUID3 :: proc(e: ^Entity) -> ^i32 { return &e.skill[17] }
entity_actmagicOrbitHitTargetUID4 :: proc(e: ^Entity) -> ^i32 { return &e.skill[18] }
entity_actmagicOrbitLifetime :: proc(e: ^Entity) -> ^i32 { return &e.skill[10] }
entity_actmagicOrbitStartZ :: proc(e: ^Entity) -> ^f64 { return &e.fskill[3] }
entity_actmagicOrbitStationaryCurrentDist :: proc(e: ^Entity) -> ^f64 { return &e.fskill[6] }
entity_actmagicOrbitStationaryHitTarget :: proc(e: ^Entity) -> ^i32 { return &e.skill[14] }
entity_actmagicOrbitStationaryX :: proc(e: ^Entity) -> ^f64 { return &e.fskill[4] }
entity_actmagicOrbitStationaryY :: proc(e: ^Entity) -> ^f64 { return &e.fskill[5] }
entity_actmagicOrbitVerticalDirection :: proc(e: ^Entity) -> ^i32 { return &e.skill[9] }
entity_actmagicOrbitVerticalSpeed :: proc(e: ^Entity) -> ^f64 { return &e.fskill[2] }
entity_actmagicProjectileArc :: proc(e: ^Entity) -> ^i32 { return &e.skill[19] }
entity_actmagicReflectionCount :: proc(e: ^Entity) -> ^i32 { return &e.skill[25] }
entity_actmagicSpellbookBonus :: proc(e: ^Entity) -> ^i32 { return &e.skill[21] }
entity_actmagicSpray :: proc(e: ^Entity) -> ^i32 { return &e.skill[27] }
entity_actmagicSprayGravity :: proc(e: ^Entity) -> ^f64 { return &e.fskill[7] }
entity_actmagicTinkerTrapFriendlyFire :: proc(e: ^Entity) -> ^i32 { return &e.skill[23] }
entity_actmagicUpdateOLDHPOnHit :: proc(e: ^Entity) -> ^i32 { return &e.skill[34] }
entity_actmagicVelXStore :: proc(e: ^Entity) -> ^f64 { return &e.fskill[8] }
entity_actmagicVelYStore :: proc(e: ^Entity) -> ^f64 { return &e.fskill[9] }
entity_actmagicVelZStore :: proc(e: ^Entity) -> ^f64 { return &e.fskill[10] }
entity_arrowArmorPierce :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_arrowBoltDropOffRange :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_arrowDropOffEquipmentModifier :: proc(e: ^Entity) -> ^i32 { return &e.skill[14] }
entity_arrowFallSpeed :: proc(e: ^Entity) -> ^f64 { return &e.fskill[5] }
entity_arrowPoisonTime :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_arrowPower :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_arrowQuiverType :: proc(e: ^Entity) -> ^i32 { return &e.skill[8] }
entity_arrowShotByParent :: proc(e: ^Entity) -> ^i32 { return &e.skill[9] }
entity_arrowShotByWeapon :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_arrowSpeed :: proc(e: ^Entity) -> ^f64 { return &e.fskill[4] }
entity_boulderShatterEarthDamage :: proc(e: ^Entity) -> ^i32 { return &e.skill[17] }
entity_boulderShatterEarthSpell :: proc(e: ^Entity) -> ^i32 { return &e.skill[16] }
entity_boulderTrapAmbience :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_boulderTrapFired :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_boulderTrapPreDelay :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_boulderTrapRefireAmount :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_boulderTrapRefireCounter :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_boulderTrapRefireDelay :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_boulderTrapRocksToSpawn :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_ceilingTileAllowTrap :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_ceilingTileBreakable :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_ceilingTileDir :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_ceilingTileModel :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_chanceToPutOutFire :: proc(e: ^Entity) -> ^i32 { return &e.skill[37] }
entity_char_drunk :: proc(e: ^Entity) -> ^i32 { return &e.skill[24] }
entity_char_energize :: proc(e: ^Entity) -> ^i32 { return &e.skill[23] }
entity_char_fire :: proc(e: ^Entity) -> ^i32 { return &e.skill[36] }
entity_char_gonnavomit :: proc(e: ^Entity) -> ^i32 { return &e.skill[26] }
entity_char_heal :: proc(e: ^Entity) -> ^i32 { return &e.skill[22] }
entity_char_poison :: proc(e: ^Entity) -> ^i32 { return &e.skill[21] }
entity_char_torchtime :: proc(e: ^Entity) -> ^i32 { return &e.skill[25] }
entity_chestAmbience :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_chestHasVampireBook :: proc(e: ^Entity) -> ^i32 { return &e.skill[11] }
entity_chestHealth :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_chestInit :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_chestLidClicked :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_chestLocked :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_chestLockpickHealth :: proc(e: ^Entity) -> ^i32 { return &e.skill[12] }
entity_chestMaxHealth :: proc(e: ^Entity) -> ^i32 { return &e.skill[8] }
entity_chestMimicChance :: proc(e: ^Entity) -> ^i32 { return &e.skill[16] }
entity_chestOldHealth :: proc(e: ^Entity) -> ^i32 { return &e.skill[15] }
entity_chestOpener :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_chestPreventLockpickCapstoneExploit :: proc(e: ^Entity) -> ^i32 { return &e.skill[10] }
entity_chestStatus :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_chestType :: proc(e: ^Entity) -> ^i32 { return &e.skill[9] }
entity_chestVoidState :: proc(e: ^Entity) -> ^i32 { return &e.skill[17] }
entity_circuit_status :: proc(e: ^Entity) -> ^i32 { return &e.skill[28] }
entity_colliderContainedEntity :: proc(e: ^Entity) -> ^i32 { return &e.skill[15] }
entity_colliderCreatedParent :: proc(e: ^Entity) -> ^i32 { return &e.skill[20] }
entity_colliderCurrentHP :: proc(e: ^Entity) -> ^i32 { return &e.skill[12] }
entity_colliderDamageTypes :: proc(e: ^Entity) -> ^i32 { return &e.skill[11] }
entity_colliderDecorationHeightOffset :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_colliderDecorationModel :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_colliderDecorationRotation :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_colliderDecorationXOffset :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_colliderDecorationYOffset :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_colliderDiggable :: proc(e: ^Entity) -> ^i32 { return &e.skill[10] }
entity_colliderDropVariable :: proc(e: ^Entity) -> ^i32 { return &e.skill[25] }
entity_colliderHasCollision :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_colliderHideMonster :: proc(e: ^Entity) -> ^i32 { return &e.skill[16] }
entity_colliderInit :: proc(e: ^Entity) -> ^i32 { return &e.skill[14] }
entity_colliderIsMapGenerated :: proc(e: ^Entity) -> ^i32 { return &e.skill[22] }
entity_colliderKillerUid :: proc(e: ^Entity) -> ^i32 { return &e.skill[17] }
entity_colliderMaxHP :: proc(e: ^Entity) -> ^i32 { return &e.skill[9] }
entity_colliderOldHP :: proc(e: ^Entity) -> ^i32 { return &e.skill[13] }
entity_colliderSizeX :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_colliderSizeY :: proc(e: ^Entity) -> ^i32 { return &e.skill[8] }
entity_colliderSpellEvent :: proc(e: ^Entity) -> ^i32 { return &e.skill[18] }
entity_colliderSpellEventCooldown :: proc(e: ^Entity) -> ^i32 { return &e.skill[19] }
entity_colliderSpellEventTrigger :: proc(e: ^Entity) -> ^i32 { return &e.skill[21] }
entity_colliderSpellTarget :: proc(e: ^Entity) -> ^i32 { return &e.skill[23] }
entity_colliderTelepathy :: proc(e: ^Entity) -> ^i32 { return &e.skill[24] }
entity_creatureHoverZ :: proc(e: ^Entity) -> ^f64 { return &e.fskill[17] }
entity_creatureShadowTaggedThisUid :: proc(e: ^Entity) -> ^i32 { return &e.skill[54] }
entity_creatureWebbedSlowCount :: proc(e: ^Entity) -> ^i32 { return &e.skill[52] }
entity_creatureWindDir :: proc(e: ^Entity) -> ^f64 { return &e.fskill[15] }
entity_creatureWindVelocity :: proc(e: ^Entity) -> ^f64 { return &e.fskill[16] }
entity_crystalGeneratedElectricityNodes :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_crystalHoverDirection :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_crystalHoverWaitTimer :: proc(e: ^Entity) -> ^i32 { return &e.skill[8] }
entity_crystalInitialised :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_crystalMaxZVelocity :: proc(e: ^Entity) -> ^f64 { return &e.fskill[1] }
entity_crystalMinZVelocity :: proc(e: ^Entity) -> ^f64 { return &e.fskill[2] }
entity_crystalNumElectricityNodes :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_crystalSpellToActivate :: proc(e: ^Entity) -> ^i32 { return &e.skill[10] }
entity_crystalStartZ :: proc(e: ^Entity) -> ^f64 { return &e.fskill[0] }
entity_crystalTurnReverse :: proc(e: ^Entity) -> ^i32 { return &e.skill[9] }
entity_crystalTurnStartDir :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_crystalTurnVelocity :: proc(e: ^Entity) -> ^f64 { return &e.fskill[3] }
entity_crystalTurning :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_doorDir :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_doorDisableLockpicks :: proc(e: ^Entity) -> ^i32 { return &e.skill[12] }
entity_doorDisableOpening :: proc(e: ^Entity) -> ^i32 { return &e.skill[13] }
entity_doorForceLockedUnlocked :: proc(e: ^Entity) -> ^i32 { return &e.skill[11] }
entity_doorHealth :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_doorInit :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_doorLocked :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_doorLockpickHealth :: proc(e: ^Entity) -> ^i32 { return &e.skill[14] }
entity_doorMaxHealth :: proc(e: ^Entity) -> ^i32 { return &e.skill[9] }
entity_doorOldHealth :: proc(e: ^Entity) -> ^i32 { return &e.skill[15] }
entity_doorOldStatus :: proc(e: ^Entity) -> ^i32 { return &e.skill[8] }
entity_doorPreventLockpickExploit :: proc(e: ^Entity) -> ^i32 { return &e.skill[10] }
entity_doorSmacked :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_doorStartAng :: proc(e: ^Entity) -> ^f64 { return &e.fskill[0] }
entity_doorStatus :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_doorTimer :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_doorUnlockWhenPowered :: proc(e: ^Entity) -> ^i32 { return &e.skill[16] }
entity_effectPolymorph :: proc(e: ^Entity) -> ^i32 { return &e.skill[50] }
entity_effectShapeshift :: proc(e: ^Entity) -> ^i32 { return &e.skill[53] }
entity_entityShowOnMap :: proc(e: ^Entity) -> ^i32 { return &e.skill[59] }
entity_floorDecorationDestroyIfNoWall :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_floorDecorationDialogueProgress :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_floorDecorationHeightOffset :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_floorDecorationInteractText1 :: proc(e: ^Entity) -> ^i32 { return &e.skill[8] }
entity_floorDecorationInteractText2 :: proc(e: ^Entity) -> ^i32 { return &e.skill[9] }
entity_floorDecorationInteractText3 :: proc(e: ^Entity) -> ^i32 { return &e.skill[10] }
entity_floorDecorationInteractText4 :: proc(e: ^Entity) -> ^i32 { return &e.skill[11] }
entity_floorDecorationInteractText5 :: proc(e: ^Entity) -> ^i32 { return &e.skill[12] }
entity_floorDecorationInteractText6 :: proc(e: ^Entity) -> ^i32 { return &e.skill[13] }
entity_floorDecorationInteractText7 :: proc(e: ^Entity) -> ^i32 { return &e.skill[14] }
entity_floorDecorationInteractText8 :: proc(e: ^Entity) -> ^i32 { return &e.skill[15] }
entity_floorDecorationModel :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_floorDecorationRotation :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_floorDecorationXOffset :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_floorDecorationYOffset :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_furnitureDir :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_furnitureHealth :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_furnitureInit :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_furnitureMaxHealth :: proc(e: ^Entity) -> ^i32 { return &e.skill[9] }
entity_furnitureOldHealth :: proc(e: ^Entity) -> ^i32 { return &e.skill[15] }
entity_furnitureTableRandomItemChance :: proc(e: ^Entity) -> ^i32 { return &e.skill[10] }
entity_furnitureTableSpawnChairs :: proc(e: ^Entity) -> ^i32 { return &e.skill[11] }
entity_furnitureType :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_gateDisableOpening :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_gateInit :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_gateInverted :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_gateRattle :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_gateStartHeight :: proc(e: ^Entity) -> ^f64 { return &e.fskill[0] }
entity_gateStatus :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_gateVelZ :: proc(e: ^Entity) -> ^f64 { return &e.vel_z }
entity_goldAmbience :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_goldAmount :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_goldAmountBonus :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_goldBouncing :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_goldDroppedByPlayer :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_goldInContainer :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_goldSokoban :: proc(e: ^Entity) -> ^i32 { return &e.skill[2] }
entity_goldTelepathy :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_grayscaleGLRender :: proc(e: ^Entity) -> ^f64 { return &e.fskill[27] }
entity_highlightForUI :: proc(e: ^Entity) -> ^f64 { return &e.fskill[29] }
entity_highlightForUIGlow :: proc(e: ^Entity) -> ^f64 { return &e.fskill[28] }
entity_interactedByMonster :: proc(e: ^Entity) -> ^i32 { return &e.skill[47] }
entity_itemAutoSalvageByPlayer :: proc(e: ^Entity) -> ^i32 { return &e.skill[26] }
entity_itemContainer :: proc(e: ^Entity) -> ^i32 { return &e.skill[29] }
entity_itemDelayMonsterPickingUp :: proc(e: ^Entity) -> ^i32 { return &e.skill[24] }
entity_itemFollowUID :: proc(e: ^Entity) -> ^i32 { return &e.skill[30] }
entity_itemGerminateResult :: proc(e: ^Entity) -> ^i32 { return &e.skill[32] }
entity_itemLevitate :: proc(e: ^Entity) -> ^f64 { return &e.fskill[3] }
entity_itemLevitateStartZ :: proc(e: ^Entity) -> ^f64 { return &e.fskill[4] }
entity_itemNotMoving :: proc(e: ^Entity) -> ^i32 { return &e.skill[18] }
entity_itemNotMovingClient :: proc(e: ^Entity) -> ^i32 { return &e.skill[19] }
entity_itemOriginalOwner :: proc(e: ^Entity) -> ^i32 { return &e.skill[21] }
entity_itemReceivedDetailsFromServer :: proc(e: ^Entity) -> ^i32 { return &e.skill[25] }
entity_itemReturnUID :: proc(e: ^Entity) -> ^i32 { return &e.skill[31] }
entity_itemShowOnMap :: proc(e: ^Entity) -> ^i32 { return &e.skill[23] }
entity_itemSokobanReward :: proc(e: ^Entity) -> ^i32 { return &e.skill[20] }
entity_itemSplooshed :: proc(e: ^Entity) -> ^i32 { return &e.skill[27] }
entity_itemStolen :: proc(e: ^Entity) -> ^i32 { return &e.skill[22] }
entity_itemWaterBob :: proc(e: ^Entity) -> ^f64 { return &e.fskill[2] }
entity_leverStatus :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_leverTimerTicks :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_lightSourceAlwaysOn :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_lightSourceBrightness :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_lightSourceDelay :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_lightSourceDelayCounter :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_lightSourceFlicker :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_lightSourceInvertPower :: proc(e: ^Entity) -> ^i32 { return &e.skill[2] }
entity_lightSourceLatchOn :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_lightSourceRGB :: proc(e: ^Entity) -> ^i32 { return &e.skill[11] }
entity_lightSourceRadius :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_mistformGLRender :: proc(e: ^Entity) -> ^f64 { return &e.fskill[22] }
entity_monsterAllyClass :: proc(e: ^Entity) -> ^i32 { return &e.skill[46] }
entity_monsterAllyIndex :: proc(e: ^Entity) -> ^i32 { return &e.skill[42] }
entity_monsterAllyInteractTarget :: proc(e: ^Entity) -> ^i32 { return &e.skill[45] }
entity_monsterAllyPickupItems :: proc(e: ^Entity) -> ^i32 { return &e.skill[44] }
entity_monsterAllySpecial :: proc(e: ^Entity) -> ^i32 { return &e.skill[48] }
entity_monsterAllySpecialCooldown :: proc(e: ^Entity) -> ^i32 { return &e.skill[49] }
entity_monsterAllyState :: proc(e: ^Entity) -> ^i32 { return &e.skill[43] }
entity_monsterAllySummonRank :: proc(e: ^Entity) -> ^i32 { return &e.skill[50] }
entity_monsterAnimationLimbDirection :: proc(e: ^Entity) -> ^i32 { return &e.skill[20] }
entity_monsterAnimationLimbOvershoot :: proc(e: ^Entity) -> ^i32 { return &e.skill[30] }
entity_monsterArmbended :: proc(e: ^Entity) -> ^i32 { return &e.skill[10] }
entity_monsterAttack :: proc(e: ^Entity) -> ^i32 { return &e.skill[8] }
entity_monsterAttackTime :: proc(e: ^Entity) -> ^i32 { return &e.skill[9] }
entity_monsterDefend :: proc(e: ^Entity) -> ^i32 { return &e.skill[47] }
entity_monsterDevilNumSummons :: proc(e: ^Entity) -> ^i32 { return &e.skill[18] }
entity_monsterEntityRenderAsTelepath :: proc(e: ^Entity) -> ^i32 { return &e.skill[41] }
entity_monsterExtraReflexTick :: proc(e: ^Entity) -> ^i32 { return &e.skill[56] }
entity_monsterFearfulOfUid :: proc(e: ^Entity) -> ^i32 { return &e.skill[53] }
entity_monsterFootstepType :: proc(e: ^Entity) -> ^i32 { return &e.skill[32] }
entity_monsterHitTime :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_monsterIllusionTauntingThisUid :: proc(e: ^Entity) -> ^i32 { return &e.skill[55] }
entity_monsterKnockbackTangentDir :: proc(e: ^Entity) -> ^f64 { return &e.fskill[11] }
entity_monsterKnockbackUID :: proc(e: ^Entity) -> ^i32 { return &e.skill[51] }
entity_monsterKnockbackVelocity :: proc(e: ^Entity) -> ^f64 { return &e.fskill[9] }
entity_monsterLastDistractedByNoisemaker :: proc(e: ^Entity) -> ^i32 { return &e.skill[55] }
entity_monsterLichAllyStatus :: proc(e: ^Entity) -> ^i32 { return &e.skill[18] }
entity_monsterLichAllyUID :: proc(e: ^Entity) -> ^i32 { return &e.skill[17] }
entity_monsterLichBattleState :: proc(e: ^Entity) -> ^i32 { return &e.skill[27] }
entity_monsterLichFireMeleePrev :: proc(e: ^Entity) -> ^i32 { return &e.skill[35] }
entity_monsterLichFireMeleeSeq :: proc(e: ^Entity) -> ^i32 { return &e.skill[34] }
entity_monsterLichIceCastPrev :: proc(e: ^Entity) -> ^i32 { return &e.skill[35] }
entity_monsterLichIceCastSeq :: proc(e: ^Entity) -> ^i32 { return &e.skill[34] }
entity_monsterLichMagicCastCount :: proc(e: ^Entity) -> ^i32 { return &e.skill[37] }
entity_monsterLichMeleeSwingCount :: proc(e: ^Entity) -> ^i32 { return &e.skill[38] }
entity_monsterLichTeleportTimer :: proc(e: ^Entity) -> ^i32 { return &e.skill[40] }
entity_monsterLookDir :: proc(e: ^Entity) -> ^f64 { return &e.fskill[4] }
entity_monsterLookTime :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_monsterMoveTime :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_monsterPathBoundaryXEnd :: proc(e: ^Entity) -> ^i32 { return &e.skill[16] }
entity_monsterPathBoundaryXStart :: proc(e: ^Entity) -> ^i32 { return &e.skill[14] }
entity_monsterPathBoundaryYEnd :: proc(e: ^Entity) -> ^i32 { return &e.skill[17] }
entity_monsterPathBoundaryYStart :: proc(e: ^Entity) -> ^i32 { return &e.skill[15] }
entity_monsterPathCount :: proc(e: ^Entity) -> ^i32 { return &e.skill[38] }
entity_monsterSentrybotLookDir :: proc(e: ^Entity) -> ^f64 { return &e.fskill[10] }
entity_monsterShadowDontChangeName :: proc(e: ^Entity) -> ^i32 { return &e.skill[35] }
entity_monsterShadowInitialMimic :: proc(e: ^Entity) -> ^i32 { return &e.skill[34] }
entity_monsterSlimeLastAttack :: proc(e: ^Entity) -> ^i32 { return &e.skill[34] }
entity_monsterSpecialAttackUnequipSafeguard :: proc(e: ^Entity) -> ^f64 { return &e.fskill[14] }
entity_monsterSpecialState :: proc(e: ^Entity) -> ^i32 { return &e.skill[33] }
entity_monsterSpecialTimer :: proc(e: ^Entity) -> ^i32 { return &e.skill[29] }
entity_monsterSpellAnimation :: proc(e: ^Entity) -> ^i32 { return &e.skill[31] }
entity_monsterState :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_monsterStoreType :: proc(e: ^Entity) -> ^i32 { return &e.skill[18] }
entity_monsterStrafeDirection :: proc(e: ^Entity) -> ^i32 { return &e.skill[39] }
entity_monsterTarget :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_monsterTargetX :: proc(e: ^Entity) -> ^f64 { return &e.fskill[2] }
entity_monsterTargetY :: proc(e: ^Entity) -> ^f64 { return &e.fskill[3] }
entity_monsterWeaponYaw :: proc(e: ^Entity) -> ^f64 { return &e.fskill[5] }
entity_noColorChangeAllyLimb :: proc(e: ^Entity) -> ^f64 { return &e.fskill[26] }
entity_orbHoverDirection :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_orbHoverWaitTimer :: proc(e: ^Entity) -> ^i32 { return &e.skill[8] }
entity_orbInitialised :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_orbMaxZVelocity :: proc(e: ^Entity) -> ^f64 { return &e.fskill[1] }
entity_orbMinZVelocity :: proc(e: ^Entity) -> ^f64 { return &e.fskill[2] }
entity_orbStartZ :: proc(e: ^Entity) -> ^f64 { return &e.fskill[0] }
entity_orbTurnVelocity :: proc(e: ^Entity) -> ^f64 { return &e.fskill[3] }
entity_particleDuration :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_particleShrink :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_particleTimerCountdownAction :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_particleTimerCountdownSprite :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_particleTimerDuration :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_particleTimerEffectLifetime :: proc(e: ^Entity) -> ^i32 { return &e.skill[10] }
entity_particleTimerEndAction :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_particleTimerEndSprite :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_particleTimerPreDelay :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_particleTimerTarget :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_particleTimerVariable1 :: proc(e: ^Entity) -> ^i32 { return &e.skill[8] }
entity_particleTimerVariable2 :: proc(e: ^Entity) -> ^i32 { return &e.skill[9] }
entity_particleTimerVariable3 :: proc(e: ^Entity) -> ^i32 { return &e.skill[11] }
entity_particleTimerVariable4 :: proc(e: ^Entity) -> ^i32 { return &e.skill[12] }
entity_pedestalAmbience :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_pedestalHasOrb :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_pedestalInGround :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_pedestalInit :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_pedestalInvertedPower :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_pedestalLockOrb :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_pedestalOrbType :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_pedestalPowerStatus :: proc(e: ^Entity) -> ^i32 { return &e.skill[8] }
entity_pistonCamDir :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_pistonCamRotateSpeed :: proc(e: ^Entity) -> ^f64 { return &e.fskill[0] }
entity_pistonCamTimer :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_playerAliveTime :: proc(e: ^Entity) -> ^i32 { return &e.skill[12] }
entity_playerAutomatonDeathCounter :: proc(e: ^Entity) -> ^i32 { return &e.skill[15] }
entity_playerCastTimeAnim :: proc(e: ^Entity) -> ^i32 { return &e.skill[17] }
entity_playerCreatedDeathCam :: proc(e: ^Entity) -> ^i32 { return &e.skill[16] }
entity_playerLevelEntrySpeech :: proc(e: ^Entity) -> ^i32 { return &e.skill[18] }
entity_playerStartDir :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_playerStrafeDir :: proc(e: ^Entity) -> ^f64 { return &e.fskill[13] }
entity_playerStrafeVelocity :: proc(e: ^Entity) -> ^f64 { return &e.fskill[12] }
entity_playerVampireCurse :: proc(e: ^Entity) -> ^i32 { return &e.skill[51] }
entity_portalAmbience :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_portalCustomLevelText1 :: proc(e: ^Entity) -> ^i32 { return &e.skill[11] }
entity_portalCustomLevelText2 :: proc(e: ^Entity) -> ^i32 { return &e.skill[12] }
entity_portalCustomLevelText3 :: proc(e: ^Entity) -> ^i32 { return &e.skill[13] }
entity_portalCustomLevelText4 :: proc(e: ^Entity) -> ^i32 { return &e.skill[14] }
entity_portalCustomLevelText5 :: proc(e: ^Entity) -> ^i32 { return &e.skill[15] }
entity_portalCustomLevelText6 :: proc(e: ^Entity) -> ^i32 { return &e.skill[16] }
entity_portalCustomLevelText7 :: proc(e: ^Entity) -> ^i32 { return &e.skill[17] }
entity_portalCustomLevelText8 :: proc(e: ^Entity) -> ^i32 { return &e.skill[18] }
entity_portalCustomLevelsToJump :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_portalCustomRequiresPower :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_portalCustomSprite :: proc(e: ^Entity) -> ^i32 { return &e.skill[8] }
entity_portalCustomSpriteAnimationFrames :: proc(e: ^Entity) -> ^i32 { return &e.skill[9] }
entity_portalCustomZOffset :: proc(e: ^Entity) -> ^i32 { return &e.skill[10] }
entity_portalFireAnimation :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_portalInit :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_portalNotSecret :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_portalVictoryType :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_pressurePlateTriggerType :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_shrineActivateDelay :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_shrineAmbience :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_shrineDaedalusState :: proc(e: ^Entity) -> ^i32 { return &e.skill[11] }
entity_shrineDestXOffset :: proc(e: ^Entity) -> ^i32 { return &e.skill[9] }
entity_shrineDestYOffset :: proc(e: ^Entity) -> ^i32 { return &e.skill[10] }
entity_shrineDir :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_shrineInit :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_shrineRefire1 :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_shrineRefire2 :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_shrineSpellEffect :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_shrineZ :: proc(e: ^Entity) -> ^i32 { return &e.skill[8] }
entity_signalActivateDelay :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_signalGateANDPowerCount :: proc(e: ^Entity) -> ^i32 { return &e.skill[9] }
entity_signalInputDirection :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_signalInvertOutput :: proc(e: ^Entity) -> ^i32 { return &e.skill[10] }
entity_signalTimerInterval :: proc(e: ^Entity) -> ^i32 { return &e.skill[2] }
entity_signalTimerLatchInput :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_signalTimerRepeatCount :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_soundSourceDelay :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_soundSourceDelayCounter :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_soundSourceFired :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_soundSourceLatchOn :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_soundSourceOrigin :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_soundSourceToPlay :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_soundSourceVolume :: proc(e: ^Entity) -> ^i32 { return &e.skill[2] }
entity_spellTrapAmbience :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_spellTrapCounter :: proc(e: ^Entity) -> ^i32 { return &e.skill[8] }
entity_spellTrapFloorTile :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_spellTrapInit :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_spellTrapLatchPower :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_spellTrapRefire :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_spellTrapRefireRate :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_spellTrapReset :: proc(e: ^Entity) -> ^i32 { return &e.skill[9] }
entity_spellTrapType :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_statueDir :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_statueId :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_statueInit :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_switch_power :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_teleporterAmbience :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_teleporterCurrentFrame :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_teleporterDuration :: proc(e: ^Entity) -> ^i32 { return &e.skill[8] }
entity_teleporterNumFrames :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_teleporterStartFrame :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_teleporterType :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_teleporterX :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_teleporterY :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_textSourceBegin :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_textSourceColorRGB :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_textSourceDelay :: proc(e: ^Entity) -> ^i32 { return &e.skill[2] }
entity_textSourceIsScript :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_textSourceVariables4W :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_thrownProjectileCharge :: proc(e: ^Entity) -> ^i32 { return &e.skill[20] }
entity_thrownProjectileParticleTimerUID :: proc(e: ^Entity) -> ^i32 { return &e.skill[9] }
entity_thrownProjectilePower :: proc(e: ^Entity) -> ^i32 { return &e.skill[19] }
entity_wallLockAutoGenKey :: proc(e: ^Entity) -> ^i32 { return &e.skill[15] }
entity_wallLockClientInteractDelay :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_wallLockDir :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_wallLockInit :: proc(e: ^Entity) -> ^i32 { return &e.skill[9] }
entity_wallLockInvertPower :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_wallLockMaterial :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_wallLockPickHealth :: proc(e: ^Entity) -> ^i32 { return &e.skill[12] }
entity_wallLockPickable :: proc(e: ^Entity) -> ^i32 { return &e.skill[11] }
entity_wallLockPickableSkeletonKey :: proc(e: ^Entity) -> ^i32 { return &e.skill[13] }
entity_wallLockPlayerInteracting :: proc(e: ^Entity) -> ^i32 { return &e.skill[7] }
entity_wallLockPower :: proc(e: ^Entity) -> ^i32 { return &e.skill[8] }
entity_wallLockPreventLockpickExploit :: proc(e: ^Entity) -> ^i32 { return &e.skill[14] }
entity_wallLockState :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_wallLockTimer :: proc(e: ^Entity) -> ^i32 { return &e.skill[10] }
entity_wallLockTurnable :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_worldTooltipActive :: proc(e: ^Entity) -> ^i32 { return &e.skill[0] }
entity_worldTooltipAlpha :: proc(e: ^Entity) -> ^f64 { return &e.fskill[0] }
entity_worldTooltipFadeDelay :: proc(e: ^Entity) -> ^i32 { return &e.skill[4] }
entity_worldTooltipIgnoreDrawing :: proc(e: ^Entity) -> ^i32 { return &e.skill[5] }
entity_worldTooltipInit :: proc(e: ^Entity) -> ^i32 { return &e.skill[3] }
entity_worldTooltipPlayer :: proc(e: ^Entity) -> ^i32 { return &e.skill[1] }
entity_worldTooltipRequiresButtonHeld :: proc(e: ^Entity) -> ^i32 { return &e.skill[6] }
entity_worldTooltipZ :: proc(e: ^Entity) -> ^f64 { return &e.fskill[1] }
