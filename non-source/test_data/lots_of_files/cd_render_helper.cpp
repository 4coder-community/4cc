/*
 * Helper functions for core engine module while rendering.
 */

// TOP

Font*
get_font(Asset_Bank *bank, i32 font_id){
    Font *font = 0;
    if (bank && font_id >= 1 && font_id <= bank->font_max_id){
        font = bank->fonts + font_id - 1;
    }
    return(font);
}

f32*
get_font_advance(Asset_Bank *bank, i32 font_id){
    f32 *advance = 0;
    Font *font = get_font(bank, font_id);
    if (font){
        advance = font->advance;
    }
    return(advance);
}

Image*
get_image(Asset_Bank *bank, i32 image_id){
    Image *result = 0;
    if (bank && image_id >= 1 && image_id <= bank->image_max_id){
        result = bank->images + image_id - 1;
    }
    return(result);
}

enum Tranform_Spec{
    TF_Pixels,
    TF_Units,
};

Transform unit_to_pixel_transform = {
    {v2(pixel_over_unit, 0.f), v2(0.f, pixel_over_unit)}, v2(x_origin_u, y_origin_u)
};

void
target_transform(Render_Target *target, Tranform_Spec t){
    switch (t){
        case TF_Pixels:
        target->transform = invert_transform(target->me_to_pixel_transform);
        break;
        
        case TF_Units:
        target->transform =
            transform_transform(unit_to_pixel_transform, invert_transform(target->me_to_pixel_transform));
        break;
        
        default:
        InvalidCodePath;
    }
}

void
clear_target(Render_Target *target){
    if (target){
        target->push_part.rel_part.pos = 0;
    }
}

void
null_render(Render_Target *target){
    if (target){
        Render_Command_Header *h;
        h = push_type(&target->push_part, Render_Command_Header);
        h->type = render_null;
        h->size = 0;
    }
}

void
do_dbg_rectangle(Render_Target *target, f32_rect rect, vec4 color){
    if (target){
        Render_Command_Header *h;
        Render_Dbg_Rectangle *r;
        
        h = push_type(&target->push_part, Render_Command_Header);
        h->type = render_dbg_rectangle;
        h->size = sizeof(Render_Command_Header) + sizeof(Render_Dbg_Rectangle);
        
        rect.p[0] = do_transform(rect.p[0], target->transform);
        rect.p[1] = do_transform(rect.p[1], target->transform);
        
        r = push_type(&target->push_part, Render_Dbg_Rectangle);
        r->rect = rect;
        r->color = color;
    }
}

void
do_dbg_parallelagram(Render_Target *target, vec2 pos, vec2 x_axis, vec2 y_axis, vec4 color){
    if (target){
        Render_Command_Header *h;
        Render_Dbg_Parallelagram *r;
        
        h = push_type(&target->push_part, Render_Command_Header);
        h->type = render_dbg_parallelagram;
        h->size = sizeof(Render_Command_Header) + sizeof(Render_Dbg_Parallelagram);
        
        r = push_type(&target->push_part, Render_Dbg_Parallelagram);
        r->transform.pos = do_transform(pos, target->transform);
        r->transform.basis.x = x_axis;
        r->transform.basis.y = y_axis;
        r->color = color;
    }
}

void
do_string(Render_Target *target, Asset_Bank *bank,
          i32 font_id, vec2 pos, char *str, int len, vec4 color){
    Render_Command_Header *h;
    Render_Glyph *g;
    i32 i;
    char c;
    f32 *advance;
    
    if (target){
        advance = get_font_advance(bank, font_id);
        if (advance){
            for (i = 0; i < len; ++i){
                h = push_type(&target->push_part, Render_Command_Header);
                h->type = render_glyph;
                h->size = sizeof(Render_Command_Header) + sizeof(Render_Glyph);
                
                c = str[i] & (127);
                g = push_type(&target->push_part, Render_Glyph);
                g->pos = do_transform(pos, target->transform);
                g->font_id = font_id;
                g->c = c;
                g->color = color;
                
                pos.x += advance[c];
            }
        }
    }
}

