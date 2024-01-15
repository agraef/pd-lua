#include <map>

extern "C"
{

#include <m_pd.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>


#ifdef PDINSTANCE
std::map<void*, lua_State*> lua_state;

lua_State* __L()
{
    return lua_state[pd_this];
}

void initialise_lua_state()
{
    lua_state[pd_this] = luaL_newstate();
}
#else

static lua_State *__lua_state;

lua_State* __L()
{
    return __lua_state;
}

void initialise_lua_state()
{
    __lua_state = luaL_newstate();
}

#endif

}


