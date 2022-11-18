/*
 *  Overreact - Mr. 4th Dimention
 *      Allen Webster
 *  03.21.2015(mm.dd.yyyy)
 *
 * Math utilities.
 */

/* VECTORS */

struct Vec2{
	union{
		struct{
			real32 x, y;
		};
		real32 v[2];
	};
};

struct Vec3{
	union{
		struct{
			real32 x, y, z;
		};
		struct{
			real32 r, g, b;
		};
		real32 v[3];
	};
};

struct Vec4{
	union{
		struct{
			real32 x, y, z, w;
		};
		struct{
			real32 r, g, b, a;
		};
		real32 v[4];
	};
};

/* CONTRUCTOR FUNCTIONS */

internal Vec2
V2(real32 x, real32 y){
	return {x, y};
}

internal Vec3
V3(real32 x, real32 y, real32 z){
	return {x, y, z};
}

internal Vec4
V4(real32 x, real32 y, real32 z, real32 w){
	return {x, y, z, w};
}

/* ADDITION */

inline internal Vec2
operator+(Vec2 a, Vec2 b){
	a = V2(a.x + b.x, a.y + b.y);
	return a;
}

inline internal Vec3
operator+(Vec3 a, Vec3 b){
	a = V3(a.x + b.x, a.y + b.y, a.z + b.z);
	return a;
}

inline internal Vec4
operator+(Vec4 a, Vec4 b){
	a = V4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
	return a;
}

inline internal Vec2&
operator+=(Vec2 &a, Vec2 b){
	a = V2(a.x + b.x, a.y + b.y);
	return a;
}

inline internal Vec3&
operator+=(Vec3 &a, Vec3 b){
	a = V3(a.x + b.x, a.y + b.y, a.z + b.z);
	return a;
}

inline internal Vec4&
operator+=(Vec4 &a, Vec4 b){
	a = V4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
	return a;
}

/* SUBTRACTION */

inline internal Vec2
operator-(Vec2 a, Vec2 b){
	a = V2(a.x - b.x, a.y - b.y);
	return a;
}

inline internal Vec3
operator-(Vec3 a, Vec3 b){
	a = V3(a.x - b.x, a.y - b.y, a.z - b.z);
	return a;
}

inline internal Vec4
operator-(Vec4 a, Vec4 b){
	a = V4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
	return a;
}

inline internal Vec2&
operator-=(Vec2 &a, Vec2 b){
	a = V2(a.x - b.x, a.y - b.y);
	return a;
}

inline internal Vec3&
operator-=(Vec3 &a, Vec3 b){
	a = V3(a.x - b.x, a.y - b.y, a.z - b.z);
	return a;
}

inline internal Vec4&
operator-=(Vec4 &a, Vec4 b){
	a = V4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
	return a;
}

/* MULTIPLICATION */

inline internal Vec2
operator*(Vec2 a, real32 k){
	a = V2(a.x * k, a.y * k);
	return a;
}

inline internal Vec3
operator*(Vec3 a, real32 k){
	a = V3(a.x * k, a.y * k, a.z * k);
	return a;
}

inline internal Vec4
operator*(Vec4 a, real32 k){
	a = V4(a.x * k, a.y * k, a.z * k, a.w * k);
	return a;
}

inline internal Vec2
operator*(real32 k, Vec2 a){
	a = V2(a.x * k, a.y * k);
	return a;
}

inline internal Vec3
operator*(real32 k, Vec3 a){
	a = V3(a.x * k, a.y * k, a.z * k);
	return a;
}

inline internal Vec4
operator*(real32 k, Vec4 a){
	a = V4(a.x * k, a.y * k, a.z * k, a.w * k);
	return a;
}

inline internal Vec2&
operator*=(Vec2& a, real32 k){
	a = V2(a.x * k, a.y * k);
	return a;
}

inline internal Vec3&
operator*=(Vec3& a, real32 k){
	a = V3(a.x * k, a.y * k, a.z * k);
	return a;
}

