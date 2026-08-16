#!/usr/bin/env python3
"""Apply all manual corrections to generated flatten forwarders.

Run AFTER flatten_methods.py --all --apply. Idempotent. Fixes the cases
libclang cannot resolve: nested typedefs/aliases, template misparses, array
params, function-pointer params, deleted-special-member forwarders, and
cross-class flat-name collisions.
"""
import os, re

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

def sub(path, old, new):
    p = os.path.join(ROOT, path)
    t = open(p, encoding='utf-8', newline='').read()
    if old in t:
        t = t.replace(old, new)
        open(p, 'w', encoding='utf-8', newline='').write(t)
        return True
    return False

def strip(path, fwd):
    p = os.path.join(ROOT, path)
    t = open(p, encoding='utf-8', newline='').read()
    if fwd in t:
        t = t.replace(fwd + '\n\n', '').replace(fwd + '\n', '')
        open(p, 'w', encoding='utf-8', newline='').write(t)
        return True
    return False

def run():
    n = 0
    # ---- draw.cpp: framebuffer::lock returns GLhalf* ----
    n += sub('src/draw.cpp',
        'extern "C" int * framebuffer_lock(framebuffer* self) { return self->lock(); }',
        'extern "C" GLhalf * framebuffer_lock(framebuffer* self) { return self->lock(); }')

    # ---- entity.cpp: array params + TileEntityListHandler nested returns ----
    n += sub('src/entity.cpp',
        'extern "C" void Entity_playerStatIncrease(Entity* self, int playerClass, int[3] chosenStats) { return self->playerStatIncrease(playerClass, chosenStats); }',
        'extern "C" void Entity_playerStatIncrease(Entity* self, int playerClass, int chosenStats[3]) { return self->playerStatIncrease(playerClass, chosenStats); }')
    n += sub('src/entity.cpp',
        'extern "C" void Entity_monsterRollLevelUpStats(int[3] increasestat) { return Entity::monsterRollLevelUpStats(increasestat); }',
        'extern "C" void Entity_monsterRollLevelUpStats(int increasestat[3]) { return Entity::monsterRollLevelUpStats(increasestat); }')
    n += sub('src/entity.cpp',
        'extern "C" int * TileEntityListHandler_addEntity(TileEntityListHandler* self, Entity & entity) { return self->addEntity(entity); }',
        'extern "C" node_t * TileEntityListHandler_addEntity(TileEntityListHandler* self, Entity & entity) { return self->addEntity(entity); }')
    n += sub('src/entity.cpp',
        'extern "C" int * TileEntityListHandler_updateEntity(TileEntityListHandler* self, Entity & entity) { return self->updateEntity(entity); }',
        'extern "C" node_t * TileEntityListHandler_updateEntity(TileEntityListHandler* self, Entity & entity) { return self->updateEntity(entity); }')
    n += sub('src/entity.cpp',
        'extern "C" int * TileEntityListHandler_getTileList(TileEntityListHandler* self, int x, int y) { return self->getTileList(x, y); }',
        'extern "C" list_t * TileEntityListHandler_getTileList(TileEntityListHandler* self, int x, int y) { return self->getTileList(x, y); }')
    n += sub('src/entity.cpp',
        'extern "C" int TileEntityListHandler_getEntitiesWithinRadius(TileEntityListHandler* self, int u, int v, int radius) { return self->getEntitiesWithinRadius(u, v, radius); }',
        'extern "C" DynamicArrayT<list_t *> TileEntityListHandler_getEntitiesWithinRadius(TileEntityListHandler* self, int u, int v, int radius) { return self->getEntitiesWithinRadius(u, v, radius); }')
    n += sub('src/entity.cpp',
        'extern "C" int TileEntityListHandler_getEntitiesWithinRadiusAroundEntity(TileEntityListHandler* self, Entity * entity, int radius) { return self->getEntitiesWithinRadiusAroundEntity(entity, radius); }',
        'extern "C" DynamicArrayT<list_t *> TileEntityListHandler_getEntitiesWithinRadiusAroundEntity(TileEntityListHandler* self, Entity * entity, int radius) { return self->getEntitiesWithinRadiusAroundEntity(entity, radius); }')

    # ---- game.cpp: Clock::now + TimerExperiments::integrate aliases ----
    n += sub('src/game.cpp',
        'extern "C" int Clock_now() { return TimerExperiments::Clock::now(); }',
        'extern "C" TimerExperiments::Clock::time_point Clock_now() { return TimerExperiments::Clock::now(); }')
    n += sub('src/game.cpp',
        'extern "C" void TimerExperiments_integrate(TimerExperiments::State & state, int a1, int dt) { return TimerExperiments::integrate(state, a1, dt); }',
        'extern "C" void TimerExperiments_integrate(TimerExperiments::State & state, TimerExperiments::time_point a1, TimerExperiments::duration dt) { return TimerExperiments::integrate(state, a1, dt); }')

    # ---- opengl.cpp: Chunk nested map_t/vector params ----
    n += sub('src/opengl.cpp',
        'extern "C" void Chunk_build(Chunk* self, const int & map, bool ceiling, int startX, int startY, int w, int h) { return self->build(map, ceiling, startX, startY, w, h); }',
        'extern "C" void Chunk_build(Chunk* self, const map_t & map, bool ceiling, int startX, int startY, int w, int h) { return self->build(map, ceiling, startX, startY, w, h); }')
    n += sub('src/opengl.cpp',
        'extern "C" void Chunk_buildBuffers(Chunk* self, const int & positions, const int & texcoords, const int & colors) { return self->buildBuffers(positions, texcoords, colors); }',
        'extern "C" void Chunk_buildBuffers(Chunk* self, const std::vector<float> & positions, const std::vector<float> & texcoords, const std::vector<float> & colors) { return self->buildBuffers(positions, texcoords, colors); }')
    n += sub('src/opengl.cpp',
        'extern "C" bool Chunk_isDirty(Chunk* self, const int & map) { return self->isDirty(map); }',
        'extern "C" bool Chunk_isDirty(Chunk* self, const map_t & map) { return self->isDirty(map); }')

    # ---- net.cpp: SteamPacketWrapper::data returns Uint8*& ----
    n += sub('src/net.cpp',
        'extern "C" int *& SteamPacketWrapper_data(SteamPacketWrapper* self) { return self->data(); }',
        'extern "C" Uint8 *& SteamPacketWrapper_data(SteamPacketWrapper* self) { return self->data(); }')

    # ---- lobbies.cpp: button_t* params ----
    n += sub('src/lobbies.cpp',
        'extern "C" void LobbyHandler_t_filterLobbyButton(int * my) { return LobbyHandler_t::filterLobbyButton(my); }',
        'extern "C" void LobbyHandler_t_filterLobbyButton(button_t * my) { return LobbyHandler_t::filterLobbyButton(my); }')
    n += sub('src/lobbies.cpp',
        'extern "C" void LobbyHandler_t_searchLobbyWithFilter(int * my) { return LobbyHandler_t::searchLobbyWithFilter(my); }',
        'extern "C" void LobbyHandler_t_searchLobbyWithFilter(button_t * my) { return LobbyHandler_t::searchLobbyWithFilter(my); }')

    # ---- input.cpp: binding_t nested typedef ----
    n += sub('src/input.cpp',
        'extern "C" binding_t Input_input(const Input* self, const char * binding) { return self->input(binding); }',
        'extern "C" Input::binding_t Input_input(const Input* self, const char * binding) { return self->input(binding); }')
    n += sub('src/input.cpp',
        'extern "C" std::string Input_getGlyphPathForBinding(const binding_t & binding, bool pressed) { return Input::getGlyphPathForBinding(binding, pressed); }',
        'extern "C" std::string Input_getGlyphPathForBinding(const Input::binding_t & binding, bool pressed) { return Input::getGlyphPathForBinding(binding, pressed); }')
    n += sub('src/input.cpp',
        'extern "C" bool Input_binaryOf(binding_t & binding) { return Input::binaryOf(binding); }',
        'extern "C" bool Input_binaryOf(Input::binding_t & binding) { return Input::binaryOf(binding); }')
    n += sub('src/input.cpp',
        'extern "C" float Input_analogOf(binding_t & binding) { return Input::analogOf(binding); }',
        'extern "C" float Input_analogOf(Input::binding_t & binding) { return Input::analogOf(binding); }')

    # ---- interface.cpp: CalloutRadialMenu nested IconEntryText_t + stray brace ----
    n += sub('src/interface/interface.cpp',
        'extern "C" std::string CalloutRadialMenu_getCalloutMessage(CalloutRadialMenu* self, const IconEntryText_t & text_map, const char * object, const int targetPlayer) { return self->getCalloutMessage(text_map, object, targetPlayer); }',
        'extern "C" std::string CalloutRadialMenu_getCalloutMessage(CalloutRadialMenu* self, const CalloutRadialMenu::IconEntryText_t & text_map, const char * object, const int targetPlayer) { return self->getCalloutMessage(text_map, object, targetPlayer); }')

    # ---- Button.cpp / Field.cpp / Slider.cpp: setCallback fnptr ----
    n += sub('src/ui/Button.cpp',
        'extern "C" void Button_setCallback(Button* self, void (*const)(Button &) fn) { return self->setCallback(fn); }',
        'extern "C" void Button_setCallback(Button* self, void (*fn)(Button &)) { return self->setCallback(fn); }')
    n += sub('src/ui/Field.cpp',
        'extern "C" void Field_setCallback(Field* self, void (*const)(Field &) fn) { return self->setCallback(fn); }',
        'extern "C" void Field_setCallback(Field* self, void (*fn)(Field &)) { return self->setCallback(fn); }')
    n += sub('src/ui/Slider.cpp',
        'extern "C" void Slider_setCallback(Slider* self, void (*const)(Slider &) fn) { return self->setCallback(fn); }',
        'extern "C" void Slider_setCallback(Slider* self, void (*fn)(Slider &)) { return self->setCallback(fn); }')
    n += sub('src/ui/Widget.cpp',
        'extern "C" void Widget_setTickCallback(Widget* self, void (*const)(Widget &) fn) { return self->setTickCallback(fn); }',
        'extern "C" void Widget_setTickCallback(Widget* self, void (*fn)(Widget &)) { return self->setTickCallback(fn); }')
    n += sub('src/ui/Widget.cpp',
        'extern "C" void Widget_setDrawCallback(Widget* self, void (*const)(const Widget &, const SDL_Rect) fn) { return self->setDrawCallback(fn); }',
        'extern "C" void Widget_setDrawCallback(Widget* self, void (*fn)(const Widget &, const SDL_Rect)) { return self->setDrawCallback(fn); }')

    # ---- Field.cpp: draw/drawPost DynamicArrayT params ----
    n += sub('src/ui/Field.cpp',
        'extern "C" void Field_draw(const Field* self, SDL_Rect _size, SDL_Rect _actualSize, const int & selectedWidgets) { return self->draw(_size, _actualSize, selectedWidgets); }',
        'extern "C" void Field_draw(const Field* self, SDL_Rect _size, SDL_Rect _actualSize, const DynamicArrayT<Widget *> & selectedWidgets) { return self->draw(_size, _actualSize, selectedWidgets); }')
    n += sub('src/ui/Field.cpp',
        'extern "C" void Field_drawPost(const Field* self, SDL_Rect _size, SDL_Rect _actualSize, const int & selectedWidgets, const int & searchParents) { return self->drawPost(_size, _actualSize, selectedWidgets, searchParents); }',
        'extern "C" void Field_drawPost(const Field* self, SDL_Rect _size, SDL_Rect _actualSize, const DynamicArrayT<Widget *> & selectedWidgets, const DynamicArrayT<Widget *> & searchParents) { return self->drawPost(_size, _actualSize, selectedWidgets, searchParents); }')

    # ---- json.cpp: operator= remove, beginArray Uint32, template values remove ----
    n += strip('src/json.cpp',
        'extern "C" FileInterface & FileInterface_assign(FileInterface* self, FileInterface && other) { return self->operator=(other); }')
    n += sub('src/json.cpp',
        'extern "C" bool FileInterface_beginArray(FileInterface* self, int & size) { return self->beginArray(size); }',
        'extern "C" bool FileInterface_beginArray(FileInterface* self, Uint32 & size) { return self->beginArray(size); }')
    for i in ['_8', '_9', '_11']:
        n += strip('src/json.cpp',
            f'extern "C" bool FileInterface_value{i}(FileInterface* self, int & v, int maxLength) {{ return self->value(v, maxLength); }}')
    n += strip('src/json.cpp',
        'extern "C" bool FileHelper_writeObjectInternal(const char * filename, EFileFormat format, const SerializationFunc & serialize) { return FileHelper::writeObjectInternal(filename, format, serialize); }')
    n += strip('src/json.cpp',
        'extern "C" bool FileHelper_readObjectInternal(const char * filename, const SerializationFunc & serialize) { return FileHelper::readObjectInternal(filename, serialize); }')

    # ---- scores.cpp: dup updateGlobalStat + cross-class serialize collision ----
    p = os.path.join(ROOT, 'src/scores.cpp')
    s = open(p, encoding='utf-8', newline='').read()
    first = s.find('AchievementObserver_updateGlobalStat')
    second = s.find('AchievementObserver_updateGlobalStat', first+10)
    if second != -1:
        st = s.rfind('\n', 0, second)
        en = s.find('\n', second) + 1
        if s[en:en+1] == '\n':
            en += 1
        s = s[:st+1] + s[en:]
        n += 1
    old = 'extern "C" bool PlayerRaceHostility_t_serialize(ShopkeeperPlayerHostility_t::PlayerRaceHostility_t* self, FileInterface * fp) { return self->serialize(fp); }'
    if old in s:
        s = s.replace(old, 'extern "C" bool ShopkeeperPlayerHostility_t_PlayerRaceHostility_t_serialize(ShopkeeperPlayerHostility_t::PlayerRaceHostility_t* self, FileInterface * fp) { return self->serialize(fp); }')
        n += 1
    open(p, 'w', encoding='utf-8', newline='').write(s)

    print(f'applied {n} fixes')

