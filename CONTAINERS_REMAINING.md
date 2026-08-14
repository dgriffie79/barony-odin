# Remaining std:: containers in headers

Regenerated 2026-08-13 (accurate at commit a91ad93, post-D3d7). This list is
**HEADER MEMBERS ONLY** — the shared-struct + global state that must de-STL to
cross the C++↔Odin boundary — plus flagged signatures that return/pass std
containers. `std::string` members are tracked separately (the bulk-string phase,
PORTING.md step 2) and are NOT counted here. `std::array` members are listed in
their own section at the bottom (fixed-size, low priority).

## DONE since the previous doc (D3cw … D3d7)

- set<string> → `DynamicSetStr`: `mods_loaded_local/workshop`,
  `achievementUnlockedLookup`, `Event_t::attributes`
- vector<pair<string,string>> → `DynamicArrayStringPair`: `mountedFilepaths*`
  (new `Kind_StringPair`)
- vector<pair<SDL_Surface**, string>> → `DynamicArraySurfacePtrStringPair`:
  `systemResourceImagesToReload` + `systemResourceImages` (new
  `Kind_SurfacePtrStringPair`)
- compendium `contents`×7 / `contents_unfiltered` / `achievementCategories` →
  `DynamicMapStrArrayStringPair` (new `MK_ArrayStringPair`)
- compendium struct maps (each a new owning map value kind + Odin mirror):
  `achievementsBookDisplay` (`MK_CompendiumAchievementsDisplay`),
  `compendiumObjectMapTiles` (`MK_CompendiumMapTiles`),
  `compendiumObjectLimbs` (`MK_ObjectLimbs`), `worldObjects` (`MK_World`),
  `codex` (`MK_Codex`), `items`+`magic` (`MK_ItemsCodex`),
  `monsters` (`MK_Monster`)

(plus the earlier D3aw–D3cu batch, see git log.)

## Value-kind families now available

- str-key: `DynamicMapStrT<V>` (`barony_dynamic_map_str_*`)
- i32-key: `DynamicMapI32T<V>` (`barony_dynamic_map_i32_*`)
- **ptr-key:** `DynamicMapPtrT<V>` (`barony_dynamic_map_ptr_*`) — `map[rawptr]V`
- sets: `DynamicSetI32` / `DynamicSetStr`
- arrays: `DynamicArrayT<V>`, raw `DynamicArray`, `DynamicArrayStr`/`S32`/`U32`/
  `Icon`/`Option`/`EntryVar`, `DynamicArrayStringPair`,
  `DynamicArraySurfacePtrStringPair`
- string: `DynamicString`; pair mirrors `DynamicStringPair_t`, `IntPair_t`,
  `SurfacePtrStringPair_t`
- nested-map value kinds: `MK_I32Map`, `MK_U32Map`, `MK_U32MapEmitterHit`,
  `MK_StrMapStr`, `MK_StrI32Map`, `MK_I32MapModelOffset`, `MK_I32MapIntPair`,
  `MK_ArrayStringPair`
- owning struct value kinds: `MK_*` for EntityColliderData, SpellItem, ItemTooltip,
  Entry, DropDown, Statue, CompendiumAchievementsDisplay, CompendiumMapTiles,
  ObjectLimbs, World, Codex, ItemsCodex, Monster, …

## Remaining header members (by file)

### src/draw.hpp (3)
- `Mesh::ElementsPerVBO` — `static const std::unordered_map<BufferType, int>` (enum key)
- `Mesh::data` — `std::vector<float>[BufferType::Max]` (array of vectors)
- `Chunk::tiles` — `std::vector<Sint32>`

### src/entity.hpp (1 signature)
- `alertAlliesOnBeingHit(Entity*, std::unordered_set<Entity*>*)` — param

### src/game.hpp (1)
- `DebugStatsClass::networkPackets` — `unordered_map<unsigned long, pair<string,int>>`

### src/interface/interface.hpp (2)
- `scrolls` — `unordered_map<string, pair<int,bool>>`
- `sortedScrolls` — `vector<pair<string,pair<int,bool>>>`
- (`systemResourceImages` is DONE in D3cz)

### src/interface/ui.hpp (1)
- `allNotifications` — `std::list<UIToastNotification>`

### src/magic/magic.hpp (3 — port-with-file)
- `allGameSpells` — `extern map<int, spell_t*>` (order-dependent)
- `surfaceCache` — `static map<int, map<tuple<...>, SDL_Surface*>>` (tuple-keyed)
- `indicators` — `static map<Uint32, Indicator_t>` (owning)

### src/menu.hpp (2)
- `savegamesList` — `extern vector<tuple<int,int,int,string>>`
- `getResolutionList(int, std::list<resolution>&)` — out-param

### src/mod_tools.hpp (event block + timepoints + one keep)

Event block (enum-keyed maps; needs `map[EventTags]V` + set value kinds):
- `events` — `static map<EventTags, Event_t>`
- `itemEventLookup` — `static map<int, set<EventTags>>`
- `eventItemLookup` / `eventMonsterLookup` — `static map<EventTags, set<int>>`
- `eventWorldLookup` / `eventCodexLookup` — `static map<EventTags, set<string>>`
- `eventClassIds` — `static map<EventTags, map<int,int>>`
- `eventLangEntries` — `static map<EventTags, map<string,string>>`
- `eventCustomLangEntries` — `static map<string, map<string,string>>`
- `playerEvents` — `static map<EventTags, map<int, EventVal_t>>`
- `serverPlayerEvents[MAXPLAYERS]` — `static map<EventTags, map<int, EventVal_t>>`

Timepoints:
- `timepoints` — `map<string, vector<pair<string, time_point>>>` (needs a `time_point` value kind)

Keep (order-dependent):
- `achievementNamesSorted` — `set<pair<string,string>, Comparator>` **KEEP**
  (custom std::function comparator + sorted iteration in init_game.cpp; port
  with file — hash sets would drop the ordering)

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
   `keystatus` were handled in D3ce (demo v2). Keep the demo-format decision in sync.
3. **Non-zero struct defaults** — per-kind `default_value` on the entry path
   (e.g. `ChunkDither` defaults `value=10`).
4. **Nested maps** — each `map<K, map<K2,V>>` needs its own value kind + Odin
   free/copy. Not yet built: `map<EventTags, set<int>>`,
   `map<EventTags, map<int,EventVal_t>>`, `map<int, set<EventTags>>`,
   `map<Monster, vector<pair<Monster,string>>>`,
   `map<string, vector<pair<string,time_point>>>`.
5. **`find()` returns a snapshot copy** — write-through must use
   `operator[]`/`contains()` on the live slot, never `find()->second = x`.
   (Fixed several of these during D3d3–D3d7: refreshCompendiumCamera, the
   `read*TranslationsFromFile` blocks, and GameUI codexEntry address-of.)
6. **`&find->second` is a dangling pointer to the snapshot** — take the address of
   `map[key]` (live slot) instead (fixed in GameUI codexEntry, D3d5).
7. **Port-with-file globals** — `allGameSpells`, `surfaceCache`, `indicators`,
   `game_packets`, `achievementNamesSorted`, voice-chat structs (order-dependent
   or owning C++ resources).
8. **Narrowing in default member initializers** — `Uint32 x = -1;` triggers C2397
   once the type lands in a list-initialized map-value slot; use `(Uint32)-1`
   (fixed for `CompendiumMap_t::ceiling` in D3d2).
