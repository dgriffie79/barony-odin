/*-------------------------------------------------------------------------------

	BARONY
	File: draw.hpp
	Desc: prototypes for draw.cpp, various drawing functions

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#pragma once

#include "shader.hpp"
#include "interface/consolecommand.hpp"

// Vector4 is defined in consolecommand.hpp (the console registry needs it by
// value for Vector4 cvars); draw.hpp uses it for hdrDraw below.

extern "C" vec4_t vec4_copy(const vec4_t* v);
extern "C" vec4_t* mul_mat_vec4(vec4_t* result, const mat4x4_t* m, const vec4_t* v);
extern "C" vec4_t* add_vec4(vec4_t* result, const vec4_t* a, const vec4_t* b);
extern "C" vec4_t* sub_vec4(vec4_t* result, const vec4_t* a, const vec4_t* b);
extern "C" vec4_t* mul_vec4(vec4_t* result, const vec4_t* a, const vec4_t* b);
extern "C" vec4_t* div_vec4(vec4_t* result, const vec4_t* a, const vec4_t* b);
extern "C" vec4_t* pow_vec4(vec4_t* result, const vec4_t* v, float f);
extern "C" float dot_vec4(const vec4_t* a, const vec4_t* b);
extern "C" vec4_t* cross_vec3(vec4_t* result, const vec4_t* a, const vec4_t* b);
extern "C" vec4_t* cross_vec4(vec4_t* result, const vec4_t* a, const vec4_t* b);
extern "C" float length_vec4(const vec4_t* v);
extern "C" vec4_t* normal_vec4(vec4_t* result, const vec4_t* v);
extern "C" mat4x4_t* mul_mat(mat4x4_t* result, const mat4x4_t* m1, const mat4x4_t* m2);
extern "C" mat4x4_t* translate_mat(mat4x4_t* result, const mat4x4_t* m, const vec4_t* v);
extern "C" mat4x4_t* rotate_mat(mat4x4_t* result, const mat4x4_t* m, float angle, const vec4_t* v);
extern "C" mat4x4_t* scale_mat(mat4x4_t* result, const mat4x4_t* m, const vec4_t* v);
extern "C" mat4x4_t* ortho(mat4x4_t* result, float left, float right, float bot, float top, float near, float far);
extern "C" mat4x4_t* frustum(mat4x4_t* result, float left, float right, float bot, float top, float near, float far);
#define perspective fast_perspective
extern "C" mat4x4_t* slow_perspective(mat4x4_t* result, float fov, float aspect, float near, float far);
extern "C" mat4x4_t* fast_perspective(mat4x4_t* result, float fov, float aspect, float near, float far);
extern "C" mat4x4_t* mat_from_array(mat4x4_t* result, float matArray[16]);
extern "C" bool invertMatrix4x4(mat4x4_t* result, const mat4x4_t* m);
extern "C" vec4_t project(
    const vec4_t* world,
    const mat4x4_t* model,
    const mat4x4_t* projview,
    const vec4_t* window);
struct ClipResult {
    enum class Direction {
        Invalid,
        Left,
        Right,
        Top,
        Bottom,
        Front,
        Behind,
    };
    Direction direction = Direction::Invalid;
    bool isBehind = false;
    vec4_t clipped_coords;
};
extern "C" ClipResult project_clipped(
    const vec4_t* world,
    const mat4x4_t* model,
    const mat4x4_t* projview,
    const vec4_t* window);
extern "C" ClipResult project_clipped2( // project_clipped, but will draw mirroed behind camera
    const vec4_t* world,
    const mat4x4_t* model,
    const mat4x4_t* projview,
    const vec4_t* window);
extern "C" vec4_t unproject(
    const vec4_t* screenCoords,
    const mat4x4_t* model,
    const mat4x4_t* projview,
    const vec4_t* window);

// The following f16/f32 conversion functions courtesy of glm
// https://github.com/g-truc/glm

union uif32 {
	uif32() :
		i(0)
	{}
	uif32(float f_) :
		f(f_)
	{}
	uif32(unsigned int i_) :
		i(i_)
	{}

	float f;
	unsigned int i;
};

extern "C" float foverflow();
extern "C" float toFloat32(GLhalf value);
extern "C" GLhalf toFloat16(float f);

class TempTexture {
public:
    // Real data members (were private _texid/_w/_h with public reference
    // aliases texid/w/h). References are not portable (Odin has none), and
    // nothing wrote through them, so the aliases are folded into the members.
    GLuint texid = 0;
    int w = 0;
    int h = 0;

    TempTexture() {
        GL_CHECK_ERR(glGenTextures(1, &texid));
    }

    ~TempTexture() {
        if( texid ) {
            GL_CHECK_ERR(glDeleteTextures(1,&texid));
            texid = 0;
        }
    }
    
    void setParameters(bool clamp, bool point);

    void load(SDL_Surface* surf, bool clamp, bool point);
    
    void loadFloat(float* data, int width, int height, bool clamp, bool point);

    void bind();
};

#include <initializer_list>
#include <cassert>

struct Mesh {
    enum class BufferType : unsigned int {
        Position, // vec3 float
        TexCoord, // vec2 float
        Color,    // vec4 float
        Max
    };
    static const int ElementsPerVBO[(int)BufferType::Max];

    Mesh() = default;
    Mesh(
        std::initializer_list<float>&& positions,
        std::initializer_list<float>&& texcoords,
        std::initializer_list<float>&& colors) :
        data{{positions}, {texcoords}, {colors}}
        {}

    Mesh(
        const std::initializer_list<float>& positions,
        const std::initializer_list<float>& texcoords,
        const std::initializer_list<float>& colors) :
        data{{positions}, {texcoords}, {colors}}
        {}

    DynamicArrayT<float> data[(int)BufferType::Max];

    void init();
    void destroy();
    void draw(GLenum type = GL_TRIANGLES, int numVertices = 0) const;
    bool isInitialized() const;

private:
    unsigned int vao = 0; // vertex array object (mesh handle)
    unsigned int vbo[(int)BufferType::Max]; // vertex buffer objects
    unsigned int numVertices = 0; // number of vertices
};

#include "shader.hpp"

struct framebuffer {
    unsigned int fbo = 0;
    unsigned int fbo_color = 0;
    unsigned int fbo_depth = 0;
    unsigned int xsize = 1280;
    unsigned int ysize = 720;
    
    static constexpr int NUM_PBOS = 2;
    unsigned int pbos[NUM_PBOS];
    unsigned int pboindex = 0;
    bool mapped = false;

    void init(unsigned int _xsize, unsigned int _ysize, GLint minFilter, GLint magFilter);
    void destroy();
    void bindForWriting();
    void bindForReading() const;
    
    GLhalf* lock();
    void unlock();
    
    void draw(float brightness = 1.f);
    void hdrDraw(const Vector4& brightness, float gamma, float exposure);
    static void unbindForWriting();
    static void unbindForReading();
    static void unbindAll();

    static Mesh mesh;
    static Shader shader;
    static Shader hdrShader;
};

// view structure
constexpr float defaultLuminance = 0.25f;
typedef struct view_t
{
    real_t x, y, z;
    real_t ang;
    real_t vang;
    Sint32 winx, winy, winw, winh;
    real_t globalLightModifier = 0.0;
    real_t globalLightModifierEntities = 0.0;
    int globalLightModifierActive = GLOBAL_LIGHT_MODIFIER_STOPPED;
    framebuffer fb[1];
    bool* vismap = nullptr;
    float luminance = defaultLuminance;
    unsigned int drawnFrames = 0;
    mat4x4 projview, proj, proj_hud;
} view_t;

#define FLIP_VERTICAL 1
#define FLIP_HORIZONTAL 2
extern "C" SDL_Surface* flipSurface(SDL_Surface* surface, int flags);
extern "C" void drawCircle(int x, int y, real_t radius, Uint32 color, Uint8 alpha);
extern "C" void drawArc(int x, int y, real_t radius, real_t angle1, real_t angle2, Uint32 color, Uint8 alpha);
extern "C" void drawArcInvertedY(int x, int y, real_t radius, real_t angle1, real_t angle2, Uint32 color, Uint8 alpha);
extern "C" void drawLine(int x1, int y1, int x2, int y2, Uint32 color, Uint8 alpha);
extern "C" int drawRect(SDL_Rect* src, Uint32 color, Uint8 alpha);
extern "C" int drawBox(SDL_Rect* src, Uint32 color, Uint8 alpha);
extern "C" void drawGear(Sint16 x, Sint16 y, real_t size, Sint32 rotation);
extern "C" void drawImage(SDL_Surface* image, SDL_Rect* src, SDL_Rect* pos);
extern "C" void drawImageRing(SDL_Surface* image, SDL_Rect* src, int radius, int thickness, int segments, real_t angStart, real_t angEnd, Uint8 alpha);
extern "C" void drawImageScaled(SDL_Surface* image, SDL_Rect* src, SDL_Rect* pos);
extern "C" void drawImageScaledPartial(SDL_Surface* image, SDL_Rect* src, SDL_Rect* pos, float percentY);
extern "C" void drawImageAlpha(SDL_Surface* image, SDL_Rect* src, SDL_Rect* pos, Uint8 alpha);
extern "C" void drawImageColor(SDL_Surface* image, SDL_Rect* src, SDL_Rect* pos, Uint32 color);
extern "C" void drawImageFancy(SDL_Surface* image, Uint32 color, real_t angle, SDL_Rect* src, SDL_Rect* pos);
extern "C" void drawImageRotatedAlpha(SDL_Surface* image, SDL_Rect* src, SDL_Rect* pos, real_t angle, Uint8 alpha);
extern "C" void drawImageScaledColor(SDL_Surface* image, SDL_Rect* src, SDL_Rect* pos, Uint32 color);
extern "C" SDL_Surface* scaleSurface(SDL_Surface* Surface, Uint16 Width, Uint16 Height);
extern "C" void drawSky3D(view_t* camera, SDL_Surface* tex);
extern "C" void drawLayer(long camx, long camy, int z, map_t* map);
extern "C" void drawBackground(long camx, long camy);
extern "C" void drawForeground(long camx, long camy);
extern "C" void drawClearBuffers();
extern "C" void raycast(const view_t& camera, Sint8 (*minimap)[MINIMAP_MAX_DIMENSION], bool fillWithColor);
extern "C" void drawFloors(view_t* camera);
extern "C" void drawSky(SDL_Surface* srfc);
extern "C" void drawVoxel(view_t* camera, Entity* entity);
extern "C" void drawEntities3D(view_t* camera, int mode);
extern "C" void drawPalette(voxel_t* model);
extern "C" void drawEntities2D(long camx, long camy);
extern "C" void drawGrid(int camx, int camy);
extern "C" void drawEditormap(long camx, long camy);
extern "C" void drawWindow(int x1, int y1, int x2, int y2);
extern "C" void drawDepressed(int x1, int y1, int x2, int y2);
extern "C" void drawWindowFancy(int x1, int y1, int x2, int y2);
extern "C" SDL_Rect ttfPrintTextColor( TTF_Font* font, int x, int y, Uint32 color, bool outline, const char* str );
extern "C" SDL_Rect ttfPrintText( TTF_Font* font, int x, int y, const char* str );
SDL_Rect ttfPrintTextFormattedColor( TTF_Font* font, int x, int y, Uint32 color, char const * const fmt, ... );
SDL_Rect ttfPrintTextFormatted( TTF_Font* font, int x, int y, char const * const fmt, ... );
void debugPrintText(int x, int y, const SDL_Rect& viewport, char const * const fmt, ...);
void printTextFormatted( SDL_Surface* font_bmp, int x, int y, char const * const fmt, ... );
void printTextFormattedAlpha(SDL_Surface* font_bmp, int x, int y, Uint8 alpha, char const * const fmt, ...);
void printTextFormattedColor(SDL_Surface* font_bmp, int x, int y, Uint32 color, char const * const fmt, ...);
void printTextFormattedFancy(SDL_Surface* font_bmp, int x, int y, Uint32 color, real_t angle, real_t scale, char* fmt, ...);
extern "C" void printText( SDL_Surface* font_bmp, int x, int y, const char* str );
extern "C" void drawSprite(view_t* camera, Entity* entity);
extern "C" void drawTooltip(SDL_Rect* src, Uint32 optionalColor = 0);
extern "C" Uint32 getPixel(SDL_Surface* surface, int x, int y);
extern "C" void putPixel(SDL_Surface* surface, int x, int y, Uint32 pixel);
extern "C" void getColor(Uint32 color, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a);
extern "C" bool behindCamera(const view_t& camera, real_t x, real_t y);
extern "C" void occlusionCulling(map_t& map, view_t& camera);

constexpr Uint32 makeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return ((Uint32)a << 24) | ((Uint32)b << 16) | ((Uint32)g << 8) | ((Uint32)r << 0);
}

constexpr Uint32 makeColorRGB(uint8_t r, uint8_t g, uint8_t b) {
    return 0xff000000 | ((Uint32)b << 16) | ((Uint32)g << 8) | ((Uint32)r << 0);
}

extern framebuffer main_framebuffer;
extern Shader voxelShader;
extern Shader voxelBrightShader;
extern Shader voxelDitheredShader;
extern Shader voxelBrightDitheredShader;
extern Shader worldShader;
extern Shader worldDitheredShader;
extern Shader worldDarkShader;
extern Shader skyShader;
extern Mesh skyMesh;
extern Shader spriteShader;
extern Shader spriteDitheredShader;
extern Shader spriteBrightShader;
extern Shader spriteUIShader;
extern Mesh spriteMesh;
extern TempTexture* lightmapTexture[MAXPLAYERS + 1];

#define TRANSPARENT_TILE 246

extern Uint32 ditherDisabledTime;
extern "C" void temporarilyDisableDithering();

struct Chunk {
    GLuint vao = 0;
    GLuint vbo_positions = 0;
    GLuint vbo_texcoords = 0;
    GLuint vbo_colors = 0;
    GLint indices = 0;
    
    Chunk() = default;
    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;
    
    Chunk(Chunk&& rhs) {
        vao = rhs.vao;
        vbo_positions = rhs.vbo_positions;
        vbo_texcoords = rhs.vbo_texcoords;
        vbo_colors = rhs.vbo_colors;
        indices = rhs.indices;
        x = rhs.x;
        y = rhs.y;
        w = rhs.w;
        h = rhs.h;
        
        rhs.vao = 0;
        rhs.vbo_positions = 0;
        rhs.vbo_texcoords = 0;
        rhs.vbo_colors = 0;
        rhs.indices = 0;
        rhs.x = 0;
        rhs.y = 0;
        rhs.w = 0;
        rhs.h = 0;
        
        tiles.swap(rhs.tiles);
        dithering.swap(rhs.dithering);
    }
    
    Chunk& operator=(Chunk&& rhs) {
        vao = rhs.vao;
        vbo_positions = rhs.vbo_positions;
        vbo_texcoords = rhs.vbo_texcoords;
        vbo_colors = rhs.vbo_colors;
        indices = rhs.indices;
        x = rhs.x;
        y = rhs.y;
        w = rhs.w;
        h = rhs.h;
        
        rhs.vao = 0;
        rhs.vbo_positions = 0;
        rhs.vbo_texcoords = 0;
        rhs.vbo_colors = 0;
        rhs.indices = 0;
        rhs.x = 0;
        rhs.y = 0;
        rhs.w = 0;
        rhs.h = 0;
        
        tiles.swap(rhs.tiles);
        dithering.swap(rhs.dithering);
        return *this;
    }
    
    ~Chunk() {
        destroyBuffers();
    }
    
    void build(const map_t& map, bool ceiling, int startX, int startY, int w, int h);
    void buildBuffers(const std::vector<float>& positions, const std::vector<float>& texcoords, const std::vector<float>& colors);
    void destroyBuffers();
    void draw();
    bool isDirty(const map_t& map);
    
    int x = 0, y = 0, w = 0, h = 0;
    DynamicArrayS32 tiles;
    
    struct Dither {
        static constexpr int MAX = 10;
        int value = MAX;
        Uint32 lastUpdateTick = 0;
    };
    DynamicMapPtrT<ChunkDither_t> dithering;
};
extern "C" void clearChunks();
extern "C" void createChunks();

extern "C" void createCommonDrawResources();
extern "C" void destroyCommonDrawResources();

extern view_t cameras[MAXPLAYERS];
extern view_t menucam;

// function prototypes for opengl.c:
#define REALCOLORS 0
#define ENTITYUIDS 1
extern "C" void beginGraphics();
extern "C" void glBeginCamera(view_t* camera, bool useHDR, map_t& map);
extern "C" void glDrawVoxel(view_t* camera, Entity* entity, int mode);
extern "C" void glDrawSprite(view_t* camera, Entity* entity, int mode);
extern "C" void glDrawWorldUISprite(view_t* camera, Entity* entity, int mode);
extern "C" void glDrawWorldDialogueSprite(view_t* camera, void* worldDialogue, int mode);
extern "C" void glDrawEnemyBarSprite(view_t* camera, int mode, int playerViewport, void* enemyHPBarDetails);
extern "C" void glDrawSpriteFromImage(view_t* camera, Entity* entity, DynamicString text, int mode, bool useTextAsImgPath = false, bool rotate = false);
extern "C" void glDrawWorld(view_t* camera, int mode);
extern "C" void glEndCamera(view_t* camera, bool useHDR, map_t& map);
extern "C" unsigned int GO_GetPixelU32(int x, int y, view_t& camera);

extern bool hdrEnabled;

#ifndef EDITOR
extern CvarVector4 cvar_hdrBrightness;
extern CvarFloat cvar_fogDistance;
extern CvarVector4 cvar_fogColor;
extern CvarFloat cvar_hdrExposure;
extern CvarFloat cvar_hdrGamma;
extern CvarFloat cvar_hdrAdjustment;
extern CvarFloat cvar_hdrLimitHigh;
extern CvarFloat cvar_hdrLimitLow;
extern const Vector4 defaultBrightness;
extern const float defaultGamma;
extern const float defaultExposure;
extern const float defaultAdjustmentRate;
extern const float defaultLimitHigh;
extern const float defaultLimitLow;
#endif
