#include <g_canvas.h>

typedef struct _pdlua_gfx
{
#if !PLUGDATA
    char active_tags[128][128];
    int num_tags;
    int current_red, current_green, current_blue;
#endif
    
    int has_gui;
    int width, height;
} t_pdlua_gfx;

static int gfx_initialize(lua_State* L);
static int start_paint(lua_State* L);
static int end_paint(lua_State* L);

static int set_color(lua_State* L);

static int fill_ellipse(lua_State* L);
static int stroke_ellipse(lua_State* L);
static int fill_all(lua_State* L);
static int fill_rect(lua_State* L);
static int stroke_rect(lua_State* L);
static int fill_rounded_rect(lua_State* L);
static int stroke_rounded_rect(lua_State* L);

static int draw_text(lua_State* L);

static int start_path(lua_State* L);
static int line_to(lua_State* L);
static int quad_to(lua_State* L);
static int stroke_path(lua_State* L);
static int fill_path(lua_State* L);

static int translate(lua_State* L);
static int scale(lua_State* L);
static int save(lua_State* L);
static int restore(lua_State* L);

lua_State* get_lua_state();
t_canvas* get_pdlua_canvas(t_object* object);
t_pdlua_gfx* get_pdlua_gfx_state(t_object* object);

void pdlua_gfx_repaint(t_object* o) {
    lua_State* __L = get_lua_state();
    lua_getglobal(__L, "pd");
    lua_getfield (__L, -1, "_repaint");
    lua_pushlightuserdata(__L, o);
    
    if (lua_pcall(__L, 1, 0, 0))
    {
        pd_error(o, "lua: error in repaint:\n%s", lua_tostring(__L, -1));
        lua_pop(__L, 1); /* pop the error string */
    }
    
    lua_pop(__L, 1); /* pop the global "pd" */
}

void pdlua_gfx_mouse_event(t_object* o, int x, int y, int type) {
    lua_State* __L = get_lua_state();
    lua_getglobal(__L, "pd");
    lua_getfield (__L, -1, "_mouseevent");
    lua_pushlightuserdata(__L, o);
    lua_pushinteger(__L, x);
    lua_pushinteger(__L, y);
    lua_pushinteger(__L, type);
    
    if (lua_pcall(__L, 4, 0, 0))
    {
        pd_error(o, "lua: error in mouseevent:\n%s", lua_tostring(__L, -1));
        lua_pop(__L, 1); /* pop the error string */
    }
    
    lua_pop(__L, 1); /* pop the global "pd" */
}

void pdlua_gfx_mouse_down(t_object* o, int x, int y) {
    pdlua_gfx_mouse_event(o, x, y, 0);
}
void pdlua_gfx_mouse_up(t_object* o, int x, int y) {
    pdlua_gfx_mouse_event(o, x, y, 1);
}

void pdlua_gfx_mouse_move(t_object* o, int x, int y) {
    pdlua_gfx_mouse_event(o, x, y, 2);
}

void pdlua_gfx_mouse_drag(t_object* o, int x, int y) {
    pdlua_gfx_mouse_event(o, x, y, 3);
}

#define PDLUA_OBJECT_REGISTRTY_ID 512

static t_object* get_current_object(lua_State* L)
{
    lua_pushvalue(L, LUA_REGISTRYINDEX);
    lua_geti(L, -1, PDLUA_OBJECT_REGISTRTY_ID);

    // Check if the value is a light userdata
    if (lua_islightuserdata(L, -1)) {
        // Retrieve the userdata pointer
        return lua_touserdata(L, -1);
    }
    
    return NULL;
}

