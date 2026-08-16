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
	game_packets:                           [dynamic]rawptr, // DynamicArrayT<SteamPacketWrapper*> (pointers, non-owning)
	continue_multithreading_steam_packets_lock: ^i32,
}
#assert(size_of(NetHandler) == 72)

// struct PingNetworkStatus_t — 64 bytes
PingNetworkStatus_t :: struct {
	pings:                     map[[4]byte]u32, // DynamicMapI32T<Uint32> (i32-keyed)
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
