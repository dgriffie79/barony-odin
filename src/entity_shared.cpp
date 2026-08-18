/*-------------------------------------------------------------------------------

BARONY
File: entity_shared.cpp
Desc: functions to be shared between editor.exe and barony.exe

Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
See LICENSE for details.

-------------------------------------------------------------------------------*/


#include "entity.hpp"
Entity::Entity(Sint32 in_sprite, Uint32 pos, list_t* entlist, list_t* creaturelist) :
	lightBonus(0.f)
// shares with above as above only applies to inner demons.





























































































































































































































































































{
	int c;
	// add the entity to the entity list
	if ( !pos )
	{
		mynode = list_AddNodeFirst(entlist);
	}
	else
	{
		mynode = list_AddNodeLast(entlist);
	}

	if ( mynode )
	{
		mynode->element = this;
		mynode->deconstructor = &entityDeconstructor;
		mynode->size = sizeof(Entity);
	}

	myCreatureListNode = nullptr;
	if ( creaturelist )
	{
		addToCreatureList(creaturelist);
	}
	myWorldUIListNode = nullptr;
	myTileListNode = nullptr;

	// now reset all of my data elements
	lastupdate = 0;
	lastupdateserver = 0;
	ticks = 0;
	x = 0;
	y = 0;
	z = 0;
	new_x = 0;
	new_y = 0;
	new_z = 0;
	focalx = 0;
	focaly = 0;
	focalz = 0;
	scalex = 1;
	scaley = 1;
	scalez = 1;
	vel_x = 0;
	vel_y = 0;
	vel_z = 0;
	sizex = 0;
	sizey = 0;
	yaw = 0;
	pitch = 0;
	roll = 0;
	new_yaw = 0;
	new_pitch = 0;
	new_roll = 0;
#ifndef EDITOR
	lerpCurrentState.resetMovement();
	lerpCurrentState.resetPosition();
	lerpPreviousState.resetMovement();
	lerpPreviousState.resetPosition();
	lerpRenderState.resetMovement();
	lerpRenderState.resetPosition();
#endif
	bNeedsRenderPositionInit = true;
	bUseRenderInterpolation = false;
	mapGenerationRoomX = 0;
	mapGenerationRoomY = 0;
	lerp_ox = 0.0;
	lerp_oy = 0.0;
	sprite = in_sprite;
	light = nullptr;
	string = nullptr;
	children.first = nullptr;
	children.last = nullptr;
	//this->magic_effects = (list_t *) malloc(sizeof(list_t));
	//this->magic_effects->first = NULL; this->magic_effects->last = NULL;
	for ( c = 0; c < NUMENTITYSKILLS; ++c )
	{
		skill[c] = 0;
	}
	for ( c = 0; c < NUMENTITYFSKILLS; ++c )
	{
		fskill[c] = 0;
	}
	skill[2] = -1;
	for ( c = 0; c < 24; c++ )
	{
		flags[c] = false;
	}
	if ( entlist != nullptr && entlist == map.entities )
	{
		if ( multiplayer != CLIENT || loading )
		{
			uid = entity_uids;
			entity_uids++;
			map.entities_map.put(uid, mynode);
		}
		else
		{
			uid = -2;
		}
	}
	else
	{
		uid = -2;
	}
	behavior = nullptr;
	ranbehavior = false;
	parent = 0;
	path = nullptr;
	monsterAllyIndex() = -1; // set to -1 to not reference player indices 0-3.
	/*if ( checkSpriteType(this->sprite) > 1 )
	{
		setSpriteAttributes(this, nullptr, nullptr);
	}*/

	clientStats = nullptr;
	clientsHaveItsStats = false;
}

