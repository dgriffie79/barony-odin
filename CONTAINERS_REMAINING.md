# Remaining std:: containers in headers

Regenerated 2026-08-13 (was stale at "117" — that count predated D3aw–D3c7 and
counted local helpers/signatures too). This list is HEADER MEMBERS ONLY (the
shared-struct + global state that must de-STL to cross the C++↔Odin boundary),
plus flagged signatures that return/pass std containers.

## DONE since the old doc (D3aw … D3cc)

- monster.hpp: `iconSpritesAndPaths`, `monsterDataEntries`, `units`
- entity.hpp + draw.hpp: `dithering` (pointer-keyed → `DynamicMapPtrT`, new family)
- input.hpp: `gameControllers`, `joysticks`
- magic.hpp: `spellTomeAppearanceToID` (nested map kind), `spellTomeIDToAppearance`
- plus the entire earlier D3aw–D3c7 batch (see git log)

## Value-kind families now available

- str-key: `DynamicMapStrT<V>` (`barony_dynamic_map_str_*`)
- i32-key: `DynamicMapI32T<V>` (`barony_dynamic_map_i32_*`)
- **ptr-key (new):** `DynamicMapPtrT<V>` (`barony_dynamic_map_ptr_*`) — `map[rawptr]V`
- sets: `DynamicSetI32` / `DynamicSetStr`
- arrays: `DynamicArrayT<V>` (`barony_dynamic_array_elem_*`), raw `DynamicArray`
- string: `DynamicString`

## Remaining header members (by file)

### src/draw.hpp (3)
- `Mesh::ElementsPerVBO` — `unordered_map<BufferType,int>` (static const, enum key)
- `Mesh::data` — `std::vector<float>[BufferType::Max]` (array of vectors)
- `Chunk::tiles` — `std::vector<Sint32>`

### src/entity.hpp (1)
- `alertAlliesOnBeingHit(..., std::unordered_set<Entity*>*)` — signature param

### src/files.hpp (2)
- `directoryContents(...)` → returns `std::list<std::string>`
- `physfsGetFileNamesInDirectory(...)` → returns `std::list<std::string>`

### src/game.hpp (1)
- `DebugStatsClass::networkPackets` — `unordered_map<unsigned long, pair<string,int>>`

### src/input.hpp (1)
- `Input::keys` — `unordered_map<SDL_Keycode,bool>` **HAZARD**: demo files do
  `write(&Input::keys, sizeof(...))` + `write(&keystatus, sizeof(...))` — raw
  `sizeof` of the map changes format on conversion. Needs a demo-format decision.

### src/main.hpp (1)
- `keystatus` — `unordered_map<SDL_Keycode,bool>` (same demo-file hazard as above)

### src/interface/interface.hpp (3)
- `scrolls` — `unordered_map<string, pair<int,bool>>`
- `sortedScrolls` — `vector<pair<string,pair<int,bool>>>`
- `systemResourceImages` — `vector<pair<SDL_Surface**, string>>` (extern)

### src/interface/ui.hpp (1)
- `allNotifications` — `std::list<UIToastNotification>`

### src/magic/magic.hpp (4)
- `particleTimerEmitterHitEntities` — `map<Uint32, map<Uint32, ParticleEmitterHit_t>>`
  (nested map, POD values)
- `allGameSpells` — `map<int, spell_t*>` **PORT-WITH-FILE** (order-dependent global)
- `surfaceCache` — `map<int, map<tuple<...>, SDL_Surface*>>` (static; port-with-file)
- `indicators` — `map<Uint32, Indicator_t>` (owning; port-with-file)

### src/menu.hpp (3)
- `currentDirectoryFiles` — `std::list<std::string>` (extern)
- `savegamesList` — `vector<tuple<int,int,int,string>>` (extern)
- `getResolutionList(int, std::list<resolution>&)` — out-param