// Register C++ functions with Lua
static const luaL_Reg gfx[] = {
    {"set_color", set_color},
    {"fill_ellipse", fill_ellipse},
    {"stroke_ellipse", stroke_ellipse},
    {"fill_rect", fill_rect},
    {"stroke_rect", stroke_rect},
    {"fill_rounded_rect", fill_rounded_rect},
    {"stroke_rounded_rect", stroke_rounded_rect},
    {"text", draw_text},
    {"start_path", start_path},
    {"line_to", line_to},
    {"quad_to", quad_to},
    {"stroke_path", stroke_path},
    {"fill_path", fill_path},
    {"fill_all", fill_all},
    {"translate", translate},
    {"scale", scale},
    {"save", save},
    {"restore", restore},
    {"_start_paint", start_paint},
    {"_end_paint", end_paint},
    {"initialize", gfx_initialize },
    {NULL, NULL} // Sentinel to end the list
};


void* gui_target;
void(*register_gui)(void*, t_object*);

int set_gui_callback(void* callback_target, void(*callback)(void*, t_object*)) {
    gui_target = callback_target;
    register_gui = callback;
    return 0;
}

int initialize_graphics(lua_State* L) {
    // Register functions with Lua
    luaL_newlib(L, gfx);
    lua_setglobal(L, "gfx");
    return 1; // Number of values pushed onto the stack
}

#if PLUGDATA
void plugdata_forward_message(void* x, t_symbol *s, int argc, t_atom *argv);

static int gfx_initialize(lua_State* L)
{
    t_object* obj = get_current_object(L);
    register_gui(gui_target, obj);
    
    t_pdlua_gfx* gfx = get_pdlua_gfx_state(obj);
    gfx->has_gui = 1;
    
    return 0;
}

static int start_paint(lua_State* L) {
    t_object* obj = get_current_object(L);
    plugdata_forward_message(obj, gensym("lua_start_paint"), 0, NULL);
    return 0;
}

static int end_paint(lua_State* L) {
    t_object* obj = get_current_object(L);
    plugdata_forward_message(obj, gensym("lua_end_paint"), 0, NULL);
    return 0;
}

static int set_color(lua_State* L) {
    t_object* obj = get_current_object(L);
    t_atom args[4];
    SETFLOAT(args, luaL_checknumber(L, 1)); // r
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // g
    SETFLOAT(args + 2, luaL_checknumber(L, 3)); // b
    SETFLOAT(args + 3, luaL_optnumber(L, 4, 1.0)); // a (optional, default to 1.0)
    plugdata_forward_message(obj, gensym("lua_set_color"), 4, args);
    return 0;
}

static int fill_ellipse(lua_State* L) {
    t_object* obj = get_current_object(L);
    t_atom args[4];
    SETFLOAT(args, luaL_checknumber(L, 1)); // x
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // y
    SETFLOAT(args + 2, luaL_checknumber(L, 3)); // w
    SETFLOAT(args + 3, luaL_checknumber(L, 4)); // h
    plugdata_forward_message(obj, gensym("lua_fill_ellipse"), 4, args);
    return 0;
}

static int stroke_ellipse(lua_State* L) {
    t_object* obj = get_current_object(L);
    t_atom args[4];
    SETFLOAT(args, luaL_checknumber(L, 1)); // x
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // y
    SETFLOAT(args + 2, luaL_checknumber(L, 3)); // w
    SETFLOAT(args + 3, luaL_checknumber(L, 4)); // h
    SETFLOAT(args + 4, luaL_checknumber(L, 5)); // width
    plugdata_forward_message(obj, gensym("lua_stroke_ellipse"), 4, args);
    return 0;
}

static int fill_all(lua_State* L) {
    t_object* obj = get_current_object(L);
    plugdata_forward_message(obj, gensym("lua_fill_all"), 0, NULL);
    return 0;
}

static int fill_rect(lua_State* L) {
    t_object* obj = get_current_object(L);
    t_atom args[4];
    SETFLOAT(args, luaL_checknumber(L, 1)); // x
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // y
    SETFLOAT(args + 2, luaL_checknumber(L, 3)); // w
    SETFLOAT(args + 3, luaL_checknumber(L, 4)); // h
    plugdata_forward_message(obj, gensym("lua_fill_rect"), 4, args);
    return 0;
}

