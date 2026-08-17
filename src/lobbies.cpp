/*-------------------------------------------------------------------------------

BARONY
File: lobbies.cpp
Desc: contains code for matchmaking handlers

Copyright 2013-2020 (c) Turning Wheel LLC, all rights reserved.
See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "main.hpp"
#include "menu.hpp"
#include "game.hpp"
#include "lobbies.hpp"
#include "draw.hpp"
#include "player.hpp"
#include "scores.hpp"
#include "interface/interface.hpp"
#include "colors.hpp"
#include "net.hpp"

LobbyHandler_t LobbyHandler;

std::string LobbyHandler_t::getCurrentRoomKey() const
{
    if (multiplayer != CLIENT && multiplayer != SERVER) {
        return "";
    }
    const LobbyServiceType type = multiplayer == SERVER ?
        getHostingType() : getJoiningType();
    if (type == LobbyHandler_t::LobbyServiceType::LOBBY_STEAM) {
    }
    else if (type == LobbyHandler_t::LobbyServiceType::LOBBY_CROSSPLAY) {
    }
    return "";
}

extern "C" std::string LobbyHandler_t_getCurrentRoomKey(const LobbyHandler_t* self) { return self->getCurrentRoomKey(); }


std::string LobbyHandler_t::getLobbyJoinFailedConnectString(int result)
{
	char buf[1024] = "";
	switch ( result )
	{
		case EResult_LobbyFailures::LOBBY_GAME_IN_PROGRESS:
			snprintf(buf, 1023, "Unable to join lobby:\nGame in progress not joinable.");
			break;
		case EResult_LobbyFailures::LOBBY_USING_SAVEGAME:
			snprintf(buf, 1023, "Unable to join lobby:\n%s", Language::get(1381));
			break;
		case EResult_LobbyFailures::LOBBY_NOT_USING_SAVEGAME:
			snprintf(buf, 1023, "Unable to join lobby:\nOnly new characters allowed.");
			break;
		case EResult_LobbyFailures::LOBBY_WRONG_SAVEGAME:
			snprintf(buf, 1023, "Unable to join lobby:\nIncompatible save game.");
			break;
		case EResult_LobbyFailures::LOBBY_JOIN_CANCELLED:
			snprintf(buf, 1023, "Lobby join cancelled.\nSafely leaving lobby.");
			break;
		case EResult_LobbyFailures::LOBBY_NO_OWNER:
			snprintf(buf, 1023, "Unable to join lobby:\nLobby has no host.");
			break;
		case EResult_LobbyFailures::LOBBY_NOT_FOUND:
			snprintf(buf, 1023, "Unable to join lobby:\nLobby no longer exists.");
			break;
		case EResult_LobbyFailures::LOBBY_TOO_MANY_PLAYERS:
			snprintf(buf, 1023, "Unable to join lobby:\nLobby is full.");
			break;
		case EResult_LobbyFailures::LOBBY_SAVEGAME_REQUIRES_DLC:
			snprintf(buf, 1023, "Unable to join lobby:\n%s", Language::get(6100));
			break;
		case EResult_LobbyFailures::LOBBY_JOIN_TIMEOUT:
			snprintf(buf, 1023, "Unable to join lobby:\nTimeout waiting for host.");
			break;
		default:
			snprintf(buf, 1023, "Unable to join lobby:\nError code: %d.", result);
			break;
	}
	printlog("[Lobbies Error]: %s", buf);
	return buf;
}

extern "C" std::string LobbyHandler_t_getLobbyJoinFailedConnectString(int result) { return LobbyHandler_t::getLobbyJoinFailedConnectString(result); }



void LobbyHandler_t::handleLobbyListRequests()
{
	if ( !intro || joiningType == LOBBY_DISABLE )
	{
		return;
	}

	return;
}

extern "C" void LobbyHandler_t_handleLobbyListRequests(LobbyHandler_t* self) { return self->handleLobbyListRequests(); }


void LobbyHandler_t::updateSearchResults()
{
	numLobbyDisplaySearchResults = 0;

	for ( auto& result : lobbyDisplayedSearchResults )
	{
		result.first = -1;
		result.second = LOBBY_DISABLE;
	}
	return;
}

extern "C" void LobbyHandler_t_updateSearchResults(LobbyHandler_t* self) { return self->updateSearchResults(); }


LobbyHandler_t::LobbyServiceType LobbyHandler_t::getDisplayedResultLobbyType(int selection)
{
	if ( selection < 0 || selection >= kNumSearchResults )
	{
		return LOBBY_DISABLE;
	}
	return lobbyDisplayedSearchResults[selection].second;
}

extern "C" LobbyHandler_t::LobbyServiceType LobbyHandler_t_getDisplayedResultLobbyType(LobbyHandler_t* self, int selection) { return self->getDisplayedResultLobbyType(selection); }

Sint32 LobbyHandler_t::getDisplayedResultLobbyIndex(int selection)
{
	if ( selection < 0 || selection >= kNumSearchResults )
	{
		return -1;
	}
	return lobbyDisplayedSearchResults[selection].first;
}

extern "C" int LobbyHandler_t_getDisplayedResultLobbyIndex(LobbyHandler_t* self, int selection) { return self->getDisplayedResultLobbyIndex(selection); }



void LobbyHandler_t::filterLobbyButton(button_t* my)
{
	LobbyHandler.showLobbyFilters = !LobbyHandler.showLobbyFilters;
}

extern "C" void LobbyHandler_t_filterLobbyButton(button_t * my) { return LobbyHandler_t::filterLobbyButton(my); }


void LobbyHandler_t::searchLobbyWithFilter(button_t* my)
{
}

extern "C" void LobbyHandler_t_searchLobbyWithFilter(button_t * my) { return LobbyHandler_t::searchLobbyWithFilter(my); }


void LobbyHandler_t::drawLobbyFilters()
{
}

extern "C" void LobbyHandler_t_drawLobbyFilters(LobbyHandler_t* self) { return self->drawLobbyFilters(); }


LobbyHandler_t::LobbyServiceType LobbyHandler_t::setLobbyJoinTypeOfCurrentSelection() {
		if ( getDisplayedResultLobbyType(selectedLobbyInList) != LOBBY_DISABLE )
		{
			joiningType = getDisplayedResultLobbyType(selectedLobbyInList);
		}
		return joiningType;
	}

extern "C" LobbyHandler_t::LobbyServiceType LobbyHandler_t_setLobbyJoinTypeOfCurrentSelection(LobbyHandler_t* self) { return self->setLobbyJoinTypeOfCurrentSelection(); }


void LobbyHandler_t::setHostingType(LobbyServiceType type) {
		hostingType = type;
	}

extern "C" void LobbyHandler_t_setHostingType(LobbyHandler_t* self, LobbyHandler_t::LobbyServiceType type) { return self->setHostingType(type); }


void LobbyHandler_t::setLobbyJoinType(LobbyServiceType type) {
	    joiningType = type;
	}

extern "C" void LobbyHandler_t_setLobbyJoinType(LobbyHandler_t* self, LobbyHandler_t::LobbyServiceType type) { return self->setLobbyJoinType(type); }


void LobbyHandler_t::setP2PType(LobbyServiceType type) {
		P2PType = type;
	}

extern "C" void LobbyHandler_t_setP2PType(LobbyHandler_t* self, LobbyHandler_t::LobbyServiceType type) { return self->setP2PType(type); }


// LobbyHandler_t::logError is inline in lobbies.hpp; forward it.