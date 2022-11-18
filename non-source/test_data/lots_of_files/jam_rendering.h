/*
 *  Overreact - Mr. 4th Dimention
 *      Allen Webster
 *  03.21.2015 (mm.dd.yyyy)
 *
 * Rendering Layer.
 */

#if RENDER_MODE == OPENGL

struct Render_Texture{
	u32 texid;
	i32 width, height;
    i32 img_width, img_height;
    real32 tex_x, tex_y;
};

#endif
