# Remaining std:: containers in headers

Regenerated 2026-08-13 (post-D3cu, accurate at commit b8bc47f). This list is
**HEADER MEMBERS ONLY** — the shared-struct + global state that must de-STL to
cross the C++↔Odin boundary — plus flagged signatures that return/pass std
containers. `std::string` members are tracked separately (the bulk-string phase,
PORTING.md step 2) and are NOT counted here. `std::array` members are listed in
their own section at the bottom (fixed-size, low priority).

## DONE since the previous doc (D3ce … D3cu)

- input.hpp `Input::keys`, main.hpp `keystatus` → `DynamicMapI32T<bool>` (demo v2)
- player.hpp `targetsCompelled` → `DynamicMapU32Map`
- magic.hpp `particleTimerEmitterHitEntities` → `DynamicMapU32MapEmitterHit`
- player.hpp `mapDisplayNamesDescriptions` → `DynamicMapStringPair`
- mod_tools.hpp `colliderRandomGenPool` → `DynamicMapStrI32Map`
- mod_tools.hpp `colliderData` → `DynamicMapI32T<EntityColliderData_t>`
- mod_tools.hpp `spellItems` → `DynamicMapI32T<spellItem_t>`
- mod_tools.hpp `tooltips` → `DynamicMapStrT<ItemTooltip_t>`
- mod_tools.hpp `allEntries` → `DynamicMapStrT<Entry_t>`
- mod_tools.hpp `allStatues` → `DynamicMapI32T<Statue_t>`
- mod_tools.hpp `monsterModelsMap` → nested `map<int,map<int,ModelOffset_t>>`
- mod_tools.hpp `adjectives` → nested `map<string,map<string,string>>`
- player.hpp `itemEvents` → `DynamicMapStrI32Map`
- player.hpp `floorEvents` → nested str-i32-i32 map
- scores.hpp `entityAchievementsToProcess` → `DynamicMapI32IntPair`
- mod_tools.hpp `TreasureRoomGenerator` sets + floor maps
- `std::list<std::string>` returns/members → `DynamicArrayStr`:
  files.hpp `directoryContents`/`physfsGetFileNamesInDirectory`,
  menu.hpp `currentDirectoryFiles`, mod_tools.hpp `localModFoldernames`,
  steam.hpp `workshopItemTags`

(plus the earlier D3aw–D3cc batch, see git log.)

## Value-kind families now available

- str-key: `DynamicMapStrT<V>` (`barony_dynamic_map_str_*`)
- i32-key: `DynamicMapI32T<V>` (`barony_dynamic_map_i32_*`)
- **ptr-key:** `DynamicMapPtrT<V>` (`barony_dynamic_map_ptr_*`) — `map[rawptr]V`
- sets: `DynamicSetI32` / `DynamicSetStr`
- arrays: `DynamicArrayT<V>` (`barony_dynamic_array_elem_*`), raw `DynamicArray`,
  `DynamicArrayStr` / `DynamicArrayS32` / `DynamicArrayU32` / `DynamicArrayIcon` /
  `DynamicArrayOption` / `DynamicArrayEntryVar` + `dynarray_pair_*` helpers
- string: `DynamicString`; owning pair mirrors `DynamicStringPair_t`, `IntPair_t`
- nested-map value kinds: `MK_I32Map`, `MK_U32Map`, `MK_U32MapEmitterHit`,
  `MK_StrMapStr`, `MK_StrI32Map`, `MK_I32MapModelOffset`, `MK_I32MapIntPair`
  (typedefs `DynamicMapI32Map`, `DynamicMapU32Map`, `DynamicMapU32MapEmitterHit`,
  `DynamicMapStrI32Map`, `DynamicMapI32IntPair`, …)

## Remaining header members (by file)

### src/draw.hpp (3)
- `Mesh::ElementsPerVBO` — `static const std::unordered_map<BufferType, int>` (enum key)
- `Mesh::data` — `std::vector<float>[BufferType::Max]` (array of vectors)
- `Chunk::tiles` — `std::vector<Sint32>`

### src/entity.hpp (1 signature)
- `alertAlliesOnBeingHit(Entity*, std::unordered_set<Entity*>*)` — param

### src/game.hpp (1)
- `DebugStatsClass::networkPackets` — `unordered_map<unsigned long, pair<string,int>>`

### src/interface/interface.hpp (3)
- `scrolls` — `unordered_map<string, pair<int,bool>>`
- `sortedScrolls` — `vector<pair<string,pair<int,bool>>>`
- `systemResourceImages` — `extern vector<pair<SDL_Surface**, string>>`

### src/interface/ui.hpp (1)
- `allNotifications` — `std::list<UIToastNotification>`

### src/magic/magic.hpp (3 — port-with-file)
- `allGameSpells` — `extern map<int, spell_t*>` (order-dependent)
- `surfaceCache` — `static map<int, map<tuple<...>, SDL_Surface*>>` (tuple-keyed)
- `indicators` — `static map<Uint32, Indicator_t>` (owning)

### src/menu.hpp (2)
- `savegamesList` — `extern vector<tuple<int,int,int,string>>`
- `getResolutionList(int, std::list<resolution>&)` — out-param

### src/mod_tools.hpp (the remaining big block)

Mods statics:
- `systemResourceImagesToReload` — `static vector<pair<SDL_Surface**, string>>`
- `mountedFilepaths` / `mountedFilepathsSaved` — `static vector<pair<string,string>>`
- `mods_loaded_local` / `mods_loaded_workshop` — `static set<string>`

