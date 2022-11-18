/*
 *  Overreact - Mr. 4th Dimention
 *      Allen Webster
 *  03.21.2015 (mm.dd.yyyy)
 *
 * Rendering System Layer.
 */

#if RENDER_MODE == SOFTWARE

struct Game_Render_Target{
	u8 *pixels;
	i32 width, height, pitch;
};

#elif RENDER_MODE == OPENGL

struct Game_Render_Target{
	i32 width, height;
};

#endif

