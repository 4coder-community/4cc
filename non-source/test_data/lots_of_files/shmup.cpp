
#define AllowLocal(x) ((void)x)

#include "lua.hpp"

typedef struct BufferLoadS{
    char *s;
    int size;
} BufferLoadS;

const char*
buffer_get_s(lua_State *S, void *u, size_t *size){
    const char *result = 0;
    BufferLoadS *load = (BufferLoadS*)u;
    AllowLocal(S);
    if (load->size != 0){
        result = load->s;
        *size = load->size;
        load->size = 0;
    }
    return(result);
}

int
load_buffer(lua_State *S, char *data, int size, char *name, char *mode){
    int result;
    BufferLoadS load;
    load.s = data;
    load.size = size;
    result = lua_load(S, buffer_get_s, &load, name, mode);
    return(result);
}

#include <stdlib.h>
#include <stdio.h>

void*
shmup_lua_alloc(void *ud, void *ptr, size_t osize, size_t nsize){
  // TODO(allen): replace this so we can get away from standard lib!
  (void)ud; (void)osize;  /* not used */
  if (nsize == 0) {
    free(ptr);
    return NULL;
  }
  else
    return realloc(ptr, nsize);
}

int
shmup_lua_panic(lua_State *lua){
    printf("you fuck up you!\n");
    return 0;
}

int
main(){
    lua_State *S;
    S = lua_newstate(shmup_lua_alloc, NULL);
    if (S) lua_atpanic(S, &shmup_lua_panic);
    
    int x = 0, result;
    char buffer[] = "return 1 + 2";
    result = load_buffer(S, buffer, sizeof(buffer) - 1, "test", NULL);
    switch (result){
        case LUA_OK:
        lua_call(S, 0, LUA_MULTRET);
        x = (int)lua_tointeger(S, 0);
        lua_pop(S, 1);
        break;
        
        case LUA_ERRSYNTAX:
        printf("LUA_ERRSYNTAX\n");
        break;
        
        case LUA_ERRMEM:
        printf("LUA_ERRMEM\n");
        break;
        
        case LUA_ERRGCMM:
        printf("LUA_ERRGCMM\n");
        break;
    }
    
    printf("x = %d\n", x);
    
    lua_close(S);
    return(0);
}