static int stroke_rect(lua_State* L) {
    t_object* obj = get_current_object(L);
    t_atom args[5];
    SETFLOAT(args, luaL_checknumber(L, 1)); // x
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // y
    SETFLOAT(args + 2, luaL_checknumber(L, 3)); // w
    SETFLOAT(args + 3, luaL_checknumber(L, 4)); // h
    SETFLOAT(args + 4, luaL_checknumber(L, 5)); // corner_radius
    plugdata_forward_message(obj, gensym("lua_stroke_rect"), 5, args);
    return 0;
}

static int fill_rounded_rect(lua_State* L) {
    t_object* obj = get_current_object(L);
    t_atom args[4];
    SETFLOAT(args, luaL_checknumber(L, 1)); // x
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // y
    SETFLOAT(args + 2, luaL_checknumber(L, 3)); // w
    SETFLOAT(args + 3, luaL_checknumber(L, 4)); // h
    plugdata_forward_message(obj, gensym("lua_fill_rounded_rect"), 4, args);
    return 0;
}

static int stroke_rounded_rect(lua_State* L) {
    t_object* obj = get_current_object(L);
    t_atom args[6];
    SETFLOAT(args, luaL_checknumber(L, 1)); // x
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // y
    SETFLOAT(args + 2, luaL_checknumber(L, 3)); // w
    SETFLOAT(args + 3, luaL_checknumber(L, 4)); // h
    SETFLOAT(args + 4, luaL_checknumber(L, 5)); // corner_radius
    SETFLOAT(args + 5, luaL_checknumber(L, 6)); // width
    plugdata_forward_message(obj, gensym("lua_stroke_rounded_rect"), 6, args);
    return 0;
}

static int draw_text(lua_State* L) {
    t_object* obj = get_current_object(L);
    const char* text = luaL_checkstring(L, 1); // Assuming text is a string
    t_atom args[5];
    SETSYMBOL(args, gensym(text));
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // x
    SETFLOAT(args + 2, luaL_checknumber(L, 3)); // y
    SETFLOAT(args + 3, luaL_checknumber(L, 4)); // w
    SETFLOAT(args + 4, luaL_checknumber(L, 5)); // h
    plugdata_forward_message(obj, gensym("lua_text"), 5, args);
    return 0;
}

static int start_path(lua_State* L) {
    t_object* obj = get_current_object(L);
    plugdata_forward_message(obj, gensym("lua_start_path"), 0, NULL);
    return 0;
}

static int line_to(lua_State* L) {
    t_object* obj = get_current_object(L);
    t_atom args[2];
    SETFLOAT(args, luaL_checknumber(L, 1)); // x
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // y
    plugdata_forward_message(obj, gensym("lua_line_to"), 2, args);
    return 0;
}

static int quad_to(lua_State* L) {
    t_object* obj = get_current_object(L);
    t_atom args[4];
    SETFLOAT(args, luaL_checknumber(L, 1)); // bx
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // by
    SETFLOAT(args + 2, luaL_checknumber(L, 3)); // cx
    SETFLOAT(args + 3, luaL_checknumber(L, 4)); // cy
    plugdata_forward_message(obj, gensym("lua_quad_to"), 4, args);
    return 0;
}

static int stroke_path(lua_State* L) {
    t_object* obj = get_current_object(L);
    plugdata_forward_message(obj, gensym("lua_stroke_path"), 0, NULL);
    return 0;
}

static int fill_path(lua_State* L) {
    t_object* obj = get_current_object(L);
    plugdata_forward_message(obj, gensym("lua_fill_path"), 0, NULL);
    return 0;
}

static int translate(lua_State* L) {
    t_object* obj = get_current_object(L);
    t_atom args[2];
    SETFLOAT(args, luaL_checknumber(L, 1)); // tx
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // ty
    plugdata_forward_message(obj, gensym("lua_translate"), 2, args);
    return 0;
}

static int scale(lua_State* L) {
    t_object* obj = get_current_object(L);
    t_atom args[2];
    SETFLOAT(args, luaL_checknumber(L, 1)); // sx
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // sy
    plugdata_forward_message(obj, gensym("lua_scale"), 2, args);
    return 0;
}

