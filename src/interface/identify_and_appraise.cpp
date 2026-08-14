/*-------------------------------------------------------------------------------

	BARONY
	File: identify_and_appraise.cpp
	Desc: contains identify and appraisal related (GUI) code.

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "../main.hpp"
#include "../draw.hpp"
#include "../game.hpp"
#include "../stat.hpp"
#include "../items.hpp"
#include "../net.hpp"
#include "../player.hpp"
#include "interface.hpp"
#include "../scores.hpp"
#include "../mod_tools.hpp"
#include "../../odin/json_shim/json_shim.hpp"

//Identify GUI definitions.
SDL_Surface* identifyGUI_img;

//void rebuildIdentifyGUIInventory(const int player)
//{
//	list_t* identify_inventory = &stats[player]->inventory;
//	node_t* node = nullptr;
//	Item* item = nullptr;
//	int c = 0;
//
//	if (identify_inventory)
//	{
//		//Count the number of items in the identify GUI "inventory".
//		for (node = identify_inventory->first; node != NULL; node = node->next)
//		{
//			item = (Item*) node->element;
//			if (item && !item->identified)
//			{
//				c++;
//			}
//		}
//		identifyscroll[player] = std::max(0, std::min(identifyscroll[player], c - 4));
//		for (c = 0; c < 4; ++c)
//		{
//			identify_items[player][c] = NULL;
//		}
//		c = 0;
//
//		//Assign the visible items to the GUI slots.
//		for (node = identify_inventory->first; node != NULL; node = node->next)
//		{
//			if (node->element)
//			{
//				item = (Item*) node->element;
//				if (item && !item->identified)   //Skip over all identified items.
//				{
//					c++;
//					if (c <= identifyscroll[player] )
//					{
//						continue;
//					}
//					identify_items[player][c - identifyscroll[player] - 1] = item;
//					if (c > 3 + identifyscroll[player] )
//					{
//						break;
//					}
//				}
//			}
//		}
//	}
//}

//void CloseIdentifyGUI(const int player)
//{
//	identifygui_active[player] = false;
//	selectedIdentifySlot[player] = -1;
//}

//const int getIdentifyGUIStartX(const int player)
//{
//	return ((players[player]->camera_midx() - (inventoryChest_bmp->w / 2)) + identifygui_offset_x[player]);
//}
//
//const int getIdentifyGUIStartY(const int player)
//{
//	return ((players[player]->camera_midy() - (inventoryChest_bmp->h / 2)) + identifygui_offset_y[player]);
//}

//void updateIdentifyGUI(const int player)
//{
//	//if (openedChest[clientnum])
//	//	return; //Cannot have the identify and chest GUIs open at the same time.
//
//	const Sint32 omousex = inputs.getMouse(clientnum, Inputs::OX);
//	const Sint32 omousey = inputs.getMouse(clientnum, Inputs::OY);
//
//	SDL_Rect pos;
//	node_t* node;
//	int y, c;
//
//	//Identify GUI.
//	if ( identifygui_active[player] )
//	{
//		//Center the identify GUI.
//		pos.x = getIdentifyGUIStartX(player);
//		pos.y = getIdentifyGUIStartY(player);
//		drawImage(identifyGUI_img, NULL, &pos);
//
//		//Buttons
//		if ( mousestatus[SDL_BUTTON_LEFT] )
//		{
//			//Identify GUI scroll up button.
//			if (omousey >= getIdentifyGUIStartY(player) + 16
//				&& omousey < getIdentifyGUIStartY(player) + 52)
//			{
//				if (omousex >= getIdentifyGUIStartX(player) + (identifyGUI_img->w - 28) 
//					&& omousex < getIdentifyGUIStartX(player) + (identifyGUI_img->w - 12))
//				{
//					buttonclick = 7;
//					identifyscroll[player]--;
//					mousestatus[SDL_BUTTON_LEFT] = 0;
//				}
//			}
//			//Identify GUI scroll down button.
//			else if (omousey >= getIdentifyGUIStartY(player) + 52
//				&& omousey < getIdentifyGUIStartY(player) + 88)
//			{
//				if (omousex >= getIdentifyGUIStartX(player) + (identifyGUI_img->w - 28)
//					&& omousex < getIdentifyGUIStartX(player) + (identifyGUI_img->w - 12))
//				{
//					buttonclick = 8;
//					identifyscroll[player]++;
//					mousestatus[SDL_BUTTON_LEFT] = 0;
//				}
//			}
//			else if (omousey >= getIdentifyGUIStartY(player)
//				&& omousey < getIdentifyGUIStartY(player) + 15)
//			{
//				//Identify GUI close button.
//				if (omousex >= getIdentifyGUIStartX(player) + 393 
//					&& omousex < getIdentifyGUIStartX(player) + 407)
//				{
//					buttonclick = 9;
//					mousestatus[SDL_BUTTON_LEFT] = 0;
//				}
//				if (omousex >= getIdentifyGUIStartX(player) 
//					&& omousex < getIdentifyGUIStartX(player) + 377 
//					&& omousey >= getIdentifyGUIStartY(player)
//					&& omousey < getIdentifyGUIStartY(player) + 15)
//				{
//					gui_clickdrag = true;
//					dragging_identifyGUI[player] = true;
//					dragoffset_x = omousex - getIdentifyGUIStartX(player);
//					dragoffset_y = omousey - getIdentifyGUIStartY(player);
//					mousestatus[SDL_BUTTON_LEFT] = 0;
//				}
//			}
//		}
//
//		// mousewheel
//		if ( omousex >= getIdentifyGUIStartX(player) + 12 && omousex < getIdentifyGUIStartX(player) + (identifyGUI_img->w - 28) )
//		{
//			if ( omousey >= getIdentifyGUIStartY(player) + 16 && omousey < getIdentifyGUIStartY(player) + (identifyGUI_img->h - 8) )
//			{
//				if ( mousestatus[SDL_BUTTON_WHEELDOWN] )
//				{
//					mousestatus[SDL_BUTTON_WHEELDOWN] = 0;
//					identifyscroll[player]++;
//				}
//				else if ( mousestatus[SDL_BUTTON_WHEELUP] )
//				{
//					mousestatus[SDL_BUTTON_WHEELUP] = 0;
//					identifyscroll[player]--;
//				}
//			}
//		}
//
//		if (dragging_identifyGUI)
//		{
//			if (gui_clickdrag[player.playernum])
//			{
//				identifygui_offset_x[player] = (omousex - dragoffset_x) - (getIdentifyGUIStartX(player) - identifygui_offset_x[player]);
//				identifygui_offset_y[player] = (omousey - dragoffset_y) - (getIdentifyGUIStartY(player) - identifygui_offset_y[player]);
//				if ( getIdentifyGUIStartX(player) <= 0)
//				{
//					identifygui_offset_x[player] = 0 - (getIdentifyGUIStartX(player) - identifygui_offset_x[player]);
//				}
//				if ( getIdentifyGUIStartX(player) > 0 + xres - identifyGUI_img->w)
//				{
//					identifygui_offset_x[player] = (0 + xres - identifyGUI_img->w) - (getIdentifyGUIStartX(player) - identifygui_offset_x[player]);
//				}
//				if (getIdentifyGUIStartY(player) <= 0)
//				{
//					identifygui_offset_y[player] = 0 - (getIdentifyGUIStartY(player) - identifygui_offset_y[player]);
//				}
//				if ( getIdentifyGUIStartY(player) > 0 + yres - identifyGUI_img->h)
//				{
//					identifygui_offset_y[player] = (0 + yres - identifyGUI_img->h) - (getIdentifyGUIStartY(player) - identifygui_offset_y[player]);
//				}
//			}
//			else
//			{
//				dragging_identifyGUI[player] = false;
//			}
//		}
//
//		list_t* identify_inventory = &stats[clientnum]->inventory;
//
//		if (!identify_inventory)
//		{
//			messagePlayer(0, "Warning: stats[%d].inventory is not a valid list. This should not happen.", clientnum);
//		}
//		else
//		{
//			//Print the window label signifying this as the identify GUI.
//			char* window_name;
//			if (identifygui_appraising)
//			{
//				window_name = Language::get(317);
//			}
//			else
//			{
//				window_name = Language::get(318);
//			}
//			ttfPrintText(ttf8, (getIdentifyGUIStartX(player) + 2 + ((identifyGUI_img->w / 2) - ((TTF8_WIDTH * longestline(window_name)) / 2))), 
//				getIdentifyGUIStartY(player) + 4, window_name);
//
//			//Identify GUI up button.
//			if (buttonclick == 7)
//			{
//				pos.x = getIdentifyGUIStartX(player) + (identifyGUI_img->w - 28);
//				pos.y = getIdentifyGUIStartY(player) + 16;
//				pos.w = 0;
//				pos.h = 0;
//				drawImage(invup_bmp, NULL, &pos);
//			}
//			//Identify GUI down button.
//			if (buttonclick == 8)
//			{
//				pos.x = getIdentifyGUIStartX(player) + (identifyGUI_img->w - 28);
//				pos.y = getIdentifyGUIStartY(player) + 52;
//				pos.w = 0;
//				pos.h = 0;
//				drawImage(invdown_bmp, NULL, &pos);
//			}
//			//Identify GUI close button.
//			if (buttonclick == 9)
//			{
//				pos.x = getIdentifyGUIStartX(player) + 393;
//				pos.y = getIdentifyGUIStartY(player);
//				pos.w = 0;
//				pos.h = 0;
//				drawImage(invclose_bmp, NULL, &pos);
//				identifygui_active[player] = false;
//				identifygui_appraising[player] = false;
//
//				//Cleanup identify GUI gamecontroller code here.
//				selectedIdentifySlot[player] = -1;
//				//TODO: closeIdentifyGUI() instead.
//			}
//
//			Item* item = NULL;
//
//			bool selectingSlot = false;
//			SDL_Rect slotPos;
//			slotPos.x = getIdentifyGUIStartX(player);
//			slotPos.w = inventoryoptionChest_bmp->w;
//			slotPos.y = getIdentifyGUIStartY(player) + 16;
//			slotPos.h = inventoryoptionChest_bmp->h;
//			for ( int i = 0; i < NUM_IDENTIFY_GUI_ITEMS; ++i, slotPos.y += slotPos.h )
//			{
//				pos.x = slotPos.x + 12;
//				pos.w = 0;
//				pos.h = 0;
//
//				if ( omousey >= slotPos.y && omousey < slotPos.y + slotPos.h && identify_items[i] )
//				{
//					pos.y = slotPos.y;
//					drawImage(inventoryoptionChest_bmp, nullptr, &pos);
//					selectedIdentifySlot[player] = i;
//					selectingSlot = true;
//					if ( mousestatus[SDL_BUTTON_LEFT] || *inputPressed(joyimpulses[INJOY_MENU_USE]) )
//					{
//						*inputPressed(joyimpulses[INJOY_MENU_USE]) = 0;
//						mousestatus[SDL_BUTTON_LEFT] = 0;
//						identifyGUIIdentify(player, identify_items[player][i]);
//
//						rebuildIdentifyGUIInventory(player);
//						if ( identify_items[player][i] == nullptr )
//						{
//							if ( identify_items[player][0] == nullptr )
//							{
//								//Go back to inventory.
//								selectedIdentifySlot[player] = -1;
//								warpMouseToSelectedInventorySlot(clientnum);
//							}
//							else
//							{
//								//Move up one slot.
//								--selectedIdentifySlot[player];
//								warpMouseToSelectedIdentifySlot(player);
//							}
//						}
//					}
//				}
//			}
//
//			if ( !selectingSlot )
//			{
//				selectedIdentifySlot[player] = -1;
//			}
//
//			//Okay, now prepare to render all the items.
//			y = getIdentifyGUIStartY(player) + 22;
//			c = 0;
//			if (identify_inventory)
//			{
//				rebuildIdentifyGUIInventory(player);
//
//				//Actually render the items.
//				c = 0;
//				for (node = identify_inventory->first; node != NULL; node = node->next)
//				{
//					if (node->element)
//					{
//						item = (Item*) node->element;
//						if (item && !item->identified)   //Skip over all identified items.
//						{
//							c++;
//							if (c <= identifyscroll[player] )
//							{
//								continue;
//							}
//							char tempstr[64] = { 0 };
//							strncpy(tempstr, item->description(), 46);
//							if ( strlen(tempstr) == 46 )
//							{
//								strcat(tempstr, " ...");
//							}
//							ttfPrintText(ttf8, getIdentifyGUIStartX(player) + 36, y, tempstr);
//							pos.x = getIdentifyGUIStartX(player) + 16;
//							pos.y = getIdentifyGUIStartY(player) + 17 + 18 * (c - identifyscroll[player] - 1);
//							pos.w = 16;
//							pos.h = 16;
//							drawImageScaled(itemSprite(item), NULL, &pos);
//							y += 18;
//							if (c > 3 + identifyscroll[player])
//							{
//								break;
//							}
//						}
//					}
//				}
//			}
//		}
//	}
//} //updateIdentifyGUI()

bool Player::Inventory_t::Appraisal_t::appraisalPossible(Item* item)
{
	if ( !item )
	{
		return false;
	}

	if ( item->identified ) { return false; }

	if ( stats[player.playernum]->getModifiedProficiency(PRO_APPRAISAL) < 100 )
	{
		int value = item->type == GEM_GLASS ? 1000 : item->getGoldValue();

		int skillLVL = stats[player.playernum]->getModifiedProficiency(PRO_APPRAISAL)
			+ statGetPER(stats[player.playernum], player.entity) * Player::Inventory_t::Appraisal_t::perStatMult;

		for ( int64_t _at = 0; _at < dynarray_size<Player::Inventory_t::Appraisal_t::AppraisalBreakpoint_t>(appraisal_tables); ++_at )
		{
			auto& table = *dynarray_at<Player::Inventory_t::Appraisal_t::AppraisalBreakpoint_t>(appraisal_tables, _at);
			if ( skillLVL >= table.skillLVL )
			{
				return value <= table.goldValueLimit;
			}
		}
		/*if ( skillLVL >= 50 ) 
		{ 
			return true;
		}
		else if ( skillLVL >= 40 )
		{
			return value <= 1500;
		}
		else if ( skillLVL >= 35 )
		{
			return value <= 425;
		}
		else if ( skillLVL >= 30 )
		{
			return value <= 320;
		}
		else if ( skillLVL >= 25 )
		{
			return value <= 260;
		}
		else if ( skillLVL >= 20 )
		{
			return value <= 200;
		}
		else if ( skillLVL >= 15 )
		{
			return value <= 150;
		}
		else if ( skillLVL >= 10 )
		{
			return value <= 100;
		}
		else if ( skillLVL >= 5 )
		{
			return value <= 65;
		}
		else if ( skillLVL >= 0 )
		{
			return value <= 30;
		}*/

		/*if ( item->type == GEM_GLASS )
		{
			if ( (stats[player.playernum]->getModifiedProficiency(PRO_APPRAISAL)
				+ statGetPER(stats[player.playernum], player.entity) * 5) >= 100 )
			{
				return true;
			}
		}
		else
		{
			if ( (stats[player.playernum]->getModifiedProficiency(PRO_APPRAISAL)
				+ statGetPER(stats[player.playernum], player.entity) * 5) >= items[item->type].value / 10 )
			{
				return true;
			}
		}*/
	}
	else
	{
		return true;
	}
	return false;
}