Monster editorSpriteTypeToMonster(Sint32 sprite)
{
	Monster monsterType = NOTHING;
	switch ( sprite )
	{
	case 27: monsterType = HUMAN; break;
	case 30: monsterType = TROLL; break;
	case 35: monsterType = SHOPKEEPER; break;
	case 36: monsterType = GOBLIN; break;
	case 48: monsterType = SPIDER; break;
	case 62: monsterType = LICH; break;
	case 70: monsterType = GNOME; break;
	case 71: monsterType = DEVIL; break;
	case 75: monsterType = DEMON; break;
	case 76: monsterType = CREATURE_IMP; break;
	case 77: monsterType = MINOTAUR; break;
	case 78: monsterType = SCORPION; break;
	case 79: monsterType = SLIME; break;
	case 193: monsterType = SLIME; break;
	case 194: monsterType = SLIME; break;
	case 195: monsterType = SLIME; break;
	case 196: monsterType = SLIME; break;
	case 197: monsterType = SLIME; break;
	case 80: monsterType = SUCCUBUS; break;
	case 81: monsterType = RAT; break;
	case 82: monsterType = GHOUL; break;
	case 83: monsterType = SKELETON; break;
	case 84: monsterType = KOBOLD; break;
	case 85: monsterType = SCARAB; break;
	case 86: monsterType = CRYSTALGOLEM; break;
	case 87: monsterType = INCUBUS; break;
	case 88: monsterType = VAMPIRE; break;
	case 89: monsterType = SHADOW; break;
	case 90: monsterType = COCKATRICE; break;
	case 91: monsterType = INSECTOID; break;
	case 92: monsterType = GOATMAN; break;
	case 93: monsterType = AUTOMATON; break;
	case 94: monsterType = LICH_ICE; break;
	case 95: monsterType = LICH_FIRE; break;
	case 163: monsterType = SENTRYBOT; break;
	case 164: monsterType = SPELLBOT; break;
	case 165: monsterType = DUMMYBOT; break;
	case 166: monsterType = GYROBOT; break;
	case 188: monsterType = BAT_SMALL; break;
	case 189: monsterType = BUGBEAR; break;
	case 204: monsterType = DRYAD; break;
	case 205: monsterType = MYCONID; break;
	case 206: monsterType = SALAMANDER; break;
	case 207: monsterType = GREMLIN; break;
	case 246: monsterType = REVENANT_SKULL; break;
	case 247: monsterType = MONSTER_ADORCISED_WEAPON; break;
	default:
		break;
	}
	return monsterType;
}

int checkSpriteType(Sint32 sprite)
{
	switch ( sprite )
	{
	case 71:
	case 70:
	case 62:
	case 48:
	case 36:
	case 35:
	case 30:
	case 27:
	case 10:
	case 83:
	case 84:
	case 85:
	case 86:
	case 87:
	case 88:
	case 89:
	case 90:
	case 91:
	case 92:
	case 93:
	case 94:
	case 95:
	case 75:
	case 76:
	case 77:
	// to test case 37
	case 37:
	case 78:
	case 79:
	case 80:
	case 81:
	case 82:
	case 163:
	case 164:
	case 165:
	case 166:
	case 188:
	case 189:
	case 193:
	case 194:
	case 195:
	case 196:
	case 197:
	case 204:
	case 205:
	case 206:
	case 207:
	case 246:
	case 247:
		//monsters
		return 1;
		break;
	case 21:
		//chest
		return 2;
		break;
	case 8:
		//items
		return 3;
		break;
	case 97:
		//summon trap
		return 4;
		break;
	case 106:
		//power crystal
		return 5;
		break;
	case 115:
		// lever timer
		return 6;
	case 102:
	case 103:
	case 104:
	case 105:
		//boulder traps
		return 7;
		break;
	case 116:
		//pedestal
		return 8;
		break;
	case 118:
		//teleporter
		return 9;
		break;
	case 119:
		//ceiling tile model
		return 10;
		break;
	case 120:
		//magic ceiling trap
		return 11;
		break;
	case 121:
	case 122:
	case 123:
	case 124:
	case 125:
	case 60:
		// general furniture/misc.
		return 12;
		break;
	case 127:
		// floor decoration
		return 13;
		break;
	case 130:
		// sound source
		return 14;
	case 131:
		// light source
		return 15;
	case 132:
		// text source
		return 16;
	case 133:
		// signal modifier
		return 17;
	case 161:
		// custom exit
		return 18;
	case 59:
		// table
		return 19;
	case 162: 
		// readablebook
		return 20;
	case 2:
	case 3:
		return 21;
	case 19:
	case 20:
	case 113:
	case 114:
		return 22;
	case 1:
		return 23;
	case 169:
		// statue
		return 24;
	case 177:
		// teleport shrine
		return 25;
	case 178:
		// generic spell shrine
		return 26;
	case 179:
		return 27;
	case 185:
	case 186:
	case 187:
		// AND gate
		return 28;
	case 33:
	case 34:
		// act trap
		return 29;
	case 208:
	case 209:
	case 210:
	case 211:
		// wall locks
		return 30;
	case 212:
	case 213:
	case 214:
	case 215:
		// wall buttons
		return 31;
	case 217:
	case 218:
		// iron doors
		return 32;
	case 220:
		// wind
		return 33;
	default:
		return 0;
		break;
	}

	return 0;
}

