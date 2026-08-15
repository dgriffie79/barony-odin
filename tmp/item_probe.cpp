#include <cstdio>
#include <cstdint>
#include <cstddef>
struct node_t { void* next; void* prev; void* list; void* element; void(*f)(void*); unsigned size; };
struct Item {
    int type; int status; short beatitude; short count; unsigned appearance; bool identified;
    unsigned uid; int x; int y; unsigned ownerUid; unsigned interactNPCUid;
    bool forcedPickupByPlayer; bool isDroppable; bool playerSoldItemToShop; bool itemHiddenFromShop;
    bool notifyIcon; bool spellNotifyIcon; unsigned char itemRequireTradingSkillInShop; bool itemSpecialShopConsumable;
    node_t* node;
};
int main(){ printf("Item=%zu\n", sizeof(Item)); return 0; }
