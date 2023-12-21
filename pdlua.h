#include "m_pd.h"

/** Global Lua interpreter state, needed in the constructor. */
static lua_State *__L;

typedef struct _pdlua_gfx
{
#if !PLUGDATA
    char object_tag[128];
    char current_paint_tag[128];
    
    int* path_segments;
    int num_path_segments;
    int num_path_segments_allocated;
    int path_start_x, path_start_y;
    
    int mouse_drag_x, mouse_drag_y, mouse_down;
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