int canWearEquip(Entity* entity, int category)
{
	Stat* stats;
	int equipType = 0;
	int type;
	if ( entity != NULL )
	{
		stats = entity->getStats();
		if ( stats != NULL )
		{
			type = stats->type;

			switch ( type )
			{
				//monsters that don't wear equipment (only rings/amulets)
				case DEVIL:
				case SPIDER:
				case TROLL:
				case RAT:
				case SLIME:
				case SCORPION:
				case MINOTAUR:
				case GHOUL:
				case SCARAB:
				case CRYSTALGOLEM:
				case COCKATRICE:
				case MIMIC:
				case MINIMIMIC:
				case REVENANT_SKULL:
				case FLAME_ELEMENTAL:
				case EARTH_ELEMENTAL:
				case HOLOGRAM:
				case MOTH_SMALL:
					equipType = 0;
					break;

				//monsters with weapons only (incl. spellbooks)
				case LICH:
				case CREATURE_IMP:
				case DEMON:
				case MONSTER_ADORCISED_WEAPON:
					equipType = 1;
					break;

				//monsters with cloak/weapon/shield/boots/mask/gloves (no helm)
				case BUGBEAR:
				case INCUBUS:
				case SUCCUBUS:
				case LICH_FIRE:
				case LICH_ICE:
					equipType = 2;
					break;

				//monsters with cloak/weapon/shield/boots/helm/armor/mask/gloves
				case GNOME:
				case GOBLIN:
				case HUMAN:
				case VAMPIRE:
				case SKELETON:
				case SHOPKEEPER:
				case SHADOW:
				case AUTOMATON:
				case GOATMAN:
				case KOBOLD:
				case INSECTOID:
				case DRYAD:
				case MYCONID:
				case SALAMANDER:
				case GREMLIN:
					equipType = 3;
					break;

				default:
					equipType = 0;
					break;
			}
		}
	}

	if ( category == 0 && equipType >= 3 ) //HELM
	{
		return 1;
	}
	else if ( category == 1 && equipType >= 1 ) //WEAPON
	{
		return 1;
	}
	else if ( category == 2 && equipType >= 2 ) //SHIELD
	{
		return 1;
	}
	else if ( category == 3 && equipType >= 3 ) //ARMOR
	{
		return 1;
	}
	else if ( category == 4 && equipType >= 2 ) //BOOTS
	{
		return 1;
	}
	else if ( category == 5 || category == 6 )  //RINGS/AMULETS WORN BY ALL
	{
		return 1;
	}
	else if ( (category >= 7 && category <= 9) && equipType >= 2 ) //CLOAK/MASK/GLOVES
	{
		return 1;
	}
	else
	{
		return 0;
	}

	return 0;
}