static int save(lua_State* L) {
    t_object* obj = get_current_object(L);
    plugdata_forward_message(obj, gensym("lua_save"), 0, NULL);
    return 0;
}

static int restore(lua_State* L) {
    t_object* obj = get_current_object(L);
    plugdata_forward_message(obj, gensym("lua_restore"), 0, NULL);
    return 0;
}
#else

static const char* register_drawing(t_object* object)
{
    t_pdlua_gfx* gfx = get_pdlua_gfx_state(object);
    snprintf(gfx->active_tags[gfx->num_tags], 128, ".x%lx%lx", object, rand());
    gfx->active_tags[gfx->num_tags][127] = '\0';
    
    return gfx->active_tags[gfx->num_tags++];
}

static int gfx_initialize(lua_State* L)
{
    t_object* obj = get_current_object(L);
    t_pdlua_gfx* gfx = get_pdlua_gfx_state(obj);
    gfx->has_gui = 1;
    return 0;
}

static int start_paint(lua_State* L) {
    t_object* obj = get_current_object(L);
    t_canvas* cnv = get_pdlua_canvas(obj);
    t_pdlua_gfx* gfx = get_pdlua_gfx_state(obj);
    
    // Delete drawings made previously
    for(int i = 0; i < gfx->num_tags; i++)
    {
        pdgui_vmess(0, "crs", cnv, "delete", gfx->active_tags[i]);
    }
    gfx->num_tags = 0;
    return 0;
}

static int end_paint(lua_State* L) {
    return 0;
}

static int set_color(lua_State* L) {
    t_object* obj = get_current_object(L);
    t_pdlua_gfx* gfx = get_pdlua_gfx_state(obj);
    
    gfx->current_red = luaL_checknumber(L, 1);
    gfx->current_green = luaL_checknumber(L, 2);
    gfx->current_blue = luaL_checknumber(L, 3);
    return 0;
}


static int fill_ellipse(lua_State* L) {
    t_object* obj = get_current_object(L);
    t_pdlua_gfx* gfx = get_pdlua_gfx_state(obj);
    t_canvas* cnv = get_pdlua_canvas(obj);
    
    int x1 = text_xpix(obj, cnv) + luaL_checknumber(L, 1);
    int y1 = text_ypix(obj, cnv) + luaL_checknumber(L, 2);
    int x2 = x1 + luaL_checknumber(L, 3);
    int y2 = y1 + luaL_checknumber(L, 4);
    
    const char* tags[] = { register_drawing(obj) };
    
    char hex_color[8];
    snprintf(hex_color, sizeof(hex_color), "#%02X%02X%02X", gfx->current_red, gfx->current_green, gfx->current_blue);

    pdgui_vmess(0, "crr iiii rs rS", cnv, "create", "oval", x1, y1, x2, y2, "-fill", hex_color, "-tags", 1, tags);
    return 0;
}

static int stroke_ellipse(lua_State* L) {
    t_object* obj = get_current_object(L);
    t_pdlua_gfx* gfx = get_pdlua_gfx_state(obj);
    t_canvas* cnv = get_pdlua_canvas(obj);

    int x1 = text_xpix(obj, cnv) + luaL_checknumber(L, 1);
    int y1 = text_ypix(obj, cnv) + luaL_checknumber(L, 2);
    int x2 = x1 + luaL_checknumber(L, 3);
    int y2 = y1 + luaL_checknumber(L, 4);
    
    const char* tags[] = { register_drawing(obj) };
    
    char hex_color[8];
    snprintf(hex_color, sizeof(hex_color), "#%02X%02X%02X", gfx->current_red, gfx->current_green, gfx->current_blue);

    pdgui_vmess(0, "crr iiii rs rS", cnv, "create", "oval", x1, y1, x2, y2, "-outline", hex_color, "-tags", 1, tags);
    return 0;
}