void Player::Inventory_t::Appraisal_t::appraiseItem(Item* item)
{
	if (!item)
	{
		return;
	}
	if (item->identified)
	{
		messagePlayer(player.playernum, MESSAGE_INVENTORY, Language::get(319), item->getName());
		old_item = 0;
		playSoundPlayer(player.playernum, 90, 64);
		return;
	}
	else if ( !appraisalPossible(item) )
	{
		messagePlayer(player.playernum, MESSAGE_INVENTORY, Language::get(3240), item->description());
		old_item = 0;
		playSoundPlayer(player.playernum, 90, 64);
		return;
	}

	/*if (!identifygui_appraising)
	{
		item->identified = true;
		messagePlayer(clientnum, Language::get(320), item->description());
		if ( players[player]->inventoryUI.appraisal.timer > 0 
			&& players[player]->inventoryUI.appraisal.current_item
			&& players[player]->inventoryUI.appraisal.current_item == item->uid)
		{
			players[player]->inventoryUI.appraisal.timer = 0;
			players[player]->inventoryUI.appraisal.current_item = 0;
		}
		identifygui_active[player] = false;
	}
	else*/
	//Appraising.

	//If appraisal skill >= LEGENDARY, then auto-complete appraisal. Else, do the normal routine.
	/*if ( stats[player.playernum]->getProficiency(PRO_APPRAISAL) >= CAPSTONE_UNLOCK_LEVEL[PRO_APPRAISAL] )
	{
		if ( !item->identified )
		{
			Compendium_t::Events_t::eventUpdate(player.playernum, Compendium_t::CPDM_APPRAISED, item->type, 1);
		}
		item->identified = true;
		item->notifyIcon = true;
		messagePlayer(player.playernum, MESSAGE_INVENTORY, Language::get(320), item->description());
		if ( timer > 0 && current_item != 0	&& current_item == item->uid)
		{
			timer = 0;
			current_item = 0;
		}
		Item::onItemIdentified(player.playernum, item);
		if ( item->type == GEM_GLASS )
		{
			steamStatisticUpdate(STEAM_STAT_RHINESTONE_COWBOY, STEAM_STAT_INT, 1);
		}
	}
	else*/
	{
		Item* oldItemToUpdate = nullptr;
		bool doMessage = true;
		if ( current_item > 0 )
		{
			oldItemToUpdate = uidToItem(current_item);
		}
		else
		{
			if ( old_item == item->uid ) // auto appraising picked the same item
			{
				doMessage = false; // cut back on appraisal spam
			}
		}

		if ( doMessage )
		{
			messagePlayer(player.playernum, MESSAGE_INVENTORY, Language::get(321), item->description());
		}

		//Tick the timer in act player.
		//Once the timer hits zero, roll to see if the item is identified.
		//If it is identified, identify it and print out a message for the player.
		timer = getAppraisalTime(item);
		//if ( stats[player.playernum]->getModifiedProficiency(PRO_APPRAISAL) >= CAPSTONE_UNLOCK_LEVEL[PRO_APPRAISAL] )
		//{
		//	timer = 1; // our modified proficiency is legendary, so make a really short timer to almost be instant
		//}
		timermax = timer;
		if ( oldItemToUpdate && current_item != item->uid )
		{
			bool itemOnPaperDoll = false;
			if ( player.paperDoll.enabled && itemIsEquipped(oldItemToUpdate, player.playernum) )
			{
				auto slotType = player.paperDoll.getSlotForItem(*oldItemToUpdate);
				if ( slotType != Player::PaperDoll_t::SLOT_MAX )
				{
					itemOnPaperDoll = true;
				}
			}

			int itemx = oldItemToUpdate->x;
			int itemy = oldItemToUpdate->y;
			if ( itemOnPaperDoll )
			{
				player.paperDoll.getCoordinatesFromSlotType(player.paperDoll.getSlotForItem(*oldItemToUpdate), itemx, itemy);
			}

			if ( auto slotFrame = player.inventoryUI.getItemSlotFrame(oldItemToUpdate, itemx, itemy) )
			{
				if ( auto appraisalFrame = slotFrame->findFrame("appraisal frame") )
				{
					appraisalFrame->setDisabled(true);
				}
			}
		}
		current_item = item->uid;
		if ( appraisalProgressionItems.find(current_item) != appraisalProgressionItems.end() )
		{
			timer = std::min(appraisalProgressionItems[current_item], timer);
		}

		if ( doMessage )
		{
			animAppraisal = PI;
			animStartTick = ticks;
		}

		manual_appraised_item = 0;
	}
	old_item = 0;
}