inline internal Vec4&
operator*=(Vec4& a, real32 k){
	a = V4(a.x * k, a.y * k, a.z * k, a.w * k);
	return a;
}

/* VECTOR VECTOR PRODUCTS */

inline internal real32
dot(Vec2 a, Vec2 b){
	real32 result = a.x*b.x + a.y*b.y;
	return result;
}

inline internal real32
dot(Vec3 a, Vec3 b){
	real32 result = a.x*b.x + a.y*b.y + a.z*b.z;
	return result;
}

inline internal real32
dot(Vec4 a, Vec4 b){
	real32 result = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
	return result;
}

inline internal Vec3
cross(Vec3 a, Vec3 b){
	a = V3(a.y*b.z - a.z*b.y,
		   a.z*b.x - a.x*b.z,
		   a.x*b.y - a.y*b.x);
	return a;
}

inline internal Vec2
hadamard(Vec2 a, Vec2 b){
	a = V2(a.x*b.x, a.y*b.y);
	return a;
}

inline internal Vec3
hadamard(Vec3 a, Vec3 b){
	a = V3(a.x*b.x, a.y*b.y, a.z*b.z);
	return a;
}

inline internal Vec4
hadamard(Vec4 a, Vec4 b){
	a = V4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);
	return a;
}

/* OTHER VECTOR OPERATIONS */
inline internal Vec2
project(Vec2 v, Vec2 normalized_p){
	return dot(v, normalized_p)*normalized_p;
}

/* LERPS */

inline internal real32
lerp(real32 a, real32 t, real32 b){
	real32 result = a + (b - a)*t;
	return result;
}

inline internal Vec2
lerp(Vec2 a, real32 t, Vec2 b){
	Vec2 result = a + (b - a)*t;
	return result;
}

inline internal Vec3
lerp(Vec3 a, real32 t, Vec3 b){
	Vec3 result = a + (b - a)*t;
	return result;
}

inline internal Vec4
lerp(Vec4 a, real32 t, Vec4 b){
	Vec4 result = a + (b - a)*t;
	return result;
}

/* COLOR */

internal Vec3
unpack_color3(u32 color){
    Vec3 result;
    result.r = ((color >> 16) & 0xFF) / 255.f;
    result.g = ((color >> 8) & 0xFF) / 255.f;
    result.b = ((color >> 0) & 0xFF) / 255.f;
    return result;
}

internal Vec4
unpack_color4(u32 color){
    Vec4 result;
    result.a = ((color >> 24) & 0xFF) / 255.f;
    result.r = ((color >> 16) & 0xFF) / 255.f;
    result.g = ((color >> 8) & 0xFF) / 255.f;
    result.b = ((color >> 0) & 0xFF) / 255.f;
    return result;
}

/* RANDOM */

struct pcg32_random{
    u64 state;
    u64 inc;
};

// STUDY(allen): LEARN THIS
u32 pcg32_random_r(pcg32_random* rng)
{
    u64 oldstate = rng->state;
    // Advance internal state
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc|1);
    // Calculate output function (XSH RR), uses old state for max ILP
    u32 xorshifted = (u32)(((oldstate >> 18u) ^ oldstate) >> 27u);
    u32 rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

/* GRID MATH */

struct Grid_Pos{
    i32 x, y;
};

Grid_Pos von_neumann[] = {
    {-1, 0},
    {0, -1},
    {1, 0},
    {0, 1}
};

Grid_Pos moore[] = {
    {-1, -1},
    {0, -1},
    {1, -1},
    {-1, 0},
    {1, 0},
    {-1, 1},
    {0, 1},
    {1, 1}
};

Grid_Pos operator+(Grid_Pos a, Grid_Pos b){
    Grid_Pos result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

/* ROUNDING */

inline internal i32
round_up_POT(i32 x){
    i32 b = 1;
    while (b < x){
        b <<= 1;
    }
    return b;
}


