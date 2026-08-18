// items.odin - Odin mirrors of items.hpp.
package main


// enums (items.hpp)

// typedef enum Status { BROKEN, DECREPIT, WORN, SERVICABLE, EXCELLENT } Status;
Status :: enum i32 {
	BROKEN,
	DECREPIT,
	WORN,
	SERVICABLE,
	EXCELLENT,
}

// typedef enum Category - big enum; use i32 for layout (full mirror later)
Category :: i32

// enum ItemEquippableSlot : int
Item_Equippable_Slot :: enum i32 {
	EQUIPPABLE_IN_SLOT_WEAPON,
	EQUIPPABLE_IN_SLOT_SHIELD,
	EQUIPPABLE_IN_SLOT_MASK,
	EQUIPPABLE_IN_SLOT_HELM,
	EQUIPPABLE_IN_SLOT_GLOVES,
	EQUIPPABLE_IN_SLOT_BOOTS,
	EQUIPPABLE_IN_SLOT_BREASTPLATE,
	EQUIPPABLE_IN_SLOT_CLOAK,
	EQUIPPABLE_IN_SLOT_AMULET,
	EQUIPPABLE_IN_SLOT_RING,
	NO_EQUIP,
}

// enum ItemStackResults : int
Item_Stack_Results :: enum i32 {
	ITEM_STACKING_ERROR,
	ITEM_DESTINATION_NOT_SAME_ITEM,
	ITEM_DESTINATION_STACK_IS_FULL,
	ITEM_ADDED_ENTIRELY_TO_DESTINATION_STACK,
	ITEM_ADDED_PARTIALLY_TO_DESTINATION_STACK,
	ITEM_ADDED_WITHOUT_NEEDING_STACK,
}

// ItemType is defined in items.hpp (big enum); Stat references it as i32.
ItemType :: i32

// class Item - 56 bytes
Item :: struct {
	type:              ItemType,
	status:            Status,
	beatitude:         i16, // Sint16
	count:             i16, // Sint16
	appearance:        u32,
	identified:        bool,
	uid:               u32,
	x:                 i32,
	y:                 i32,
	owner_uid:         u32, // ownerUid
	interact_npc_uid:  u32, // interactNPCUid
	forced_pickup_by_player: bool,
	is_droppable:           bool,
	player_sold_item_to_shop: bool,
	item_hidden_from_shop: bool,
	notify_icon:           bool,
	spell_notify_icon:     bool,
	item_require_trading_skill_in_shop: u8,
	item_special_shop_consumable: bool,
	node:              ^node_t,
}

#assert(size_of(Item) == 56)

// class ItemGeneric - 152 bytes
ItemGeneric :: struct {
	item_name_identified:   string, // 16B
	item_name_unidentified: string,
	index:                  i32,
	index_short:            i32, // indexShort
	fpindex:                i32,
	variations:             i32,
	weight:                 i32,
	gold_value:             i32,
	images:                 list_t,
	surfaces:               list_t,
	category:               Category,
	level:                  i32,
	item_slot:              Item_Equippable_Slot,
	attributes:             map[string]i32, // DynamicMapI32 = DynamicMapStrT<int32_t> (string-keyed)
	tooltip:                string,
}

#assert(size_of(ItemGeneric) == 152)

// struct ItemStackResult - 16 bytes
// { ItemStackResults resultType; Item* itemToStackInto; }
ItemStackResult :: struct {
	result_type:       Item_Stack_Results,
	item_to_stack_into: ^Item,
}

#assert(size_of(ItemStackResult) == 16)