DynamicArray Player::Inventory_t::Appraisal_t::appraisal_time_points;  // vector<pair<int,int>>
DynamicArray Player::Inventory_t::Appraisal_t::appraisal_tables;  // vector<AppraisalBreakpoint_t> (POD)
int Player::Inventory_t::Appraisal_t::fastTimeAppraisal = 10 * TICKS_PER_SECOND;
int Player::Inventory_t::Appraisal_t::perStatMult = 3;
void Player::Inventory_t::Appraisal_t::readFromFile()
{
	std::string filename = "data/appraisal_tables.json";
	if ( !PHYSFS_getRealDir(filename.c_str()) )
	{
		printlog("[JSON]: Error: Could not locate json file %s", filename.c_str());
		return;
	}

	std::string inputPath = PHYSFS_getRealDir(filename.c_str());
	inputPath.append(PHYSFS_getDirSeparator());
	inputPath.append(filename.c_str());

	File* fp = FileIO::open(inputPath.c_str(), "rb");
	if ( !fp )
	{
		printlog("[JSON]: Error: Could not locate json file %s", inputPath.c_str());
		return;
	}

	static char buf[32000];
	int count = fp->read(buf, sizeof(buf[0]), sizeof(buf) - 1);
	buf[count] = '\0';
	FileIO::close(fp);

	void* jsonReader = json_reader_parse(buf);
	if ( !jsonReader )
	{
		return;
	}
	void* root = json_node_root(jsonReader);
	if ( !json_node_is_object(root) )
	{
		json_reader_destroy(jsonReader);
		return;
	}
	if ( !json_node_has_member(root, "version") 
		|| !json_node_has_member(root, "appraisal_times") 
		|| !json_node_has_member(root, "appraisal_tables")
		|| !json_node_has_member(root, "fast_time_seconds") 
		|| !json_node_has_member(root, "per_stat_mult") )
	{
		printlog("[JSON]: Error: No 'version' value in json file, or JSON syntax incorrect! %s", inputPath.c_str());
		json_reader_destroy(jsonReader);
		return;
	}

	barony_dynamic_array_clear(&appraisal_time_points);
	barony_dynamic_array_clear(&appraisal_tables);

	int32_t ft = 0; json_node_get_int(json_node_get_member(root, "fast_time_seconds"), &ft);
	fastTimeAppraisal = ft * TICKS_PER_SECOND;
	int32_t psm = 0; json_node_get_int(json_node_get_member(root, "per_stat_mult"), &psm);
	perStatMult = psm;

	void* appraisal_times = json_node_get_member(root, "appraisal_times");
	uint32_t timesCount = json_node_array_size(appraisal_times);
	for ( uint32_t i = 0; i < timesCount; ++i )
	{
		void* elem = json_node_element_at(appraisal_times, i);
		int32_t value = 0; json_node_get_int(json_node_get_member(elem, "value"), &value);
		int32_t slow_time = 0; json_node_get_int(json_node_get_member(elem, "slow_time_seconds"), &slow_time);

		dynarray_pair_push<std::pair<int, int>>(appraisal_time_points, std::make_pair((int)value, (int)(slow_time * TICKS_PER_SECOND)));
	}

	void* appraisal_tables_node = json_node_get_member(root, "appraisal_tables");
	uint32_t tablesCount = json_node_array_size(appraisal_tables_node);
	for ( uint32_t i = 0; i < tablesCount; ++i )
	{
		void* elem = json_node_element_at(appraisal_tables_node, i);
		int32_t skill = 0; json_node_get_int(json_node_get_member(elem, "skill"), &skill);
		int32_t gold_value_limit = 0; json_node_get_int(json_node_get_member(elem, "gold_value_limit"), &gold_value_limit);
		int32_t fast_time_gold = 0; json_node_get_int(json_node_get_member(elem, "fast_time_gold"), &fast_time_gold);

		dynarray_push<Player::Inventory_t::Appraisal_t::AppraisalBreakpoint_t>(appraisal_tables, Player::Inventory_t::Appraisal_t::AppraisalBreakpoint_t());
		auto& table = *dynarray_at<Player::Inventory_t::Appraisal_t::AppraisalBreakpoint_t>(appraisal_tables, dynarray_size<Player::Inventory_t::Appraisal_t::AppraisalBreakpoint_t>(appraisal_tables) - 1);

		table.skillLVL = skill;
		table.goldValueLimit = gold_value_limit;
		table.fastTimeGold = fast_time_gold;
	}

	json_reader_destroy(jsonReader);
}