#define Render_Normal 0x0
#define Render_Exact 0x1

void
do_image_general(Render_Target *target, Asset_Bank *bank,
                 Render_Target *dbg, i32 image_id,
                 u32 flags, vec2 pos, vec2 origin,
                 vec2 x_axis, vec2 y_axis){
    Render_Command_Header *h;
    Render_Image img = {0};
    
    if (target){
        f32 x = 0, y = 0;
        
        Image *image = get_image(bank, image_id);
        
        if (Render_Exact & flags){
            x = 1.f;
            y = 1.f;
        }
        else{
            if (image){
                x = (0.5f*image->width);
                y = (0.5f*image->height);
            }
            else{
                x = 32.f;
                y = 32.f;
            }
        }
        
        img.transform.basis.x = x_axis * x;
        img.transform.basis.y = y_axis * y;
        origin = do_basis(origin, img.transform.basis);
        
        pos.x -= origin.x;
        pos.y -= origin.y;
        
        img.transform.pos = do_transform(pos, target->transform);
        img.image_id = image_id;
        
        if (image){
            h = push_type(&target->push_part, Render_Command_Header);
            h->type = render_image;
            h->size = sizeof(Render_Command_Header) + sizeof(Render_Image);
            
            Render_Image *imgptr = push_type(&target->push_part, Render_Image);
            *imgptr = img;
        }
        
        do_dbg_parallelagram(dbg,
                             img.transform.pos,
                             img.transform.basis.x,
                             img.transform.basis.y,
                             v4(1.f, 0, 0, 0.25f));
    }
}

void
do_image(Render_Target *target, Asset_Bank *bank, Render_Target *dbg, i32 image_id,
         vec2 pos, f32 size = 1.f){
    vec2 x_axis, y_axis;
    
    if (target){
        x_axis = v2(size, 0.0f);
        y_axis = v2(0.0f, size);
        
        do_image_general(target, bank, dbg, image_id,
                         Render_Normal, pos, v2(0,0),
                         x_axis, y_axis);
    }
}

void
do_image(Render_Target *target, Asset_Bank *bank, Render_Target *dbg, i32 image_id,
         vec2 pos, vec2 origin, f32 size = 1.f){
    vec2 x_axis, y_axis;
    
    if (target){
        x_axis = v2(size, 0.0f);
        y_axis = v2(0.0f, size);
        
        do_image_general(target, bank, dbg, image_id,
                         Render_Normal, pos, origin, x_axis, y_axis);
    }
}

void
set_lane(Render_Target *target, Render_Target *dbg,
         i32 lane_index, f32 y){
    Render_Command_Header *h;
    Render_Lane *l;
    
    if (target){
        h = push_type(&target->push_part, Render_Command_Header);
        h->type = render_lane;
        h->size = sizeof(Render_Command_Header) + sizeof(Render_Lane);
        
        l = push_type(&target->push_part, Render_Lane);
        l->lane_index = lane_index;
        l->y_position = y;
        
        do_dbg_parallelagram(dbg,
                             v2(0.f, y),
                             v2(2000.f, 0),
                             v2(0, 5.f),
                             v4(1.f, 0.5f, 0.f, 0.25f));
    }
}

void
do_obstacle(Render_Target *target, Render_Target *dbg,
            u32 type, u32 seed, f32 x, i32 lane){
    Render_Command_Header *h;
    Render_Obstacle *o;
    
    if (target){
        h = push_type(&target->push_part, Render_Command_Header);
        h->type = render_obstacle;
        h->size = sizeof(Render_Command_Header) + sizeof(Render_Obstacle);
        
        o = push_type(&target->push_part, Render_Obstacle);
        o->type = type;
        o->seed = seed;
        o->x = x;
        o->lane = lane;
    }
}

// BOTTOM