static int fill_all(lua_State* L) {
    t_object* obj = get_current_object(L);
    t_pdlua_gfx* gfx = get_pdlua_gfx_state(obj);
    t_canvas* cnv = get_pdlua_canvas(obj);
    
    int x1 = text_xpix(obj, cnv);
    int y1 = text_ypix(obj, cnv);
    int x2 = x1 + gfx->width;
    int y2 = y1 + gfx->height;
    
    const char* tags[] =  { register_drawing(obj) };
    
    char hex_color[8];  // Assuming #RRGGBB format
    snprintf(hex_color, sizeof(hex_color), "#%02X%02X%02X", gfx->current_red, gfx->current_green, gfx->current_blue);

    pdgui_vmess(0, "crr iiii rs rS", cnv, "create", "rectangle", x1, y1, x2, y2, "-fill", hex_color, "-tags", 1, tags);
    return 0;
}

static int fill_rect(lua_State* L) {
    t_object* obj = get_current_object(L);
    t_pdlua_gfx* gfx = get_pdlua_gfx_state(obj);
    t_canvas* cnv = get_pdlua_canvas(obj);
    
    int x1 = text_xpix(obj, cnv) + luaL_checknumber(L, 1);
    int y1 = text_ypix(obj, cnv) + luaL_checknumber(L, 2);
    int x2 = x1 + luaL_checknumber(L, 3);
    int y2 = y1 + luaL_checknumber(L, 4);
    
    const char* tags[] = { register_drawing(obj) };
    
    char hex_color[8];  // Assuming #RRGGBB format
    snprintf(hex_color, sizeof(hex_color), "#%02X%02X%02X", gfx->current_red, gfx->current_green, gfx->current_blue);

    pdgui_vmess(0, "crr iiii rs rS", cnv, "create", "rectangle", x1, y1, x2, y2, "-fill", hex_color, "-tags", 1, tags);
    return 0;
}

static int stroke_rect(lua_State* L) {
    t_object* obj = get_current_object(L);
    t_pdlua_gfx* gfx = get_pdlua_gfx_state(obj);
    t_canvas* cnv = get_pdlua_canvas(obj);
    
    int x1 = text_xpix(obj, cnv) + luaL_checknumber(L, 1);
    int y1 = text_ypix(obj, cnv) + luaL_checknumber(L, 2);
    int x2 = x1 + luaL_checknumber(L, 3);
    int y2 = y1 + luaL_checknumber(L, 4);

    const char* tags[] = { register_drawing(obj) };
    
    char hex_color[8];  // Assuming #RRGGBB format
    snprintf(hex_color, sizeof(hex_color), "#%02X%02X%02X", gfx->current_red, gfx->current_green, gfx->current_blue);

    pdgui_vmess(0, "crr iiii rs rS", cnv, "create", "rectangle", x1, y1, x2, y2, "-outline", hex_color, "-tags", 1, tags);
    return 0;
}

static int fill_rounded_rect(lua_State* L) {
    t_object* obj = get_current_object(L);
    t_pdlua_gfx* gfx = get_pdlua_gfx_state(obj);
    t_canvas* cnv = get_pdlua_canvas(obj);
    
    int x1 = text_xpix(obj, cnv) + luaL_checknumber(L, 1);
    int y1 = text_ypix(obj, cnv) + luaL_checknumber(L, 2);
    int x2 = x1 + luaL_checknumber(L, 3);
    int y2 = y1 + luaL_checknumber(L, 4);
    int radius = luaL_checknumber(L, 5);  // Radius for rounded corners
    
    const char* tags[] = { register_drawing(obj) };
    
    char hex_color[8];  // Assuming #RRGGBB format
    snprintf(hex_color, sizeof(hex_color), "#%02X%02X%02X", gfx->current_red, gfx->current_green, gfx->current_blue);

    // Draw the main body of the rounded rectangle
    pdgui_vmess(0, "crr iiii rs rS", cnv, "create", "rectangle", x1, y1, x2, y2, "-fill", hex_color, "-tags", 1, tags);
    
    // Draw rounded corners using arcs
    pdgui_vmess(0, "crr iiii rs rS", cnv, "create", "arc", x1, y1, x1+2*radius, y1+2*radius, "-start", 90, "-extent", 90, "-fill", hex_color, "-width", 1, "-tags", 1, tags);
    pdgui_vmess(0, "crr iiii rs rS", cnv, "create", "arc", x2-2*radius, y1, x2, y1+2*radius, "-start", 0, "-extent", 90, "-fill", hex_color, "-width", 1, "-tags", 1, tags);
    pdgui_vmess(0, "crr iiii rs rS", cnv, "create", "arc", x1, y2-2*radius, x1+2*radius, y2, "-start", 180, "-extent", 90, "-fill", hex_color, "-width", 1, "-tags", 1, tags);
    pdgui_vmess(0, "crr iiii rs rS", cnv, "create", "arc", x2-2*radius, y2-2*radius, x2, y2, "-start", 270, "-extent", 90, "-fill", hex_color, "-width", 1, "-tags", 1, tags);
    return 0;
}

