//
//  main.c
//  graphics_osx_lua_test
//
//  Created by Rory B. Bellows on 19/02/2018.
//  Copyright © 2018 Rory B. Bellows. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "../../../graphics.h"

#define SURFACE "Surface"

int main(int argc, const char* argv[]) {
  lua_State* L = luaL_newstate();
  luaL_openlibs(L);
  
  luaL_dofile(L, "/Users/roryb/Documents/git/graphics/projects/graphics_osx_lua_test/graphics_osx_lua_test/test.lua");
  
  lua_close(L);
  return 0;
}
