/*
 * Implementation of render target for debug rendering
 */

// TOP

#include "cd_random.h"
#include "cd_world.h"

struct Game_Renderer_Vars{
    Partition part;
    b32 initialized;
};

inline void
do_quad(u32 texture_id, vec4 color,
        vec2 center, vec2 half_x, vec2 half_y,
        f32 s, f32 t){
    vec2 p;
    
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_QUADS);
    {
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

Render_Execute_Sig(game_render_execute){
    Game_Renderer_Vars *vars = (Game_Renderer_Vars*)target->memory;
    
    if (!vars->initialized){
        vars->initialized = true;
        vars->part = make_partition(target->memory, target->memory_size);
        push_type(&vars->part, Game_Renderer_Vars);
    }
    
    Transform me_to_pixel = target->me_to_pixel_transform;
    
    Partition *part = &vars->part;
    Temp_Memory temp = begin_temp(part);
    
    f32 *lane_y = push_array(part, f32, total_lanes);
    for (i32 i = 0; i < total_lanes; ++i){
        lane_y[i] = -5000.f;
    }
    
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    Render_Command_Header *h;
    for (h = (Render_Command_Header*)target->push_part.base;
         h->type;
         h = NextHeader(h)){
        switch (h->type){
            case render_lane:
            {
                Render_Lane *l = (Render_Lane*)(h + 1);
                lane_y[l->lane_index] = l->y_position;
            }break;
            
            case render_obstacle:
            {
                Render_Obstacle *o = (Render_Obstacle*)(h + 1);
                Random_Series randgen = random_seed(o->seed);
                Image *image = 0;
                u32 image_id = 0;
                
                f32 theta = random_between(&randgen, 0.0f, 0.3f);
                theta *= theta * o->x;
                theta += random_between(&randgen, 0.f, 6.2f);
                if (random_unilateral(&randgen) > 0.7f){
                    theta *= -1.f;
                }
                
                f32 scale = 0.5f;
                
                switch (o->type){
                    case ET_Obstacle:
                    image_id = ASTEROID1 + random_choice(&randgen, 6);
                    scale = 0.4f;
                    break;
                }
                
                image = get_image(bank, image_id);
                if (image){
                    vec2 center = v2(o->x, lane_y[o->lane]);
                    vec2 half_x = v2(image->width*scale, 0);
                    vec2 half_y = v2(0, image->height*scale);
                    f32 s = image->tex_x;
                    f32 t = image->tex_y;
                    
                    center = do_transform(center, me_to_pixel);
                    
                    Basis rotation = get_rotation(theta);
                    half_x = do_basis(half_x, rotation);
                    half_y = do_basis(half_y, rotation);
                    
                    do_quad(image->texture_id, v4(1.f, 1.f, 1.f, 1.f),
                            center, half_x, half_y, s, t);
                }
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
                    
                    center = do_transform(center, me_to_pixel);
                    
                    do_quad(image->texture_id, v4(1.f, 1.f, 1.f, 1.f),
                            center, half_x, half_y, s, t);
                }
            }break;
        }
    }
    
    end_temp(temp);
    
    // NOTE(allen): In dev mode multiple renderers may
    // be used in each frame so clearing the depth buffer
    // when everything is done keeps everything in order.
#ifdef DEVELOPER
    glClearDepth(1.f);
#endif
}

Render_Get_Functions_Sig(target_get_functions){
    target->execute = game_render_execute;
    target->init = init;
    target->display = display;
    target->set_screen = set_screen;
}

Bank_Get_Functions_Sig(bank_get_functions){
    bank->begin_setup = begin_setup;
    bank->end_setup = end_setup;
    bank->bind_font = bind_font;
    bank->bind_bmp = bind_bmp;
    bank->replace_bmp = replace_bmp;
}

// BOTTOM