### src/mod_tools.hpp (40+)
- `spellItems` — `map<Sint32, spellItem_t>`
- `tooltips` — `map<string, ItemTooltip_t>`
- `adjectives` — `map<string, map<string,string>>`
- `allStatues` — `map<Uint32, Statue_t>`
- `timepoints` — `map<string, vector<pair<string, time_point>>>`
- `allEntries` — `map<string, Entry_t>`
- `colliderData` — `map<int, EntityColliderData_t>` (static)
- `colliderRandomGenPool` — `map<string, map<int,int>>` (static)
- `systemResourceImagesToReload` / `mountedFilepaths` / `mountedFilepathsSaved` — vectors
- `mods_loaded_local` / `mods_loaded_workshop` — `set<string>`
- `localModFoldernames` — `list<string>`
- `monsterModelsMap` — `map<int, map<int, ModelOffset_t>>`
- compendium block (`achievementCategories`, `contents`, `monsters`, `worldObjects`,
  `codex`, `items`, `magic`, `achievementsBookDisplay`, etc.)
- event block (`events`, `itemEventLookup`, `eventItemLookup`, `eventMonsterLookup`,
  `eventWorldLookup`, `eventCodexLookup`, `eventClassIds`, `eventLangEntries`,
  `eventCustomLangEntries`, `playerEvents`, `serverPlayerEvents`, floor maps)

### src/net.hpp (1)
- `game_packets` — `queue<SteamPacketWrapper*>` (port-with-file)

### src/player.hpp (10)
- `allDropDowns` — `map<string, DropDown_t>`
- `mapDisplayNamesDescriptions` — `map<string, pair<string,string>>`
- `leadershipAllyTableSpecialRecruitment` — `map<Monster, vector<pair<Monster,string>>>`
- `notification_messages` — `list<Message*>`
- `sharedDialogues` — `map<Uint32, Dialogue_t>`
- `settings` — `map<DialogueType_t, Setting_t>`
- `mapDetails` — `vector<pair<string,string>>`
- `itemEvents` — `map<string, map<int,Sint32>>`
- `floorEvents` — `map<int, map<string, map<int,Sint32>>>`
- `targetsCompelled` — `map<Uint32, map<Uint32,Uint32>>`

### src/scores.hpp (3)
- `map_messages` / `additional_data` — `vector<pair<string,string>>` **KEEP** (JSON
  serialization — reverted in D3aj; port with file)
- `entityAchievementsToProcess` — `unordered_map<Uint32, unordered_map<int, pair<int,int>>>`

### src/steam.hpp (1)
- `workshopItemTags` — `list<string>`

### src/ui/* (Field/Frame/Button/Slider/Widget/MainMenu) (~20)
- `Field::linesToColor` (`map<int,Uint32>`), `individualLinePadding` (`map<int,int>`),
  `cache` (`vector<pair<string,Text*>>`)
- `Widget::widgets` — `list<Widget*>`
- `findSelectedWidgets(std::vector<Widget*>&)` + `selectedWidgets`/`searchParents`
  `vector<const Widget*>` params across Widget/Frame/Field/Button/Slider
- `MainMenu::survivalComplexity` (`vector<tuple<int,string,Uint32>>`),
  `data` (`unordered_map<int/string, DescData_t>`)

## Hazards to keep in mind (from the current batch)

1. **Pointer-keyed maps** need the new `map[rawptr]V` family (done in D3c9).
2. **bool-valued maps + `sizeof()` demo serialization** — `Input::keys` and
   `keystatus` are written raw to demo files; converting changes the format.
   Decide: leave std::unordered_map (port-with-file), or fix demo format.
3. **Non-zero struct defaults** — `Chunk::Dither.value = 10` vs `Entity::Dither = 0`;
   the ptr-map entry path takes a per-kind `default_value`.
4. **Nested maps** — each `map<K, map<K2,V>>` needs its own value kind
   (MK_I32Map done for `map<int,map<int,int>>`; more needed for the others).
5. **`find()` returns a copy** — write-through must use `operator[]`/`contains()`
   on the live slot, never `find()->second = x`.
6. **Port-with-file globals** — `allGameSpells`, console commands, `surfaceCache`,
   `indicators`, `game_packets` (order-dependent or owning C++ resources).