Timepoints:
- `timepoints` — `map<string, vector<pair<string, time_point>>>` (needs a `time_point` value kind)

Compendium (across all six sections — achievements/monsters/world/codex/items/magic):
- `achievementNamesSorted` — `set<pair<string,string>, Comparator>`
- `achievementCategories` — `map<string, vector<pair<string,string>>>`
- `achievementUnlockedLookup` — `unordered_set<string>`
- `achievementsBookDisplay` — `map<string, CompendiumAchievementsDisplay>`
- `contents` / `contents_unfiltered` — `static map<string, vector<pair<string,string>>>` (×6 sections)
- `monsters` / `worldObjects` / `codex` / `items` / `magic` — `map<string, X_t>`
- `compendiumObjectLimbs` — `map<string, ObjectLimbs_t>`
- `compendiumObjectMapTiles` — `map<string, pair<CompendiumMap_t, vector<int>>>`
- `attributes` — `set<string>` (inside monster variant tooltips)

Event block:
- `events` — `static map<EventTags, Event_t>`
- `itemEventLookup` — `static map<int, set<EventTags>>`
- `eventItemLookup` / `eventMonsterLookup` — `static map<EventTags, set<int>>`
- `eventWorldLookup` / `eventCodexLookup` — `static map<EventTags, set<string>>`
- `eventClassIds` — `static map<EventTags, map<int,int>>`
- `eventLangEntries` — `static map<EventTags, map<string,string>>`
- `eventCustomLangEntries` — `static map<string, map<string,string>>`
- `playerEvents` — `static map<EventTags, map<int, EventVal_t>>`
- `serverPlayerEvents[MAXPLAYERS]` — `static map<EventTags, map<int, EventVal_t>>`

### src/net.hpp (1 — port-with-file)
- `game_packets` — `queue<SteamPacketWrapper*>`

### src/player.hpp (6)
- `allDropDowns` — `static map<string, DropDown_t>` (has a `MK_DropDown` value kind already)
- `leadershipAllyTableSpecialRecruitment` — `map<Monster, vector<pair<Monster,string>>>`
- `notification_messages` — `list<Message*>`
- `sharedDialogues` — `map<Uint32, Dialogue_t>`
- `settings` — `static map<DialogueType_t, Setting_t>`
- `mapDetails` — `static vector<pair<string,string>>`

### src/scores.hpp (2 — KEEP)
- `map_messages` / `additional_data` — `vector<pair<string,string>>` **KEEP**
  (JSON serialization — reverted in D3aj; port with file)

### src/engine/audio/sound.hpp (3 — previously untracked)
Voice-chat subsystem (port-with-file candidate; listed so it isn't missed):
- `VoiceChat_t::recordingDatagrams` — `vector<vector<char>>`
- `VoiceChat_t::PlayerChannels_t::audioQueue` — `vector<char>`
- `VoiceChat_t::PlayerChannels_t::voiceDatagrams` — `priority_queue<pair<int, vector<char>>>`

### src/ui/* (~20 — port last)
- `Field::linesToColor` (`map<int,Uint32>`), `Field::individualLinePadding`
  (`map<int,int>`), `Field::cache` (`vector<pair<string,Text*>>`)
- `Widget::widgets` — `list<Widget*>`; `findSelectedWidgets(vector<Widget*>&)`
- `MainMenu::survivalComplexity` (`vector<tuple<int,string,Uint32>>`),
  `MainMenu::data` (`unordered_map<int, DescData_t>` + `unordered_map<string, DescData_t>`)
- `draw(... const vector<const Widget*>& selectedWidgets/searchParents)` params
  across Widget/Frame/Field/Button/Slider

## std::array members (fixed-size — trivial `[N]T` port, do last)

- player.hpp: `game_controllers`, `dollSlots`, `hotbar`, `hotbar_alternate`,
  `hotbarSlotFrames`, `faceButtonPositions`
- mod_tools.hpp: `Monster_t::resistances` (`std::array<int,7>`)
- steam.hpp: `m_subscribedItemPreviewURL` (`std::array<std::string,50>`)

## Hazards to keep in mind

1. **Pointer-keyed maps** need `DynamicMapPtrT<V>` / `map[rawptr]V` (done in D3c9).
2. **bool-valued maps + `sizeof()` demo serialization** — `Input::keys` and
   `keystatus` are written raw to demo files; the demo v2 format was handled in
   D3ce. Keep the demo-format decision in sync with any future conversion.
3. **Non-zero struct defaults** — per-kind `default_value` on the entry path
   (e.g. `ChunkDither` defaults `value=10`).
4. **Nested maps** — each `map<K, map<K2,V>>` needs its own value kind + Odin
   free/copy ops. Not yet built: `map<string, vector<pair<string,string>>>`,
   `map<EventTags, set<int>>`, `map<EventTags, map<int,EventVal_t>>`,
   `map<Monster, vector<pair<Monster,string>>>`, `map<string, vector<pair<string,time_point>>>`.
5. **`find()` returns a copy** — write-through must use `operator[]`/`contains()`
   on the live slot, never `find()->second = x`.
6. **Port-with-file globals** — `allGameSpells`, `surfaceCache`, `indicators`,
   `game_packets`, voice-chat structs (order-dependent or owning C++ resources).
