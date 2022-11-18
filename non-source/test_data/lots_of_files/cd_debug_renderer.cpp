/*
 * Implementation of render target for debug rendering
 */

// TOP

struct Dbg_Renderer_Vars{
    Partition part;
    b32 initialized;
};

Render_Execute_Sig(dbg_render_execute){
    Dbg_Renderer_Vars *vars = (Dbg_Renderer_Vars*)target->memory;
    vars->part = make_partition(target->memory, target->memory_size);
    
    if (!vars->initialized){
        vars->initialized = true;
    }
    
    Render_Command_Header *h;
    
    for (h = (Render_Command_Header*)target->push_part.base;
        h->type;
        h = NextHeader(h)){

        switch (h->type){
            case render_dbg_parallelagram:
            {
                Render_Dbg_Parallelagram *r =
                    (Render_Dbg_Parallelagram*)(h + 1);
                
                vec2 center = r->transform.pos;
                vec2 half_x = r->transform.basis.x;
                vec2 half_y = r->transform.basis.y;
                vec2 p;
                
                glBindTexture(GL_TEXTURE_2D, 0);
                glColor4f(r->color.r, r->color.g, r->color.b, r->color.a);
                glBegin(GL_QUADS);
                {
                    p = center - half_x + half_y;
                    glVertex2f(p.x, p.y);
                    
                    p = center + half_x + half_y;
                    glVertex2f(p.x, p.y);
                    
                    p = center + half_x - half_y;
                    glVertex2f(p.x, p.y);
                    
                    p = center - half_x - half_y;
                    glVertex2f(p.x, p.y);
                }
                glEnd();
            }break;
            
            case render_dbg_rectangle:
            {
                Render_Dbg_Rectangle *r =
                    (Render_Dbg_Rectangle*)(h + 1);
                
                glBindTexture(GL_TEXTURE_2D, 0);
                glColor4f(r->color.r, r->color.g, r->color.b, r->color.a);
                glBegin(GL_QUADS);
                {
                    glVertex2f(r->rect.x0, r->rect.y0);
                    glVertex2f(r->rect.x1, r->rect.y0);
                    glVertex2f(r->rect.x1, r->rect.y1);
                    glVertex2f(r->rect.x0, r->rect.y1);
                }
                glEnd();
            }break;
            
            case render_image:
            {
                Render_Image *i = (Render_Image*)(h + 1);
                Image *image = get_image(bank, i->image_id);
                if (image){
                    vec2 center = i->transform.pos;
                    vec2 half_x = i->transform.basis.x;
                    vec2 half_y = i->transform.basis.y;
                    f32 s = image->tex_x;
                    f32 t = image->tex_y;
                    
                    center = do_transform(center, target->me_to_pixel_transform);
                    
                    {
                        i32 texture_id = image->texture_id;
                        vec4 white = v4(1.f, 1.f, 1.f, 1.f);
                        
                        glBindTexture(GL_TEXTURE_2D, texture_id);
                        glColor4f(white.r, white.g, white.b, white.a);
                        glBegin(GL_QUADS);
                        {
                            vec2 p;
                            
                            p = center - half_x + half_y;
                            glTexCoord2f(0.f, t);
                            glVertex2f(p.x, p.y);
                            
                            p = center + half_x + half_y;
                            glTexCoord2f(s, t);
                            glVertex2f(p.x, p.y);
                            
                            p = center + half_x - half_y;
                            glTexCoord2f(s, 0.f);
                            glVertex2f(p.x, p.y);
                            
                            p = center - half_x - half_y;
                            glTexCoord2f(0.f, 0.f);
                            glVertex2f(p.x, p.y);
                        }
                        glEnd();
                    }
                }
            }break;
            
            case render_glyph:
            {
                Render_Glyph *g = (Render_Glyph*)(h + 1);
                Font *font = get_font(bank, g->font_id);
                if (font){
                    stbtt_aligned_quad q;
                    f32 y = g->pos.y;
                    
                    g->pos.y = 0;
                    stbtt_GetBakedQuad(font->chardata, font->texture_w, font->texture_h,
                                       g->c, &g->pos.x, &g->pos.y, &q, 1);
                    
                    glBindTexture(GL_TEXTURE_2D, font->texture_id);
                    glColor4f(0.f, 0.f, 0.f, g->color.a);
                    glBegin(GL_QUADS);
                    {
                        glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0 + 1, y - q.y1 - 1);
                        glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1 + 1, y - q.y1 - 1);
                        glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1 + 1, y - q.y0 - 1);
                        glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0 + 1, y - q.y0 - 1);
                    }
                    glEnd();
                    
                    glColor4f(g->color.r, g->color.g, g->color.b, g->color.a);
                    glBegin(GL_QUADS);
                    {
                        glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, y - q.y1);
                        glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, y - q.y1);
                        glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, y - q.y0);
                        glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, y - q.y0);
                    }
                    glEnd();
                }
            }break;
        }

    }
}

// BOTTOM

