# Remaining std:: containers in headers (117 total)


## src/draw.hpp (5)
  164	std::unordered_map<BufferType, int>
  181	std::vector<float>
  397	std::vector<float>
  403	std::vector<Sint32>
  410	std::unordered_map<view_t*, Dither>

## src/entity.hpp (2)
  106	std::unordered_map<view_t*, Dither>
  976	std::unordered_set<Entity*>

## src/files.hpp (2)
  391	std::list<std::string>
  400	std::list<std::string>

## src/game.hpp (1)
  516	std::unordered_map<unsigned long, std::pair<std::string, int>

## src/input.hpp (3)
  148	std::unordered_map<int, SDL_GameController*>
  149	std::unordered_map<int, SDL_Joystick*>
  150	std::unordered_map<SDL_Keycode, bool>

## src/interface/interface.hpp (3)
  1174	std::unordered_map<std::string, std::pair<int, bool>
  1175	std::vector<std::pair<std::string, std::pair<int, bool>
  1604	std::vector<std::pair<SDL_Surface**, std::string>

## src/light.hpp (2)
  64	std::vector<void*>
  98	std::vector<void*>

## src/magic/magic.hpp (6)
  481	std::map<Uint32, std::map<Uint32, ParticleEmitterHit_t>
  873	std::map<int, spell_t*>
  874	std::map<int, std::map<int, int>
  875	std::map<int, int>
  1300	std::map<int, std::map<std::tuple<Uint8, Uint8, Uint8, Uint8, real_t, real_t, int>
  1302	std::map<Uint32, Indicator_t>

## src/main.hpp (1)
  691	std::unordered_map<SDL_Keycode, bool>

## src/menu.hpp (3)
  141	std::list<std::string>
  156	std::vector<std::tuple<int, int, int, std::string>
  250	std::list<resolution>

## src/mod_tools.hpp (53)
  2907	std::map<Sint32, spellItem_t>
  2908	std::map<std::string, ItemTooltip_t>
  2909	std::map<std::string, std::map<std::string, std::string>
  3015	std::map<Uint32, Statue_t>
  3026	std::map<std::string, std::vector<std::pair<std::string, std::chrono::high_resolution_clock::time_point>
  3143	std::map<std::string, Entry_t>
  3409	std::map<int, EntityColliderData_t>
  3410	std::map<std::string, std::map<int, int>
  3429	std::vector<std::pair<SDL_Surface**, std::string>
  3430	std::vector<std::pair<std::string, std::string>
  3431	std::vector<std::pair<std::string, std::string>
  3432	std::set<std::string>
  3433	std::set<std::string>
  3434	std::list<std::string>
  3521	std::map<int, std::map<int, ModelOffset_t>
  3574	std::map<std::string, std::vector<std::pair<std::string, std::string>
  3612	std::set<std::pair<std::string, std::string>
  3613	std::map<std::string, std::vector<std::pair<std::string, std::string>
  3614	std::unordered_set<std::string>
  3616	std::map<std::string, std::vector<std::pair<std::string, std::string>
  3629	std::map<std::string, CompendiumAchievementsDisplay>
  4070	std::map<std::string, std::vector<std::pair<std::string, std::string>
  4072	std::map<std::string, std::vector<std::pair<std::string, std::string>
  4078	std::map<std::string, CompendiumMonsters_t::Monster_t>
  4090	std::map<std::string, ObjectLimbs_t>
  4098	std::map<std::string, std::pair<CompendiumMap_t, std::vector<int>
  4116	std::map<std::string, std::vector<std::pair<std::string, std::string>
  4123	std::map<std::string, CompendiumWorld_t::World_t>
  4144	std::map<std::string, std::vector<std::pair<std::string, std::string>
  4151	std::map<std::string, CompendiumCodex_t::Codex_t>
  4175	std::map<std::string, std::vector<std::pair<std::string, std::string>
  4183	std::map<std::string, CompendiumItems_t::Codex_t>
  4189	std::map<std::string, std::vector<std::pair<std::string, std::string>
  4195	std::map<std::string, CompendiumItems_t::Codex_t>
  4288	std::set<std::string>
  4307	std::map<EventTags, Event_t>
  4309	std::map<int, std::set<EventTags>
  4318	std::map<EventTags, std::set<int>
  4319	std::map<EventTags, std::set<int>
  4320	std::map<EventTags, std::set<std::string>
  4321	std::map<EventTags, std::set<std::string>
  4324	std::map<EventTags, std::map<int, int>
  4326	std::map<EventTags, std::map<std::string, std::string>
  4327	std::map<std::string, std::map<std::string, std::string>
  4328	std::vector<std::pair<std::string, Sint32>
  4329	std::map<std::string, std::string>
  4339	std::map<EventTags, std::map<int, EventVal_t>
  4340	std::map<EventTags, std::map<int, EventVal_t>
  4368	std::unordered_set<unsigned int>
  4369	std::unordered_set<unsigned int>
  4370	std::map<unsigned int, std::string>
  4371	std::map<unsigned int, std::string>
  4372	std::map<unsigned int, std::string>

## src/monster.hpp (3)
  1285	std::map<int, IconLookup_t>
  1302	std::map<int, MonsterDataEntry_t>
  1401	std::unordered_map<Uint32, MonsterAllies_t>

## src/net.hpp (1)
  117	std::queue<SteamPacketWrapper* >

## src/player.hpp (10)
  741	std::map<std::string, DropDown_t>
  1303	std::map<std::string, std::pair<std::string, std::string>
  1465	std::map<Monster, std::vector<std::pair<Monster, std::string>
  1925	std::list<Message*>
  2073	std::map<Uint32, Dialogue_t>
  2096	std::map<Player::WorldUI_t::WorldTooltipDialogue_t::DialogueType_t, Setting_t>
  2329	std::vector<std::pair<std::string, std::string>
  2360	std::map<std::string, std::map<int, Sint32>
  2361	std::map<int, std::map<std::string, std::map<int, Sint32>
  2431	std::map<Uint32, std::map<Uint32, Uint32>

## src/scores.hpp (3)
  776	std::vector<std::pair<std::string, std::string>
  777	std::vector<std::pair<std::string, std::string>
  837	std::unordered_map<Uint32, std::unordered_map<int, std::pair<int,int>

## src/ui/Field.hpp (6)
  61	std::vector<const Widget*>
  68	std::vector<const Widget*>
  69	std::vector<const Widget*>
  189	std::map<int, Uint32>
  191	std::map<int, int>
  194	std::vector<std::pair<std::string,Text*>

## src/ui/Frame.hpp (3)
  465	std::vector<const Widget*>
  472	std::vector<const Widget*>
  473	std::vector<const Widget*>

## src/ui/MainMenu.hpp (3)
  131	std::vector<std::tuple<int, std::string, Uint32>
  138	std::unordered_map<int, DescData_t>
  161	std::unordered_map<std::string, DescData_t>

## src/ui/Widget.hpp (7)
  162	std::vector<Widget*>
  166	std::vector<const Widget*>
  190	std::list<Widget*>
  216	std::unordered_map<std::string, std::string>
  218	std::unordered_map<std::string, std::string>
  223	std::vector<const Widget*>
  224	std::vector<const Widget*>

TOTAL: 117
