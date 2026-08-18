// shops.odin -- Odin mirror of shops.hpp.
package main

NUMCHITCHAT                         :: 20
hamletTradingSkillLimit             :: 7

SHOP_TYPE_ARMS_ARMOR                :: 0
SHOP_TYPE_HAT                       :: 1
SHOP_TYPE_JEWELRY                   :: 2
SHOP_TYPE_BOOKS                     :: 3
SHOP_TYPE_POTIONS                   :: 4
SHOP_TYPE_STAFFS                    :: 5
SHOP_TYPE_FOOD                      :: 6
SHOP_TYPE_HARDWARE                  :: 7
SHOP_TYPE_HUNTING                   :: 8
SHOP_TYPE_GENERAL                   :: 9
SHOP_CONSUMABLE_SKILL_REQ_PER_POINT :: 10

// ---------------------------------------------------------------------------
// Globals owned by Odin  (ownership flip from C++ - see PORTING.md -3)
// C++ declares these extern "C" and references them; Odin owns storage.
// ---------------------------------------------------------------------------
@(export)
shopInv : [MAXPLAYERS]^list_t

@(export)
shopkeeper : [MAXPLAYERS]u32

@(export)
shoptimer : [MAXPLAYERS]u32

@(export)
shopspeech : [MAXPLAYERS]string

@(export)
shopkeepertype : [MAXPLAYERS]i32

@(export)
shopkeepername : [MAXPLAYERS]string

@(export)
shopkeepername_client : [MAXPLAYERS][64]u8

@(export)
hamletShopkeeperSkillLimit : [MAXPLAYERS]map[[4]byte]i32  // DynamicMapI32T<int>

@(export)
shopkeeperMysteriousItems : map[[4]byte]map[i32]struct{}  // DynamicMapI32T<DynamicSetI32>
