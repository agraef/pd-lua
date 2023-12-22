 /**
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 **/

#include "m_pd.h"

/** Global Lua interpreter state, needed in the constructor. */
static lua_State *__L;

typedef struct _pdlua_gfx
{
#if !PLUGDATA
    char object_tag[128]; // Tcl/tk tag that is attached to all drawings
    char current_paint_tag[128]; // Tcl/tk tag that is only attached to the current drawing in progress
    
    // Variables for managing vector paths
    int* path_segments;
    int num_path_segments;
    int num_path_segments_allocated;
    int path_start_x, path_start_y;
    
    // Variables to keep track of mouse button state and drag position
    int mouse_drag_x, mouse_drag_y, mouse_down;
    
    // Variables for managing transforms
    int translate_x, translate_y;
    float scale_x, scale_y;
    
    char current_color[8]; // Keep track of current color
#else
    void(*plugdata_draw_callback)(void*, t_symbol*, int, t_atom*); // Callback to perform drawing in plugdata
    void* plugdata_callback_target; // Pointer to plugdata object, the target for our draw callback
#endif
    
    // Size variables
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


