// colors.odin -- Odin mirror of colors.hpp.
package main

// SDL_Color — the constexpr in colors.hpp expands via SDL.h: {Uint8 r,g,b,a}
// In Odin we use the same layout so sizeof stays 4.
SDL_Color :: struct {
	r: u8,
	g: u8,
	b: u8,
	a: u8,
}
#assert(size_of(SDL_Color) == 4)

sdlColorWhite :: SDL_Color{255, 255, 255, 255}

// 32-bit packed RGBA colors — all mirror the makeColor/makeColorRGB calls
// in colors.hpp. Values are the constexpr result on little-endian x64.
uint32ColorBlack             :: 0xff000000 // makeColorRGB(0,0,0)
uint32ColorWhite             :: 0xffffffff // makeColorRGB(255,255,255)
uint32ColorGray              :: 0xff7f7f7f // makeColorRGB(127,127,127)
uint32ColorBlue              :: 0xffff5c00 // makeColor(0,92,255,255)
uint32ColorLightBlue         :: 0xffffff00 // makeColor(0,255,255,255)
uint32ColorBaronyBlue        :: 0xffffc000 // makeColor(0,192,255,255) — Dodger Blue
uint32ColorRed               :: 0xff0000ff // makeColor(255,0,0,255)
uint32ColorGreen             :: 0xff00ff00 // makeColor(0,255,0,255)
uint32ColorOrange            :: 0xff0080ff // makeColor(255,128,0,255)
uint32ColorYellow            :: 0xff00ffff // makeColor(255,255,0,255)

uint32ColorPlayer1           :: 0xff40d4ff // makeColorRGB(255,212,64)
uint32ColorPlayer2           :: 0xff40ff40 // makeColorRGB(64,255,64)
uint32ColorPlayer3           :: 0xff4040ff // makeColorRGB(255,64,64)
uint32ColorPlayer4           :: 0xffffa0ff // makeColorRGB(255,160,255)
uint32ColorPlayer5           :: 0xffffc020 // makeColorRGB(32,192,255)
uint32ColorPlayer6           :: 0xff2080ff // makeColorRGB(255,128,32)
uint32ColorPlayer7           :: 0xff8020ff // makeColorRGB(255,32,128)
uint32ColorPlayer8           :: 0xffffffff // makeColorRGB(255,255,255)
uint32ColorPlayerX           :: 0xffbfbfbf // makeColorRGB(191,191,191)

uint32ColorPlayer1_colorblind :: 0xff4040ff // makeColorRGB(255,64,64)
uint32ColorPlayer2_colorblind :: 0xffffa0ff // makeColorRGB(255,160,255)
uint32ColorPlayer3_colorblind :: 0xff40ff40 // makeColorRGB(64,255,64)
uint32ColorPlayer4_colorblind :: 0xffffffff // makeColorRGB(255,255,255)
uint32ColorPlayer5_colorblind :: 0xffffc020 // makeColorRGB(32,192,255)
uint32ColorPlayer6_colorblind :: 0xff2080ff // makeColorRGB(255,128,32)
uint32ColorPlayer7_colorblind :: 0xff8020ff // makeColorRGB(255,32,128)
uint32ColorPlayer8_colorblind :: 0xff40d4ff // makeColorRGB(255,212,64)
uint32ColorPlayerX_colorblind :: 0xffbfbfbf // makeColorRGB(191,191,191)
