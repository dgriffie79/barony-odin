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
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
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
	static void addMemberToSubkey(rapidjson::Document& d, std::string subkey, std::string name, const rapidjson::Value& value)
	{
		rapidjson::Value key(name.c_str(), d.GetAllocator()); // copy string name
		rapidjson::Value val(value, d.GetAllocator());
		d[subkey.c_str()].AddMember(key, val, d.GetAllocator());
	}
	static void addMemberToRoot(rapidjson::Document& d, std::string name, const rapidjson::Value& value)
	{
		rapidjson::Value key(name.c_str(), d.GetAllocator()); // copy string name
		rapidjson::Value val(value, d.GetAllocator());
		d.AddMember(key, val, d.GetAllocator());
	}
	static void addArrayMemberToSubkey(rapidjson::Document& d, std::string subkey, const rapidjson::Value& value)
	{
		rapidjson::Value val(value, d.GetAllocator());        // some value
		d[subkey.c_str()].PushBack(val, d.GetAllocator());
	}
	static bool isLevelPartOfSet(int level, bool secret, std::pair<std::unordered_set<int>, std::unordered_set<int>>& pairOfSets)
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

	int getSlotFromKeyName(std::string keyName)
	{
		if ( keyName.compare("weapon") == 0 )
		{
			return ITEM_SLOT_WEAPON;
		}
		else if ( keyName.compare("shield") == 0 )
		{
			return ITEM_SLOT_SHIELD;
		}
		else if ( keyName.compare("helmet") == 0 )
		{
			return ITEM_SLOT_HELM;
		}
		else if ( keyName.compare("breastplate") == 0 )
		{
			return ITEM_SLOT_ARMOR;
		}
		else if ( keyName.compare("gloves") == 0 )
		{
			return ITEM_SLOT_GLOVES;
		}
		else if ( keyName.compare("shoes") == 0 )
		{
			return ITEM_SLOT_BOOTS;
		}
		else if ( keyName.compare("cloak") == 0 )
		{
			return ITEM_SLOT_CLOAK;
		}
		else if ( keyName.compare("ring") == 0 )
		{
			return ITEM_SLOT_RING;
		}
		else if ( keyName.compare("amulet") == 0 )
		{
			return ITEM_SLOT_AMULET;
		}
		else if ( keyName.compare("mask") == 0 )
		{
			return ITEM_SLOT_MASK;
		}
		return 0;
	}

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
		void readFromItem(const Item& itemToRead)
		{
			type = itemToRead.type;
			status = itemToRead.status;
			beatitude = itemToRead.beatitude;
			count = itemToRead.count;
			appearance = itemToRead.appearance;
			identified = itemToRead.identified;
			if ( itemToRead.appearance == MONSTER_ITEM_UNDROPPABLE_APPEARANCE )
			{
				dropItemOnDeath = false;
			}
		}
		void setValueFromAttributes(rapidjson::Document& d, rapidjson::Value& outObject)
		{
			rapidjson::Value key1("type", d.GetAllocator());
			rapidjson::Value val1(itemNameStrings[type + 2], d.GetAllocator());
			outObject.AddMember(key1, val1, d.GetAllocator());

			rapidjson::Value key2("status", d.GetAllocator());
			rapidjson::Value val2(itemStatusStrings.at(status).c_str(), d.GetAllocator());
			outObject.AddMember(key2, val2, d.GetAllocator());

			outObject.AddMember("beatitude", rapidjson::Value(beatitude), d.GetAllocator());
			outObject.AddMember("count", rapidjson::Value(count), d.GetAllocator());
			outObject.AddMember("appearance", rapidjson::Value(appearance), d.GetAllocator());
			outObject.AddMember("identified", rapidjson::Value(identified), d.GetAllocator());
			outObject.AddMember("spawn_percent_chance", rapidjson::Value(100), d.GetAllocator());
			outObject.AddMember("drop_percent_chance", rapidjson::Value(dropItemOnDeath ? 100 : 0), d.GetAllocator());
			outObject.AddMember("slot_weighted_chance", rapidjson::Value(1), d.GetAllocator());
		}

		const char* getRandomArrayStr(const rapidjson::GenericArray<true, rapidjson::GenericValue<rapidjson::UTF8<>>>& arr, const char* invalidEntry)
		{
			if ( arr.Size() == 0 )
			{
				return invalidEntry;
			}
			return (arr[rapidjson::SizeType(monster_stat_rng.rand() % arr.Size())].GetString());
		}
		int getRandomArrayInt(const rapidjson::GenericArray<true, rapidjson::GenericValue<rapidjson::UTF8<>>>& arr, int invalidEntry)
		{
			if ( arr.Size() == 0 )
			{
				return invalidEntry;
			}
			return (arr[rapidjson::SizeType(monster_stat_rng.rand() % arr.Size())].GetInt());
		}

		bool readKeyToItemEntry(rapidjson::Value::ConstMemberIterator& itr)
		{
			DynamicString name = itr->name.GetString();
			if ( name.compare("type") == 0 )
			{
				DynamicString itemName = "empty";
				if ( itr->value.IsArray() )
				{
					itemName = getRandomArrayStr(itr->value.GetArray(), "empty");
				}
				else if ( itr->value.IsString() )
				{
					itemName = itr->value.GetString();
				}

				if ( itemName.compare("empty") == 0 )
				{
					emptyItemEntry = true;
					return true;
				}
				for ( int i = 0; i < NUMITEMS; ++i )
				{
					if ( itemName.compare(itemNameStrings[i + 2]) == 0 )
					{
						this->type = static_cast<ItemType>(i);
						return true;
					}
				}
			}
			else if ( name.compare("status") == 0 )
			{
				DynamicString status = "broken";
				if ( itr->value.IsArray() )
				{
					status = getRandomArrayStr(itr->value.GetArray(), "broken");
				}
				else if ( itr->value.IsString() )
				{
					status = itr->value.GetString();
				}
				for ( Uint32 i = 0; i < itemStatusStrings.size(); ++i )
				{
					if ( status.compare(itemStatusStrings.at(i)) == 0 )
					{
						this->status = static_cast<Status>(i);
						return true;
					}
				}
			}
			else if ( name.compare("beatitude") == 0 )
			{
				if ( itr->value.IsArray() )
				{
					this->beatitude = static_cast<Sint16>(getRandomArrayInt(itr->value.GetArray(), 0));
				}
				else if ( itr->value.IsInt() )
				{
					this->beatitude = static_cast<Sint16>(itr->value.GetInt());
				}
				return true;
			}
			else if ( name.compare("count") == 0 )
			{
				if ( itr->value.IsArray() )
				{
					this->count = static_cast<Sint16>(getRandomArrayInt(itr->value.GetArray(), 1));
				}
				else if ( itr->value.IsInt() )
				{
					this->count = static_cast<Sint16>(itr->value.GetInt());
				}
				return true;
			}
			else if ( name.compare("appearance") == 0 )
			{
				if ( itr->value.IsArray() )
				{
					this->appearance = static_cast<Uint32>(getRandomArrayInt(itr->value.GetArray(), monster_stat_rng.rand()));
				}
				else if ( itr->value.IsInt() )
				{
					this->appearance = static_cast<Uint32>(itr->value.GetInt());
				}
				else if ( itr->value.IsString() )
				{
					DynamicString str = itr->value.GetString();
					if ( str.compare("random") == 0 )
					{
						this->appearance = monster_stat_rng.rand();
					}
				}
				return true;
			}
			else if ( name.compare("identified") == 0 )
			{
				this->identified = itr->value.GetBool();
				return true;
			}
			else if ( name.compare("spawn_percent_chance") == 0 )
			{
				this->percentChance = itr->value.GetInt();
				return true;
			}
			else if ( name.compare("drop_percent_chance") == 0 )
			{
				this->dropChance = itr->value.GetInt();
				if ( monster_stat_rng.rand() % 100 >= this->dropChance )
				{
					this->dropItemOnDeath = false;
				}
				else
				{
					this->dropItemOnDeath = true;
				}
			}
			else if ( name.compare("slot_weighted_chance") == 0 )
			{
				this->weightedChance = std::max(1, itr->value.GetInt());
				return true;
			}
			return false;
		}
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

		std::vector<std::pair<ItemEntry, int>> equipped_items;
		std::vector<ItemEntry> inventory_items;
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

		std::string getFollowerVariant()
		{
			if ( followerVariants.size() > 0 )
			{
				std::vector<unsigned int> variantChances(followerVariants.size(), 0);
				int index = 0;
				for ( auto& pair : followerVariants )
				{
					variantChances.at(index) = pair.chance;
					++index;
				}

				int result = monster_stat_rng.discrete(variantChances.data(), variantChances.size());
				return followerVariants.at(result).name.c_str();
			}
			return "none";
		}

		void readFromStats(const Stat* myStats)
		{
			strcpy(name, myStats->name);
			type = myStats->type;
			sex = myStats->sex;
			appearance = myStats->stat_appearance;
			HP = myStats->HP;
			MAXHP = myStats->MAXHP;
			OLDHP = HP;
			MP = myStats->MP;
			MAXMP = myStats->MAXMP;
			STR = myStats->STR;
			DEX = myStats->DEX;
			CON = myStats->CON;
			INT = myStats->INT;
			PER = myStats->PER;
			CHR = myStats->CHR;
			EXP = myStats->EXP;
			LVL = myStats->LVL;
			GOLD = myStats->GOLD;

			RANDOM_STR = myStats->RANDOM_STR;
			RANDOM_DEX = myStats->RANDOM_DEX;
			RANDOM_CON = myStats->RANDOM_CON;
			RANDOM_INT = myStats->RANDOM_INT;
			RANDOM_PER = myStats->RANDOM_PER;
			RANDOM_CHR = myStats->RANDOM_CHR;
			RANDOM_MAXHP = myStats->RANDOM_MAXHP;
			RANDOM_HP = myStats->RANDOM_HP;
			RANDOM_MAXMP = myStats->RANDOM_MAXMP;
			RANDOM_MP = myStats->RANDOM_MP;
			RANDOM_LVL = myStats->RANDOM_LVL;
			RANDOM_GOLD = myStats->RANDOM_GOLD;

			for ( int i = 0; i < NUMPROFICIENCIES; ++i )
			{
				PROFICIENCIES[i] = 0;
			}
			for ( int i = 0; i < NUMPROFICIENCIES; ++i )
			{
				PROFICIENCIES[i] = myStats->getProficiency(i);
			}
		}

		void setStats(Stat* myStats)
		{
			strcpy(myStats->name, name);
			myStats->type = static_cast<Monster>(type);
			myStats->sex = static_cast<sex_t>(sex);
			myStats->stat_appearance = appearance;
			myStats->HP = HP;
			myStats->MAXHP = MAXHP;
			myStats->OLDHP = myStats->HP;
			myStats->MP = MP;
			myStats->MAXMP = MAXMP;
			myStats->STR = STR;
			myStats->DEX = DEX;
			myStats->CON = CON;
			myStats->INT = INT;
			myStats->PER = PER;
			myStats->CHR = CHR;
			myStats->EXP = EXP;
			myStats->LVL = LVL;
			myStats->GOLD = GOLD;

			myStats->RANDOM_STR = RANDOM_STR;
			myStats->RANDOM_DEX = RANDOM_DEX;
			myStats->RANDOM_CON = RANDOM_CON;
			myStats->RANDOM_INT = RANDOM_INT;
			myStats->RANDOM_PER = RANDOM_PER;
			myStats->RANDOM_CHR = RANDOM_CHR;
			myStats->RANDOM_MAXHP = RANDOM_MAXHP;
			myStats->RANDOM_HP = RANDOM_HP;
			myStats->RANDOM_MAXMP = RANDOM_MAXMP;
			myStats->RANDOM_MP = RANDOM_MP;
			myStats->RANDOM_LVL = RANDOM_LVL;
			myStats->RANDOM_GOLD = RANDOM_GOLD;

			for ( int i = 0; i < NUMPROFICIENCIES; ++i )
			{
				myStats->setProficiency(i, PROFICIENCIES[i]);
			}
		}

		void setItems(Stat* myStats)
		{
			std::unordered_set<int> equippedSlots;
			for ( auto& it : equipped_items )
			{
				equippedSlots.insert(it.second);
				if ( it.first.percentChance < 100 )
				{
					if ( monster_stat_rng.rand() % 100 >= it.first.percentChance )
					{
						continue;
					}
				}
				if ( it.first.emptyItemEntry )
				{
					continue;
				}
				switch ( it.second )
				{
					case ITEM_SLOT_WEAPON:
						myStats->weapon = newItem(it.first.type, it.first.status, it.first.beatitude, it.first.count, it.first.appearance, it.first.identified, nullptr);
						if ( myStats->weapon )
						{
							myStats->weapon->isDroppable = it.first.dropItemOnDeath;
						}
						break;
					case ITEM_SLOT_SHIELD:
						myStats->shield = newItem(it.first.type, it.first.status, it.first.beatitude, it.first.count, it.first.appearance, it.first.identified, nullptr);
						if ( myStats->shield )
						{
							myStats->shield->isDroppable = it.first.dropItemOnDeath;
						}
						break;
					case ITEM_SLOT_HELM:
						myStats->helmet = newItem(it.first.type, it.first.status, it.first.beatitude, it.first.count, it.first.appearance, it.first.identified, nullptr);
						if ( myStats->helmet )
						{
							myStats->helmet->isDroppable = it.first.dropItemOnDeath;
						}
						break;
					case ITEM_SLOT_ARMOR:
						myStats->breastplate = newItem(it.first.type, it.first.status, it.first.beatitude, it.first.count, it.first.appearance, it.first.identified, nullptr);
						if ( myStats->breastplate )
						{
							myStats->breastplate->isDroppable = it.first.dropItemOnDeath;
						}
						break;
					case ITEM_SLOT_GLOVES:
						myStats->gloves = newItem(it.first.type, it.first.status, it.first.beatitude, it.first.count, it.first.appearance, it.first.identified, nullptr);
						if ( myStats->gloves )
						{
							myStats->gloves->isDroppable = it.first.dropItemOnDeath;
						}
						break;
					case ITEM_SLOT_BOOTS:
						myStats->shoes = newItem(it.first.type, it.first.status, it.first.beatitude, it.first.count, it.first.appearance, it.first.identified, nullptr);
						if ( myStats->shoes )
						{
							myStats->shoes->isDroppable = it.first.dropItemOnDeath;
						}
						break;
					case ITEM_SLOT_CLOAK:
						myStats->cloak = newItem(it.first.type, it.first.status, it.first.beatitude, it.first.count, it.first.appearance, it.first.identified, nullptr);
						if ( myStats->cloak )
						{
							myStats->cloak->isDroppable = it.first.dropItemOnDeath;
						}
						break;
					case ITEM_SLOT_RING:
						myStats->ring = newItem(it.first.type, it.first.status, it.first.beatitude, it.first.count, it.first.appearance, it.first.identified, nullptr);
						if ( myStats->ring )
						{
							myStats->ring->isDroppable = it.first.dropItemOnDeath;
						}
						break;
					case ITEM_SLOT_AMULET:
						myStats->amulet = newItem(it.first.type, it.first.status, it.first.beatitude, it.first.count, it.first.appearance, it.first.identified, nullptr);
						if ( myStats->amulet )
						{
							myStats->amulet->isDroppable = it.first.dropItemOnDeath;
						}
						break;
					case ITEM_SLOT_MASK:
						myStats->mask = newItem(it.first.type, it.first.status, it.first.beatitude, it.first.count, it.first.appearance, it.first.identified, nullptr);
						if ( myStats->mask )
						{
							myStats->mask->isDroppable = it.first.dropItemOnDeath;
						}
						break;
					default:
						break;
				}
			}
			for ( int equipSlots = 0; equipSlots < 10; ++equipSlots )
			{
				if ( !useDefaultEquipment )
				{
					// disable any default item slot spawning.
					myStats->EDITOR_ITEMS[equipSlots * ITEM_SLOT_NUMPROPERTIES] = 0;
				}
				else
				{
					if ( equippedSlots.find(equipSlots * ITEM_SLOT_NUMPROPERTIES) != equippedSlots.end() )
					{
						// disable item slots we (attempted) to fill in.
						myStats->EDITOR_ITEMS[equipSlots * ITEM_SLOT_NUMPROPERTIES] = 0;
					}
				}
			}
			for ( auto& it : inventory_items )
			{
				if ( it.emptyItemEntry )
				{
					continue;
				}
				if ( it.percentChance < 100 )
				{
					if ( monster_stat_rng.rand() % 100 >= it.percentChance )
					{
						continue;
					}
				}
				Item* item = newItem(it.type, it.status, it.beatitude, it.count, it.appearance, it.identified, &myStats->inventory);
				if ( item )
				{
					item->isDroppable = it.dropItemOnDeath;
				}
			}
			if ( !useDefaultInventoryItems )
			{
				for ( int invSlots = ITEM_SLOT_INV_1; invSlots <= ITEM_SLOT_INV_6; invSlots = invSlots + ITEM_SLOT_NUMPROPERTIES )
				{
					myStats->EDITOR_ITEMS[invSlots] = 0;
				}
			}
		}

		void setStatsAndEquipmentToMonster(Stat* myStats)
		{
			//myStats->clearStats();
			setStats(myStats);
			setItems(myStats);

			if ( isMonsterNameGeneric )
			{
				myStats->MISC_FLAGS[STAT_FLAG_MONSTER_NAME_GENERIC] = 1;
			}
			if ( disableMiniboss )
			{
				myStats->MISC_FLAGS[STAT_FLAG_DISABLE_MINIBOSS] = 1;
			}
			if ( forceFriendlyToPlayer )
			{
				myStats->MISC_FLAGS[STAT_FLAG_FORCE_ALLEGIANCE_TO_PLAYER] = 
					Stat::MonsterForceAllegiance::MONSTER_FORCE_PLAYER_ALLY;
			}
			if ( forceEnemyToPlayer )
			{
				myStats->MISC_FLAGS[STAT_FLAG_FORCE_ALLEGIANCE_TO_PLAYER] =
					Stat::MonsterForceAllegiance::MONSTER_FORCE_PLAYER_ENEMY;
			}
			if ( forceRecruitableToPlayer )
			{
				myStats->MISC_FLAGS[STAT_FLAG_FORCE_ALLEGIANCE_TO_PLAYER] =
					Stat::MonsterForceAllegiance::MONSTER_FORCE_PLAYER_RECRUITABLE;
			}
			if ( disableItemDrops )
			{
				myStats->MISC_FLAGS[STAT_FLAG_NO_DROP_ITEMS] = 1;
			}
			if ( xpAwardPercent != 100 )
			{
				myStats->MISC_FLAGS[STAT_FLAG_XP_PERCENT_AWARD] = 1 + std::min(std::max(0, xpAwardPercent), 100);
			}
			if ( castSpellbooksFromInventory )
			{
				myStats->MISC_FLAGS[STAT_FLAG_MONSTER_CAST_INVENTORY_SPELLBOOKS] = 1;
				myStats->MISC_FLAGS[STAT_FLAG_MONSTER_CAST_INVENTORY_SPELLBOOKS] |= (spellbookCastCooldown << 4);
			}
			if ( myStats->type == SHOPKEEPER )
			{
				if ( chosenShopkeeperStore >= 0 )
				{
					myStats->MISC_FLAGS[STAT_FLAG_NPC] = chosenShopkeeperStore + 1;
				}
				Uint8 numItems = 0;
				myStats->MISC_FLAGS[STAT_FLAG_SHOPKEEPER_CUSTOM_PROPERTIES] = 0;
				if ( shopkeeperGenDefaultItems )
				{
					if ( shopkeeperMinItems >= 0 && shopkeeperMaxItems >= 0 )
					{
						numItems = shopkeeperMinItems + monster_stat_rng.rand() % std::max(1, (shopkeeperMaxItems - shopkeeperMinItems + 1));
						myStats->MISC_FLAGS[STAT_FLAG_SHOPKEEPER_CUSTOM_PROPERTIES] |= numItems + 1;
					}
					if ( shopkeeperMaxGeneratedBlessing >= 0 )
					{
						myStats->MISC_FLAGS[STAT_FLAG_SHOPKEEPER_CUSTOM_PROPERTIES] |= (static_cast<Uint8>(shopkeeperMaxGeneratedBlessing + 1) << 8);
					}
					myStats->MISC_FLAGS[STAT_FLAG_SHOPKEEPER_CUSTOM_PROPERTIES] |= (ShopkeeperCustomFlags::ENABLE_GEN_ITEMS << 12); // indicate to use this property.
				}
				else
				{
					myStats->MISC_FLAGS[STAT_FLAG_SHOPKEEPER_CUSTOM_PROPERTIES] |= (ShopkeeperCustomFlags::DISABLE_GEN_ITEMS << 12); // indicate to disable gen items.
				}
			}
		}

		void setStatsAndEquipmentToPlayer(Stat* myStats, int player)
		{
			//if ( player == 0 )
			//{
			//	TextSourceScript tmpScript;
			//	tmpScript.playerClearInventory(true);
			//}
			//else
			//{
			//	// other players
			//	myStats->freePlayerEquipment();
			//	myStats->clearStats();
			//	TextSourceScript tmpScript;
			//	tmpScript.updateClientInformation(player, true, true, TextSourceScript::CLIENT_UPDATE_ALL);
			//}
		}
	};

	void writeAllFromStats(Stat* myStats)
	{
		rapidjson::Document d;
		d.SetObject();
		rapidjson::Value version;
		version.SetInt(1);
		CustomHelpers::addMemberToRoot(d, "version", version);
		readAttributesFromStats(myStats, d);
		readItemsFromStats(myStats, d);
		
		// misc properties
		rapidjson::Value propsObject;
		propsObject.SetObject();
		CustomHelpers::addMemberToRoot(d, "properties", propsObject);
		CustomHelpers::addMemberToSubkey(d, "properties", "monster_name_always_display_as_generic_species", rapidjson::Value(false));
		CustomHelpers::addMemberToSubkey(d, "properties", "populate_empty_equipped_items_with_default", rapidjson::Value(true));
		CustomHelpers::addMemberToSubkey(d, "properties", "populate_default_inventory", rapidjson::Value(true));
		CustomHelpers::addMemberToSubkey(d, "properties", "disable_miniboss_chance", rapidjson::Value(false));
		CustomHelpers::addMemberToSubkey(d, "properties", "force_player_recruitable", rapidjson::Value(false));
		CustomHelpers::addMemberToSubkey(d, "properties", "force_player_friendly", rapidjson::Value(false));
		CustomHelpers::addMemberToSubkey(d, "properties", "force_player_enemy", rapidjson::Value(false));
		CustomHelpers::addMemberToSubkey(d, "properties", "disable_item_drops", rapidjson::Value(false));
		CustomHelpers::addMemberToSubkey(d, "properties", "xp_award_percent", rapidjson::Value(100));
		CustomHelpers::addMemberToSubkey(d, "properties", "enable_casting_inventory_spellbooks", rapidjson::Value(false));
		CustomHelpers::addMemberToSubkey(d, "properties", "spellbook_cast_cooldown", rapidjson::Value(250));

		if ( myStats->type == SHOPKEEPER )
		{
			// shop properties
			CustomHelpers::addMemberToRoot(d, "shopkeeper_properties", propsObject);

			rapidjson::Value shopObject(rapidjson::kObjectType);
			shopObject.SetObject();

			rapidjson::Value storeTypesObject(rapidjson::kObjectType);
			storeTypesObject.AddMember("equipment", rapidjson::Value(1), d.GetAllocator());
			storeTypesObject.AddMember("hats", rapidjson::Value(1), d.GetAllocator());
			storeTypesObject.AddMember("jewelry", rapidjson::Value(1), d.GetAllocator());
			storeTypesObject.AddMember("books", rapidjson::Value(1), d.GetAllocator());
			storeTypesObject.AddMember("apothecary", rapidjson::Value(1), d.GetAllocator());
			storeTypesObject.AddMember("staffs", rapidjson::Value(1), d.GetAllocator());
			storeTypesObject.AddMember("food", rapidjson::Value(1), d.GetAllocator());
			storeTypesObject.AddMember("hardware", rapidjson::Value(1), d.GetAllocator());
			storeTypesObject.AddMember("hunting", rapidjson::Value(1), d.GetAllocator());
			storeTypesObject.AddMember("general", rapidjson::Value(1), d.GetAllocator());

			CustomHelpers::addMemberToSubkey(d, "shopkeeper_properties", "store_type_chances", storeTypesObject);
			CustomHelpers::addMemberToSubkey(d, "shopkeeper_properties", "generate_default_shop_items", rapidjson::Value(true));
			CustomHelpers::addMemberToSubkey(d, "shopkeeper_properties", "num_generated_items_min", rapidjson::Value(10));
			CustomHelpers::addMemberToSubkey(d, "shopkeeper_properties", "num_generated_items_max", rapidjson::Value(15));
			CustomHelpers::addMemberToSubkey(d, "shopkeeper_properties", "generated_item_blessing_max", rapidjson::Value(0));
		}

		// follower details
		rapidjson::Value followersObject;
		followersObject.SetObject();
		CustomHelpers::addMemberToRoot(d, "followers", followersObject);
		CustomHelpers::addMemberToSubkey(d, "followers", "num_followers", rapidjson::Value(0));
		rapidjson::Value followerVariantsObject;
		followerVariantsObject.SetObject();
		CustomHelpers::addMemberToSubkey(d, "followers", "follower_variants", followerVariantsObject);

		writeToFile(d, monstertypename[myStats->type]);
	}

	void readItemsFromStats(Stat* myStats, rapidjson::Document& d)
	{
		rapidjson::Value equippedItemsObject;
		equippedItemsObject.SetObject();
		CustomHelpers::addMemberToRoot(d, "equipped_items", equippedItemsObject);
		addMemberFromItem(d, "equipped_items", "weapon", myStats->weapon);
		addMemberFromItem(d, "equipped_items", "shield", myStats->shield);
		addMemberFromItem(d, "equipped_items", "helmet", myStats->helmet);
		addMemberFromItem(d, "equipped_items", "breastplate", myStats->breastplate);
		addMemberFromItem(d, "equipped_items", "gloves", myStats->gloves);
		addMemberFromItem(d, "equipped_items", "shoes", myStats->shoes);
		addMemberFromItem(d, "equipped_items", "cloak", myStats->cloak);
		addMemberFromItem(d, "equipped_items", "ring", myStats->ring);
		addMemberFromItem(d, "equipped_items", "amulet", myStats->amulet);
		addMemberFromItem(d, "equipped_items", "mask", myStats->mask);

		rapidjson::Value invItemsArray;
		invItemsArray.SetArray();
		CustomHelpers::addMemberToRoot(d, "inventory_items", invItemsArray);
		for ( node_t* node = myStats->inventory.first; node; node = node->next )
		{
			Item* item = (Item*)node->element;
			if ( item )
			{
				addArrayMemberFromItem(d, "inventory_items", item);
			}
		}
	}

	void readAttributesFromStats(Stat* myStats, rapidjson::Document& d)
	{
		rapidjson::Value statsObject;
		statsObject.SetObject();
		CustomHelpers::addMemberToRoot(d, "stats", statsObject);

		StatEntry statEntry(myStats);
		CustomHelpers::addMemberToSubkey(d, "stats", "name", rapidjson::Value(statEntry.name, d.GetAllocator()));
		CustomHelpers::addMemberToSubkey(d, "stats", "type", rapidjson::Value(monstertypename[statEntry.type], d.GetAllocator()));
		CustomHelpers::addMemberToSubkey(d, "stats", "sex", rapidjson::Value(statEntry.sex));
		CustomHelpers::addMemberToSubkey(d, "stats", "appearance", rapidjson::Value(statEntry.appearance));
		CustomHelpers::addMemberToSubkey(d, "stats", "HP", rapidjson::Value(statEntry.HP));
		CustomHelpers::addMemberToSubkey(d, "stats", "MAXHP", rapidjson::Value(statEntry.MAXHP));
		CustomHelpers::addMemberToSubkey(d, "stats", "MP", rapidjson::Value(statEntry.MP));
		CustomHelpers::addMemberToSubkey(d, "stats", "MAXMP", rapidjson::Value(statEntry.MAXMP));
		CustomHelpers::addMemberToSubkey(d, "stats", "STR", rapidjson::Value(statEntry.STR));
		CustomHelpers::addMemberToSubkey(d, "stats", "DEX", rapidjson::Value(statEntry.DEX));
		CustomHelpers::addMemberToSubkey(d, "stats", "CON", rapidjson::Value(statEntry.CON));
		CustomHelpers::addMemberToSubkey(d, "stats", "INT", rapidjson::Value(statEntry.INT));
		CustomHelpers::addMemberToSubkey(d, "stats", "PER", rapidjson::Value(statEntry.PER));
		CustomHelpers::addMemberToSubkey(d, "stats", "CHR", rapidjson::Value(statEntry.CHR));
		CustomHelpers::addMemberToSubkey(d, "stats", "EXP", rapidjson::Value(statEntry.EXP));
		CustomHelpers::addMemberToSubkey(d, "stats", "LVL", rapidjson::Value(statEntry.LVL));
		CustomHelpers::addMemberToSubkey(d, "stats", "GOLD", rapidjson::Value(statEntry.GOLD));

		rapidjson::Value miscStatsObject;
		miscStatsObject.SetObject();
		CustomHelpers::addMemberToRoot(d, "misc_stats", miscStatsObject);

		CustomHelpers::addMemberToSubkey(d, "misc_stats", "RANDOM_STR", rapidjson::Value(statEntry.RANDOM_STR));
		CustomHelpers::addMemberToSubkey(d, "misc_stats", "RANDOM_DEX", rapidjson::Value(statEntry.RANDOM_DEX));
		CustomHelpers::addMemberToSubkey(d, "misc_stats", "RANDOM_CON", rapidjson::Value(statEntry.RANDOM_CON));
		CustomHelpers::addMemberToSubkey(d, "misc_stats", "RANDOM_INT", rapidjson::Value(statEntry.RANDOM_INT));
		CustomHelpers::addMemberToSubkey(d, "misc_stats", "RANDOM_PER", rapidjson::Value(statEntry.RANDOM_PER));
		CustomHelpers::addMemberToSubkey(d, "misc_stats", "RANDOM_CHR", rapidjson::Value(statEntry.RANDOM_CHR));
		CustomHelpers::addMemberToSubkey(d, "misc_stats", "RANDOM_MAXHP", rapidjson::Value(statEntry.RANDOM_MAXHP));
		CustomHelpers::addMemberToSubkey(d, "misc_stats", "RANDOM_HP", rapidjson::Value(statEntry.RANDOM_HP));
		CustomHelpers::addMemberToSubkey(d, "misc_stats", "RANDOM_MAXMP", rapidjson::Value(statEntry.RANDOM_MAXMP));
		CustomHelpers::addMemberToSubkey(d, "misc_stats", "RANDOM_MP", rapidjson::Value(statEntry.RANDOM_MP));
		CustomHelpers::addMemberToSubkey(d, "misc_stats", "RANDOM_LVL", rapidjson::Value(statEntry.RANDOM_LVL));
		CustomHelpers::addMemberToSubkey(d, "misc_stats", "RANDOM_GOLD", rapidjson::Value(statEntry.RANDOM_GOLD));

		rapidjson::Value profObject;
		profObject.SetObject();
		CustomHelpers::addMemberToRoot(d, "proficiencies", profObject);

		for ( int i = 0; i < NUMPROFICIENCIES; ++i )
		{
			CustomHelpers::addMemberToSubkey(d, "proficiencies", getSkillLangEntry(i), rapidjson::Value(statEntry.PROFICIENCIES[i]));
		}
	}

	bool readKeyToStatEntry(StatEntry& statEntry, rapidjson::Value::ConstMemberIterator& itr)
	{
		DynamicString name = itr->name.GetString();
		if ( name.compare("name") == 0 )
		{
			strcpy(statEntry.name, itr->value.GetString());
			return true;
		}
		else if ( name.compare("type") == 0 )
		{
			DynamicString val = itr->value.GetString();
			for ( int i = 0; i < NUMMONSTERS; ++i )
			{
				if ( val.compare(monstertypename[i]) == 0 )
				{
					statEntry.type = i;
					break;
				}
			}
			return true;
		}
		else if ( name.compare("sex") == 0 )
		{
			statEntry.sex = static_cast<sex_t>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("appearance") == 0 )
		{
			statEntry.appearance = static_cast<Uint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("HP") == 0 )
		{
			statEntry.HP = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("MAXHP") == 0 )
		{
			statEntry.MAXHP = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("MP") == 0 )
		{
			statEntry.MP = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("MAXMP") == 0 )
		{
			statEntry.MAXMP = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("STR") == 0 )
		{
			statEntry.STR = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("DEX") == 0 )
		{
			statEntry.DEX = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("CON") == 0 )
		{
			statEntry.CON = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("INT") == 0 )
		{
			statEntry.INT = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("PER") == 0 )
		{
			statEntry.PER = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("CHR") == 0 )
		{
			statEntry.CHR = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("EXP") == 0 )
		{
			statEntry.EXP = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("LVL") == 0 )
		{
			statEntry.LVL = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("GOLD") == 0 )
		{
			statEntry.GOLD = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_STR") == 0 )
		{
			statEntry.RANDOM_STR = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_DEX") == 0 )
		{
			statEntry.RANDOM_DEX = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_CON") == 0 )
		{
			statEntry.RANDOM_CON = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_INT") == 0 )
		{
			statEntry.RANDOM_INT = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_PER") == 0 )
		{
			statEntry.RANDOM_PER = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_CHR") == 0 )
		{
			statEntry.RANDOM_CHR = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_MAXHP") == 0 )
		{
			statEntry.RANDOM_MAXHP = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_HP") == 0 )
		{
			statEntry.RANDOM_HP = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_MAXMP") == 0 )
		{
			statEntry.RANDOM_MAXMP = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_MP") == 0 )
		{
			statEntry.RANDOM_MP = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_LVL") == 0 )
		{
			statEntry.RANDOM_LVL = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else if ( name.compare("RANDOM_GOLD") == 0 )
		{
			statEntry.RANDOM_GOLD = static_cast<Sint32>(itr->value.GetInt());
			return true;
		}
		else
		{
			for ( int i = 0; i < NUMPROFICIENCIES; ++i )
			{
				if ( name.compare(getSkillLangEntry(i)) == 0 )
				{
					statEntry.PROFICIENCIES[i] = static_cast<Sint32>(itr->value.GetInt());
					return true;
				}
			}
		}
		return false;
	}

	void addArrayMemberFromItem(rapidjson::Document& d, std::string rootKey, Item* item)
	{
		if ( item )
		{
			rapidjson::Value itemObject(rapidjson::kObjectType);
			ItemEntry itemEntry(*item);
			itemEntry.setValueFromAttributes(d, itemObject);
			CustomHelpers::addArrayMemberToSubkey(d, rootKey, itemObject);
		}
	}
	void addMemberFromItem(rapidjson::Document& d, std::string rootKey, std::string key, Item* item)
	{
		if ( item )
		{
			rapidjson::Value itemObject(rapidjson::kObjectType);
			ItemEntry itemEntry(*item);
			itemEntry.setValueFromAttributes(d, itemObject);
			CustomHelpers::addMemberToSubkey(d, rootKey, key.c_str(), itemObject);
		}
	}

	void writeToFile(rapidjson::Document& d, std::string monsterFileName)
	{
		int filenum = 0;
		DynamicString testPath = "/data/custom-monsters/monster_" + monsterFileName + "_export" + std::to_string(filenum) + ".json";
		while ( PHYSFS_getRealDir(testPath.c_str()) != nullptr && filenum < 1000 )
		{
			++filenum;
			testPath = "/data/custom-monsters/monster_" + monsterFileName + "_export" + std::to_string(filenum) + ".json";
		}
		DynamicString outputPath = PHYSFS_getRealDir("/data/custom-monsters/");
		outputPath.append(PHYSFS_getDirSeparator());
		DynamicString fileName = "data/custom-monsters/monster_" + monsterFileName + "_export" + std::to_string(filenum) + ".json";
		outputPath.append(fileName.c_str());


		File* fp = FileIO::open(outputPath.c_str(), "wb");
		if ( !fp )
		{
			return;
		}
		rapidjson::StringBuffer os;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(os);
		d.Accept(writer);
		fp->write(os.GetString(), sizeof(char), os.GetSize());
		FileIO::close(fp);
	}

	StatEntry* readFromFile(std::string monsterFileName)
	{
		DynamicString filePath = "/data/custom-monsters/";
		filePath.append(monsterFileName);
		if ( filePath.find(".json") == std::string::npos )
		{
			filePath.append(".json");
		}
		if ( PHYSFS_getRealDir(filePath.c_str()) )
		{
			DynamicString inputPath = PHYSFS_getRealDir(filePath.c_str());
			inputPath.append(filePath);

			File* fp = FileIO::open(inputPath.c_str(), "rb");
			if ( !fp )
			{
				printlog("[JSON]: Error: Could not locate json file %s", inputPath.c_str());
				return nullptr;
			}
			char buf[65536];
			int count = fp->read(buf, sizeof(buf[0]), sizeof(buf));
			buf[count] = '\0';
			rapidjson::StringStream is(buf);
			FileIO::close(fp);

			rapidjson::Document d;
			d.ParseStream(is);


			if ( !d.HasMember("version") )
			{
				printlog("[JSON]: Error: No 'version' value in json file, or JSON syntax incorrect! %s", inputPath.c_str());
				return nullptr;
			}
			StatEntry* statEntry = new StatEntry();
			int version = d["version"].GetInt();
			const rapidjson::Value& stats = d["stats"];
			for ( rapidjson::Value::ConstMemberIterator stat_itr = stats.MemberBegin(); stat_itr != stats.MemberEnd(); ++stat_itr )
			{
				readKeyToStatEntry(*statEntry, stat_itr);
			}
			const rapidjson::Value& miscStats = d["misc_stats"];
			for ( rapidjson::Value::ConstMemberIterator stat_itr = miscStats.MemberBegin(); stat_itr != miscStats.MemberEnd(); ++stat_itr )
			{
				readKeyToStatEntry(*statEntry, stat_itr);
			}
			const rapidjson::Value& proficiencies = d["proficiencies"];
			for ( rapidjson::Value::ConstMemberIterator stat_itr = proficiencies.MemberBegin(); stat_itr != proficiencies.MemberEnd(); ++stat_itr )
			{
				readKeyToStatEntry(*statEntry, stat_itr);
			}
			const rapidjson::Value& equipped_items = d["equipped_items"];
			for ( rapidjson::Value::ConstMemberIterator itemSlot_itr = equipped_items.MemberBegin(); itemSlot_itr != equipped_items.MemberEnd(); ++itemSlot_itr )
			{
				DynamicString slotName = itemSlot_itr->name.GetString();
				if ( itemSlot_itr->value.IsArray() )
				{
					std::vector<std::pair<ItemEntry, int>> itemsToChoose;
					// a selection of items in the slot. need to choose 1.
					for ( rapidjson::Value::ConstValueIterator itemArray_itr = itemSlot_itr->value.Begin(); itemArray_itr != itemSlot_itr->value.End(); ++itemArray_itr )
					{
						ItemEntry item;
						for ( rapidjson::Value::ConstMemberIterator item_itr = itemArray_itr->MemberBegin(); item_itr != itemArray_itr->MemberEnd(); ++item_itr )
						{
							item.readKeyToItemEntry(item_itr);
						}
						itemsToChoose.push_back(std::make_pair(item, getSlotFromKeyName(slotName)));
					}
					if ( itemsToChoose.size() > 0 )
					{
						std::vector<unsigned int> itemChances(itemsToChoose.size(), 0);
						int index = 0;
						for ( auto& pair : itemsToChoose )
						{
							itemChances.at(index) = pair.first.weightedChance;
							++index;
						}

						int result = monster_stat_rng.discrete(itemChances.data(), itemChances.size());
						statEntry->equipped_items.push_back(std::make_pair(itemsToChoose.at(result).first, itemsToChoose.at(result).second));
					}
				}
				else if ( itemSlot_itr->value.MemberCount() > 0 )
				{
					ItemEntry item;
					for ( rapidjson::Value::ConstMemberIterator item_itr = itemSlot_itr->value.MemberBegin(); item_itr != itemSlot_itr->value.MemberEnd(); ++item_itr )
					{
						item.readKeyToItemEntry(item_itr);
					}
					statEntry->equipped_items.push_back(std::make_pair(item, getSlotFromKeyName(slotName)));
				}
			}
			const rapidjson::Value& inventory_items = d["inventory_items"];
			for ( rapidjson::Value::ConstValueIterator itemSlot_itr = inventory_items.Begin(); itemSlot_itr != inventory_items.End(); ++itemSlot_itr )
			{
				if ( itemSlot_itr->IsArray() )
				{
					std::vector<ItemEntry> itemsToChoose;
					// a selection of items in the slot. need to choose 1.
					for ( rapidjson::Value::ConstValueIterator itemArray_itr = itemSlot_itr->Begin(); itemArray_itr != itemSlot_itr->End(); ++itemArray_itr )
					{
						ItemEntry item;
						for ( rapidjson::Value::ConstMemberIterator item_itr = itemArray_itr->MemberBegin(); item_itr != itemArray_itr->MemberEnd(); ++item_itr )
						{
							item.readKeyToItemEntry(item_itr);
						}
						itemsToChoose.push_back(item);
					}
					if ( itemsToChoose.size() > 0 )
					{
						std::vector<unsigned int> itemChances(itemsToChoose.size(), 0);
						int index = 0;
						for ( auto& i : itemsToChoose )
						{
							itemChances.at(index) = i.weightedChance;
							++index;
						}

						int result = monster_stat_rng.discrete(itemChances.data(), itemChances.size());
						statEntry->inventory_items.push_back(itemsToChoose.at(result));
					}
				}
				else
				{
					ItemEntry item;
					for ( rapidjson::Value::ConstMemberIterator item_itr = itemSlot_itr->MemberBegin(); item_itr != itemSlot_itr->MemberEnd(); ++item_itr )
					{
						item.readKeyToItemEntry(item_itr);
					}
					statEntry->inventory_items.push_back(item);
				}
			}
			if ( d.HasMember("followers") )
			{
				const rapidjson::Value& numFollowersVal = d["followers"]["num_followers"];
				statEntry->numFollowers = numFollowersVal.GetInt();
				const rapidjson::Value& followers = d["followers"]["follower_variants"];

				statEntry->followerVariants.clear();
				for ( rapidjson::Value::ConstMemberIterator follower_itr = followers.MemberBegin(); follower_itr != followers.MemberEnd(); ++follower_itr )
				{
										MonsterStatCustomManager::StatEntry::VariantPair_t vp;
					vp.name = follower_itr->name.GetString();
					vp.chance = follower_itr->value.GetInt();
					statEntry->followerVariants.push_back(vp);
				}
			}
			if ( d.HasMember("properties") )
			{
				if ( d["properties"].HasMember("monster_name_always_display_as_generic_species") )
				{
					statEntry->isMonsterNameGeneric = d["properties"]["monster_name_always_display_as_generic_species"].GetBool();
				}
				if ( d["properties"].HasMember("populate_empty_equipped_items_with_default") )
				{
					statEntry->useDefaultEquipment = d["properties"]["populate_empty_equipped_items_with_default"].GetBool();
				}
				if ( d["properties"].HasMember("populate_default_inventory") )
				{
					statEntry->useDefaultInventoryItems = d["properties"]["populate_default_inventory"].GetBool();
				}
				if ( d["properties"].HasMember("disable_miniboss_chance") )
				{
					statEntry->disableMiniboss = d["properties"]["disable_miniboss_chance"].GetBool();
				}
				if ( d["properties"].HasMember("force_player_recruitable") )
				{
					statEntry->forceRecruitableToPlayer = d["properties"]["force_player_recruitable"].GetBool();
				}
				if ( d["properties"].HasMember("force_player_friendly") )
				{
					statEntry->forceFriendlyToPlayer = d["properties"]["force_player_friendly"].GetBool();
				}
				if ( d["properties"].HasMember("force_player_enemy") )
				{
					statEntry->forceEnemyToPlayer = d["properties"]["force_player_enemy"].GetBool();
				}
				if ( d["properties"].HasMember("disable_item_drops") )
				{
					statEntry->disableItemDrops = d["properties"]["disable_item_drops"].GetBool();
				}
				if ( d["properties"].HasMember("xp_award_percent") )
				{
					statEntry->xpAwardPercent = d["properties"]["xp_award_percent"].GetInt();
				}
				if ( d["properties"].HasMember("enable_casting_inventory_spellbooks") )
				{
					statEntry->castSpellbooksFromInventory = d["properties"]["enable_casting_inventory_spellbooks"].GetBool();
				}
				if ( d["properties"].HasMember("spellbook_cast_cooldown") )
				{
					statEntry->spellbookCastCooldown = d["properties"]["spellbook_cast_cooldown"].GetInt();
				}
			}
			if ( d.HasMember("shopkeeper_properties") )
			{
				if ( d["shopkeeper_properties"].HasMember("store_type_chances") )
				{
					for ( rapidjson::Value::ConstMemberIterator types_itr = d["shopkeeper_properties"]["store_type_chances"].MemberBegin(); 
						types_itr != d["shopkeeper_properties"]["store_type_chances"].MemberEnd(); ++types_itr )
					{
											MonsterStatCustomManager::StatEntry::VariantPair_t vp2;
					vp2.name = types_itr->name.GetString();
					vp2.chance = types_itr->value.GetInt();
					statEntry->shopkeeperStoreTypes.push_back(vp2);
					}
					if ( !statEntry->shopkeeperStoreTypes.empty() )
					{
						std::vector<unsigned int> storeChances(statEntry->shopkeeperStoreTypes.size(), 0);
						int index = 0;
						for ( auto& chance : storeChances )
						{
							chance = statEntry->shopkeeperStoreTypes.at(index).chance;
							++index;
						}

						DynamicString result = statEntry->shopkeeperStoreTypes.at(monster_stat_rng.discrete(storeChances.data(), storeChances.size())).name;
						index = 0;
						for ( auto& lookup : shopkeeperTypeStrings )
						{
							if ( lookup.compare(result) == 0 )
							{
								statEntry->chosenShopkeeperStore = index;
								break;
							}
							++index;
						}
					}
					if ( d["shopkeeper_properties"].HasMember("generate_default_shop_items") )
					{
						statEntry->shopkeeperGenDefaultItems = d["shopkeeper_properties"]["generate_default_shop_items"].GetBool();
					}
					if ( d["shopkeeper_properties"].HasMember("num_generated_items_min") )
					{
						statEntry->shopkeeperMinItems = d["shopkeeper_properties"]["num_generated_items_min"].GetInt();
					}
					if ( d["shopkeeper_properties"].HasMember("num_generated_items_max") )
					{
						statEntry->shopkeeperMaxItems = d["shopkeeper_properties"]["num_generated_items_max"].GetInt();
					}
					if ( d["shopkeeper_properties"].HasMember("generated_item_blessing_max") )
					{
						statEntry->shopkeeperMaxGeneratedBlessing = d["shopkeeper_properties"]["generated_item_blessing_max"].GetInt();
					}
				}
			}
			printlog("[JSON]: Successfully read json file %s", inputPath.c_str());
			return statEntry;
		}
		else
		{
			printlog("[JSON]: Error: Could not locate json file %s", filePath.c_str());
		}
		return nullptr;
	}
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
		void addVariant(std::string variantName, int chance)
		{
			MonsterVariant_t v;
			v.name = variantName.c_str();
			v.chance = chance;
			variants.push_back(v);
		}
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

	void readFromFile(Uint32 seed)
	{
		monster_curve_rng.seedBytes(&seed, sizeof(seed));
		MonsterStatCustomManager::monster_stat_rng.seedBytes(&seed, sizeof(seed));

		allLevelCurves.clear();
		usingCustomManager = false;
		if ( PHYSFS_getRealDir("/data/monstercurve.json") )
		{
			DynamicString inputPath = PHYSFS_getRealDir("/data/monstercurve.json");
			inputPath.append("/data/monstercurve.json");

			File* fp = FileIO::open(inputPath.c_str(), "rb");
			if ( !fp )
			{
				printlog("[JSON]: Error: Could not locate json file %s", inputPath.c_str());
				return;
			}
			char buf[65536];
			int count = fp->read(buf, sizeof(buf[0]), sizeof(buf));
			buf[count] = '\0';
			rapidjson::StringStream is(buf);
			FileIO::close(fp);

			rapidjson::Document d;
			d.ParseStream(is);
			if ( !d.HasMember("version") )
			{
				printlog("[JSON]: Error: No 'version' value in json file, or JSON syntax incorrect! %s", inputPath.c_str());
				return;
			}
			int version = d["version"].GetInt();

			if ( d.HasMember("levels") )
			{
				usingCustomManager = true;
				const rapidjson::Value& levels = d["levels"];
				for ( rapidjson::Value::ConstMemberIterator map_itr = levels.MemberBegin(); map_itr != levels.MemberEnd(); ++map_itr )
				{
					LevelCurve newCurve;
					newCurve.mapName = map_itr->name.GetString();
					if ( map_itr->value.HasMember("random_generation_monsters") )
					{
						const rapidjson::Value& randomGeneration = map_itr->value["random_generation_monsters"];
						for ( rapidjson::Value::ConstValueIterator monsters_itr = randomGeneration.Begin(); monsters_itr != randomGeneration.End(); ++monsters_itr )
						{
							const rapidjson::Value& monster = *monsters_itr;
							MonsterCurveEntry newMonster(monster["name"].GetString(),
								monster["dungeon_depth_minimum"].GetInt(),
								monster["dungeon_depth_maximum"].GetInt(),
								monster["weighted_chance"].GetInt(),
								"");

							if ( monster.HasMember("variants") )
							{
								for ( rapidjson::Value::ConstMemberIterator var_itr = monster["variants"].MemberBegin();
									var_itr != monster["variants"].MemberEnd(); ++var_itr )
								{
									newMonster.addVariant(var_itr->name.GetString(), var_itr->value.GetInt());
								}
							}
							newCurve.monsterCurve.push_back(newMonster);
						}
					}

					if ( map_itr->value.HasMember("fixed_monsters") )
					{
						const rapidjson::Value& fixedGeneration = map_itr->value["fixed_monsters"];
						for ( rapidjson::Value::ConstValueIterator monsters_itr = fixedGeneration.Begin(); monsters_itr != fixedGeneration.End(); ++monsters_itr )
						{
							const rapidjson::Value& monster = *monsters_itr;
							MonsterCurveEntry newMonster(monster["name"].GetString(), 0, 255, 1, "");

							if ( monster.HasMember("variants") )
							{
								for ( rapidjson::Value::ConstMemberIterator var_itr = monster["variants"].MemberBegin();
									var_itr != monster["variants"].MemberEnd(); ++var_itr )
								{
									newMonster.addVariant(var_itr->name.GetString(), var_itr->value.GetInt());
								}
							}
							newCurve.fixedSpawns.push_back(newMonster);
						}
					}
					allLevelCurves.push_back(newCurve);
				}
			}
			printCurve(allLevelCurves);
			printlog("[JSON]: Successfully read json file %s", inputPath.c_str());
		}
	}

	static int getMonsterTypeFromString(std::string monsterStr)
	{
		if ( monsterStr.compare("") == 0 )
		{
			return NOTHING;
		}
		for ( int i = NOTHING; i < NUMMONSTERS; ++i )
		{
			if ( monsterStr.compare(monstertypename[i]) == 0 )
			{
				return i;
			}
		}
		return NOTHING;
	}
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
	bool curveExistsForCurrentMapName(std::string currentMap)
	{
		if ( !inUse() )
		{
			return false;
		}
		if ( currentMap.compare("") == 0 )
		{
			return false;
		}
		for ( LevelCurve curve : allLevelCurves )
		{
			if ( curve.mapName.compare(currentMap) == 0 )
			{
				//printlog("[MonsterCurveCustomManager]: curveExistsForCurrentMapName: true");
				return true;
			}
		}
		return false;
	}
	int rollMonsterFromCurve(std::string currentMap)
	{
		std::vector<unsigned int> monsterCurveChances(NUMMONSTERS, 0);

		for ( LevelCurve curve : allLevelCurves )
		{
			if ( curve.mapName.compare(currentMap) == 0 )
			{
				for ( MonsterCurveEntry& monster : curve.monsterCurve )
				{
					if ( currentlevel >= monster.levelmin && currentlevel <= monster.levelmax )
					{
						if ( monster.monsterType != NOTHING )
						{
							monsterCurveChances[monster.monsterType] += monster.chance;
						}
					}
					else
					{
						if ( monster.fallbackMonsterType != NOTHING )
						{
							monsterCurveChances[monster.fallbackMonsterType] += monster.chance;
						}
					}
				}
				int result = monster_curve_rng.discrete(monsterCurveChances.data(), monsterCurveChances.size());
				//printlog("[MonsterCurveCustomManager]: Rolled: %d", result);
				return result;
			}
		}
		printlog("[MonsterCurveCustomManager]: Error: default to nothing.");
		return NOTHING;
	}
	std::string rollMonsterVariant(DynamicString currentMap, int monsterType)
	{
		for ( LevelCurve& curve : allLevelCurves )
		{
			if ( curve.mapName.compare(currentMap) == 0 )
			{
				std::vector<DynamicString> variantResults;
				std::vector<unsigned int> variantChances;
				for ( MonsterCurveEntry& monster : curve.monsterCurve )
				{
					if ( currentlevel >= monster.levelmin && currentlevel <= monster.levelmax )
					{
						if ( monster.monsterType == monsterType && monster.variants.size() > 0 )
						{
							for ( auto& pair : monster.variants )
							{
								auto find = std::find(variantResults.begin(), variantResults.end(), pair.name);
								if ( find == variantResults.end() )
								{
									variantResults.push_back(pair.name);
									variantChances.push_back(pair.chance);
								}
								else
								{
									size_t dist = static_cast<size_t>(std::distance(variantResults.begin(), find));
									variantChances.at(dist) += pair.chance;
								}
							}

						}
					}
				}
				if ( !variantResults.empty() )
				{
					int result = monster_curve_rng.discrete(variantChances.data(), variantChances.size());
					return variantResults[result];
				}
			}
		}
		return "default";
	}
	std::string rollFixedMonsterVariant(DynamicString currentMap, int monsterType)
	{
		for ( LevelCurve& curve : allLevelCurves )
		{
			if ( curve.mapName.compare(currentMap) == 0 )
			{
				for ( MonsterCurveEntry& monster : curve.fixedSpawns )
				{
					if ( monster.monsterType == monsterType && monster.variants.size() > 0 )
					{
						std::vector<unsigned int> variantChances(monster.variants.size(), 0);
						int index = 0;
						for ( auto& pair : monster.variants )
						{
							variantChances.at(index) = pair.chance;
							++index;
						}

						int result = monster_curve_rng.discrete(variantChances.data(), variantChances.size());
						return monster.variants.at(result).name.c_str();
					}
				}
			}
		}
		return "default";
	}

	void createMonsterFromFile(Entity* entity, Stat* myStats, const std::string& filename, Monster& outMonsterType)
	{
		MonsterStatCustomManager::StatEntry* statEntry = monsterStatCustomManager.readFromFile(filename.c_str());
		if ( statEntry )
		{
			statEntry->setStatsAndEquipmentToMonster(myStats);
			outMonsterType = myStats->type;
			while ( statEntry->numFollowers > 0 )
			{
				DynamicString followerName = statEntry->getFollowerVariant();
				if ( followerName.compare("") && followerName.compare("none") )
				{
					followersToGenerateForLeaders.push_back(FollowerGenerateDetails_t());
					auto& entry = followersToGenerateForLeaders.back();
					entry.followerName = followerName;
					entry.x = entity->x;
					entry.y = entity->y;
					entry.uid = entity->getUID();
					entry.leaderType = myStats->type;
				}
				--statEntry->numFollowers;
			}
			delete statEntry;
		}
	}

	void generateFollowersForLeaders()
	{
		if ( multiplayer != CLIENT )
		{
			for ( auto& entry : followersToGenerateForLeaders )
			{
				MonsterStatCustomManager::StatEntry* followerEntry = monsterStatCustomManager.readFromFile(entry.followerName.c_str());
				if ( followerEntry )
				{
					Entity* summonedFollower = summonMonsterNoSmoke(static_cast<Monster>(followerEntry->type), entry.x, entry.y);
					if ( summonedFollower )
					{
						if ( summonedFollower->getStats() )
						{
							followerEntry->setStatsAndEquipmentToMonster(summonedFollower->getStats());
							summonedFollower->getStats()->leader_uid = entry.uid;
						}
						summonedFollower->seedEntityRNG(monster_curve_rng.getU32());
					}
					delete followerEntry;
				}
				else
				{
					Entity* summonedFollower = summonMonsterNoSmoke(static_cast<Monster>(entry.leaderType), entry.x, entry.y);
					if ( summonedFollower )
					{
						if ( summonedFollower->getStats() )
						{
							summonedFollower->getStats()->leader_uid = entry.uid;
						}
						summonedFollower->seedEntityRNG(monster_curve_rng.getU32());
					}
				}
			}
		}
		followersToGenerateForLeaders.clear();
	}

	void writeSampleToDocument()
	{
		rapidjson::Document d;
		d.SetObject();

		CustomHelpers::addMemberToRoot(d, "version", rapidjson::Value(1));
		rapidjson::Value levelObj(rapidjson::kObjectType);
		levelObj.AddMember("The Mines", rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		levelObj["The Mines"].AddMember("fixed_monsters", rapidjson::Value(rapidjson::kArrayType), d.GetAllocator());

		auto& fm = levelObj["The Mines"]["fixed_monsters"];
		fm.PushBack(rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		fm[rapidjson::SizeType(0)].AddMember("name", "rat", d.GetAllocator());
		fm[rapidjson::SizeType(0)].AddMember("variants", rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		fm[rapidjson::SizeType(0)]["variants"].AddMember("default", rapidjson::Value(1), d.GetAllocator());

		fm.PushBack(rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		fm[rapidjson::SizeType(1)].AddMember("name", "skeleton", d.GetAllocator());
		fm[rapidjson::SizeType(1)].AddMember("variants", rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		fm[rapidjson::SizeType(1)]["variants"].AddMember("default", rapidjson::Value(1), d.GetAllocator());
		
		fm.PushBack(rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		fm[rapidjson::SizeType(2)].AddMember("name", "spider", d.GetAllocator());
		fm[rapidjson::SizeType(2)].AddMember("variants", rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		fm[rapidjson::SizeType(2)]["variants"].AddMember("default", rapidjson::Value(1), d.GetAllocator());

		fm.PushBack(rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		fm[rapidjson::SizeType(3)].AddMember("name", "troll", d.GetAllocator());
		fm[rapidjson::SizeType(3)].AddMember("variants", rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		fm[rapidjson::SizeType(3)]["variants"].AddMember("default", rapidjson::Value(1), d.GetAllocator());

		levelObj["The Mines"].AddMember("random_generation_monsters", rapidjson::Value(rapidjson::kArrayType), d.GetAllocator());

		auto& mines = levelObj["The Mines"]["random_generation_monsters"];
		mines.PushBack(rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		mines[rapidjson::SizeType(0)].AddMember("name", "rat", d.GetAllocator());
		mines[rapidjson::SizeType(0)].AddMember("weighted_chance", rapidjson::Value(4), d.GetAllocator());
		mines[rapidjson::SizeType(0)].AddMember("dungeon_depth_minimum", rapidjson::Value(0), d.GetAllocator());
		mines[rapidjson::SizeType(0)].AddMember("dungeon_depth_maximum", rapidjson::Value(99), d.GetAllocator());
		mines[rapidjson::SizeType(0)].AddMember("variants", rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		mines[rapidjson::SizeType(0)]["variants"].AddMember("default", rapidjson::Value(1), d.GetAllocator());

		mines.PushBack(rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		mines[rapidjson::SizeType(1)].AddMember("name", "skeleton", d.GetAllocator());
		mines[rapidjson::SizeType(1)].AddMember("weighted_chance", rapidjson::Value(4), d.GetAllocator());
		mines[rapidjson::SizeType(1)].AddMember("dungeon_depth_minimum", rapidjson::Value(0), d.GetAllocator());
		mines[rapidjson::SizeType(1)].AddMember("dungeon_depth_maximum", rapidjson::Value(99), d.GetAllocator());
		mines[rapidjson::SizeType(1)].AddMember("variants", rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		mines[rapidjson::SizeType(1)]["variants"].AddMember("default", rapidjson::Value(1), d.GetAllocator());

		mines.PushBack(rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		mines[rapidjson::SizeType(2)].AddMember("name", "spider", d.GetAllocator());
		mines[rapidjson::SizeType(2)].AddMember("weighted_chance", rapidjson::Value(1), d.GetAllocator());
		mines[rapidjson::SizeType(2)].AddMember("dungeon_depth_minimum", rapidjson::Value(2), d.GetAllocator());
		mines[rapidjson::SizeType(2)].AddMember("dungeon_depth_maximum", rapidjson::Value(99), d.GetAllocator());
		mines[rapidjson::SizeType(2)].AddMember("variants", rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		mines[rapidjson::SizeType(2)]["variants"].AddMember("default", rapidjson::Value(1), d.GetAllocator());

		mines.PushBack(rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		mines[rapidjson::SizeType(3)].AddMember("name", "troll", d.GetAllocator());
		mines[rapidjson::SizeType(3)].AddMember("weighted_chance", rapidjson::Value(1), d.GetAllocator());
		mines[rapidjson::SizeType(3)].AddMember("dungeon_depth_minimum", rapidjson::Value(2), d.GetAllocator());
		mines[rapidjson::SizeType(3)].AddMember("dungeon_depth_maximum", rapidjson::Value(99), d.GetAllocator());
		mines[rapidjson::SizeType(3)].AddMember("variants", rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		mines[rapidjson::SizeType(3)]["variants"].AddMember("default", rapidjson::Value(1), d.GetAllocator());

		levelObj.AddMember("The Swamp", rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		levelObj["The Swamp"].AddMember("random_generation_monsters", rapidjson::Value(rapidjson::kArrayType), d.GetAllocator());
		levelObj["The Swamp"]["random_generation_monsters"].PushBack(rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());

		auto& swamp = levelObj["The Swamp"]["random_generation_monsters"];
		swamp[rapidjson::SizeType(0)].AddMember("name", "spider", d.GetAllocator());
		swamp[rapidjson::SizeType(0)].AddMember("weighted_chance", rapidjson::Value(2), d.GetAllocator());
		swamp[rapidjson::SizeType(0)].AddMember("dungeon_depth_minimum", rapidjson::Value(0), d.GetAllocator());
		swamp[rapidjson::SizeType(0)].AddMember("dungeon_depth_maximum", rapidjson::Value(99), d.GetAllocator());
		swamp[rapidjson::SizeType(0)].AddMember("variants", rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		swamp[rapidjson::SizeType(0)]["variants"].AddMember("default", rapidjson::Value(1), d.GetAllocator());

		swamp.PushBack(rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		swamp[rapidjson::SizeType(1)].AddMember("name", "goblin", d.GetAllocator());
		swamp[rapidjson::SizeType(1)].AddMember("weighted_chance", rapidjson::Value(3), d.GetAllocator());
		swamp[rapidjson::SizeType(1)].AddMember("dungeon_depth_minimum", rapidjson::Value(0), d.GetAllocator());
		swamp[rapidjson::SizeType(1)].AddMember("dungeon_depth_maximum", rapidjson::Value(99), d.GetAllocator());
		swamp[rapidjson::SizeType(1)].AddMember("variants", rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		swamp[rapidjson::SizeType(1)]["variants"].AddMember("default", rapidjson::Value(1), d.GetAllocator());

		swamp.PushBack(rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		swamp[rapidjson::SizeType(2)].AddMember("name", "slime", d.GetAllocator());
		swamp[rapidjson::SizeType(2)].AddMember("weighted_chance", rapidjson::Value(3), d.GetAllocator());
		swamp[rapidjson::SizeType(2)].AddMember("dungeon_depth_minimum", rapidjson::Value(0), d.GetAllocator());
		swamp[rapidjson::SizeType(2)].AddMember("dungeon_depth_maximum", rapidjson::Value(99), d.GetAllocator());
		swamp[rapidjson::SizeType(2)].AddMember("variants", rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		swamp[rapidjson::SizeType(2)]["variants"].AddMember("default", rapidjson::Value(1), d.GetAllocator());

		swamp.PushBack(rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		swamp[rapidjson::SizeType(3)].AddMember("name", "ghoul", d.GetAllocator());
		swamp[rapidjson::SizeType(3)].AddMember("weighted_chance", rapidjson::Value(2), d.GetAllocator());
		swamp[rapidjson::SizeType(3)].AddMember("dungeon_depth_minimum", rapidjson::Value(0), d.GetAllocator());
		swamp[rapidjson::SizeType(3)].AddMember("dungeon_depth_maximum", rapidjson::Value(99), d.GetAllocator());
		swamp[rapidjson::SizeType(3)].AddMember("variants", rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		swamp[rapidjson::SizeType(3)]["variants"].AddMember("default", rapidjson::Value(1), d.GetAllocator());

		levelObj.AddMember("My level", rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());

		levelObj["My level"].AddMember("random_generation_monsters", rapidjson::Value(rapidjson::kArrayType), d.GetAllocator());
		levelObj["My level"]["random_generation_monsters"].PushBack(rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		auto& customLevel = levelObj["My level"]["random_generation_monsters"];
		customLevel[rapidjson::SizeType(0)].AddMember("name", "demon", d.GetAllocator());
		customLevel[rapidjson::SizeType(0)].AddMember("weighted_chance", rapidjson::Value(1), d.GetAllocator());
		customLevel[rapidjson::SizeType(0)].AddMember("dungeon_depth_minimum", rapidjson::Value(0), d.GetAllocator());
		customLevel[rapidjson::SizeType(0)].AddMember("dungeon_depth_maximum", rapidjson::Value(99), d.GetAllocator());
		customLevel[rapidjson::SizeType(0)].AddMember("variants", rapidjson::Value(rapidjson::kObjectType), d.GetAllocator());
		customLevel[rapidjson::SizeType(0)]["variants"].AddMember("default", rapidjson::Value(1), d.GetAllocator());

		CustomHelpers::addMemberToRoot(d, "levels", levelObj);

		writeToFile(d);
	}

	void writeToFile(rapidjson::Document& d)
	{
		int filenum = 0;
		DynamicString testPath = "/data/monstercurve_export" + std::to_string(filenum) + ".json";
		while ( PHYSFS_getRealDir(testPath.c_str()) != nullptr && filenum < 1000 )
		{
			++filenum;
			testPath = "/data/monstercurve_export" + std::to_string(filenum) + ".json";
		}
		DynamicString outputPath = PHYSFS_getRealDir("/data/");
		outputPath.append(PHYSFS_getDirSeparator());
		DynamicString fileName = "data/monstercurve_export" + std::to_string(filenum) + ".json";
		outputPath.append(fileName.c_str());

		File* fp = FileIO::open(outputPath.c_str(), "wb");
		if ( !fp )
		{
			return;
		}
		rapidjson::StringBuffer os;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(os);
		d.Accept(writer);
		fp->write(os.GetString(), sizeof(char), os.GetSize());

		FileIO::close(fp);
	}
};
extern MonsterCurveCustomManager monsterCurveCustomManager;

class GameplayCustomManager
{
public:
	bool usingCustomManager = false;
	int xpShareRange = XPSHARERANGE;
	std::pair<std::unordered_set<int>, std::unordered_set<int>> minotaurForceEnableFloors;
	std::pair<std::unordered_set<int>, std::unordered_set<int>> minotaurForceDisableFloors;
	std::pair<std::unordered_set<int>, std::unordered_set<int>> hungerDisableFloors;
	std::pair<std::unordered_set<int>, std::unordered_set<int>> herxChatterDisableFloors;
	std::pair<std::unordered_set<int>, std::unordered_set<int>> minimapDisableFloors;
	int globalXPPercent = 100;
	int globalGoldPercent = 100;
	bool minimapShareProgress = false;
	int playerWeightPercent = 100;
	double playerSpeedMax = 12.5;
	inline bool inUse() { return usingCustomManager; };
	void resetValues()
	{
		usingCustomManager = false;
		xpShareRange = XPSHARERANGE;
		globalXPPercent = 100;
		globalGoldPercent = 100;
		minimapShareProgress = false;
		playerWeightPercent = 100;
		playerSpeedMax = 12.5;

		minotaurForceEnableFloors.first.clear();
		minotaurForceEnableFloors.second.clear();
		minotaurForceDisableFloors.first.clear();
		minotaurForceDisableFloors.second.clear();
		hungerDisableFloors.first.clear();
		hungerDisableFloors.second.clear();
		herxChatterDisableFloors.first.clear();
		herxChatterDisableFloors.second.clear();
		minimapDisableFloors.first.clear();
		minimapDisableFloors.second.clear();
		allMapGenerations.clear();
	}

	class MapGeneration
	{
	public:
		MapGeneration(std::string name) { mapName = name; };
		DynamicString mapName = "";
		DynamicArrayStr trapTypes;
		std::unordered_set<int> minoFloors;
		std::unordered_set<int> darkFloors;
		std::unordered_set<int> shopFloors;
		std::unordered_set<int> npcSpawnFloors;
		bool usingTrapTypes = false;
		int minoPercent = -1;
		int shopPercent = -1;
		int darkPercent = -1;
		int npcSpawnPercent = -1;
	};

	std::vector<MapGeneration> allMapGenerations;
	bool mapGenerationExistsForMapName(std::string name)
	{
		for ( auto& it : allMapGenerations )
		{
			if ( it.mapName.compare(name) == 0 )
			{
				return true;
			}
		}
		return false;
	}
	MapGeneration* getMapGenerationForMapName(std::string name)
	{
		for ( auto& it : allMapGenerations )
		{
			if ( it.mapName.compare(name) == 0 )
			{
				return &it;
			}
		}
		return nullptr;
	}

	void writeAllToDocument()
	{
		rapidjson::Document d;
		d.SetObject();

		CustomHelpers::addMemberToRoot(d, "version", rapidjson::Value(1));
		CustomHelpers::addMemberToRoot(d, "xp_share_range", rapidjson::Value(xpShareRange));
		CustomHelpers::addMemberToRoot(d, "global_xp_award_percent", rapidjson::Value(globalXPPercent));
		CustomHelpers::addMemberToRoot(d, "global_gold_drop_scale_percent", rapidjson::Value(globalGoldPercent));
		CustomHelpers::addMemberToRoot(d, "player_share_minimap_progress", rapidjson::Value(minimapShareProgress));
		CustomHelpers::addMemberToRoot(d, "player_speed_weight_impact_percent", rapidjson::Value(playerWeightPercent));
		CustomHelpers::addMemberToRoot(d, "player_speed_max", rapidjson::Value(playerSpeedMax));

		rapidjson::Value obj(rapidjson::kObjectType);
		rapidjson::Value arr(rapidjson::kArrayType);
		CustomHelpers::addMemberToRoot(d, "minotaur_force_disable_on_floors", obj);
		CustomHelpers::addMemberToSubkey(d, "minotaur_force_disable_on_floors", "normal_floors", arr);
		CustomHelpers::addMemberToSubkey(d, "minotaur_force_disable_on_floors", "secret_floors", arr);
		CustomHelpers::addMemberToRoot(d, "minotaur_force_enable_on_floors", obj);
		CustomHelpers::addMemberToSubkey(d, "minotaur_force_enable_on_floors", "normal_floors", arr);
		CustomHelpers::addMemberToSubkey(d, "minotaur_force_enable_on_floors", "secret_floors", arr);
		CustomHelpers::addMemberToRoot(d, "disable_herx_messages_on_floors", obj);
		CustomHelpers::addMemberToSubkey(d, "disable_herx_messages_on_floors", "normal_floors", arr);
		CustomHelpers::addMemberToSubkey(d, "disable_herx_messages_on_floors", "secret_floors", arr);
		CustomHelpers::addMemberToRoot(d, "disable_minimap_on_floors", obj);
		CustomHelpers::addMemberToSubkey(d, "disable_minimap_on_floors", "normal_floors", arr);
		CustomHelpers::addMemberToSubkey(d, "disable_minimap_on_floors", "secret_floors", arr);

		rapidjson::Value mapGenObj;
		mapGenObj.SetObject();
		CustomHelpers::addMemberToRoot(d, "map_generation", mapGenObj);
		rapidjson::Value key1("The Mines", d.GetAllocator());
		rapidjson::Value minesObj(rapidjson::kObjectType);

		rapidjson::Value trapArray1(rapidjson::kArrayType);
		trapArray1.PushBack("boulders", d.GetAllocator());
		minesObj.AddMember("trap_generation_types", trapArray1, d.GetAllocator());
		minesObj.AddMember("minotaur_floors", rapidjson::Value(rapidjson::kArrayType), d.GetAllocator());
		minesObj["minotaur_floors"].PushBack(2, d.GetAllocator());
		minesObj["minotaur_floors"].PushBack(3, d.GetAllocator());
		minesObj.AddMember("minotaur_floor_percent", rapidjson::Value(50), d.GetAllocator());

		minesObj.AddMember("dark_floors", rapidjson::Value(rapidjson::kArrayType), d.GetAllocator());
		minesObj["dark_floors"].PushBack(1, d.GetAllocator());
		minesObj["dark_floors"].PushBack(2, d.GetAllocator());
		minesObj["dark_floors"].PushBack(3, d.GetAllocator());
		minesObj["dark_floors"].PushBack(4, d.GetAllocator());
		minesObj.AddMember("dark_floor_percent", rapidjson::Value(25), d.GetAllocator());

		minesObj.AddMember("shop_floors", rapidjson::Value(rapidjson::kArrayType), d.GetAllocator());
		minesObj["shop_floors"].PushBack(2, d.GetAllocator());
		minesObj["shop_floors"].PushBack(3, d.GetAllocator());
		minesObj["shop_floors"].PushBack(4, d.GetAllocator());
		minesObj.AddMember("shop_floor_percent", rapidjson::Value(50), d.GetAllocator());

		minesObj.AddMember("npc_floors", rapidjson::Value(rapidjson::kArrayType), d.GetAllocator());
		minesObj["npc_floors"].PushBack(2, d.GetAllocator());
		minesObj["npc_floors"].PushBack(3, d.GetAllocator());
		minesObj["npc_floors"].PushBack(4, d.GetAllocator());
		minesObj.AddMember("npc_spawn_chance", rapidjson::Value(10), d.GetAllocator());

		d["map_generation"].AddMember(key1, minesObj, d.GetAllocator());
		
		rapidjson::Value key2("The Swamp", d.GetAllocator());
		rapidjson::Value swampObj(rapidjson::kObjectType);

		rapidjson::Value trapArray2(rapidjson::kArrayType);
		trapArray2.PushBack("boulders", d.GetAllocator());
		trapArray2.PushBack("arrows", d.GetAllocator());
		swampObj.AddMember("trap_generation_types", trapArray2, d.GetAllocator());
		swampObj.AddMember("minotaur_floors", rapidjson::Value(rapidjson::kArrayType), d.GetAllocator());
		swampObj["minotaur_floors"].PushBack(7, d.GetAllocator());
		swampObj["minotaur_floors"].PushBack(8, d.GetAllocator());
		swampObj.AddMember("minotaur_floor_percent", rapidjson::Value(50), d.GetAllocator());

		swampObj.AddMember("dark_floors", rapidjson::Value(rapidjson::kArrayType), d.GetAllocator());
		swampObj["dark_floors"].PushBack(6, d.GetAllocator());
		swampObj["dark_floors"].PushBack(7, d.GetAllocator());
		swampObj["dark_floors"].PushBack(8, d.GetAllocator());
		swampObj["dark_floors"].PushBack(9, d.GetAllocator());
		swampObj.AddMember("dark_floor_percent", rapidjson::Value(25), d.GetAllocator());

		swampObj.AddMember("shop_floors", rapidjson::Value(rapidjson::kArrayType), d.GetAllocator());
		swampObj["shop_floors"].PushBack(6, d.GetAllocator());
		swampObj["shop_floors"].PushBack(7, d.GetAllocator());
		swampObj["shop_floors"].PushBack(8, d.GetAllocator());
		swampObj["shop_floors"].PushBack(9, d.GetAllocator());
		swampObj.AddMember("shop_floor_percent", rapidjson::Value(50), d.GetAllocator());

		swampObj.AddMember("npc_floors", rapidjson::Value(rapidjson::kArrayType), d.GetAllocator());
		swampObj["npc_floors"].PushBack(6, d.GetAllocator());
		swampObj["npc_floors"].PushBack(7, d.GetAllocator());
		swampObj["npc_floors"].PushBack(8, d.GetAllocator());
		swampObj["npc_floors"].PushBack(9, d.GetAllocator());
		swampObj.AddMember("npc_spawn_chance", rapidjson::Value(10), d.GetAllocator());

		d["map_generation"].AddMember(key2, swampObj, d.GetAllocator());

		writeToFile(d);
	}

	void writeToFile(rapidjson::Document& d)
	{
		int filenum = 0;
		DynamicString testPath = "/data/gameplaymodifiers_export" + std::to_string(filenum) + ".json";
		while ( PHYSFS_getRealDir(testPath.c_str()) != nullptr && filenum < 1000 )
		{
			++filenum;
			testPath = "/data/gameplaymodifiers_export" + std::to_string(filenum) + ".json";
		}
		DynamicString outputPath = PHYSFS_getRealDir("/data/");
		outputPath.append(PHYSFS_getDirSeparator());
		DynamicString fileName = "data/gameplaymodifiers_export" + std::to_string(filenum) + ".json";
		outputPath.append(fileName.c_str());

		File* fp = FileIO::open(outputPath.c_str(), "wb");
		if ( !fp )
		{
			return;
		}
		rapidjson::StringBuffer os;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(os);
		d.Accept(writer);
		fp->write(os.GetString(), sizeof(char), os.GetSize());

		FileIO::close(fp);
	}

	void readFromFile()
	{
		resetValues();
		if ( PHYSFS_getRealDir("/data/gameplaymodifiers.json") )
		{
			DynamicString inputPath = PHYSFS_getRealDir("/data/gameplaymodifiers.json");
			inputPath.append("/data/gameplaymodifiers.json");

			File* fp = FileIO::open(inputPath.c_str(), "rb");
			if ( !fp )
			{
				printlog("[JSON]: Error: Could not locate json file %s", inputPath.c_str());
				return;
			}
			char buf[65536];
			int count = fp->read(buf, sizeof(buf[0]), sizeof(buf));
			buf[count] = '\0';
			rapidjson::StringStream is(buf);
			FileIO::close(fp);

			rapidjson::Document d;
			d.ParseStream(is);
			if ( !d.HasMember("version") )
			{
				printlog("[JSON]: Error: No 'version' value in json file, or JSON syntax incorrect! %s", inputPath.c_str());
				return;
			}
			int version = d["version"].GetInt();

			for ( rapidjson::Value::ConstMemberIterator prop_itr = d.MemberBegin(); prop_itr != d.MemberEnd(); ++prop_itr )
			{
				if ( readKeyToGameplayProperty(prop_itr) )
				{
					usingCustomManager = true;
				}
			}
			
			printlog("[JSON]: Successfully read json file %s", inputPath.c_str());
		}
	}

	bool readKeyToGameplayProperty(rapidjson::Value::ConstMemberIterator& itr)
	{
		DynamicString name = itr->name.GetString();
		if ( name.compare("version") == 0 )
		{
			return true;
		}
		else if ( name.compare("xp_share_range") == 0 )
		{
			xpShareRange = itr->value.GetInt();
			return true;
		}
		else if ( name.compare("global_xp_award_percent") == 0 )
		{
			globalXPPercent = itr->value.GetInt();
			return true;
		}
		else if ( name.compare("global_gold_drop_scale_percent") == 0 )
		{
			globalGoldPercent = itr->value.GetInt();
			return true;
		}
		else if ( name.compare("player_share_minimap_progress") == 0 )
		{
			minimapShareProgress = itr->value.GetBool();
			return true;
		}
		else if ( name.compare("player_speed_weight_impact_percent") == 0 )
		{
			playerWeightPercent = itr->value.GetInt();
			return true;
		}
		else if ( name.compare("player_speed_max") == 0 )
		{
			playerSpeedMax = itr->value.GetDouble();
			return true;
		}
		else if ( name.compare("minotaur_force_disable_on_floors") == 0 )
		{
			for ( rapidjson::Value::ConstValueIterator arr_itr = itr->value["normal_floors"].Begin(); arr_itr != itr->value["normal_floors"].End(); ++arr_itr )
			{
				minotaurForceDisableFloors.first.insert(arr_itr->GetInt());
			}
			for ( rapidjson::Value::ConstValueIterator arr_itr = itr->value["secret_floors"].Begin(); arr_itr != itr->value["secret_floors"].End(); ++arr_itr )
			{
				minotaurForceDisableFloors.second.insert(arr_itr->GetInt());
			}
			return true;
		}
		else if ( name.compare("minotaur_force_enable_on_floors") == 0 )
		{
			for ( rapidjson::Value::ConstValueIterator arr_itr = itr->value["normal_floors"].Begin(); arr_itr != itr->value["normal_floors"].End(); ++arr_itr )
			{
				minotaurForceEnableFloors.first.insert(arr_itr->GetInt());
			}
			for ( rapidjson::Value::ConstValueIterator arr_itr = itr->value["secret_floors"].Begin(); arr_itr != itr->value["secret_floors"].End(); ++arr_itr )
			{
				minotaurForceEnableFloors.second.insert(arr_itr->GetInt());
			}
			return true;
		}
		else if ( name.compare("disable_hunger_on_floors") == 0 )
		{
			for ( rapidjson::Value::ConstValueIterator arr_itr = itr->value["normal_floors"].Begin(); arr_itr != itr->value["normal_floors"].End(); ++arr_itr )
			{
				hungerDisableFloors.first.insert(arr_itr->GetInt());
			}
			for ( rapidjson::Value::ConstValueIterator arr_itr = itr->value["secret_floors"].Begin(); arr_itr != itr->value["secret_floors"].End(); ++arr_itr )
			{
				hungerDisableFloors.second.insert(arr_itr->GetInt());
			}
			return true;
		}
		else if ( name.compare("disable_herx_messages_on_floors") == 0 )
		{
			for ( rapidjson::Value::ConstValueIterator arr_itr = itr->value["normal_floors"].Begin(); arr_itr != itr->value["normal_floors"].End(); ++arr_itr )
			{
				herxChatterDisableFloors.first.insert(arr_itr->GetInt());
			}
			for ( rapidjson::Value::ConstValueIterator arr_itr = itr->value["secret_floors"].Begin(); arr_itr != itr->value["secret_floors"].End(); ++arr_itr )
			{
				herxChatterDisableFloors.second.insert(arr_itr->GetInt());
			}
			return true;
		}
		else if ( name.compare("disable_minimap_on_floors") == 0 )
		{
			for ( rapidjson::Value::ConstValueIterator arr_itr = itr->value["normal_floors"].Begin(); arr_itr != itr->value["normal_floors"].End(); ++arr_itr )
			{
				minimapDisableFloors.first.insert(arr_itr->GetInt());
			}
			for ( rapidjson::Value::ConstValueIterator arr_itr = itr->value["secret_floors"].Begin(); arr_itr != itr->value["secret_floors"].End(); ++arr_itr )
			{
				minimapDisableFloors.second.insert(arr_itr->GetInt());
			}
			return true;
		}
		else if ( name.compare("map_generation") == 0 )
		{
			for ( rapidjson::Value::ConstMemberIterator map_itr = itr->value.MemberBegin(); map_itr != itr->value.MemberEnd(); ++map_itr )
			{
				DynamicString mapName = map_itr->name.GetString();
				MapGeneration m(mapName);
				for ( rapidjson::Value::ConstMemberIterator obj_itr = map_itr->value.MemberBegin(); obj_itr != map_itr->value.MemberEnd(); ++obj_itr )
				{
					readKeyToMapGenerationProperty(m, obj_itr);
				}
				allMapGenerations.push_back(m);
			}
			return true;
		}
		printlog("[JSON]: Unknown property '%s'", name.c_str());
		return false;
	}

	bool readKeyToMapGenerationProperty(MapGeneration& m, rapidjson::Value::ConstMemberIterator& itr)
	{
		DynamicString name = itr->name.GetString();
		if ( name.compare("trap_generation_types") == 0 )
		{
			m.usingTrapTypes = true;
			for ( rapidjson::Value::ConstValueIterator arr_itr = itr->value.Begin(); arr_itr != itr->value.End(); ++arr_itr )
			{
				m.trapTypes.push_back(arr_itr->GetString());
			}
			return true;
		}
		else if ( name.compare("minotaur_floors") == 0 )
		{
			for ( rapidjson::Value::ConstValueIterator arr_itr = itr->value.Begin(); arr_itr != itr->value.End(); ++arr_itr )
			{
				m.minoFloors.insert(arr_itr->GetInt());
			}
			return true;
		}
		else if ( name.compare("dark_floors") == 0 )
		{
			for ( rapidjson::Value::ConstValueIterator arr_itr = itr->value.Begin(); arr_itr != itr->value.End(); ++arr_itr )
			{
				m.darkFloors.insert(arr_itr->GetInt());
			}
			return true;
		}
		else if ( name.compare("shop_floors") == 0 )
		{
			for ( rapidjson::Value::ConstValueIterator arr_itr = itr->value.Begin(); arr_itr != itr->value.End(); ++arr_itr )
			{
				m.shopFloors.insert(arr_itr->GetInt());
			}
			return true;
		}
		else if ( name.compare("npc_floors") == 0 )
		{
			for ( rapidjson::Value::ConstValueIterator arr_itr = itr->value.Begin(); arr_itr != itr->value.End(); ++arr_itr )
			{
				m.npcSpawnFloors.insert(arr_itr->GetInt());
			}
			return true;
		}
		else if ( name.compare("dark_floor_percent") == 0 )
		{
			m.darkPercent = itr->value.GetInt();
			return true;
		}
		else if ( name.compare("minotaur_floor_percent") == 0 )
		{
			m.minoPercent = itr->value.GetInt();
			return true;
		}
		else if ( name.compare("shop_floor_percent") == 0 )
		{
			m.shopPercent = itr->value.GetInt();
			return true;
		}
		else if ( name.compare("npc_spawn_chance") == 0 )
		{
			m.npcSpawnPercent = itr->value.GetInt();
			return true;
		}
		printlog("[JSON]: Unknown property '%s'", name.c_str());
		return false;
	}

	bool processedMinotaurSpawn(int level, bool secret, std::string mapName)
	{
		if ( !inUse() )
		{
			return false;
		}

		if ( CustomHelpers::isLevelPartOfSet(level, secret, minotaurForceEnableFloors) )
		{
			minotaurlevel = 1;
			return true;
		}
		if ( CustomHelpers::isLevelPartOfSet(level, secret, minotaurForceDisableFloors) )
		{
			minotaurlevel = 0;
			return true;
		}

		auto m = getMapGenerationForMapName(mapName);
		if ( m )
		{
			if ( m->minoPercent == -1 )
			{
				// no key value read in.
				return false;
			}

			if ( m->minoFloors.find(level) == m->minoFloors.end() )
			{
				// not found
				minotaurlevel = 0;
				return true;
			}
			// found, roll prng
			if ( map_rng.rand() % 100 < m->minoPercent )
			{
				minotaurlevel = 1;
			}
			else
			{
				minotaurlevel = 0;
			}
			return true;
		}
		return false;
	}

	bool processedDarkFloor(int level, bool secret, std::string mapName)
	{
		if ( !inUse() )
		{
			return false;
		}

		auto m = getMapGenerationForMapName(mapName);
		if ( m )
		{
			if ( m->darkPercent == -1 )
			{
				// no key value read in.
				return false;
			}

			if ( m->darkFloors.find(level) == m->darkFloors.end() )
			{
				// not found
				darkmap = false;
				return true;
			}
			// found, roll prng
			if ( map_rng.rand() % 100 < m->darkPercent )
			{
				darkmap = true;
			}
			else
			{
				darkmap = false;
			}
			return true;
		}
		return false;
	}

	bool processedShopFloor(int level, bool secret, std::string mapName, bool& shoplevel)
	{
		if ( !inUse() )
		{
			return false;
		}

		auto m = getMapGenerationForMapName(mapName);
		if ( m )
		{
			if ( m->shopPercent == -1 )
			{
				// no key value read in.
				return false;
			}

			if ( m->shopFloors.find(level) == m->shopFloors.end() )
			{
				// not found
				shoplevel = false;
				return true;
			}
			// found, roll prng
			if ( map_rng.rand() % 100 < m->shopPercent )
			{
				shoplevel = true;
			}
			else
			{
				shoplevel = false;
			}
			return true;
		}
		return false;
	}

	enum PropertyTypes : int
	{
		PROPERTY_NPC
	};

	bool processedPropertyForFloor(int level, bool secret, std::string mapName, PropertyTypes propertyType, bool& bOut)
	{
		if ( !inUse() )
		{
			return false;
		}

		auto m = getMapGenerationForMapName(mapName);
		if ( m )
		{
			int percentValue = -1;
			switch ( propertyType )
			{
				case PROPERTY_NPC:
					if ( m->npcSpawnFloors.find(level) == m->npcSpawnFloors.end() )
					{
						// not found
						bOut = false;
						return true;
					}
					percentValue = m->npcSpawnPercent;
					break;
				default:
					break;
			}

			if ( percentValue == -1 )
			{
				// no key value read in.
				return false;
			}

			// found, roll prng
			if ( map_rng.rand() % 100 < percentValue )
			{
				bOut = true;
			}
			else
			{
				bOut = false;
			}
			return true;
		}
		return false;
	}
};
template <> struct DynamicArrayKindOf<MonsterCurveCustomManager::FollowerGenerateDetails_t> { static constexpr int value = Kind_FollowerDetails; };
template <> struct DynamicArrayKindOf<MonsterStatCustomManager::StatEntry::VariantPair_t> { static constexpr int value = Kind_VariantPair; };
template <> struct DynamicArrayKindOf<MonsterCurveCustomManager::MonsterCurveEntry::MonsterVariant_t> { static constexpr int value = Kind_VariantPair; };
template <> struct DynamicArrayKindOf<MonsterCurveCustomManager::MonsterCurveEntry> { static constexpr int value = Kind_MonsterCurveEntry; };
template <> struct DynamicArrayKindOf<MonsterCurveCustomManager::LevelCurve> { static constexpr int value = Kind_LevelCurve; };



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
		void restoreSavedServerFlags()
		{ 
			if ( bHasSavedServerFlags )
			{
				bHasSavedServerFlags = false;
				svFlags = serverFlags;
				printlog("[SESSION]: Restoring server flags\n");
			}
		}
		void saveServerFlags()
		{
			serverFlags = svFlags;
			bHasSavedServerFlags = true;
			printlog("[SESSION]: Saving server flags\n");
		}

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
			bool isActive(ChallengeEvents_t _eventType) { return inUse && (eventType == _eventType); }
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

	bool isServerflagDisabledForCurrentMode(int i)
	{
		if ( getMode() == GAME_MODE_DEFAULT )
		{
			return false;
		}
		/*else if ( getMode() == GAME_MODE_TUTORIAL )
		{
			int flag = power(2, i);
			switch ( flag )
			{
				case SV_FLAG_HARDCORE:
				case SV_FLAG_HUNGER:
				case SV_FLAG_FRIENDLYFIRE:
				case SV_FLAG_LIFESAVING:
				case SV_FLAG_TRAPS:
				case SV_FLAG_CLASSIC:
				case SV_FLAG_MINOTAURS:
				case SV_FLAG_KEEPINVENTORY:
					return true;
					break;
				default:
					break;
			}
			return false;
		}*/
		else if ( getMode() == GAME_MODE_CUSTOM_RUN_ONESHOT
			|| getMode() == GAME_MODE_CUSTOM_RUN )
		{
			if ( currentSession.challengeRun.lockedFlags & i )
			{
				return true;
			}
			return false;
		}
		return false;
	}

	class Tutorial_t
	{
		DynamicString currentMap = "";
		const Uint32 kNumTutorialLevels = 10;
	public:
		void init()
		{
			readFromFile();
		}
		int dungeonLevel = -1;
		bool showFirstTutorialCompletedPrompt = false;
		bool firstTutorialCompleted = false;
		void createFirstTutorialCompletedPrompt();
		void setTutorialMap(std::string& mapname)
		{
			loadCustomNextMap = mapname.c_str();
			currentMap = loadCustomNextMap;
		}
		void launchHub()
		{
			loadCustomNextMap = "tutorial_hub.lmp";
			currentMap = loadCustomNextMap;
		}
		void startTutorial(std::string mapToSet);
		static void buttonReturnToTutorialHub(button_t* my);
		static void buttonRestartTrial(button_t* my);
		const Uint32 getNumTutorialLevels() { return kNumTutorialLevels; }
		void openGameoverWindow();
		void onMapRestart(int levelNum)
		{
#ifndef EDITOR
			achievementObserver.updateGlobalStat(
				std::min(STEAM_GSTAT_TUTORIAL1_ATTEMPTS - 1 + levelNum, static_cast<int>(STEAM_GSTAT_TUTORIAL10_ATTEMPTS)), -1);
#endif // !EDITOR
		}

		class Menu_t
		{
			bool bWindowOpen = false;
		public:
			bool isOpen() { return bWindowOpen; }
			void open();
			void close() { bWindowOpen = false; }
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
			void close() { bWindowOpen = false; }
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
		void writeToFile(rapidjson::Document& d)
		{
			DynamicString outputPath = outputdir;
			outputPath.append(tutorialScoresFilename.c_str());

			File* fp = FileIO::open(outputPath.c_str(), "wb");
			if ( !fp )
			{
				return;
			}
			rapidjson::StringBuffer os;
			rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(os);
			d.Accept(writer);
			fp->write(os.GetString(), sizeof(char), os.GetSize());
			fp->write("", sizeof(char), 1);

			FileIO::close(fp);
		}
	} Tutorial;
};
extern GameModeManager_t gameModeManager;

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
		std::string iconLabelPath = "";
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
		std::set<SpellTagTypes> spellTags;
		DynamicArrayStr spellFormatTags;
		DynamicArrayS32 spellbookItemIconPaddingLines;
		std::set<spell_t::SpellOnCastEventTypes> spellLevelTags;

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
		void setColorHeading(Uint32 color) { headingTextColor = color; }
		void setColorDescription(Uint32 color) { descriptionTextColor = color; }
		void setColorDetails(Uint32 color) { detailsTextColor = color; }
		void setColorPositive(Uint32 color) { positiveTextColor = color; }
		void setColorNegative(Uint32 color) { negativeTextColor = color; }
		void setColorStatus(Uint32 color) { statusEffectTextColor = color; }
		void setColorFaintText(Uint32 color) { faintTextColor = color; }
	};
	void setSpellValueIfKeyPresent(spellItem_t& t, rapidjson::Value::ConstMemberIterator item_itr, Uint32& hash, Uint32& hashShift, const char* key, int& toSet);
	void setSpellValueIfKeyPresent(spellItem_t& t, rapidjson::Value::ConstMemberIterator item_itr, Uint32& hash, Uint32& hashShift, const char* key, real_t& toSet);
	void readItemsFromFile();
	static const Uint32 kItemsJsonHash;
	static Uint32 itemsJsonHashRead;
	void readItemLocalizationsFromFile(bool forceLoadBaseDirectory = false);
	void readTooltipsFromFile(bool forceLoadBaseDirectory = false);
	void readBookLocalizationsFromFile(bool forceLoadBaseDirectory = false);
	std::vector<tmpItem_t> tmpItems;
	std::map<Sint32, spellItem_t> spellItems;
	std::map<std::string, ItemTooltip_t> tooltips;
	std::map<std::string, std::map<std::string, std::string>> adjectives;
	DynamicMapStrArrStr templates;
	//std::vector<std::pair<int, Sint32>> itemValueTable;
	//std::map<int, std::vector<std::pair<int, Sint32>>> itemValueTableByCategory;
	typedef ItemLocalization_tMirror ItemLocalization_t;
	DynamicMapItemLoc itemNameLocalizations;
	DynamicMapStr bookNameLocalizations;
	DynamicMapStr spellNameLocalizations;
	DynamicMapI32 itemNameStringToItemID;
	DynamicMapI32 spellNameStringToSpellID;
	std::string defaultString = "";
	char buf[2048];
	bool autoReload = false;
	bool itemDebug = false;
	std::string& getItemStatusAdjective(Uint32 itemType, Status status);
	std::string& getItemBeatitudeAdjective(Sint16 beatitude);
	std::string& getItemPotionAlchemyAdjective(const int player, Uint32 itemType);
	std::string& getItemPotionHarmAllyAdjective(Item& item);
	std::string& getItemProficiencyName(int proficiency);
	std::string& getItemSlotName(ItemEquippableSlot slotname);
	std::string& getItemStatShortName(const char* attribute);
	std::string& getItemStatFullName(const char* attribute);
	std::string& getItemEquipmentEffectsForIconText(std::string& attribute);
	std::string& getItemEquipmentEffectsForAttributesText(std::string& attribute);
	std::string& getProficiencyLevelName(Sint32 proficiencyLevel);
	std::string& getIconLabel(Item& item);
	std::string getSpellIconText(const int player, Item& item, const bool excludePlayerStats);
	std::string getSpellIconFormatText(const int player, Item& item, std::string& format, const spell_t* spell, const int iconIndex, const bool compendiumTooltipIntro);
	std::string getSpellDescriptionText(const int player, Item& item);
	std::string getSpellIconPath(const int player, Item& item, int spellID);
	std::string getCostOfSpellString(const int player, Item& item);
	std::string& getSpellTypeString(const int player, Item& item);
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
		std::map<int, Uint32>& highlightIndexes, std::map<int, Uint32>& positiveIndexes, std::map<int, Uint32>& negativeIndexes, ItemTooltip_t& tooltip);
	void getWordIndexesItemDetails(void* field, DynamicString& str, DynamicString& highlightValues, DynamicString& positiveValues, DynamicString& negativeValues,
		std::map<int, Uint32>& highlightIndexes, std::map<int, Uint32>& positiveIndexes, std::map<int, Uint32>& negativeIndexes, ItemTooltip_t& tooltip);
};
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
	rapidjson::Document exportDocument;

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
		std::map<std::string, std::vector<StatueLimb_t>> limbs;
		Statue_t() {
			id = statueId; 
			++statueId;
		}
		real_t heightOffset = 0.0;
	};

	const DynamicArrayStr directionKeys{ "east", "south", "west", "north" };
	std::map<Uint32, Statue_t> allStatues;
};
extern StatueManager_t StatueManager;

class DebugTimers_t
{
	std::map<std::string, std::vector<std::pair<std::string, std::chrono::high_resolution_clock::time_point>>> timepoints;
public:
	void addTimePoint(std::string key, DynamicString desc = "") { timepoints[key].push_back(std::make_pair(desc, std::chrono::high_resolution_clock::now())); }
	void printTimepoints(std::string key, int& posy);
	void clearTimepoints(std::string key) { timepoints[key].clear(); }
	void clearAllTimepoints() { timepoints.clear(); }
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
	std::map<int, GlyphData_t> allGlyphs;

	GlyphRenderer_t() {};
	~GlyphRenderer_t() {};
	bool readFromFile();
	void renderGlyphsToPNGs();
	DynamicString& getGlyphPath(int scancode, bool pressed = false) 
	{ 
		if ( allGlyphs.find(scancode) != allGlyphs.end() )
		{ 
			if ( pressed )
			{
				return allGlyphs[scancode].pressedRenderedFullpath;
			}
			else
			{
				return allGlyphs[scancode].unpressedRenderedFullpath;
			}
		}
		return defaultstring;
	}
};
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
	std::map<std::string, Entry_t> allEntries;
};
extern ScriptTextParser_t ScriptTextParser;

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
	static std::map<int, std::vector<StoreSlots_t>> entries; // shop type as key
	static void readFromFile();
};
template <> struct DynamicArrayKindOf<ShopkeeperConsumables_t::ItemEntry> { static constexpr int value = Kind_ShopkeeperItem; };


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
			std::vector<HotbarEntry_t> hotbar;
			std::vector<std::vector<HotbarEntry_t>> hotbar_alternates;
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

struct LocalAchievements_t
{
	typedef Achievement_tMirror Achievement_t;
	struct Statistic_t
	{
		DynamicString name;
		int value = 0;
	};
	DynamicMapAchievement achievements;
	std::map<int, Statistic_t> statistics;
	static void readFromFile();
	static void writeToFile();
	static void init();
	void updateAchievement(const char* name, const bool unlocked);
	void updateStatistic(const int stat_num, const int value);
};
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
		void reset()
		{
			value = 0;
			needsUpdate = true;
		}
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
	static int getGameConfigValue(GameConfigIndexes index)
	{
		if ( index >= 0 && index < GOPT_ENUM_END )
		{
			return gameConfig[index].value;
		}
		return 0;
	}
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
		std::map<std::string, std::vector<int>> hideMonsters;
		DynamicArrayS32 spellTriggers;
		std::set<int> pathableMonsters;
		int colliderJumpLangEntry = 6234;
		DynamicMapI32 overrideProperties;
		bool hasOverride(std::string key)
		{
			auto find = overrideProperties.find(key);
			if ( find != overrideProperties.end() )
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		int getOverride(std::string key)
		{
			auto find = overrideProperties.find(key);
			if ( find != overrideProperties.end() )
			{
				return find->second;
			}
			return 0;
		}
	};
	typedef ColliderDmgProperties_tMirror ColliderDmgProperties_t;
	static const int COLLIDER_COLLISION_FLAG_MINO = 2;
	static const int COLLIDER_COLLISION_FLAG_NPC = 4;
	static DynamicMapColliderDmg colliderDmgTypes;
	static std::map<int, EntityColliderData_t> colliderData;
	static std::map<std::string, std::map<int, int>> colliderRandomGenPool;
	static DynamicMapI32 colliderNameIndexes;
	static int getColliderIndexFromName(std::string name)
	{
		auto find = colliderNameIndexes.find(name);
		if ( find != colliderNameIndexes.end() )
		{
			return find->second;
		}
		return 0;
	}
	static void readFromFile();
};
extern EditorEntityData_t editorEntityData;

struct Mods
{
	static DynamicArrayS32 modelsListModifiedIndexes;
	static DynamicArrayS32 soundsListModifiedIndexes;
	static std::vector<std::pair<SDL_Surface**, std::string>> systemResourceImagesToReload;
	static std::vector<std::pair<std::string, std::string>> mountedFilepaths;
	static std::vector<std::pair<std::string, std::string>> mountedFilepathsSaved; // saved from config file
	static std::set<std::string> mods_loaded_local;
	static std::set<std::string> mods_loaded_workshop;
	static std::list<std::string> localModFoldernames;
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
		std::map<int, AdditionalOffset_t> adjustToOversizeMask;
		std::map<int, AdditionalOffset_t> adjustToExpandedHelm;
	};
	std::map<int, std::map<int, ModelOffset_t>> monsterModelsMap;
	std::map<int, ModelOffset_t> miscItemsBaseOffsets;
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

	static void readContentsLang(std::string name, std::map<std::string, std::vector<std::pair<std::string, std::string>>>& contents,
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
		static std::map<std::string, std::vector<std::pair<std::string, std::string>>> achievementCategories;
		static std::unordered_set<std::string> achievementUnlockedLookup;
		static void onAchievementUnlock(const char* ach);
		static std::map<std::string, std::vector<std::pair<std::string, std::string>>> contents;
		static DynamicMapStr contentsMap;
		static DynamicMapI32 unlocks;
		static int completionPercent;
		static int numUnread;
		static void readContentsLang();

		struct CompendiumAchievementsDisplay
		{
			std::vector<std::vector<DynamicString>> pages;
			int currentPage = 0;
			int numHidden = 0;
		};
		static std::map<std::string, CompendiumAchievementsDisplay> achievementsBookDisplay;
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
			std::array<int, 7> resistances;
			DynamicArrayStr abilities;
			DynamicArrayStr inventory;
			DynamicString imagePath = "";
			DynamicArrayStr models;
			DynamicSetStr unlockAchievements;
			int lorePoints = 0;
			DynamicArrayS32 getDisplayStat(const char* name);
		};
		static std::map<std::string, std::vector<std::pair<std::string, std::string>>> contents;
		static DynamicMapStr contentsMap;
		static std::map<std::string, std::vector<std::pair<std::string, std::string>>> contents_unfiltered;
		static void readContentsLang();
		static DynamicMapI32 unlocks;
		static int completionPercent;
		static int numUnread;
	};
	std::map<std::string, CompendiumMonsters_t::Monster_t> monsters;
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
	std::map<std::string, ObjectLimbs_t> compendiumObjectLimbs;
	CompendiumView_t currentView;
	struct CompendiumMap_t
	{
		Uint32 width = 0;
		Uint32 height = 0;
		Uint32 ceiling = -1;
	};
	std::map<std::string, std::pair<CompendiumMap_t, std::vector<int>>> compendiumObjectMapTiles;
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
		static std::map<std::string, std::vector<std::pair<std::string, std::string>>> contents;
		static DynamicMapStr contentsMap;
		static void readContentsLang();
		static DynamicMapI32 unlocks;
		static int completionPercent;
		static int numUnread;
	};
	std::map<std::string, CompendiumWorld_t::World_t> worldObjects;
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
		static std::map<std::string, std::vector<std::pair<std::string, std::string>>> contents;
		static DynamicMapStr contentsMap;
		static void readContentsLang();
		static DynamicMapI32 unlocks;
		static int completionPercent;
		static int numUnread;
	};
	std::map<std::string, CompendiumCodex_t::Codex_t> codex;
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
		static std::map<std::string, std::vector<std::pair<std::string, std::string>>> contents;
		static DynamicMapStr contentsMap;
		static void readContentsLang();
		static DynamicMapI32 unlocks;
		static std::map<int, CompendiumUnlockStatus> itemUnlocks;
		static int completionPercent;
		static int numUnread;
	};
	std::map<std::string, CompendiumItems_t::Codex_t> items;
	void readItemsFromFile(bool forceLoadBaseDirectory = false);
	void readItemsTranslationsFromFile(bool forceLoadBaseDirectory = false);

	struct CompendiumMagic_t
	{
		static std::map<std::string, std::vector<std::pair<std::string, std::string>>> contents;
		static DynamicMapStr contentsMap;
		static void readContentsLang();
		static int completionPercent;
		static int numUnread;
	};
	std::map<std::string, CompendiumItems_t::Codex_t> magic;
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

	static const char* getSkillStringForCompendium(const int skill)
	{
		switch ( skill )
		{
		case PRO_LOCKPICKING: return "tinkering skill";
		case PRO_STEALTH: return "stealth skill";
		case PRO_TRADING: return "trading skill";
		case PRO_APPRAISAL: return "lore skill";
		case PRO_LEGACY_SWIMMING: return "swimming skill";
		case PRO_THAUMATURGY: return "thaumaturgy skill";
		case PRO_LEADERSHIP: return "leadership skill";
		case PRO_LEGACY_SPELLCASTING: return "casting skill";
		case PRO_MYSTICISM: return "mysticism skill";
		case PRO_LEGACY_MAGIC: return "magic skill";
		case PRO_SORCERY: return "sorcery skill";
		case PRO_RANGED: return "ranged skill";
		case PRO_SWORD: return "sword skill";
		case PRO_MACE: return "mace skill";
		case PRO_AXE: return "axe skill";
		case PRO_POLEARM: return "polearm skill";
		case PRO_SHIELD: return "blocking skill";
		case PRO_UNARMED: return "unarmed skill";
		case PRO_ALCHEMY: return "alchemy skill";
		default:
			break;
		}
		return "";
	}

	struct CompendiumEntityCurrent
	{
		DynamicString contentsName = "";
		DynamicString modelName = "";
		int modelIndex = -1;
		Uint32 modelRNG = 0;
		void set(std::string _contentsName, std::string _modelName, int _modelIndex = -1)
		{
			contentsName = _contentsName;
			modelName = _modelName;
			modelIndex = _modelIndex;
			++modelRNG;
		}
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
			std::set<std::string> attributes;
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
		static std::map<EventTags, Event_t> events;
		static DynamicMapI32 eventIdLookup;
		static std::map<int, std::set<EventTags>> itemEventLookup;
		static DynamicMapI32 monsterUniqueIDLookup;
		static DynamicMapI32Str itemIDToString;
		static DynamicMapI32Str monsterIDToString;
		static DynamicMapI32Str codexIDToString;
		static DynamicMapI32Str worldIDToString;
		static std::map<int, std::vector<EventTags>> itemDisplayedEventsList;
		static std::map<int, std::vector<DynamicString>> itemDisplayedCustomEventsList;
		static DynamicMapStr customEventsValues;
		static std::map<EventTags, std::set<int>> eventItemLookup;
		static std::map<EventTags, std::set<int>> eventMonsterLookup;
		static std::map<EventTags, std::set<std::string>> eventWorldLookup;
		static std::map<EventTags, std::set<std::string>> eventCodexLookup;
		static DynamicMapI32 eventWorldIDLookup;
		static DynamicMapI32 eventCodexIDLookup;
		static std::map<EventTags, std::map<int, int>> eventClassIds;
		static const int kEventClassesMax = 40;
		static std::map<EventTags, std::map<std::string, std::string>> eventLangEntries;
		static std::map<std::string, std::map<std::string, std::string>> eventCustomLangEntries;
		static std::vector<std::pair<std::string, Sint32>> getCustomEventValue(std::string key, std::string compendiumSection, std::string compendiumContentsSelected, int specificClass = -1);
		static std::string formatEventRecordText(Sint32 value, const char* formatType, int formatVal, std::map<std::string, std::string>& langMap);
		static void readEventsFromFile();
		static void writeItemsSaveData();
		static void loadItemsSaveData();
		static void readEventsTranslations();
		static void createDummyClientData(const int playernum);
		static void eventUpdate(int playernum, const EventTags tag, const ItemType type, Sint32 value, const bool loadingValue = false, const int spellID = -1);
		static void eventUpdateMonster(int playernum, const EventTags tag, const Entity* entity, Sint32 value, const bool loadingValue = false, const int entryID = -1);
		static void eventUpdateWorld(int playernum, const EventTags tag, const char* category, Sint32 value, const bool loadingValue = false, const int entryID = -1, const bool commitUniqueValue = true);
		static void eventUpdateCodex(int playernum, const EventTags tag, const char* category, Sint32 value, const bool loadingValue = false, const int entryID = -1, const bool floorEvent = false);
		static std::map<EventTags, std::map<int, EventVal_t>> playerEvents;
		static std::map<EventTags, std::map<int, EventVal_t>> serverPlayerEvents[MAXPLAYERS];
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
template <> struct DynamicArrayKindOf<Compendium_t::CompendiumItems_t::Codex_t::CodexItem_t> { static constexpr int value = Kind_CodexItem; };


extern Compendium_t CompendiumEntries;

struct TreasureRoomGenerator
{
	BaronyRNG treasure_rng;
	std::unordered_set<unsigned int> treasure_floors;
	std::unordered_set<unsigned int> treasure_secret_floors;
	std::map<unsigned int, std::string> orb_floors;
	std::map<unsigned int, std::string> station_floors;
	std::map<unsigned int, std::string> station_secret_floors;
	void init();
	bool bForceSpawnForCurrentFloor(int secretlevelexit, bool minotaur, BaronyRNG& mapRNG);
	bool bForceStationSpawnForCurrentFloor(int secretlevelexit);
};
extern TreasureRoomGenerator treasure_room_generator;