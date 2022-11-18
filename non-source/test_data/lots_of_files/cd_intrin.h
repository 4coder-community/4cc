/*
 * Small functions that probably want to be sped up
 * on a per platform basis in the future.
 */

// TOP

// NOTE(allen): GOAL,
// Let's be libc independent someday!
// Then we'll be totally badass.

#include <math.h>
#include <string.h>

f32
COS(f32 x){
    return(cosf(x));
}

f32
SIN(f32 x){
    return(sinf(x));
}

f32
POW(f32 b, f32 e){
    return(powf(b, e));
}

void
cd_memset(void *mem, int val, int size){
    memset(mem, val, size);
}

void
cd_memcpy(void *dst, void *src, int size){
    memcpy(dst, src, size);
}

void
cd_memmove(void *dst, void *src, int size){
    memmove(dst, src, size);
}

int
cd_strlen(char *s){
    char *b = s;
    while (*s) ++s;
    return ((int)(s - b));
}

f32
lerp(f32 a, f32 t, f32 b){
    return(a + (b - a)*t);
}

// TODO(allen): Reduce these equations out
// so that we can do it all in one expression.
f32
slerp(f32 a, f32 t, f32 b){
    f32 a2 = lerp(a, 0.25f, b);
    f32 b2 = lerp(a, 0.75f, b);
    
    f32 f1 = lerp(a, t, a2);
    f32 f2 = lerp(a2, t, b2);
    f32 f3 = lerp(b2, t, b);
    
    f32 g1 = lerp(f1, t, f2);
    f32 g2 = lerp(f2, t, f3);
    
    f32 r = lerp(g1, t, g2);
    
    return(r);
}

f32
clamp(f32 a, f32 t, f32 b){
    if (t < a) t = a;
    else if (t > b) t = b;
    return(t);
}

f32
clamp_bottom(f32 a, f32 t){
    if (t < a) t = a;
    return(t);
}

f32
clamp_top(f32 t, f32 b){
    if (t > b) t = b;
    return(t);
}

// BOTTOM