void setSpriteAttributes(Entity* entityNew, Entity* entityToCopy, Entity* entityStatToCopy)
{
	Stat* tmpStats = nullptr;
	if ( !entityNew )
	{
		return;
	}

	if ( entityStatToCopy != nullptr )
	{
		tmpStats = entityStatToCopy->getStats();
	}

	int spriteType = checkSpriteType(entityNew->sprite);
	// monsters.
	if ( spriteType == 1 )
	{
		//STAT ASSIGNMENT
		Stat* myStats = nullptr;
		if ( multiplayer != CLIENT )
		{
			// need to give the entity its list stuff.
			// create an empty first node for traversal purposes
			node_t* node2 = list_AddNodeFirst(&entityNew->children);
			node2->element = nullptr;
			node2->deconstructor = &emptyDeconstructor;

			node2 = list_AddNodeLast(&entityNew->children);
			if ( tmpStats != nullptr )
			{
				node2->element = tmpStats->copyStats();
				node2->size = sizeof(tmpStats);
			}
			else
			{
				// if the previous sprite did not have stats initialised, or creating a new entity.
				myStats = new Stat(entityNew->sprite);
				node2->element = myStats;
				node2->size = sizeof(myStats);
			}
			node2->deconstructor = &statDeconstructor;
		}
	}
	// chests.
	else if ( spriteType == 2 )
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->yaw = entityToCopy->yaw;
			entityNew->skill[9] = entityToCopy->skill[9];
			entityNew->chestLocked() = entityToCopy->chestLocked();
			entityNew->chestMimicChance() = entityToCopy->chestMimicChance();
		}
		else
		{
			// set default new entity attributes.
			entityNew->yaw = 1;
			entityNew->skill[9] = 0;
			entityNew->chestLocked() = -1;
			entityNew->chestMimicChance() = -1;
		}
	}
	// items.
	else if ( spriteType == 3 )
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->skill[10] = entityToCopy->skill[10];
			entityNew->skill[11] = entityToCopy->skill[11];
			entityNew->skill[12] = entityToCopy->skill[12];
			entityNew->skill[13] = entityToCopy->skill[13];
			entityNew->skill[15] = entityToCopy->skill[15];
			entityNew->skill[16] = entityToCopy->skill[16];
		}
		else
		{
			// set default new entity attributes.
			entityNew->skill[10] = 1;
			entityNew->skill[11] = 0;
			entityNew->skill[12] = 10;
			entityNew->skill[13] = 1;
			entityNew->skill[15] = 0;
			entityNew->skill[16] = 0;
		}
	}
	// summoning trap.
	else if ( spriteType == 4 )
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->skill[0] = entityToCopy->skill[0];
			entityNew->skill[1] = entityToCopy->skill[1];
			entityNew->skill[2] = entityToCopy->skill[2];
			entityNew->skill[3] = entityToCopy->skill[3];
			entityNew->skill[4] = entityToCopy->skill[4];
			entityNew->skill[5] = entityToCopy->skill[5];
			entityNew->skill[9] = entityToCopy->skill[9];
		}
		else
		{
			// set default new entity attributes.
			entityNew->skill[0] = 0;
			entityNew->skill[1] = 1;
			entityNew->skill[2] = 1;
			entityNew->skill[3] = 1;
			entityNew->skill[4] = 0;
			entityNew->skill[5] = 0;
			entityNew->skill[9] = 0;
		}
	}
	// power crystal
	else if ( spriteType == 5 )
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->yaw = entityToCopy->yaw;
			entityNew->crystalNumElectricityNodes() = entityToCopy->crystalNumElectricityNodes();
			entityNew->crystalTurnReverse() = entityToCopy->crystalTurnReverse();
			entityNew->crystalSpellToActivate() = entityToCopy->crystalSpellToActivate();
		}
		else
		{
			// set default new entity attributes.
			entityNew->yaw = 0;
			entityNew->crystalNumElectricityNodes() = 5;
			entityNew->crystalTurnReverse() = 0;
			entityNew->crystalSpellToActivate() = 0;
		}
	}
	// lever timer
	else if ( spriteType == 6 )
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->leverTimerTicks() = entityToCopy->leverTimerTicks();
		}
		else
		{
			// set default new entity attributes.
			entityNew->leverTimerTicks() = 3;
		}
	}
	// boulder trap with re-fire
	else if ( spriteType == 7 )
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->boulderTrapRefireDelay() = entityToCopy->boulderTrapRefireDelay();
			entityNew->boulderTrapRefireAmount() = entityToCopy->boulderTrapRefireAmount();
			entityNew->boulderTrapPreDelay() = entityToCopy->boulderTrapPreDelay();
		}
		else
		{
			// set default new entity attributes.
			entityNew->boulderTrapRefireDelay() = 3;
			entityNew->boulderTrapRefireAmount() = 0;
			entityNew->boulderTrapPreDelay() = 0;
		}
	}
	// pedestal
	else if ( spriteType == 8 )
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->pedestalOrbType() = entityToCopy->pedestalOrbType();
			entityNew->pedestalHasOrb() = entityToCopy->pedestalHasOrb();
			entityNew->pedestalInvertedPower() = entityToCopy->pedestalInvertedPower();
			entityNew->pedestalInGround() = entityToCopy->pedestalInGround();
			entityNew->pedestalLockOrb() = entityToCopy->pedestalLockOrb();
		}
		else
		{
			// set default new entity attributes.
			entityNew->pedestalOrbType() = 0;
			entityNew->pedestalHasOrb() = 0;
			entityNew->pedestalInvertedPower() = 0;
			entityNew->pedestalInGround() = 0;
			entityNew->pedestalLockOrb() = 0;
		}
	}
	// teleporter
	else if ( spriteType == 9 )
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->teleporterX() = entityToCopy->teleporterX();
			entityNew->teleporterY() = entityToCopy->teleporterY();
			entityNew->teleporterType() = entityToCopy->teleporterType();
		}
		else
		{
			// set default new entity attributes.
			entityNew->teleporterX() = 1;
			entityNew->teleporterY() = 1;
			entityNew->teleporterType() = 0;
		}
	}
	// ceiling tile
	else if ( spriteType == 10 )
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->ceilingTileModel() = entityToCopy->ceilingTileModel();
			entityNew->ceilingTileDir() = entityToCopy->ceilingTileDir();
			entityNew->ceilingTileAllowTrap() = entityToCopy->ceilingTileAllowTrap();
			entityNew->ceilingTileBreakable() = entityToCopy->ceilingTileBreakable();
		}
		else
		{
			// set default new entity attributes.
			entityNew->ceilingTileModel() = 0;
			entityNew->ceilingTileDir() = 0;
			entityNew->ceilingTileAllowTrap() = 0;
			entityNew->ceilingTileBreakable() = 0;
		}
	}
	// spell trap
	else if ( spriteType == 11 )
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->spellTrapType() = entityToCopy->spellTrapType();
			entityNew->spellTrapRefire() = entityToCopy->spellTrapRefire();
			entityNew->spellTrapLatchPower() = entityToCopy->spellTrapLatchPower();
			entityNew->spellTrapFloorTile() = entityToCopy->spellTrapFloorTile();
			entityNew->spellTrapRefireRate() = entityToCopy->spellTrapRefireRate();
		}
		else
		{
			// set default new entity attributes.
			// copy old entity attributes to newly created.
			entityNew->spellTrapType() = -1;
			entityNew->spellTrapRefire() = -1;
			entityNew->spellTrapLatchPower() = 0;
			entityNew->spellTrapFloorTile() = 0;
			entityNew->spellTrapRefireRate() = 1;
		}
	}
	// furniture
	else if ( spriteType == 12 )
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->furnitureDir() = entityToCopy->furnitureDir();
		}
		else
		{
			// set default new entity attributes.
			if ( entityNew->sprite == 60 ) // chair
			{
				entityNew->furnitureDir() = -1;
			}
			else
			{
				entityNew->furnitureDir() = 0;
			}
		}
	}
	// floor decoration
	else if ( spriteType == 13 )
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->floorDecorationModel() = entityToCopy->floorDecorationModel();
			entityNew->floorDecorationRotation() = entityToCopy->floorDecorationRotation();
			entityNew->floorDecorationHeightOffset() = entityToCopy->floorDecorationHeightOffset();
			entityNew->floorDecorationXOffset() = entityToCopy->floorDecorationXOffset();
			entityNew->floorDecorationYOffset() = entityToCopy->floorDecorationYOffset();
			entityNew->floorDecorationDestroyIfNoWall() = entityToCopy->floorDecorationDestroyIfNoWall();
			for ( int i = 8; i < 60; ++i )
			{
				entityNew->skill[i] = entityToCopy->skill[i];
			}
		}
		else
		{
			// set default new entity attributes.
			entityNew->floorDecorationModel() = 0;
			entityNew->floorDecorationRotation() = 0;
			entityNew->floorDecorationHeightOffset() = 0;
			entityNew->floorDecorationXOffset() = 0;
			entityNew->floorDecorationYOffset() = 0;
			entityNew->floorDecorationDestroyIfNoWall() = -1;
			for ( int i = 8; i < 60; ++i )
			{
				entityNew->skill[i] = 0;
			}
		}
	}
	else if ( spriteType == 14 )
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->soundSourceToPlay() = entityToCopy->soundSourceToPlay();
			entityNew->soundSourceVolume() = entityToCopy->soundSourceVolume();
			entityNew->soundSourceLatchOn() = entityToCopy->soundSourceLatchOn();
			entityNew->soundSourceDelay() = entityToCopy->soundSourceDelay();
			entityNew->soundSourceOrigin() = entityToCopy->soundSourceOrigin();
		}
		else
		{
			// set default new entity attributes.
			entityNew->soundSourceToPlay() = 0;
			entityNew->soundSourceVolume() = 0;
			entityNew->soundSourceLatchOn() = 0;
			entityNew->soundSourceDelay() = 0;
			entityNew->soundSourceOrigin() = 0;
		}
	}
	else if ( spriteType == 15 )
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->lightSourceAlwaysOn() = entityToCopy->lightSourceAlwaysOn();
			entityNew->lightSourceBrightness() = entityToCopy->lightSourceBrightness();
			entityNew->lightSourceInvertPower() = entityToCopy->lightSourceInvertPower();
			entityNew->lightSourceLatchOn() = entityToCopy->lightSourceLatchOn();
			entityNew->lightSourceRadius() = entityToCopy->lightSourceRadius();
			entityNew->lightSourceFlicker() = entityToCopy->lightSourceFlicker();
			entityNew->lightSourceDelay() = entityToCopy->lightSourceDelay();
			entityNew->lightSourceRGB() = entityToCopy->lightSourceRGB();
		}
		else
		{
			// set default new entity attributes.
			entityNew->lightSourceAlwaysOn() = 0;
			entityNew->lightSourceBrightness() = 128;
			entityNew->lightSourceInvertPower() = 0;
			entityNew->lightSourceLatchOn() = 0;
			entityNew->lightSourceRadius() = 5;
			entityNew->lightSourceFlicker() = 0;
			entityNew->lightSourceDelay() = 0;
			entityNew->lightSourceRGB() = 0;
			entityNew->lightSourceRGB() |= 255;
			entityNew->lightSourceRGB() |= (255 << 8);
			entityNew->lightSourceRGB() |= (255 << 16);
		}
	}
	else if ( spriteType == 16 )
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->textSourceColorRGB() = entityToCopy->textSourceColorRGB();
			entityNew->textSourceVariables4W() = entityToCopy->textSourceVariables4W();
			entityNew->textSourceDelay() = entityToCopy->textSourceDelay();
			entityNew->textSourceIsScript() = entityToCopy->textSourceIsScript();
			for ( int i = 4; i < 60; ++i )
			{
				entityNew->skill[i] = entityToCopy->skill[i];
			}
		}
		else
		{
			// set default new entity attributes.
			entityNew->textSourceColorRGB() = 0xFFFFFFFF;
			entityNew->textSourceVariables4W() = 0;
			entityNew->textSourceDelay() = 0;
			entityNew->textSourceIsScript() = 0;
			for ( int i = 4; i < 60; ++i )
			{
				entityNew->skill[i] = 0;
			}
		}
	}
	else if ( spriteType == 17 )
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->signalInputDirection() = entityToCopy->signalInputDirection();
			entityNew->signalActivateDelay() = entityToCopy->signalActivateDelay();
			entityNew->signalTimerInterval() = entityToCopy->signalTimerInterval();
			entityNew->signalTimerRepeatCount() = entityToCopy->signalTimerRepeatCount();
			entityNew->signalTimerLatchInput() = entityToCopy->signalTimerLatchInput();
			entityNew->signalInvertOutput() = entityToCopy->signalInvertOutput();
		}
		else
		{
			// set default new entity attributes.
			entityNew->signalInputDirection() = 0;
			entityNew->signalActivateDelay() = 0;
			entityNew->signalTimerInterval() = 0;
			entityNew->signalTimerRepeatCount() = 0;
			entityNew->signalTimerLatchInput() = 0;
			entityNew->signalInvertOutput() = 0;
		}
	}
	else if ( spriteType == 28 )
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->signalInputDirection() = entityToCopy->signalInputDirection();
			entityNew->signalActivateDelay() = entityToCopy->signalActivateDelay();
			entityNew->signalTimerInterval() = entityToCopy->signalTimerInterval();
			entityNew->signalTimerRepeatCount() = entityToCopy->signalTimerRepeatCount();
			entityNew->signalTimerLatchInput() = entityToCopy->signalTimerLatchInput();
			entityNew->signalInvertOutput() = entityToCopy->signalInvertOutput();
		}
		else
		{
			// set default new entity attributes.
			entityNew->signalInputDirection() = 0;
			entityNew->signalActivateDelay() = 0;
			entityNew->signalTimerInterval() = 0;
			entityNew->signalTimerRepeatCount() = 0;
			entityNew->signalTimerLatchInput() = 0;
			entityNew->signalInvertOutput() = 0;
		}
	}
	else if ( spriteType == 18 )
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->portalCustomSprite() = entityToCopy->portalCustomSprite();
			entityNew->portalCustomSpriteAnimationFrames() = entityToCopy->portalCustomSpriteAnimationFrames();
			entityNew->portalCustomZOffset() = entityToCopy->portalCustomZOffset();
			entityNew->portalCustomLevelsToJump() = entityToCopy->portalCustomLevelsToJump();
			entityNew->portalNotSecret() = entityToCopy->portalNotSecret();
			entityNew->portalCustomRequiresPower() = entityToCopy->portalCustomRequiresPower();
			for ( int i = 11; i <= 18; ++i )
			{
				entityNew->skill[i] = entityToCopy->skill[i];
			}
		}
		else
		{
			// set default new entity attributes.
			entityNew->portalCustomSprite() = 161;
			entityNew->portalCustomSpriteAnimationFrames() = 0;
			entityNew->portalCustomZOffset() = 8;
			entityNew->portalCustomLevelsToJump() = 1;
			entityNew->portalNotSecret() = 1;
			entityNew->portalCustomRequiresPower() = 0;
			for ( int i = 11; i <= 18; ++i )
			{
				entityNew->skill[i] = 0;
			}
		}
	}
	else if ( spriteType == 19 ) // tables
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->furnitureDir() = entityToCopy->furnitureDir();
			entityNew->furnitureTableSpawnChairs() = entityToCopy->furnitureTableSpawnChairs();
			entityNew->furnitureTableRandomItemChance() = entityToCopy->furnitureTableRandomItemChance();
		}
		else
		{
			// set default new entity attributes.
			entityNew->furnitureDir() = -1;
			entityNew->furnitureTableSpawnChairs() = -1;
			entityNew->furnitureTableRandomItemChance() = -1;
		}
	}
	else if ( spriteType == 20 ) // readable book
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->skill[11] = entityToCopy->skill[11];
			entityNew->skill[12] = entityToCopy->skill[12];
			entityNew->skill[15] = entityToCopy->skill[15];
			for ( int i = 40; i <= 52; ++i )
			{
				entityNew->skill[i] = entityToCopy->skill[i];
			}
		}
		else
		{
			// set default new entity attributes.
			entityNew->skill[11] = 0;
			entityNew->skill[12] = 10;
			entityNew->skill[15] = 0;
			for ( int i = 40; i <= 52; ++i )
			{
				entityNew->skill[i] = 0;
			}
		}
	}
	else if ( spriteType == 21 ) // doors
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->doorForceLockedUnlocked() = entityToCopy->doorForceLockedUnlocked();
			entityNew->doorDisableLockpicks() = entityToCopy->doorDisableLockpicks();
			entityNew->doorDisableOpening()= entityToCopy->doorDisableOpening();
		}
		else
		{
			// set default new entity attributes.
			entityNew->doorForceLockedUnlocked() = 0;
			entityNew->doorDisableLockpicks() = 0;
			entityNew->doorDisableOpening() = 0;
		}
	}
	else if ( spriteType == 32 ) // iron doors
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->doorUnlockWhenPowered() = entityToCopy->doorUnlockWhenPowered();
			entityNew->doorDisableLockpicks() = entityToCopy->doorDisableLockpicks();
			entityNew->doorForceLockedUnlocked() = entityToCopy->doorForceLockedUnlocked();
			entityNew->doorDisableOpening() = entityToCopy->doorDisableOpening();
		}
		else
		{
			// set default new entity attributes.
			entityNew->doorUnlockWhenPowered() = 1;
			entityNew->doorDisableLockpicks() = 1;
			entityNew->doorDisableOpening() = 1;
			entityNew->doorForceLockedUnlocked() = 1;
		}
	}
	else if ( spriteType == 22 ) // gates
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->gateDisableOpening() = entityToCopy->gateDisableOpening();
		}
		else
		{
			// set default new entity attributes.
			entityNew->gateDisableOpening() = 0;
		}
	}
	else if ( spriteType == 23 ) // player spawns
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->playerStartDir() = entityToCopy->playerStartDir();
		}
		else
		{
			// set default new entity attributes.
			entityNew->playerStartDir() = 0;
		}
	}
	else if ( spriteType == 24 ) // statue
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->statueDir() = entityToCopy->statueDir();
			entityNew->statueId() = entityToCopy->statueId();
		}
		else
		{
			// set default new entity attributes.
			entityNew->statueDir() = 0;
			entityNew->statueId() = 0;
		}
	}
	else if ( spriteType == 25 ) // teleport shrine
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->shrineDir() = entityToCopy->shrineDir();
			entityNew->shrineZ() = entityToCopy->shrineZ();
			entityNew->shrineDestXOffset() = entityToCopy->shrineDestXOffset();
			entityNew->shrineDestYOffset() = entityToCopy->shrineDestYOffset();
		}
		else
		{
			// set default new entity attributes.
			entityNew->shrineDir() = 0;
			entityNew->shrineZ() = 0;
			entityNew->shrineDestXOffset() = 0;
			entityNew->shrineDestYOffset() = 0;
		}
	}
	else if ( spriteType == 26 ) // spell shrine
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->shrineDir() = entityToCopy->shrineDir();
			entityNew->shrineZ() = entityToCopy->shrineZ();
			entityNew->shrineDestXOffset() = entityToCopy->shrineDestXOffset();
			entityNew->shrineDestYOffset() = entityToCopy->shrineDestYOffset();
		}
		else
		{
			// set default new entity attributes.
			entityNew->shrineDir() = 0;
			entityNew->shrineZ() = 0;
			entityNew->shrineDestXOffset() = 0;
			entityNew->shrineDestYOffset() = 0;
		}
	}
	else if ( spriteType == 27 ) // collider deco
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->colliderDecorationModel() = entityToCopy->colliderDecorationModel();
			entityNew->colliderDecorationRotation() = entityToCopy->colliderDecorationRotation();
			entityNew->colliderDecorationHeightOffset() = entityToCopy->colliderDecorationHeightOffset();
			entityNew->colliderDecorationXOffset() = entityToCopy->colliderDecorationXOffset();
			entityNew->colliderDecorationYOffset() = entityToCopy->colliderDecorationYOffset();
			entityNew->colliderHasCollision() = entityToCopy->colliderHasCollision();
			entityNew->colliderSizeX() = entityToCopy->colliderSizeX();
			entityNew->colliderSizeY() = entityToCopy->colliderSizeY();
			entityNew->colliderMaxHP() = entityToCopy->colliderMaxHP();
			entityNew->colliderDiggable() = entityToCopy->colliderDiggable();
			entityNew->colliderDamageTypes() = entityToCopy->colliderDamageTypes();
		}
		else
		{
			// set default new entity attributes.
			entityNew->colliderDecorationModel() = 0;
			entityNew->colliderDecorationRotation() = 0;
			entityNew->colliderDecorationHeightOffset() = 0;
			entityNew->colliderDecorationXOffset() = 0;
			entityNew->colliderDecorationYOffset() = 0;
			entityNew->colliderHasCollision() = 1;
			entityNew->colliderSizeX() = 0;
			entityNew->colliderSizeY() = 0;
			entityNew->colliderMaxHP() = 0;
			entityNew->colliderDiggable() = 0;
			entityNew->colliderDamageTypes() = 0;
		}
	}
	else if ( spriteType == 29 ) // pressure plates
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->pressurePlateTriggerType() = entityToCopy->pressurePlateTriggerType();
		}
		else
		{
			// set default new entity attributes.
			entityNew->pressurePlateTriggerType() = 0;
		}
	}
	else if ( spriteType == 30 ) // wall locks
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->wallLockMaterial() = entityToCopy->wallLockMaterial();
			entityNew->wallLockInvertPower() = entityToCopy->wallLockInvertPower();
			entityNew->wallLockTurnable() = entityToCopy->wallLockTurnable();
			entityNew->wallLockPickable() = entityToCopy->wallLockPickable();
			entityNew->wallLockPickableSkeletonKey() = entityToCopy->wallLockPickableSkeletonKey();
			entityNew->wallLockAutoGenKey() = entityToCopy->wallLockAutoGenKey();
		}
		else
		{
			// set default new entity attributes.
			entityNew->wallLockMaterial() = 0;
			entityNew->wallLockInvertPower() = 0;
			entityNew->wallLockTurnable() = 0;
			entityNew->wallLockPickable() = -1;
			entityNew->wallLockPickableSkeletonKey() = 0;
			entityNew->wallLockAutoGenKey() = 0;
		}
	}
	else if ( spriteType == 31 ) // wall buttons
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->wallLockInvertPower() = entityToCopy->wallLockInvertPower();
			entityNew->wallLockTimer() = entityToCopy->wallLockTimer();
		}
		else
		{
			// set default new entity attributes.
			entityNew->wallLockInvertPower() = 0;
			entityNew->wallLockTimer() = 0;
		}
	}
	else if ( spriteType == 33 ) // wind
	{
		if ( entityToCopy != nullptr )
		{
			// copy old entity attributes to newly created.
			entityNew->skill[0] = entityToCopy->skill[0];
		}
		else
		{
			// set default new entity attributes.
			entityNew->skill[0] = 0;
		}
	}

	if ( entityToCopy != nullptr )
	{
		// if we are duplicating sprite, then copy the x and y coordinates.
		entityNew->x = entityToCopy->x;
		entityNew->y = entityToCopy->y;
	}
	else
	{
		// new entity, will follow the mouse movements when created.
	}
}

void Entity::seedEntityRNG(Uint32 seed)
{
	if ( !entity_rng )
	{
		entity_rng = new BaronyRNG();
	}
	if ( entity_rng )
	{
		entity_rng->seedBytes(&seed, sizeof(seed));
	}
}

extern "C" void Entity_seedEntityRNG(Entity* self, Uint32 seed) { return self->seedEntityRNG(seed); }



