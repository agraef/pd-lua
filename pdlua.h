#include "m_pd.h"

/** Global Lua interpreter state, needed in the constructor. */
static lua_State *__L;

typedef struct _pdlua_gfx
{
#if !PLUGDATA
    char object_tag[128];
    char active_tags[128][128];
    int num_tags;
    
    int path_segments[4096][2];
    int num_path_segments;
    int path_start_x, path_start_y;
    
    int mouse_x, mouse_y, mouse_up;
    int translate_x, translate_y;
    float scale_x, scale_y;
    char current_color[8];
    
    t_symbol* mouse_proxy_sym;
    void* mouse_proxy;
#else
    void(*plugdata_draw_callback)(void*, t_symbol*, int, t_atom*);
    void* plugdata_callback_target;
#endif
    int width, height;
} t_pdlua_gfx;

/** Pd object data. */
typedef struct pdlua 
{
    t_object                pd; /**< We are a Pd object. */
    int                     inlets; /**< Number of inlets. */
    struct pdlua_proxyinlet *in; /**< The inlets themselves. */
    int                     outlets; /**< Number of outlets. */
    t_outlet                **out; /**< The outlets themselves. */
    t_canvas                *canvas; /**< The canvas that the object was created on. */
    int                     has_gui;  /**< True if graphics are enabled. */
    t_pdlua_gfx             gfx;      /**< Holds state for graphics. */
} t_pdlua;


