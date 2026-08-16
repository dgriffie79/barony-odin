// ui_mainmenu.odin — Odin mirror of ui/MainMenu.hpp.
package main

// enum class MainMenu::FadeDestination : Uint8 (31 values)
FadeDestination :: enum u8 {
	None,
	TitleScreen,
	RootMainMenu,
	RootMainMenuNoEndGame,
	Endgame,
	Victory,
	IntroStoryScreen,
	IntroStoryScreenNoMusicFade,
	HerxMidpointHuman,
	HerxMidpointAutomaton,
	HerxMidpointBeast,
	HerxMidpointEvil,
	BaphometMidpointHuman,
	BaphometMidpointAutomaton,
	BaphometMidpointBeast,
	BaphometMidpointEvil,
	EndingHuman,
	EndingAutomaton,
	EndingBeast,
	EndingEvil,
	ClassicEndingHuman,
	ClassicEndingAutomaton,
	ClassicEndingBeast,
	ClassicEndingEvil,
	ClassicBaphometEndingHuman,
	ClassicBaphometEndingAutomaton,
	ClassicBaphometEndingBeast,
	ClassicBaphometEndingEvil,
	GameStart,
	GameStartDummy,
	HallOfTrials,
}

// enum class MainMenu::DLC
MainMenu_DLC :: enum i32 {
	Base,
	MythsAndOutcasts,
	LegendsAndPariahs,
	DesertersAndDisciples,
}

// SurvivalComplexityEntry_t — 32B (tuple<int,string,Uint32> mirror).
// Value for ClassDescriptions::DescData_t::survivalComplexity.
SurvivalComplexityEntry_T :: struct {
	value: i32,   // complexity level (1-5)
	label: string, // "*".."*****"
	color: u32,
}
#assert(size_of(SurvivalComplexityEntry_T) == 32)

// struct ClassDescriptions::DescData_t — 200 bytes
ClassDescData_T :: struct {
	text:               string, // DynamicString
	internal_name:      string,
	survival_complexity: [dynamic]SurvivalComplexityEntry_T, // DynamicArrayT (40B)
	stat_ratings:       [dynamic]u32,    // DynamicArrayU32
	stat_ratings_strings: [dynamic]string, // DynamicArrayStr
	hp:                 i32, // Sint32 (default DEFAULT_HP)
	mp:                 i32, // Sint32 (default DEFAULT_MP)
	line_paddings:      [dynamic]i32, // DynamicArrayS32
}
#assert(size_of(ClassDescData_T) == 200)

// struct RaceDescriptions::DescData_t — 248 bytes
RaceDescData_T :: struct {
	text_left:                 string, // DynamicString
	text_right:                string,
	trait_lines:               map[i32]struct{}, // DynamicSetI32
	pro_lines:                 map[i32]struct{}, // DynamicSetI32
	line_paddings:             [dynamic]i32, // DynamicArrayS32
	title:                     string,
	traits_based_on_player_race: string,
	traits_based_on_monster_type: string,
	resistances:               string,
	weaknesses:                string,
	friendly_with:             string,
	racial_spells:             string,
}
#assert(size_of(RaceDescData_T) == 248)
