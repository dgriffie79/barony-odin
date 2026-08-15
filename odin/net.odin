// net.odin — Odin mirrors of net.hpp.
package main

import "containers"

// struct SteamPacketWrapper — 16 bytes
SteamPacketWrapper :: struct {
	_data: ^i32, // int*
	_len:  i32,
}
#assert(size_of(SteamPacketWrapper) == 16)

// class NetHandler — 72 bytes
NetHandler :: struct {
	steam_packet_thread:                    ^i32, // int*
	continue_multithreading_steam_packets:  bool,
	game_packets_lock:                      ^i32, // int*
	game_packets:                           containers.Raw_Dynamic_Array, // DynamicArrayT<SteamPacketWrapper*> (40B)
	continue_multithreading_steam_packets_lock: ^i32,
}
#assert(size_of(NetHandler) == 72)

// struct PingNetworkStatus_t — 64 bytes
PingNetworkStatus_t :: struct {
	pings:                     containers.Raw_Map, // DynamicMapI32T<Uint32> (32B)
	last_pingtime:             u32,
	last_sequence:             u32,
	oldest_sequence_ticks:     u32,
	sequence:                  u32,
	display_millis:            u32,
	display_millis_immediate:  u32,
	hud_display_ok_ticks:      u32,
	needs_update:              bool,
}
#assert(size_of(PingNetworkStatus_t) == 64)
