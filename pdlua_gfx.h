
// Functions that need to be implemented separately for each Pd flavour
static int gfx_initialize(t_pdlua *obj);

static int set_size(lua_State *L);
static int start_paint(lua_State *L);
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
static int cubic_to(lua_State* L);
static int close_path(lua_State* L);
static int stroke_path(lua_State* L);
static int fill_path(lua_State* L);

static int translate(lua_State* L);
static int scale(lua_State* L);
static int reset_transform(lua_State* L);

void pdlua_gfx_clear(t_pdlua *obj);

typedef struct _pdlua_mouse_proxy{
    t_object    p_obj;
    t_symbol   *p_sym;
    t_clock    *p_clock;
    t_pdlua    *p_parent;
}t_pdlua_mouse_proxy;

void pdlua_gfx_repaint(t_pdlua *o) {
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

void pdlua_gfx_mouse_event(t_pdlua *o, int x, int y, int type) {
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

void pdlua_gfx_mouse_down(t_pdlua *o, int x, int y) {
    pdlua_gfx_mouse_event(o, x, y, 0);
}
void pdlua_gfx_mouse_up(t_pdlua *o, int x, int y) {
    pdlua_gfx_mouse_event(o, x, y, 1);
}

void pdlua_gfx_mouse_move(t_pdlua *o, int x, int y) {
    pdlua_gfx_mouse_event(o, x, y, 2);
}

void pdlua_gfx_mouse_drag(t_pdlua *o, int x, int y) {
    pdlua_gfx_mouse_event(o, x, y, 3);
}

#define PDLUA_OBJECT_REGISTRTY_ID 512

static t_pdlua* get_current_object(lua_State* L)
{
    lua_pushvalue(L, LUA_REGISTRYINDEX);
    lua_geti(L, -1, PDLUA_OBJECT_REGISTRTY_ID);

    if (lua_islightuserdata(L, -1)) {
        return lua_touserdata(L, -1);
    }
    
    return NULL;
}

// Register functions with Lua
static const luaL_Reg gfx_lib[] = {
    {"set_size", set_size},
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
    {"cubic_to", cubic_to},
    {"close_path", close_path},
    {"stroke_path", stroke_path},
    {"fill_path", fill_path},
    {"fill_all", fill_all},
    {"translate", translate},
    {"scale", scale},
    {"reset_transform", reset_transform},
    {"_start_paint", start_paint},
    {"_end_paint", end_paint},
    {NULL, NULL} // Sentinel to end the list
};


void* gui_target;
void(*register_gui)(void*, t_object*);


int set_gui_callback(void* callback_target, void(*callback)(void*, t_object*)) {
    gui_target = callback_target;
    register_gui = callback;
    return 0;
}

#if !PLUGDATA
t_class* pdlua_mouse_proxy_class;

static void pdlua_mouse_proxy_any(t_pdlua_mouse_proxy *p, t_symbol*s, int ac, t_atom *av){
    
    if(s == gensym("motion") && ! p->p_parent->gfx.mouse_up)
    {
        t_canvas *cnv = glist_getcanvas(p->p_parent->canvas);
        int zoom = glist_getzoom(cnv);
        int x = (int)(av->a_w.w_float / zoom);
        int y = (int)((av+1)->a_w.w_float / zoom);
        x -= text_xpix(p->p_parent, cnv);
        y -= text_ypix(p->p_parent, cnv);
       
        if(x > 0 && y > 0 && x < p->p_parent->gfx.width && y < p->p_parent->gfx.height) {
            pdlua_gfx_mouse_move(p->p_parent, x, y);
        }
    }
}

static void pdlua_mouse_proxy_free(t_pdlua_mouse_proxy *p){
    pd_unbind(&p->p_obj.ob_pd, p->p_sym);
    //clock_free(p->p_clock);
    pd_free(&p->p_obj.ob_pd);
}

static t_pdlua_mouse_proxy *pdlua_mouse_proxy_new(t_pdlua *x, t_symbol *s){
    t_pdlua_mouse_proxy *p = (t_pdlua_mouse_proxy*)pd_new(pdlua_mouse_proxy_class);
    p->p_parent = x;
    pd_bind(&p->p_obj.ob_pd, p->p_sym = s);
    //p->p_clock = clock_new(p, (t_method)pdlua_mouse_proxy_free);
    return(p);
}
#endif


int pdlua_gfx_setup(lua_State* L) {
    // Register functions with Lua
    luaL_newlib(L, gfx_lib);
    lua_setglobal(L, "gfx");
    
#if !PLUGDATA
    pdlua_mouse_proxy_class = class_new(0, 0, 0, sizeof(t_pdlua_mouse_proxy), CLASS_NOINLET | CLASS_PD, 0);
    class_addanything(pdlua_mouse_proxy_class, pdlua_mouse_proxy_any);
#endif
    
    return 1; // Number of values pushed onto the stack
}

#if PLUGDATA
void plugdata_forward_message(void* x, t_symbol *s, int argc, t_atom *argv);

void pdlua_gfx_clear(t_pdlua* obj) {
}

static int gfx_initialize(t_pdlua* obj)
{
    register_gui(gui_target, obj);
    pdlua_gfx_repaint(obj);
    return 0;
}

static int set_size(lua_State* L)
{
    t_pdlua* obj = get_current_object(L);
    obj->gfx.width = luaL_checknumber(L, 1);
    obj->gfx.height = luaL_checknumber(L, 2);
    t_atom args[2];
    SETFLOAT(args, luaL_checknumber(L, 1)); // w
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // h
    plugdata_forward_message(obj, gensym("lua_resized"), 2, args);
    return 0;
}

static int start_paint(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    plugdata_forward_message(obj, gensym("lua_start_paint"), 0, NULL);
    return 0;
}

static int end_paint(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    plugdata_forward_message(obj, gensym("lua_end_paint"), 0, NULL);
    return 0;
}

static int set_color(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_atom args[4];
    SETFLOAT(args, luaL_checknumber(L, 1)); // r
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // g
    SETFLOAT(args + 2, luaL_checknumber(L, 3)); // b
    SETFLOAT(args + 3, luaL_optnumber(L, 4, 1.0)); // a (optional, default to 1.0)
    plugdata_forward_message(obj, gensym("lua_set_color"), 4, args);
    return 0;
}

static int fill_ellipse(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_atom args[4];
    SETFLOAT(args, luaL_checknumber(L, 1)); // x
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // y
    SETFLOAT(args + 2, luaL_checknumber(L, 3)); // w
    SETFLOAT(args + 3, luaL_checknumber(L, 4)); // h
    plugdata_forward_message(obj, gensym("lua_fill_ellipse"), 4, args);
    return 0;
}

static int stroke_ellipse(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
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
    t_pdlua* obj = get_current_object(L);
    plugdata_forward_message(obj, gensym("lua_fill_all"), 0, NULL);
    return 0;
}

static int fill_rect(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_atom args[4];
    SETFLOAT(args, luaL_checknumber(L, 1)); // x
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // y
    SETFLOAT(args + 2, luaL_checknumber(L, 3)); // w
    SETFLOAT(args + 3, luaL_checknumber(L, 4)); // h
    plugdata_forward_message(obj, gensym("lua_fill_rect"), 4, args);
    return 0;
}

static int stroke_rect(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
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
    t_pdlua* obj = get_current_object(L);
    t_atom args[4];
    SETFLOAT(args, luaL_checknumber(L, 1)); // x
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // y
    SETFLOAT(args + 2, luaL_checknumber(L, 3)); // w
    SETFLOAT(args + 3, luaL_checknumber(L, 4)); // h
    plugdata_forward_message(obj, gensym("lua_fill_rounded_rect"), 4, args);
    return 0;
}

static int stroke_rounded_rect(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
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
    t_pdlua* obj = get_current_object(L);
    const char* text = luaL_checkstring(L, 1);
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
    t_pdlua* obj = get_current_object(L);
    t_atom args[2];
    SETFLOAT(args , luaL_checknumber(L, 1)); // x
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // y
    plugdata_forward_message(obj, gensym("lua_start_path"), 2, args);
    return 0;
}

static int line_to(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_atom args[2];
    SETFLOAT(args, luaL_checknumber(L, 1)); // x
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // y
    plugdata_forward_message(obj, gensym("lua_line_to"), 2, args);
    return 0;
}

static int quad_to(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_atom args[4]; // Assuming quad_to takes 3 arguments
    SETFLOAT(args, luaL_checknumber(L, 1)); // x1
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // y1
    SETFLOAT(args + 2, luaL_checknumber(L, 3)); // x2
    SETFLOAT(args + 3, luaL_checknumber(L, 4)); // y2

    // Forward the message to the appropriate function
    plugdata_forward_message(obj, gensym("lua_quad_to"), 4, args);
    return 0;
}

static int cubic_to(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_atom args[6]; // Assuming cubic_to takes 4 arguments
    SETFLOAT(args, luaL_checknumber(L, 1)); // x1
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // y1
    SETFLOAT(args + 2, luaL_checknumber(L, 3)); // x2
    SETFLOAT(args + 3, luaL_checknumber(L, 4)); // y2
    SETFLOAT(args + 4, luaL_checknumber(L, 5)); // x3
    SETFLOAT(args + 5, luaL_checknumber(L, 6)); // y3

    // Forward the message to the appropriate function
    plugdata_forward_message(obj, gensym("lua_cubic_to"), 6, args);
    return 0;
}

static int close_path(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    plugdata_forward_message(obj, gensym("lua_close_path"), 0, NULL);
    return 0;
}


static int stroke_path(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_atom arg;
    SETFLOAT(&arg, luaL_checknumber(L, 1)); // line thickness
    plugdata_forward_message(obj, gensym("lua_stroke_path"), 1, &arg);
    return 0;
}

static int fill_path(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    plugdata_forward_message(obj, gensym("lua_fill_path"), 0, NULL);
    return 0;
}

static int translate(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_atom args[2];
    SETFLOAT(args, luaL_checknumber(L, 1)); // tx
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // ty
    plugdata_forward_message(obj, gensym("lua_translate"), 2, args);
    return 0;
}

static int scale(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_atom args[2];
    SETFLOAT(args, luaL_checknumber(L, 1)); // sx
    SETFLOAT(args + 1, luaL_checknumber(L, 2)); // sy
    plugdata_forward_message(obj, gensym("lua_scale"), 2, args);
    return 0;
}

static int reset_transform(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    plugdata_forward_message(obj, gensym("lua_reset_transform"), 0, NULL);
    return 0;
}
#else

static int max(int a, int b) {
    return (a > b) ? a : b;
}

static int min(int a, int b) {
    return (a < b) ? a : b;
}

void pdlua_gfx_clear(t_pdlua *obj) {
    t_pdlua_gfx *gfx = &obj->gfx;
    t_canvas *cnv = glist_getcanvas(obj->canvas);
    
    for(int i = 0; i < gfx->num_tags; i++)
    {
        pdgui_vmess(0, "crs", cnv, "delete", gfx->active_tags[i]);
    }
    gfx->num_tags = 0;
}

static void get_bounds_args(lua_State* L, t_pdlua* obj, t_pdlua_gfx *gfx, int* x1, int* y1, int* x2, int* y2) {
    t_canvas *cnv = glist_getcanvas(obj->canvas);
    
    int x = luaL_checknumber(L, 1);
    int y = luaL_checknumber(L, 2);
    int w = luaL_checknumber(L, 3);
    int h = luaL_checknumber(L, 4);
    
    // Calculate the clipped rectangle
    x = min(max(x, 0), gfx->width - w);
    y = min(max(y, 0), gfx->height - h);
    w = min(x + w, gfx->width) - x;
    h = min(y + h, gfx->height) - y;
    
    x *= gfx->scale_x * glist_getzoom(cnv);
    y *= gfx->scale_y * glist_getzoom(cnv);
    w *= gfx->scale_x * glist_getzoom(cnv);
    h *= gfx->scale_y * glist_getzoom(cnv);
    
    x += gfx->translate_x;
    y += gfx->translate_y;
    
    x += text_xpix(obj, cnv);
    y += text_ypix(obj, cnv);
    
    *x1 = x;
    *y1 = y;
    *x2 = x + w;
    *y2 = y + h;
}


static const char* register_drawing(t_pdlua *object)
{
    t_pdlua_gfx *gfx = &object->gfx;
    snprintf(gfx->active_tags[gfx->num_tags], 128, ".x%lx", rand());
    gfx->active_tags[gfx->num_tags][127] = '\0';
    
    return gfx->active_tags[gfx->num_tags++];
}


static int gfx_initialize(t_pdlua *obj)
{
    t_pdlua_gfx *gfx = &obj->gfx;
    t_canvas *cnv = glist_getcanvas(obj->canvas);
    
    snprintf(gfx->object_tag, 128, ".x%lx", obj);
    gfx->object_tag[127] = '\0';
    pdlua_gfx_repaint(obj);
    
    char buf[MAXPDSTRING];
    snprintf(buf, MAXPDSTRING-1, ".x%lx", (unsigned long)cnv);
    buf[MAXPDSTRING-1] = 0;
    
    gfx->mouse_proxy = pdlua_mouse_proxy_new(obj, gensym(buf));
    
    return 0;
}

static int set_size(lua_State* L)
{
    t_pdlua* obj = get_current_object(L);
    obj->gfx.width = luaL_checknumber(L, 1);
    obj->gfx.height = luaL_checknumber(L, 2);
    pdlua_gfx_repaint(obj);
    return 0;
}

static int start_paint(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_canvas *cnv = glist_getcanvas(obj->canvas);
    t_pdlua_gfx *gfx = &obj->gfx;
    
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
    t_pdlua* obj = get_current_object(L);
    t_pdlua_gfx *gfx = &obj->gfx;
    
    int r = luaL_checknumber(L, 1);
    int g = luaL_checknumber(L, 2);
    int b = luaL_checknumber(L, 3);
    
    snprintf(gfx->current_color, 8, "#%02X%02X%02X", r, g, b);
    gfx->current_color[7] = '\0';
    
    return 0;
}

static int fill_ellipse(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_pdlua_gfx *gfx = &obj->gfx;
    t_canvas *cnv = glist_getcanvas(obj->canvas);
    
    int x1, y1, x2, y2;
    get_bounds_args(L, obj, gfx, &x1, &y1, &x2, &y2);
    
    const char* tags[] = { gfx->object_tag, register_drawing(obj) };
    
    pdgui_vmess(0, "crr iiii rs ri rS", cnv, "create", "oval", x1, y1, x2, y2, "-fill", gfx->current_color, "-width", 0, "-tags", 2, tags);
        
    return 0;
}

static int stroke_ellipse(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_pdlua_gfx *gfx = &obj->gfx;
    t_canvas *cnv = glist_getcanvas(obj->canvas);

    int x1, y1, x2, y2;
    get_bounds_args(L, obj, gfx, &x1, &y1, &x2, &y2);
    
    const char* tags[] = { gfx->object_tag, register_drawing(obj) };
    
    pdgui_vmess(0, "crr iiii rs rS", cnv, "create", "oval", x1, y1, x2, y2, "-outline", gfx->current_color, "-tags", 2, tags);
        
    return 0;
}

static int fill_all(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_pdlua_gfx *gfx = &obj->gfx;
    t_canvas *cnv = glist_getcanvas(obj->canvas);
    
    int x1 = text_xpix(obj, cnv);
    int y1 = text_ypix(obj, cnv);
    int x2 = x1 + gfx->width * glist_getzoom(cnv);
    int y2 = y1 + gfx->height * glist_getzoom(cnv);
    
    const char* tags[] =  { gfx->object_tag, register_drawing(obj) };
    
    pdgui_vmess(0, "crr iiii rs rS", cnv, "create", "rectangle", x1, y1, x2, y2, "-fill", gfx->current_color, "-tags", 2, tags);
    
    return 0;
}

static int fill_rect(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_pdlua_gfx *gfx = &obj->gfx;
    t_canvas *cnv = glist_getcanvas(obj->canvas);
    
    int x1, y1, x2, y2;
    get_bounds_args(L, obj, gfx, &x1, &y1, &x2, &y2);
    
    const char* tags[] = { gfx->object_tag, register_drawing(obj) };
    
    pdgui_vmess(0, "crr iiii rs ri rS", cnv, "create", "rectangle", x1, y1, x2, y2, "-fill", gfx->current_color, "-width", 0, "-tags", 2, tags);
    
    return 0;
}

static int stroke_rect(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_pdlua_gfx *gfx = &obj->gfx;
    t_canvas *cnv = glist_getcanvas(obj->canvas);
    
    int x1, y1, x2, y2;
    get_bounds_args(L, obj, gfx, &x1, &y1, &x2, &y2);

    const char* tags[] = { gfx->object_tag, register_drawing(obj) };

    pdgui_vmess(0, "crr iiii rs rS", cnv, "create", "rectangle", x1, y1, x2, y2, "-outline", gfx->current_color, "-tags", 2, tags);
    
    return 0;
}

static int fill_rounded_rect(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_pdlua_gfx *gfx = &obj->gfx;
    t_canvas *cnv = glist_getcanvas(obj->canvas);
    
    int x1, y1, x2, y2;
    get_bounds_args(L, obj, gfx, &x1, &y1, &x2, &y2);

    int radius = luaL_checknumber(L, 5);  // Radius for rounded corners
    int radius_x = radius * gfx->scale_x * glist_getzoom(cnv);
    int radius_y = radius * gfx->scale_y * glist_getzoom(cnv);
    
    int width = x2 - x1;
    int height = y2 - y1;
    
    const char* tags[] = { gfx->object_tag, register_drawing(obj) };

    // Tcl/tk can't fill rounded rectangles, so we draw 2 smaller rectangles with 4 ovals over the corners
    pdgui_vmess(0, "crr iiii ri rs rS", cnv, "create", "oval", x1, y1, x1 + radius_x * 2, y1 + radius_y * 2, "-width", 0, "-fill", gfx->current_color, "-tags", 2, tags);
    pdgui_vmess(0, "crr iiii ri rs rS", cnv, "create", "oval", x2 - radius_x * 2 , y1, x2, y1 + radius_y * 2, "-width", 0, "-fill", gfx->current_color, "-tags", 2, tags);
    pdgui_vmess(0, "crr iiii ri rs rS", cnv, "create", "oval", x1, y2 - radius_y * 2, x1 + radius_x * 2, y2, "-width", 0, "-fill", gfx->current_color, "-tags", 2, tags);
    pdgui_vmess(0, "crr iiii ri rs rS", cnv, "create", "oval", x2 - radius_x * 2, y2 - radius_y * 2, x2, y2, "-width", 0, "-fill", gfx->current_color, "-tags", 2, tags);
    pdgui_vmess(0, "crr iiii ri rs rS", cnv, "create", "rectangle", x1 + radius_x, y1, x2 - radius_x, y2, "-width", 0, "-fill", gfx->current_color, "-tag", 2, tags);
    pdgui_vmess(0, "crr iiii ri rs rS", cnv, "create", "rectangle", x1, y1 + radius_y, x2, y2 - radius_y, "-width", 0, "-fill", gfx->current_color, "-tags", 2, tags);

    return 0;
}

static int stroke_rounded_rect(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_pdlua_gfx *gfx = &obj->gfx;
    t_canvas *cnv = glist_getcanvas(obj->canvas);
    
    int x1, y1, x2, y2;
    get_bounds_args(L, obj, gfx, &x1, &y1, &x2, &y2);
    
    int radius = luaL_checknumber(L, 5);       // Radius for rounded corners
    int radius_x = radius * gfx->scale_x * glist_getzoom(cnv);
    int radius_y = radius * gfx->scale_y * glist_getzoom(cnv);
    
    int width = x2 - x1;
    int height = y2 - y1;
    
    const char* tags[] = { gfx->object_tag, register_drawing(obj) };
    
    // Tcl/tk can't stroke rounded rectangles either, so we draw 2 lines connecting with 4 arcs at the corners
    pdgui_vmess(0, "crr iiii ri ri ri ri rs rs rS", cnv, "create", "arc", x1, y1 + radius_y*2, x1 + radius_x*2, y1,
                "-start", 0, "-extent", 90, "-width", 1, "-start", 90, "-outline", gfx->current_color, "-style", "arc", "-tags", 2, tags);
    pdgui_vmess(0, "crr iiii ri ri ri ri rs rs rS", cnv, "create", "arc", x2 - radius_x*2, y1, x2, y1 + radius_y*2,
                "-start", 270, "-extent", 90, "-width", 1, "-start", 0, "-outline", gfx->current_color, "-style", "arc", "-tags", 2, tags);
    pdgui_vmess(0, "crr iiii ri ri ri ri rs rs rS", cnv, "create", "arc", x1, y2 - radius_y*2, x1 + radius_x*2, y2,
                "-start", 180, "-extent", 90, "-width", 1, "-start", 180, "-outline", gfx->current_color, "-style", "arc", "-tags", 2, tags);
    pdgui_vmess(0, "crr iiii ri ri ri ri rs rs rS", cnv, "create", "arc", x2 - radius_x*2, y2, x2, y2 - radius_y*2,
                "-start", 90, "-extent", 90, "-width", 1, "-start", 270, "-outline", gfx->current_color, "-style", "arc", "-tags", 2, tags);
    
    // Connect with lines
    pdgui_vmess(0, "crr iiii ri rs rS", cnv, "create", "line", x1 + radius_x, y1, x2 - radius_x, y1,
                "-width", 1, "-fill", gfx->current_color, "-tags", 2, tags);
    pdgui_vmess(0, "crr iiii ri rs rS", cnv, "create", "line", x1 + radius_y, y2, x1 + width - radius_y, y2,
                "-width", 1,  "-fill", gfx->current_color, "-tags", 2, tags);
    pdgui_vmess(0, "crr iiii ri rs rS", cnv, "create", "line", x1 , y1 + radius_y, x1, y2 - radius_y,
                "-width", 1, "-fill", gfx->current_color, "-tags", 2, tags);
    pdgui_vmess(0, "crr iiii ri rs rS", cnv, "create", "line", x2 , y1 + radius_y, x2, y2 - radius_y,
                "-width", 1,  "-fill", gfx->current_color, "-tags", 2, tags);
    
    return 0;
}

static int draw_text(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_pdlua_gfx *gfx = &obj->gfx;
    t_canvas *cnv = glist_getcanvas(obj->canvas);
    
    const char* text = luaL_checkstring(L, 1); // Assuming text is a string
    int x = luaL_checknumber(L, 2);
    int y = luaL_checknumber(L, 3);
    int w = luaL_checknumber(L, 4);
    int fontHeight = luaL_checknumber(L, 5);
    
    // Calculate the clipped rectangle
    x = min(max(x, 0), gfx->width - w);
    y = min(max(y, 0), gfx->height);
    w = min(x + w, gfx->width) - x;
    
    int zoom = glist_getzoom(cnv);
    x *= gfx->scale_x * zoom;
    y *= gfx->scale_y * zoom;
    w *= gfx->scale_x * zoom;
    
    x += gfx->translate_x;
    y += gfx->translate_y;
    
    x += text_xpix(obj, cnv);
    y += text_ypix(obj, cnv);
    
    // Font size and position are offset to make sure they match drawing in plugdata
    fontHeight *= 0.8f;
    
    const char* tags[] = { gfx->object_tag, register_drawing(obj) };
    
    pdgui_vmess(0, "crr ii rs ri rs rS", cnv, "create", "text",
                0, 0, "-anchor", "nw", "-width", w, "-text", text, "-tags", 2, tags);

    t_atom fontatoms[3];
    SETSYMBOL(fontatoms+0, gensym(sys_font));
    SETFLOAT (fontatoms+1, fontHeight * zoom * gfx->scale_y);
    SETSYMBOL(fontatoms+2, gensym(sys_fontweight));

    pdgui_vmess(0, "crs rA rs rs", cnv, "itemconfigure", tags[1],
            "-font", 3, fontatoms,
            "-fill", gfx->current_color,
            "-justify", "left");
    
    pdgui_vmess(0, "crs ii", cnv, "coords", tags[1], x + 18, y);
    
    return 0;
}

static int start_path(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_pdlua_gfx *gfx = &obj->gfx;
    
    gfx->num_path_segments = 0;
    gfx->path_start_x = luaL_checknumber(L, 1);
    gfx->path_start_y = luaL_checknumber(L, 2);

    gfx->path_segments[gfx->num_path_segments][0] = gfx->path_start_x;
    gfx->path_segments[gfx->num_path_segments][1] = gfx->path_start_y;
    gfx->num_path_segments++;
    return 0;
}

// Function to add a line to the current path
static int line_to(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_pdlua_gfx *gfx = &obj->gfx;
    
    if(gfx->num_path_segments >= 4096)  {
        luaL_error(L, "Too many path segments");
        return 0;
    }
    
    int x = luaL_checknumber(L, 1);
    int y = luaL_checknumber(L, 2);

    gfx->path_segments[gfx->num_path_segments][0] = x;
    gfx->path_segments[gfx->num_path_segments][1] = y;
    gfx->num_path_segments++;
    
    return 0;
}

static int quad_to(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_pdlua_gfx *gfx = &obj->gfx;
    
    int x2 = luaL_checknumber(L, 1);
    int y2 = luaL_checknumber(L, 2);
    int x3 = luaL_checknumber(L, 3);
    int y3 = luaL_checknumber(L, 4);
    
    int x1 = gfx->num_path_segments > 0 ? gfx->path_segments[gfx->num_path_segments - 1][0] : x2;
    int y1 = gfx->num_path_segments > 0 ? gfx->path_segments[gfx->num_path_segments - 1][1] : y2;
    
    // Get the last point
    float t = 0.0;
    const float resolution = 1000;
    while (t <= 1.0) {
        t += 1.0 / resolution;
        if (gfx->num_path_segments < 4096) {
            // Calculate quadratic bezier curve as points (source: https://en.wikipedia.org/wiki/B%C3%A9zier_curve)
            int x = (1.0f - t) * (1.0f - t) * x1 + 2.0f * (1.0f - t) * t * x2 + t * t * x3;
            int y = (1.0f - t) * (1.0f - t) * y1 + 2.0f * (1.0f - t) * t * y2 + t * t * y3;
            
            gfx->path_segments[gfx->num_path_segments][0] = x;
            gfx->path_segments[gfx->num_path_segments][1] = y;
            gfx->num_path_segments++;
        } else {
            luaL_error(L, "Too many path segments");
            return 0;
        }
    }
    
    return 0;
}
static int cubic_to(lua_State* L) {
  t_pdlua* obj = get_current_object(L);
  t_pdlua_gfx *gfx = &obj->gfx;

    int x2 = luaL_checknumber(L, 1);
    int y2 = luaL_checknumber(L, 2);
    int x3 = luaL_checknumber(L, 3);
    int y3 = luaL_checknumber(L, 4);
    int x4 = luaL_checknumber(L, 5);
    int y4 = luaL_checknumber(L, 6);
    
    int x1 = gfx->num_path_segments > 0 ? gfx->path_segments[gfx->num_path_segments - 1][0] : x2;
    int y1 = gfx->num_path_segments > 0 ? gfx->path_segments[gfx->num_path_segments - 1][1] : y2;
   
    // Get the last point
    float t = 0.0;
    const float resolution = 1000;
    while (t <= 1.0) {
        t += 1.0 / resolution;
        if (gfx->num_path_segments < 4096) {
            
            int x = (1 - t)*(1 - t)*(1 - t) * x1 + 3 * (1 - t)*(1 - t) * t * x2 + 3 * (1 - t) * t*t * x3 + t*t*t * x4;
            int y = (1 - t)*(1 - t)*(1 - t) * y1 + 3 * (1 - t)*(1 - t) * t * y2 + 3 * (1 - t) * t*t * y3 + t*t*t * y4;
            
            // Calculate bezier curve as points (source: https://en.wikipedia.org/wiki/B%C3%A9zier_curve)
            gfx->path_segments[gfx->num_path_segments][0] = x;
            gfx->path_segments[gfx->num_path_segments][1] = y;
            gfx->num_path_segments++;
        } else {
            luaL_error(L, "Too many path segments");
            return 0;
        }
    }
    
    return 0;
}

// Function to close the current path
static int close_path(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_pdlua_gfx *gfx = &obj->gfx;
    
    if(gfx->num_path_segments >= 4096) return 0;

    gfx->path_segments[gfx->num_path_segments][0] = gfx->path_start_x;
    gfx->path_segments[gfx->num_path_segments][1] = gfx->path_start_y;
    gfx->num_path_segments++;
    
    return 0;
}

static int stroke_path(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_pdlua_gfx *gfx = &obj->gfx;
    t_canvas *cnv = glist_getcanvas(obj->canvas);
    
    int stroke_width = luaL_checknumber(L, 1) * glist_getzoom(cnv);
    
    // Apply transformations to all coordinates
    int obj_x = text_xpix(obj, cnv);
    int obj_y = text_ypix(obj, cnv);
    for (int i = 0; i < gfx->num_path_segments; i++) {
        int x = gfx->path_segments[i][0], y = gfx->path_segments[i][1];
        
        x *= gfx->scale_x * glist_getzoom(cnv);
        y *= gfx->scale_y * glist_getzoom(cnv);
        
        x += gfx->translate_x + obj_x;
        y += gfx->translate_y + obj_y;
        
        gfx->path_segments[i][0] = x;
        gfx->path_segments[i][1] = y;
    }
    
    int totalSize = 0;
    // Determine the total size needed
    for (int i = 0; i < gfx->num_path_segments; i++) {
        int x = gfx->path_segments[i][0], y = gfx->path_segments[i][1];
        // Calculate size for x and y
        totalSize += snprintf(NULL, 0, "%i %i ", x, y);
    }
    char *coordinates = (char*)getbytes(totalSize + 1); // +1 for null terminator

    int offset = 0;
    for (int i = 0; i < gfx->num_path_segments; i++) {
        int x = gfx->path_segments[i][0], y = gfx->path_segments[i][1];
        int charsWritten = snprintf(coordinates + offset, totalSize - offset, "%i %i ",  x, y);
        if (charsWritten >= 0) {
            offset += charsWritten;
        } else {
            break;
        }
    }
    // Replace the trailing space with string terminator
    if (offset > 0) {
        coordinates[offset - 1] = '\0';
    }
    
    const char* tags[] = { gfx->object_tag, register_drawing(obj) };

    pdgui_vmess(0, "crr r ri rs rs rS", cnv, "create", "polygon", coordinates, "-width", stroke_width, "-outline", gfx->current_color, "-fill", "", "-tags", 2, tags);
    
    freebytes(coordinates, totalSize+1);
    
    return 0;
}

static int fill_path(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_pdlua_gfx *gfx = &obj->gfx;
    t_canvas *cnv = glist_getcanvas(obj->canvas);

    // Apply transformations to all coordinates
    int obj_x = text_xpix(obj, cnv);
    int obj_y = text_ypix(obj, cnv);
    for (int i = 0; i < gfx->num_path_segments; i++) {
        int x = gfx->path_segments[i][0], y = gfx->path_segments[i][1];
        
        x *= gfx->scale_x * glist_getzoom(cnv);
        y *= gfx->scale_y * glist_getzoom(cnv);
        
        x += gfx->translate_x + obj_x;
        y += gfx->translate_y + obj_y;
        
        gfx->path_segments[i][0] = x;
        gfx->path_segments[i][1] = y;
    }
    
    int totalSize = 0;
    // Determine the total size needed
    for (int i = 0; i < gfx->num_path_segments; i++) {
        int x = gfx->path_segments[i][0], y = gfx->path_segments[i][1];
        // Calculate size for x and y
        totalSize += snprintf(NULL, 0, "%i %i ", x, y);
    }
    char *coordinates = (char*)getbytes(totalSize + 1); // +1 for null terminator
    
    int offset = 0;
    for (int i = 0; i < gfx->num_path_segments; i++) {
        int x = gfx->path_segments[i][0], y = gfx->path_segments[i][1];
        int charsWritten = snprintf(coordinates + offset, totalSize - offset, "%i %i ",  x, y);
        if (charsWritten >= 0) {
            offset += charsWritten;
        } else {
            break;
        }
    }
    
    // Remove the trailing space
    if (offset > 0) {
        coordinates[offset - 1] = '\0';
    }
    
    const char* tags[] = { gfx->object_tag, register_drawing(obj) };

    pdgui_vmess(0, "crr r ri rs rS", cnv, "create", "polygon", coordinates, "-width", 0, "-fill", gfx->current_color, "-tags", 2, tags);
    
    freebytes(coordinates, totalSize+1);
    
    return 0;
}


static int translate(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_pdlua_gfx *gfx = &obj->gfx;
    gfx->translate_x = luaL_checknumber(L, 1);
    gfx->translate_y = luaL_checknumber(L, 2);
    return 0;
}

static int scale(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_pdlua_gfx *gfx = &obj->gfx;
    gfx->scale_x = luaL_checknumber(L, 1);
    gfx->scale_y = luaL_checknumber(L, 2);
    return 0;
}


static int reset_transform(lua_State* L) {
    t_pdlua* obj = get_current_object(L);
    t_pdlua_gfx *gfx = &obj->gfx;
    gfx->translate_x = 0;
    gfx->translate_y = 0;
    gfx->scale_x = 1.0f;
    gfx->scale_y = 1.0f;
    return 0;
}
#endif
