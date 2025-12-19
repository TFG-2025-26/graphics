cmake_minimum_required(VERSION 3.5)

project(LuaWrapper LANGUAGES C)

# 1) Busca todos los ficheros .c y .h de la carpeta lua/src
file(GLOB LUA_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/lua/*.c"
)
file(GLOB LUA_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/lua/*.h"
)

# 2) Si NO quieres compilar lua.c y luac.c (los ejecutables),
#    bórralos de la lista (ya que normalmente son el intérprete y compilador).
list(REMOVE_ITEM LUA_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/lua/lua.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/lua/luac.c"
)

# 3) Crear la librería estática (también podrías usar SHARED para .dll)
add_library(lua STATIC ${LUA_SOURCES})

# 4) Añadir la carpeta de includes para que se vea lua.h, etc.
target_include_directories(lua PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/lua"
)

# 5) (Opcional) Definir flags adicionales, o alias
# target_compile_definitions(lua PRIVATE LUA_COMPAT_5_2)
# set_target_properties(lua PROPERTIES OUTPUT_NAME "lua")

# 6) Si quieres, podrías compilar algo más aparte de Lua, 
#    pero lo esencial es que 'lua' ya es un target con la librería.