static int stroke_rounded_rect(lua_State* L) {
    t_object* obj = get_current_object(L);
    t_pdlua_gfx* gfx = get_pdlua_gfx_state(obj);
    t_canvas* cnv = get_pdlua_canvas(obj);
    
    int x1 = text_xpix(obj, cnv) + luaL_checknumber(L, 1);
    int y1 = text_ypix(obj, cnv) + luaL_checknumber(L, 2);
    int x2 = x1 + luaL_checknumber(L, 3);
    int y2 = y1 + luaL_checknumber(L, 4);
    int radius = luaL_checknumber(L, 5);  // Radius for rounded corners
    
    const char* tags[] = { register_drawing(obj) };
    
    char hex_color[8];  // Assuming #RRGGBB format
    snprintf(hex_color, sizeof(hex_color), "#%02X%02X%02X", gfx->current_red, gfx->current_green, gfx->current_blue);

    // Draw the main body of the rounded rectangle with an outline
    pdgui_vmess(0, "crr iiii rs rS", cnv, "create", "rectangle", x1, y1, x2, y2, "-outline", hex_color, "-width", 1, "-tags", 1, tags);
    
    // Draw rounded corners using arcs
    pdgui_vmess(0, "crr iiii rs rS", cnv, "create", "arc", x1, y1, x1+2*radius, y1+2*radius, "-start", 90, "-extent", 90, "-outline", hex_color, "-width", 1, "-tags", 1, tags);
    pdgui_vmess(0, "crr iiii rs rS", cnv, "create", "arc", x2-2*radius, y1, x2, y1+2*radius, "-start", 0, "-extent", 90, "-outline", hex_color, "-width", 1, "-tags", 1, tags);
    pdgui_vmess(0, "crr iiii rs rS", cnv, "create", "arc", x1, y2-2*radius, x1+2*radius, y2, "-start", 180, "-extent", 90, "-outline", hex_color, "-width", 1, "-tags", 1, tags);
    pdgui_vmess(0, "crr iiii rs rS", cnv, "create", "arc", x2-2*radius, y2-2*radius, x2, y2, "-start", 270, "-extent", 90, "-outline", hex_color, "-width", 1, "-tags", 1, tags);
    return 0;
}

static int draw_text(lua_State* L) {
    t_object* obj = get_current_object(L);
    return 0;
}

static int start_path(lua_State* L) {
    t_object* obj = get_current_object(L);
    return 0;
}

static int line_to(lua_State* L) {
    t_object* obj = get_current_object(L);
    return 0;
}

static int quad_to(lua_State* L) {
    t_object* obj = get_current_object(L);
    return 0;
}

static int stroke_path(lua_State* L) {
    t_object* obj = get_current_object(L);
    return 0;
}

static int fill_path(lua_State* L) {
    t_object* obj = get_current_object(L);
    return 0;
}

static int translate(lua_State* L) {
    t_object* obj = get_current_object(L);
    return 0;
}

static int scale(lua_State* L) {
    t_object* obj = get_current_object(L);
    return 0;
}

static int save(lua_State* L) {
    t_object* obj = get_current_object(L);
    return 0;
}

static int restore(lua_State* L) {
    t_object* obj = get_current_object(L);
    return 0;
}

#endif
