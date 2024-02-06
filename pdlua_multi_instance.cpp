#include <map>

extern "C"
{

#include <m_pd.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>


static inline lua_State *__lua_state = nullptr;

#ifdef PDINSTANCE

std::map<void*, lua_State*> lua_threads;

lua_State* __L()
{
    return lua_threads[pd_this];
}

void initialise_lua_state()
{
    if(!__lua_state) {
        __lua_state = luaL_newstate();
        lua_threads[pd_this] = __lua_state;
    }
    else {
        lua_threads[pd_this] = lua_newthread(__lua_state);
    }
}
#else

lua_State* __L()
{
    return __lua_state;
}

void initialise_lua_state()
{
    if(!__lua_state) __lua_state = luaL_newstate();
}

#endif

}


