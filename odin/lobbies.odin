// lobbies.odin -- Odin mirror of lobbies.hpp.
package main

// enum LobbyServiceType : int
LobbyServiceType :: enum i32 {
	LOBBY_DISABLE,
	LOBBY_STEAM,
	LOBBY_CROSSPLAY,
	LOBBY_COMBINED,
}

// struct LobbyHandler_t::LobbySearchResult_t - 8 bytes
// (POD mirror of std::pair<Sint32, LobbyServiceType>)
LobbySearchResult_T :: struct {
	first:  i32, // Sint32
	second: LobbyServiceType,
}
#assert(size_of(LobbySearchResult_T) == 8)

// enum EResult_LobbyFailures : int
EResult_LobbyFailures :: enum i32 {
	LOBBY_NO_ERROR            = 1,
	LOBBY_USING_SAVEGAME      = 50000,
	LOBBY_WRONG_SAVEGAME      = 50001,
	LOBBY_NOT_USING_SAVEGAME  = 50002,
	LOBBY_NO_OWNER            = 50003,
	LOBBY_GAME_IN_PROGRESS    = 50004,
	LOBBY_UNHANDLED_ERROR     = 50005,
	LOBBY_JOIN_CANCELLED      = 50006,
	LOBBY_JOIN_TIMEOUT        = 50007,
	LOBBY_NOT_FOUND           = 50008,
	LOBBY_TOO_MANY_PLAYERS    = 50009,
	LOBBY_NOT_ALLOWED         = 50010,
	LOBBY_YOU_ARE_BANNED      = 50011,
	LOBBY_TOO_MANY_JOINS      = 50012,
	LOBBY_SAVEGAME_REQUIRES_DLC = 50013,
}

// class LobbyHandler_t - the class has a 200-element LobbySearchResult_t array.
// Its exact C++ layout includes vtable/private bits; the Odin mirror stores
// the payload fields that are accessed via the struct (hostingType etc. + the
// 200-element array).
LobbyHandler_T :: struct {
	filter_show_in_progress_lobbies: bool, // private, but layout-visible
	hosting_type:          LobbyServiceType,
	joining_type:          LobbyServiceType,
	search_type:           LobbyServiceType,
	p2p_type:              LobbyServiceType,
	lobby_displayed_search_results: [200]LobbySearchResult_T,
	num_lobby_display_search_results: u32,
	selected_lobby_in_list: i32,
	show_lobby_filters:    bool,
	crossplay_enabled:     bool,
}