def relabel_headers():
    """Insert `public:` before private members/classes that forwarders reach.
    Layout-safe: only relabels access, never reorders/drops members."""
    import re
    n = 0
    jobs = {
        # path -> list of declaration fragments to make public
        'src/input.hpp': ['static SDL_Keycode getKeycodeFromName', 'static bool binaryOf', 'static float analogOf'],
        'src/player.hpp': ['class VirtualMouse', 'class UIStatus'],
        'src/json.hpp': ['writeStringInternalBinary', 'readStringInternalBinary'],
        'src/mod_tools.hpp': ['struct spellItem_t'],
        'src/interface/ui.hpp': ['matchesAchievementName'],
        'src/ui/Field.hpp': ['buildCache'],
        'src/ui/Frame.hpp': ['process', 'processField', 'processButton', 'processSlider',
                            'getRelativeMousePositionImpl', 'capturesMouseImpl', 'activateEntry'],
        'src/ui/Image.hpp': ['setupGL', 'finalize'],
        'src/ui/Text.hpp': ['countNumTextLines'],
        'src/ui/Widget.hpp': ['deselectBase', 'removeBase', 'drawPost'],
    }
    for path, frags in jobs.items():
        p = os.path.join(ROOT, path)
        h = open(p, encoding='utf-8', newline='').read()
        for frag in frags:
            # match the declaration line containing frag (name followed by '(' or 'class ')
            if frag.startswith('class ') or frag.startswith('struct '):
                name = frag.split()[1]
                pat = re.compile(r'(\n[ \t]*)((?:class|struct) ' + re.escape(name) + r'\b)')
            else:
                name = frag.split()[-1].split('(')[0]
                # Single-line declaration match: type + name + '(' on ONE line.
                # No \s* crossing newlines (that misfired inside function bodies).
                pat = re.compile(r'(\n[ \t]*)([A-Za-z_][A-Za-z0-9_:<>*&, ]*?[ \t]+' + re.escape(name) + r'\s*\()')
            m = pat.search(h)
            if not m:
                continue
            pre = h[:m.start()]
            last_priv = pre.rfind('private:')
            last_pub = pre.rfind('public:')
            last_prot = pre.rfind('protected:')
            # relabel only if a private: OR protected: governs this declaration,
            # or if the member is in a `class` with no governing access spec
            # (class default is private) - e.g. Inputs::VirtualMouse.
            if (last_priv > last_pub and last_priv > last_prot) or \
               (last_prot > last_pub and last_prot > last_priv):
                indent = m.group(1)
                h = h[:m.start()] + f"\n{indent}public:" + h[m.start():]
                n += 1
            elif last_priv < 0 and last_prot < 0:
                # no explicit access spec at all before it: check the member is
                # in a class (not struct) with default-private. Find enclosing
                # class and its governing spec.
                classes = list(re.finditer(r'\n[ \t]*(?:class|struct) (\w+)', pre))
                if classes:
                    enc = classes[-1]
                    # find the last access spec AFTER the enclosing class decl
                    after = pre[enc.start():]
                    priv2 = after.rfind('private:')
                    pub2 = after.rfind('public:')
                    prot2 = after.rfind('protected:')
                    # if the enclosing class is a `class` (default private) and
                    # no public/protected governs since it opened
                    is_class = 'class ' in enc.group(0)
                    if is_class and not (pub2 > priv2 or prot2 > priv2):
                        indent = m.group(1)
                        h = h[:m.start()] + f"\n{indent}public:" + h[m.start():]
                        n += 1
        open(p, 'w', encoding='utf-8', newline='').write(h)
    print(f'relabeled {n} private members/classes')

if __name__ == '__main__':
    run()
    relabel_headers()
