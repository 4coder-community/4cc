/*
 * Standard types accross modules.
 */

// TOP

#ifndef CD_STANDARD_TYPES_H
#define CD_STANDARD_TYPES_H

union vec2{
    struct{
        f32 v[2];
    };
    struct{
        f32 x,y;
    };
};

union vec3{
    struct{
        f32 v[3];
    };
    struct{
        f32 r,g,b;
    };
    struct{
        f32 x,y,z;
    };
    struct{
        vec2 p;
    };
};

union vec4{
    struct{
        f32 v[4];
    };
    struct{
        f32 r,g,b,a;
    };
    struct{
        f32 x,y,z,w;
    };
    struct{
        f32 x0,y0,x1,y1;
    };
    struct{
        vec2 p[2];
    };
    struct{
        vec3 xyz;
    };
    struct{
        vec3 rgb;
    };
};

inline vec2
v2(f32 x, f32 y){
    vec2 result;
    result.x = x;
    result.y = y;
    return(result);
}

inline vec3
v3(f32 x, f32 y, f32 z){
    vec3 result;
    result.x = x;
    result.y = y;
    result.z = z;
    return(result);
}

inline vec4
v4(f32 x, f32 y, f32 z, f32 w){
    vec4 result;
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    return(result);
}

inline vec4
v4(vec3 xyz, f32 w){
    vec4 result;
    result.xyz = xyz;
    result.w = w;
    return(result);
}

inline vec2
operator+(vec2 a, vec2 b){
    a.x += b.x;
    a.y += b.y;
    return(a);
}

inline vec2
operator-(vec2 a, vec2 b){
    a.x -= b.x;
    a.y -= b.y;
    return(a);
}

inline vec2
operator-(vec2 a){
    a.x = -a.x;
    a.y = -a.y;
    return(a);
}

inline vec2
operator*(vec2 a, f32 k){
    a.x *= k;
    a.y *= k;
    return(a);
}

typedef vec4 f32_rect;

inline f32_rect
intersect(f32_rect a, f32_rect b){
    f32_rect result;
    result.x0 = Max(a.x0, b.x0);
    result.y0 = Max(a.y0, b.y0);
    result.x1 = Min(a.x1, b.x1);
    result.y1 = Min(a.y1, b.y1);
    return(result);
}

inline f32_rect
containing(f32_rect a, f32_rect b){
    f32_rect result;
    result.x0 = Min(a.x0, b.x0);
    result.y0 = Min(a.y0, b.y0);
    result.x1 = Max(a.x1, b.x1);
    result.y1 = Max(a.y1, b.y1);
    return(result);
}

inline b32
hit_check(i32 x, i32 y, f32_rect r){
    b32 result = 0;
    if (x >= r.x0 && x <= r.x1 && y >= r.y0 && y <= r.y1){
        result = 1;
    }
    return(result);
}

static f32 pi32 = 3.1415926f;

struct Basis{
    vec2 x, y;
};

inline Basis
get_rotation(f32 theta){
    Basis result = {0};
    result.x = v2(COS(theta), SIN(theta));
    result.y = v2(-SIN(theta), COS(theta));
    return(result);
}

inline Basis
invert_basis(Basis b){
    f32 det;
    Basis r;
    
    det = 1.f/(b.x.x*b.y.y - b.x.y*b.y.x);
    r.x = v2(b.y.y, -b.x.y) * det;
    r.y = v2(-b.y.x, b.x.x) * det;
    
    return(r);
}

inline vec2
do_basis(vec2 p, Basis b){
    vec2 q;
    q.x = p.x*b.x.x + p.y*b.y.x;
    q.y = p.x*b.x.y + p.y*b.y.y;
    return(q);
}

inline Basis
basis_basis(Basis p, Basis b){
    Basis r;
    r.x = do_basis(p.x, b);
    r.y = do_basis(p.y, b);
    return(r);
}

struct Transform{
    Basis basis;
    vec2 pos;
};

inline Transform
invert_transform(Transform t){
    Transform r;
    r.basis = invert_basis(t.basis);
    r.pos = do_basis(-t.pos, t.basis);
    return(r);
}

inline vec2
do_transform(vec2 p, Transform t){
    vec2 q;
    q = p + t.pos;
    p.x = q.x*t.basis.x.x + q.y*t.basis.y.x;
    p.y = q.x*t.basis.x.y + q.y*t.basis.y.y;
    return(p);
}

inline Transform
transform_transform(Transform p, Transform t){
    Transform r;
    r.basis = basis_basis(p.basis, t.basis);
    r.pos = p.pos + do_basis(t.pos, invert_basis(p.basis));
    return(r);
}

static Transform identity_transform = {
    {v2(1.f, 0.f), v2(0.f, 1.f)}, v2(0.f, 0.f)
};

#endif

// BOTTOM