int Player::Inventory_t::Appraisal_t::getAppraisalTime(Item* item)
{
	int appraisal_time = 0;

	//if ( item->type != GEM_GLASS )
	{
		int skillLVL = stats[player.playernum]->getModifiedProficiency(PRO_APPRAISAL)
			+ statGetPER(stats[player.playernum], player.entity) * Player::Inventory_t::Appraisal_t::perStatMult;
		int value = item->type == GEM_GLASS ? 1000 : item->getGoldValue();

		bool fast_time = false;

		for ( int64_t _at = 0; _at < dynarray_size<Player::Inventory_t::Appraisal_t::AppraisalBreakpoint_t>(appraisal_tables); ++_at )
		{
			auto& table = *dynarray_at<Player::Inventory_t::Appraisal_t::AppraisalBreakpoint_t>(appraisal_tables, _at);
			if ( skillLVL >= table.skillLVL )
			{
				fast_time = value <= table.fastTimeGold;
				break;
			}
		}

		appraisal_time = Player::Inventory_t::Appraisal_t::fastTimeAppraisal;
		if ( !fast_time )
		{
			for ( int64_t _tp = 0; _tp < dynarray_pair_size<std::pair<int, int>>(appraisal_time_points); ++_tp )
			{
				auto& pair = *dynarray_pair_at<std::pair<int, int>>(appraisal_time_points, _tp);
				if ( value >= pair.first )
				{
					appraisal_time = pair.second;
					break;
				}
			}
		}

		Category cat = itemCategory(item);
		if ( cat == FOOD || cat == SCROLL || cat == POTION )
		{
			real_t ratio = std::max(0.2, 1.0 + (-skillLVL) / 100.0);
			appraisal_time = std::max((real_t)Player::Inventory_t::Appraisal_t::fastTimeAppraisal * ratio, appraisal_time * ratio);
			appraisal_time = std::max(2 * TICKS_PER_SECOND, appraisal_time);
		}
		else if ( skillLVL >= 50 )
		{
			real_t ratio = std::max(0.2, 0.5 + (100 - skillLVL) / 100.0);
			appraisal_time = std::max((real_t)Player::Inventory_t::Appraisal_t::fastTimeAppraisal * ratio, appraisal_time * ratio);
			appraisal_time = std::max(2 * TICKS_PER_SECOND, appraisal_time);
		}
		/*if ( fast_time )
		{
			appraisal_time = 10;
		}
		else if ( value > 3000 )
		{
			appraisal_time = 12 * 60;
		}
		else if ( value > 1000 )
		{
			appraisal_time = 6 * 60;
		}
		else if ( value > 500 )
		{
			appraisal_time = 3 * 60;
		}
		else if ( value > 250 )
		{
			appraisal_time = 2 * 60;
		}
		else if ( value > 100 )
		{
			appraisal_time = 1 * 60;
		}
		else if ( value > 50 )
		{
			appraisal_time = 30;
		}
		else
		{
			appraisal_time = 15;
		}*/

		//appraisal_time = (items[item->type].value * 60) / (stats[this->player.playernum]->getModifiedProficiency(PRO_APPRAISAL) + 1);    // time in ticks until item is appraised
		if ( stats[player.playernum] && stats[player.playernum]->mask && stats[player.playernum]->mask->type == MONOCLE )
		{
			if ( cat == GEM )
			{
				real_t mult = 1.0;
				if ( stats[player.playernum]->mask->beatitude == 0 )
				{
					mult = .5;
				}
				else if ( stats[player.playernum]->mask->beatitude > 0 || shouldInvertEquipmentBeatitude(stats[player.playernum]) )
				{
					mult = .25;
				}
				else if ( stats[player.playernum]->mask->beatitude < 0 )
				{
					mult = 2.0;
				}
				appraisal_time *= mult;
			}
		}
		int playerCount = 0;
		for ( int i = 0; i < MAXPLAYERS; ++i )
		{
			if ( !client_disconnected[i] )
			{
				++playerCount;
			}
		}
		if ( playerCount == 3 )
		{
			appraisal_time /= 1.25;
		}
		else if ( playerCount == 4 )
		{
			appraisal_time /= 1.5;
		}
		if ( stats[player.playernum]->getModifiedProficiency(PRO_APPRAISAL) >= SKILL_LEVEL_LEGENDARY )
		{
			appraisal_time *= 0.75;
		}
		//messagePlayer(clientnum, "time: %d", appraisal_time);
	}
	//else
	//{
	//	appraisal_time = (1000 * 60) / (stats[this->player.playernum]->getModifiedProficiency(PRO_APPRAISAL) + 1);    // time in ticks until item is appraised+-
	//	if ( stats[player.playernum] && stats[player.playernum]->mask && stats[player.playernum]->mask->type == MONOCLE )
	//	{
	//		real_t mult = 1.0;
	//		if ( stats[player.playernum]->mask->beatitude == 0 )
	//		{
	//			mult = .5;
	//		}
	//		else if ( stats[player.playernum]->mask->beatitude >= 0 || shouldInvertEquipmentBeatitude(stats[player.playernum]) )
	//		{
	//			mult = .25;
	//		}
	//		else if ( stats[player.playernum]->mask->beatitude < 0 )
	//		{
	//			mult = 2.0;
	//		}
	//		appraisal_time *= mult;
	//	}
	//	int playerCount = 0;
	//	for ( int i = 0; i < MAXPLAYERS; ++i )
	//	{
	//		if ( !client_disconnected[i] )
	//		{
	//			++playerCount;
	//		}
	//	}
	//	if ( playerCount == 3 )
	//	{
	//		appraisal_time /= 1.15;
	//	}
	//	else if ( playerCount == 4 )
	//	{
	//		appraisal_time /= 1.25;
	//	}
	//}
	appraisal_time = std::min(std::max(1, appraisal_time), 36000);
	return appraisal_time;
}
//
//inline Item* getItemInfoFromIdentifyGUI(const int player, int slot)
//{
//	if ( slot >= 4 )
//	{
//		return nullptr; //Out of bounds,
//	}
//
//	return identify_items[player][slot];
//}
//
//void selectIdentifySlot(const int player, int slot)
//{
//	if ( slot < selectedIdentifySlot[player] )
//	{
//		//Moving up.
//
//		/*
//		 * Possible cases:
//		 * * 1) Move cursor up the GUI through different selectedIdentifySlot.
//		 * * 2) Page up through identifyscroll--
//		 * * 3) Scrolling up past top of Identify GUI, no identifyscroll (move back to inventory)
//		 */
//
//		if ( selectedIdentifySlot[player] <= 0 )
//		{
//			//Covers cases 2 & 3.
//
//			/*
//			 * Possible cases:
//			 * * A) Hit very top of Identify "inventory", can't go any further. Return to inventory.
//			 * * B) Page up, scrolling through identifyscroll.
//			 */
//
//			if ( identifyscroll <= 0 )
//			{
//				//Case 3/A: Return to inventory.
//				selectedIdentifySlot[player] = -1;
//			}
//			else
//			{
//				//Case 2/B: Page up through Identify "inventory".
//				--identifyscroll[player];
//			}
//		}
//		else
//		{
//			//Covers case 1.
//
//			//Move cursor up the GUI through different selectedIdentifySlot (--selectedIdentifySlot).
//			--selectedIdentifySlot[player];
//			warpMouseToSelectedIdentifySlot(player);
//		}
//	}
//	else if ( slot > selectedIdentifySlot[player] )
//	{
//		//Moving down.
//
//		/*
//		 * Possible cases:
//		 * * 1) Moving cursor down through GUI through different selectedIdentifySlot.
//		 * * 2) Scrolling down past bottom of Identify GUI through identifyscroll++
//		 * * 3) Scrolling down past bottom of Identify GUI, max Identify scroll (revoke move -- can't go beyond limit of Identify GUI).
//		 */
//
//		if ( selectedIdentifySlot[player] >= NUM_IDENTIFY_GUI_ITEMS - 1 )
//		{
//			//Covers cases 2 & 3.
//			++identifyscroll[player]; //identifyscroll is automatically sanitized in updateIdentifyGUI().
//		}
//		else
//		{
//			//Covers case 1.
//			//Move cursor down through the GUi through different selectedIdentifySlot (++selectedIdentifySlot).
//			//This is a little bit trickier since must revoke movement if there is no item in the next slot!
//
//			/*
//			 * Two possible cases:
//			 * * A) Items below this. Advance selectedIdentifySlot to them.
//			 * * B) On last item already. Do nothing (revoke movement).
//			 */
//
//			Item* item = getItemInfoFromIdentifyGUI(player, selectedIdentifySlot[player] + 1);
//
//			if ( item )
//			{
//				++selectedIdentifySlot[player];
//				warpMouseToSelectedIdentifySlot(player);
//			}
//			else
//			{
//				//No more items. Stop.
//			}
//		}
//	}
//}

//void warpMouseToSelectedIdentifySlot(const int player)
//{
//	SDL_Rect slotPos;
//	slotPos.x = getIdentifyGUIStartX(player);
//	slotPos.w = inventoryoptionChest_bmp->w;
//	slotPos.h = inventoryoptionChest_bmp->h;
//	slotPos.y = getIdentifyGUIStartY(player) + 16 + (slotPos.h * selectedIdentifySlot[player]);
//
//	int newx = slotPos.x + (slotPos.w / 2);
//	int newy = slotPos.y + (slotPos.h / 2);
//	//SDL_WarpMouseInWindow(screen, newx, newy);
//	Uint32 flags = (Inputs::SET_CONTROLLER | Inputs::SET_MOUSE);
//	inputs.warpMouse(player, newx, newy, flags);
//}
