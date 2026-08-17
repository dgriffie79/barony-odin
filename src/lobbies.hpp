/*-------------------------------------------------------------------------------

BARONY
File: lobbies.hpp
Desc: header for lobbies.cpp (matchmaking handlers)

Copyright 2013-2020 (c) Turning Wheel LLC, all rights reserved.
See LICENSE for details.

-------------------------------------------------------------------------------*/

#pragma once

#include <string>

class LobbyHandler_t
{
	static constexpr int kNumSearchResults = 200;
	bool filterShowInProgressLobbies = false;
public:
	LobbyHandler_t() :
		lobbyDisplayedSearchResults{}
	{
		for ( int _i = 0; _i < kNumSearchResults; ++_i )
		{
			lobbyDisplayedSearchResults[_i] = { -1, LOBBY_DISABLE };
		}
	};

	enum LobbyServiceType : int
	{
		LOBBY_DISABLE,
		LOBBY_STEAM,
		LOBBY_CROSSPLAY,
		LOBBY_COMBINED
	};
	// POD mirror of std::pair<Sint32, LobbyServiceType> (8B: index + enum).
	struct LobbySearchResult_t {
		Sint32 first;
		LobbyServiceType second;
	};
	LobbyServiceType hostingType = LOBBY_DISABLE;
	LobbyServiceType joiningType = LOBBY_DISABLE;
	LobbyServiceType searchType = LOBBY_DISABLE;
	LobbyServiceType P2PType = LOBBY_DISABLE;
	void handleLobbyListRequests();
	void updateSearchResults();
	static void filterLobbyButton(button_t* my);
	static void searchLobbyWithFilter(button_t* my);
	void drawLobbyFilters();
	LobbyServiceType getDisplayedResultLobbyType(int selection);
	Sint32 getDisplayedResultLobbyIndex(int selection);
	LobbySearchResult_t lobbyDisplayedSearchResults[kNumSearchResults];
	Uint32 numLobbyDisplaySearchResults = 0;
	int selectedLobbyInList = 0;
	bool showLobbyFilters = false;

	bool crossplayEnabled = false;

	std::string getCurrentRoomKey() const;

	LobbyServiceType getHostingType() const
	{
		return hostingType;
	}
	LobbyServiceType getJoiningType() const
	{
		return joiningType;
	}
	LobbyServiceType getP2PType() const
	{
		return P2PType;
	}
	LobbyServiceType setLobbyJoinTypeOfCurrentSelection();
	void setHostingType(LobbyServiceType type);
	void setLobbyJoinType(LobbyServiceType type);
	void setP2PType(LobbyServiceType type);
	static void logError(const char* str, ...);
	static void vlogError(const char* str, va_list argptr);
	static std::string getLobbyJoinFailedConnectString(int result);
	enum EResult_LobbyFailures : int
	{
	    LOBBY_NO_ERROR = 1,             // no error (success)
		LOBBY_USING_SAVEGAME = 50000,   // trying to join a savegame lobby with a new character
		LOBBY_WRONG_SAVEGAME,           // trying to join a savegame lobby with the wrong save file
		LOBBY_NOT_USING_SAVEGAME,       // trying to join a newgame lobby with a savegame
		LOBBY_NO_OWNER,                 // no one in lobby (ghost lobby)
		LOBBY_GAME_IN_PROGRESS,         // game is already in progress
		LOBBY_UNHANDLED_ERROR,          // unknown/unhandled error type
		LOBBY_JOIN_CANCELLED,           // cancelled join request
		LOBBY_JOIN_TIMEOUT,             // timeout connecting to server
		LOBBY_NOT_FOUND,                // server no longer exists
		LOBBY_TOO_MANY_PLAYERS,         // server is full
		LOBBY_NOT_ALLOWED,              // server won't allow you in for one reason or another
		LOBBY_YOU_ARE_BANNED,           // can't join lobby because you are banned
        LOBBY_TOO_MANY_JOINS,           // overloaded lobby with join requests
		LOBBY_SAVEGAME_REQUIRES_DLC		// our client does not have DLC detected for their savefile
	};
};
extern LobbyHandler_t LobbyHandler;
