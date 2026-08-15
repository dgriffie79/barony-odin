#include <cstdio>
#include <cstdint>
#include <cstddef>
typedef struct vec4 { float x,y,z,w; } vec4_t;
typedef struct node_t { struct node_t* next; struct node_t* prev; struct list_t* list; void* element; void (*deconstructor)(void*); unsigned size; } node_t;
typedef struct light_t { int x,y; int radius; vec4_t* tiles; int index; node_t* node; } light_t;
struct LightDef { int radius; float r,g,b,a; float falloff_exp; bool shadows; };
struct KV { const char* first; LightDef second; };
struct Iterator { KV kv; bool valid; };
int main(){
    printf("light_t=%zu LightDef=%zu KV=%zu Iterator=%zu\n", sizeof(light_t), sizeof(LightDef), sizeof(KV), sizeof(Iterator));
    return 0;
}
