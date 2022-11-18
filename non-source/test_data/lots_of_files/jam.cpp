/*
 *  Overreact - Mr. 4th Dimention
 *      Allen Webster
 *  03.09.2015 (mm.dd.yyyy)
 *
 * Game Layer.
 */

// TOP

internal Memory_Block
make_memory_block(void *base, i32 size){
	Memory_Block block;
	block.base = base;
	block.size = size;
	block.cursor = 0;
	return block;
}

internal void*
get_memory_size(Memory_Block *block, i32 size){
	Assert(block->cursor + size <= block->size);
	void *result = (u8*)block->base + block->cursor;
	block->cursor += size;
	return result;
}

#define get_mem(t,block) (t*)get_memory_size(block, sizeof(t))
#define get_mem_size(block,size) get_memory_size(block, size)
#define get_mem_array(t,block,size) (t*)get_memory_size(block, sizeof(t)*(size))
#define remaining_mem_i32(block) (i32)((block).size - (block).cursor)

#define get_i(x,y) ((x)+(y)*WIDTH)

struct Temp_Memory{
    Memory_Block *block;
    i32 prev_cursor;
};

internal Temp_Memory
begin_temp_memory(Memory_Block *block){
    Temp_Memory result;
    result.block = block;
    result.prev_cursor = block->cursor;
    return result;
}

internal void
end_temp_memory(Temp_Memory temp){
    temp.block->cursor = temp.prev_cursor;
}

struct Wave_Pair{
    i32 left, right;
};

internal bool32
mix_track(Track *track, Wave_Pair *value){
    bool32 result = 0;
    if (track->playing && track->frames_delay == 0){
        Sound sound = track->sound;
        
        i16 *src_prev = sound.samples + track->sample_pos*2;
        i16 *src_next;
        if (track->sample_pos + 1 == sound.sample_count){
            src_next = sound.samples;
        }
        else{
            src_next = src_prev + 2;
        }
        
        real32 dec = track->sample_pos_dec;
        real32 inv_dec = 1.f - dec;
        
        dec *= track->volume;
        inv_dec *= track->volume;
        
        value->left += (i32)((*src_prev)*inv_dec + (*src_next)*dec);
        
        ++src_prev;
        ++src_next;
        
        value->right += (i16)((*src_prev)*inv_dec + (*src_next)*dec);
        
        track->sample_pos_dec += sound.scan_speed*track->bend;
        if (track->sample_pos_dec >= 1.f){
            track->sample_pos += (i32)(track->sample_pos_dec);
            track->sample_pos_dec = DecPart(track->sample_pos_dec);
            
            if (track->sample_pos >= sound.sample_count){
                if (track->looping){
                    track->sample_pos -= sound.sample_count;
                }
                else{
                    track->playing = 0;
                    result = 1;
                }
            }
        }
    }
    
    return result;
}

internal
SIG_APP_FILL_SAMPLES(game_fill_samples){
    App_Vars *vars = (App_Vars*)memory.data;
    
    if (vars->silence_timer == 0){

#if 0
        i16 *dst = target.samples;
        
        for (i32 i = 0; i < target.count_per_channel; ++i){
            real32 wave_hz = 440.f;
            real32 t = vars->wave_t * TAU32;
            real32 v = sinf(t);
            
            i16 value = (i16)(max_i16 * v);
            *dst++ = value;
            *dst++ = value;
            
            vars->wave_t += wave_hz / target.samples_per_second;
        }
        
        vars->wave_t = DecPart(vars->wave_t);
#endif
        
        i16 *dst = target.samples;
        for (i32 i = 0; i < target.count_per_channel; ++i){
            Wave_Pair value = {};
            
            for (i32 j = 0; j < ArrayCount(vars->sfx_tracks); ++j){
                Track *track = vars->sfx_tracks + j;
                if (mix_track(track, &value)){
                    track->next_free = vars->sfx_free;
                    vars->sfx_free = track;
                }
            }
            
            if (vars->music.playing){
                if (mix_track(&vars->music, &value)){
                    vars->song_done = 1;
                }
            }
            
            if (value.left > max_i16){
                value.left = max_i16;
            }
            else if (value.left < min_i16){
                value.left = min_i16;
            }
            
            if (value.right > max_i16){
                value.right = max_i16;
            }
            else if (value.right < min_i16){
                value.right = min_i16;
            }
            
            *dst++ = (i16)value.left;
            *dst++ = (i16)value.right;
        }
    }
    
    for (i32 j = 0; j < ArrayCount(vars->sfx_tracks); ++j){
        Track *track = vars->sfx_tracks + j;
        if (track->playing && track->frames_delay > 0){
            --track->frames_delay;
        }
    }
}

#define A_SHIFT 24
#define R_SHIFT 16
#define G_SHIFT 8
#define B_SHIFT 0

#define A_MASK 0xFF000000
#define R_MASK 0x00FF0000
#define G_MASK 0x0000FF00
#define B_MASK 0x000000FF

internal u32
compress_color(Vec3 color){
	return
		((u8)(255*color.r) << R_SHIFT) +
		((u8)(255*color.g) << G_SHIFT) +
		((u8)(255*color.b) << B_SHIFT);
}

internal u32
compress_color(Vec4 color){
	return
		((u8)(255*color.a) << A_SHIFT) +
		((u8)(255*color.r) << R_SHIFT) +
		((u8)(255*color.g) << G_SHIFT) +
		((u8)(255*color.b) << B_SHIFT);
}

internal Vec3
decompress_color(u32 color){
	return V3(((color & R_MASK) >> R_SHIFT) / 255.f,
			  ((color & G_MASK) >> G_SHIFT) / 255.f,
			  ((color & B_MASK) >> B_SHIFT)  / 255.f);
}

internal Vec4
decompress_color_alpha(u32 color){
	return V4(((color & R_MASK) >> R_SHIFT)  / 255.f,
			  ((color & G_MASK) >> G_SHIFT)  / 255.f,
			  ((color & B_MASK) >> B_SHIFT) / 255.f,
			  ((color & A_MASK) >> A_SHIFT) / 255.f);
}

internal void
draw_clear(Vec4 color){
	glClearColor(color.r, color.g, color.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

internal Render_Texture
make_render_texture(Image image){
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height,
				 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
    
	Render_Texture texture;
	texture.texid = tex;
	texture.width = image.width;
	texture.height = image.height;
    texture.img_width = image.img_width;
    texture.img_height = image.img_height;
    texture.tex_x = (real32)texture.img_width / texture.width;
    texture.tex_y = (real32)texture.img_height / texture.height;
    
	return texture;
}

internal void
draw_texture(Render_Texture texture, Vec2 center, Vec2 halfdim,
             real32 rotation = 0.f, Vec4 blend = {1.f, 1.f, 1.f, 1.f}){
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    
    glTranslatef(center.x, center.y, 0.f);
    glRotatef(rotation, 0, 0, 1);
    
    glColor4f(blend.r, blend.g, blend.b, blend.a);
    glBindTexture(GL_TEXTURE_2D, texture.texid);
    glBegin(GL_QUADS);
    {
        glTexCoord2f(0.f, texture.tex_y);
        glVertex3f(-halfdim.x, halfdim.y, 0.f);
        
        glTexCoord2f(texture.tex_x, texture.tex_y);
        glVertex3f(halfdim.x, halfdim.y, 0.f);
        
        glTexCoord2f(texture.tex_x, 0.f);
        glVertex3f(halfdim.x, -halfdim.y, 0.f);
        
        glTexCoord2f(0.f, 0.f);
        glVertex3f(-halfdim.x, -halfdim.y, 0.f);
    }
    glEnd();
    
    glPopMatrix();
}

inline internal void
draw_texture(Render_Texture texture, Vec2 center,
             real32 rotation = 0.f, Vec4 blend = {1.f, 1.f, 1.f, 1.f}){
    Vec2 halfdim;
    halfdim.x = texture.img_width * 0.5f;
    halfdim.y = texture.img_height * 0.5f;
    draw_texture(texture, center, halfdim, rotation, blend);
}

internal void
draw_rectangle(real32 x1, real32 y1,
			   real32 x2, real32 y2,
			   Vec4 color, real32 z = 0.f){
    
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor4f(color.r, color.g, color.b, color.a);
	glBegin(GL_POLYGON);
	{
		glVertex3f(x1, y2, z);
		glVertex3f(x2, y2, z);
		glVertex3f(x2, y1, z);
		glVertex3f(x1, y1, z);
	}
	glEnd();
}

internal void
draw_text(Game_Render_Target *target, Font *font, real32 *x, real32 *y, char *str, i32 len, Vec4 color,
          real32 start_x = 0.f, real32 x_limit = 10000.f){
    u32 packed_color = compress_color(color);
    for (i32 i = 0; i < len; ++i){
        char c = str[i];
        font_draw_glyph(target, font, c, *x, *y, packed_color);
        *x += font->glyphs[c].advance;
        if (*x > x_limit){
            *x = start_x;
            *y += font->height;
        }
    }
}

internal bool32
load_texture(char *filename, Render_Texture *texture, Memory_Block *block){
    bool32 result = 0;
    Image image;
    Bitmap_File file = {};
    bitmap_open_file(filename, &file);
    if (file.file.data){
        Temp_Memory temp = begin_temp_memory(block);
        
        i32 size = bitmap_data_requirement(&file);
        image.data = (u32*)get_mem_size(block, size);
        bitmap_fill_image(&file, &image);
        bitmap_free_file(&file);
        
        *texture = make_render_texture(image);
        
        end_temp_memory(temp);
        result = 1;
    }
    
    return result;
}

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

internal bool32
load_texture_png(char *filename, Render_Texture *texture, Memory_Block *block){
    Image image;
    i32 n;
    image.data = (u32*)stbi_load(filename, &image.width, &image.height, &n, 0);
    TentativeAssert(image.data);
    
    if (image.data){
        Assert(n == 4);
        Image POT_image;
        POT_image.width = round_up_POT(image.width);
        POT_image.height = round_up_POT(image.height);
        POT_image.img_width = image.width;
        POT_image.img_height = image.height;
        
        Temp_Memory temp = begin_temp_memory(block);
        
        POT_image.data = get_mem_array(u32, block, POT_image.width*POT_image.height);
        
        u32 *src_line = image.data;
        u32 *dst_line = POT_image.data;
        for (i32 y = 0; y < image.height; ++y){
            u32 *src = src_line;
            u32 *dst = dst_line;
            for (i32 x = 0; x < image.width; ++x){
                *dst++ = *src++;
            }
            src_line += image.width;
            dst_line += POT_image.width;
        }
        
        *texture = make_render_texture(POT_image);
        
        end_temp_memory(temp);
        stbi_image_free(image.data);
        
        return 1;
    }
    
    return 0;
}

internal bool32
load_sound(char *filename, Sound *sound, i32 target_samples_per_second, Memory_Block *block){
    bool32 result = 0;
    Wav_File file;
    wav_open_file(filename, &file);
    if (file.file.data){
        i32 size = wav_data_requirement(&file);
        sound->samples = (i16*)get_mem_size(block, size);
        wav_fill_sound(&file, sound, target_samples_per_second);
        result = 1;
    }
    return result;
}

internal bool32
load_font(char *filename, Font *font, i32 pt_size, Memory_Block *block){
    bool32 result = 0;
    Temp_Memory temp = begin_temp_memory(block);
    i32 size = remaining_mem_i32(*block);
    void *mem = get_mem_size(block, size);
    if (font_load(filename, font, pt_size, mem, size)){
        result = 0;
    }
    end_temp_memory(temp);
    return result;
}

internal bool32
can_move_to(App_Vars *vars, Entity *entity, i32 x, i32 y, bool32 *out_of_grid){
    AllowLocal(entity);
    bool32 result = 1;
    if (y >= HEIGHT || y < 0 ||
        x >= WIDTH || x < 0){
        result = 0;
        *out_of_grid = 1;
    }
    else{
        Entity *other_entity = vars->grid[get_i(x, y)];
        if (other_entity){
            result = 0;
        }
        *out_of_grid = 0;
    }
    return result;
}

inline internal bool32
can_move_to(App_Vars *vars, Entity *entity, i32 x, i32 y){
    bool32 throw_away;
    return can_move_to(vars, entity, x, y, &throw_away);
}

internal Entity*
spawn_entity(App_Vars *vars){
    Assert(vars->entity_count < vars->entity_max);
    
    Entity *result = vars->entities + vars->entity_count++;
    *result = {};
    
    return result;
}

internal void
make_random_block(App_Vars *vars){
    Assert(vars->spawn_count < vars->spawn_max);
    
    Spawn_Request *spawn = vars->spawns + vars->spawn_count++;
    spawn->random = 1;
}

internal void
make_zombie(App_Vars *vars, i32 x, i32 y){
    Assert(vars->spawn_count < vars->spawn_max);
    
    Spawn_Request *spawn = vars->spawns + vars->spawn_count++;
    spawn->random = 0;
    spawn->x = x;
    spawn->y = y;
    spawn->type = ZOMBIE;
}

internal Entity*
make_brain(App_Vars *vars, i32 x, i32 y){
    Assert(vars->spawn_count < vars->spawn_max);
    
    Spawn_Request *spawn = vars->spawns + vars->spawn_count++;
    spawn->random = 0;
    spawn->x = x;
    spawn->y = y;
    spawn->type = BRAIN;
}

internal void
move_entity(App_Vars *vars, Entity *entity, i32 x, i32 y){
    i32 old_i = get_i(entity->grid_x, entity->grid_y);
    i32 new_i = get_i(x, y);
    
    Assert(vars->grid[old_i] == entity);
    Assert(vars->grid[new_i] == 0);
    
    vars->grid[old_i] = 0;
    vars->grid[new_i] = entity;
    
    entity->grid_x = x;
    entity->grid_y = y;
}

internal Entity*
get_grid_entity(App_Vars *vars, i32 x, i32 y, bool32 *out_of_bounds){
    if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT){
        *out_of_bounds = 1;
        return 0;
    }
    else{
        *out_of_bounds = 0;
        return vars->grid[get_i(x, y)];
    }
}

inline internal Entity*
get_grid_entity(App_Vars *vars, i32 x, i32 y){
    bool32 throw_away;
    return get_grid_entity(vars, x, y, &throw_away);
}

internal i32
points_for_destruction(Block_Type type){
    i32 result = 0;
    
    switch (type){
    case ZOMBIE:
        result = 15;
        break;
        
    case HUMAN:
        result = 1;
        break;
        
    case BRAIN:
        result = 1;
        break;
        
    case AMMO:
        result = 1;
        break;
        
    case BOMB:
        result = 5;
        break;
        
    case WALL:
        result = 100;
        break;
    }

    return result;
}

internal bool32
play_sound_effect(App_Vars *vars, Sound sound, real32 volume, real32 bend,
                  i32 delay = 0){
    bool32 result = 0;
    
    if (vars->sfx_free){
        result = 1;
        Track *track = vars->sfx_free;
        vars->sfx_free = track->next_free;
        *track = {};
        
        track->sound = sound;
        track->playing = 1;
        track->volume = volume;
        track->bend = bend;
        track->frames_delay = delay;
    }
    
    return result;
}

internal real32
random_real32(App_Vars *vars, real32 min, real32 max){
    real32 result;
    u32 x = pcg32_random_r(&vars->rnd) % 1000000;
    result = min + (x / 1000000.f)*(max - min);
    return result;
}

globalvar real32 BEND_UP = 1.2f;
globalvar real32 BEND_DOWN = 0.8f;

internal void
mark_block_destroyed(App_Vars *vars, Entity *entity){
    if (!entity->death_marked){
        Assert(vars->free_count < vars->free_max);
        vars->to_free[vars->free_count++] = entity;
        vars->need_to_fill_gaps = 1;
        vars->fill_timer = COMBO_TIME;
        entity->death_marked = 1;
        
        vars->score += points_for_destruction(entity->type);
        vars->nonsense_score = pcg32_random_r(&vars->rnd) % 1000;
    }
}

inline internal i32
get_entity_index(App_Vars *vars, Entity *entity){
    Assert(entity >= vars->entities);
    return (i32)(entity - vars->entities);
}

inline internal Grid_Pos
get_grid_pos(Entity *entity){
    Grid_Pos result;
    result.x = entity->grid_x;
    result.y = entity->grid_y;
    return result;
}

struct Entity_Y_Data{
    i32 index, y;
};

internal i32
quick_partition(Entity_Y_Data *data, i32 start, i32 pivot){
    i32 mid = (start + pivot)/2;
    Swap(Entity_Y_Data, data[mid], data[pivot]);
    i32 pivot_y = data[pivot].y;
    for (i32 i = start; i < pivot; ++i){
        if (data[i].y < pivot_y){
            Swap(Entity_Y_Data, data[i], data[start]);
            ++start;
        }
    }
    Swap(Entity_Y_Data, data[pivot], data[start]);
    return start;
}

internal void
quick_sort(Entity_Y_Data *data, i32 start, i32 pivot){
    if (start < pivot){
        i32 mid = quick_partition(data, start, pivot);
        quick_sort(data, start, mid-1);
        quick_sort(data, mid+1, pivot);
    }
}

internal i32
quick_partition(void **data, i32 start, i32 pivot){
    i32 mid = (start + pivot)/2;
    Swap(void*, data[mid], data[pivot]);
    void *pivot_ptr = data[pivot];
    for (i32 i = start; i < pivot; ++i){
        if (data[i] > pivot_ptr){
            Swap(void*, data[i], data[start]);
            ++start;
        }
    }
    Swap(void*, data[pivot], data[start]);
    return start;
}

internal void
quick_sort(void **data, i32 start, i32 pivot){
    if (start < pivot){
        i32 mid = quick_partition(data, start, pivot);
        quick_sort(data, start, mid-1);
        quick_sort(data, mid+1, pivot);
    }
}

internal i32
int_to_string(char *buffer, i32 x, i32 force_len = 0){
    Assert(x >= 0);
    i32 i = 16;
    while (x != 0){
        char c = (char)(x % 10);
        x /= 10;
        c += '0';
        buffer[--i] = c;
    }
    i32 l = 16 - i;
    while (l < force_len){
        ++l;
        buffer[--i] = '0';
    }
    return i;
}

internal bool32
do_text_field(i32 my_index, i32 active_index, bool32 blink_on,
              Game_Render_Target *render_target, Game_Input input,
              Font *font, real32 text_x, real32 text_y,
              i32 *len, char *buffer, i32 max){
    
    bool32 active = (my_index == active_index);
    
    if (active){
        if (input.key_input){
            if (input.key_code == 0){
                if (*len > 0){
                    *len -= 1;
                }
            }
            else if (input.key_code != 1){
                if (*len < max){
                    buffer[*len] = input.key_code;
                    *len += 1;
                }
            }
        }
    }
    
    Vec4 color = V4(0.9f, 0.9f, 0.9f, 1.f);
    if (active){
        color = V4(1.f, 1.f, 1.f, 1.f);
    }
    
    draw_text(render_target, font, &text_x, &text_y, buffer, *len, color);
    
    if (active && blink_on){
        char cursor = '|';
        draw_text(render_target, font, &text_x, &text_y, &cursor, 1, color);
    }
    
    return (active && input.key_input && input.key_code == 1);
}

internal bool32
do_button(i32 my_index, i32 active_index,
          Game_Input input, Render_Texture *textures,
          real32 x, real32 y){
    
    bool32 active = (my_index == active_index);
    
    draw_texture(textures[active], V2(x, y));
    
    return (active && input.key_input && input.key_code == 1);
}

#define START_WITH_GAME_OVER 0
#define SKIP_TITLE_SCREEN 0

inline internal i32
get_level_fall_tick_time(App_Vars *vars, i32 level){
    i32 real_level = level;
    if (real_level >= ArrayCount(levels)){
        real_level = ArrayCount(levels) - 1;
    }
    return (levels[real_level].fall_tick_time);
}

inline internal i32
get_level_min_score(App_Vars *vars, i32 level){
    i32 result;
    i32 real_level = level;
    if (real_level >= ArrayCount(levels)){
        result = levels[ArrayCount(levels)-1].fall_tick_time;
        result = (level - ArrayCount(levels) + 1)*1200;
    }
    else{
        result = (levels[real_level].min_score);
    }
    return result;
}

internal void
game_set_to_new(App_Vars *vars){
    vars->level = 0;
    vars->fall_timer = get_level_fall_tick_time(vars, vars->level);
    
    vars->spawn_count = 0;
    vars->spawn_max = ArrayCount(vars->spawns);
    
    vars->free_count = 0;
    vars->free_max = ArrayCount(vars->to_free);
    
    vars->entity_count = 0;
    vars->entity_max = ArrayCount(vars->entities);
    vars->need_new_block = 1;
    
    vars->particle_count = 0;
    vars->particle_max = ArrayCount(vars->particles);
    
    vars->refall_timer = 0;
    vars->game_over = 0;
    vars->need_to_fill_gaps = 0;
    vars->chain_reacting = 0;
    vars->reaction_timer = 0;
    vars->fill_timer = 0;
    vars->score = 0;
    vars->nonsense_score = 0;
    vars->active_field = 0;
    vars->blink_timer = 0;
    vars->user_name_len = 0;
    vars->user_token_len = 0;
    
    memset(vars->grid, 0, sizeof(Entity*)*WIDTH*HEIGHT);

    vars->looked_up_teaser = 0;
    vars->teaser_name_len = 0;
    vars->teaser_score_len = 0;
    
    vars->controls_screen = 0;
    vars->title_screen = 1;
    
    vars->music = {};
    vars->music.sound = vars->menu_music;
    vars->music.playing = 1;
    vars->music.looping = 1;
    vars->music.volume = 0.3f;
    vars->music.bend = 1.f;
    
    for (i32 i = ArrayCount(vars->sfx_tracks) - 2; i >= 0; --i){
        vars->sfx_tracks[i].next_free = vars->sfx_tracks + i + 1;
    }
    
    vars->sfx_free = vars->sfx_tracks;
}

persist real32 GRID_STRIDE = 94.0f;

internal Vec2
get_screen_pos(Entity *entity){
    persist Vec2 top_left = {265.f, 70.f - GRID_STRIDE};
    
    Vec2 result;
    result.x = entity->show_x*GRID_STRIDE;
    result.y = entity->show_y*GRID_STRIDE;
    result += top_left;
    
    return result;
}

// THING GETS BROKEN
internal void
spawn_particles_type_1(App_Vars *vars, i32 prt_index, Vec2 pos, i32 count){
    Particle_Type prt_type = vars->prt_types[prt_index];
    Particle *particles = vars->particles;
    i32 particle_count = vars->particle_count;
    
    if (vars->particle_max < particle_count + count){
        count = vars->particle_max - particle_count;
    }
    
    for (i32 r = count; r > 0; --r){
        Particle part;
        part.tex_index = (pcg32_random_r(&vars->rnd) % prt_type.tex_count) + prt_type.first_tex_id;
        part.pos = pos;
        part.vel.x = random_real32(vars, -20.f, 20.f);
        part.vel.y = random_real32(vars, -20.f, 0.f);
        part.life_counter = max_i32;
        part.rot = random_real32(vars, 0.f, 360.f);
        part.rot_vel = random_real32(vars, -10.f, 10.f);
        
        Assert(particle_count < vars->particle_max);
        particles[particle_count++] = part;
    }
    
    vars->particle_count = particle_count;
}

// AMMO BOX GETS USED
internal void
spawn_particles_type_2(App_Vars *vars, i32 prt_index, Vec2 pos, i32 count){
    Particle_Type prt_type = vars->prt_types[prt_index];
    Particle *particles = vars->particles;
    i32 particle_count = vars->particle_count;
    
    if (vars->particle_max < particle_count + count){
        count = vars->particle_max - particle_count;
    }
    
    for (i32 r = count; r > 0; --r){
        Particle part;
        part.tex_index = (pcg32_random_r(&vars->rnd) % prt_type.tex_count) + prt_type.first_tex_id;
        part.pos = pos;
        part.vel.x = random_real32(vars, -10.f, 10.f);
        part.vel.y = random_real32(vars, 0.f, 0.f);
        part.life_counter = max_i32;
        part.rot = random_real32(vars, 0.f, 360.f);
        part.rot_vel = random_real32(vars, -10.f, 10.f);
        
        Assert(particle_count < vars->particle_max);
        particles[particle_count++] = part;
    }
    
    vars->particle_count = particle_count;
}

// STATIC PARTICLE
internal void
spawn_particles_type_3(App_Vars *vars, i32 prt_index, Vec2 pos, real32 rot){
    Particle_Type prt_type = vars->prt_types[prt_index];
    Particle *particles = vars->particles;
    i32 particle_count = vars->particle_count;
    
    if (particle_count < vars->particle_max){
        Particle part;
        part.tex_index = (pcg32_random_r(&vars->rnd) % prt_type.tex_count) + prt_type.first_tex_id;
        part.pos = pos;
        part.vel = {};
        part.life_counter = 5;
        part.rot = rot;
        part.rot_vel = 0;
        
        particles[particle_count++] = part;
        
        vars->particle_count = particle_count;
    }
}

internal
SIG_APP_STEP(game_step){
    App_Vars *vars = (App_Vars*)memory.data;
    Assert(sizeof(App_Vars) < memory.size);
    
    if (first){
        font_init();
        *vars = {};
        vars->block = make_memory_block((u8*)memory.data + sizeof(App_Vars),
                                        memory.size - sizeof(App_Vars));
        
        // TODO(allen): thread the gamejolt api?
        vars->gj = gj_init(80985, "031106786fa9dc6103062c42ee6f04ea");
        vars->table_id = 84092;
        
#if 0
        gj_login(vars->gj, "OVERREACT", "F3DE5E");
        gj_post_score(vars->gj, vars->table_id, "ZERO", 1, "", "");
#endif
        
        {
            Temp_Memory temp = begin_temp_memory(&vars->block);
            
            GJ_Score_Data *scores = get_mem_array(GJ_Score_Data, &vars->block, 5);
            i32 score_count = gj_get_scores(vars->gj, vars->table_id, 5, scores);
            
            for (i32 i = 0; i < score_count; ++i){
                gj_free_score(scores[i]);
            }
            
            end_temp_memory(temp);
        }
        
        vars->silence_timer = 70;
        
        load_texture_png("BRAINZ_background.png", &vars->background, &vars->block);
        
        load_texture_png("BRAINZ_human1.png", &vars->human[0][0], &vars->block);
        load_texture_png("BRAINZ_humanzombie1_1.png", &vars->human[0][1], &vars->block);
        load_texture_png("BRAINZ_humanzombie1_2.png", &vars->human[0][2], &vars->block);
        load_texture_png("BRAINZ_humanzombie1_3.png", &vars->human[0][3], &vars->block);
        load_texture_png("BRAINZ_zombie1.png", &vars->zombie[0], &vars->block);
        load_texture_png("BRAINZ_ammo.png", &vars->ammo, &vars->block);
        load_texture_png("BRAINZ_brain.png", &vars->brain, &vars->block);
        load_texture_png("BRAINZ_tnt.png", &vars->bomb, &vars->block);
        load_texture_png("BRAINZ_wall.png", &vars->wall, &vars->block);
        
        load_texture_png("BRAINZ_score.png", &vars->scorename, &vars->block);
        load_texture_png("BRAINZ_scoreback.png", &vars->scoreback, &vars->block);
        load_texture_png("BRAINZ_gameover.png", &vars->gameover, &vars->block);
        load_texture_png("BRAINZ_shadow.png", &vars->shadow, &vars->block);
        
        load_texture_png("BRAINZ_gameover_button1.png", &vars->finish_button[0], &vars->block);
        load_texture_png("BRAINZ_gameover_button2.png", &vars->finish_button[1], &vars->block);
        
        load_texture_png("BRAINZ_OVERREACT.png", &vars->overreact, &vars->block);
        load_texture_png("BRAINZ_title.png", &vars->title, &vars->block);
        load_texture_png("BRAINZ_title_button.png", &vars->title_button, &vars->block);
        load_texture_png("BRAINZ_controls.png", &vars->controls, &vars->block);
        
        Particle_Textures prt_textures;
        
        load_texture_png("prts\\BrainZ_AMMO_part_1.png", &prt_textures.prt_ammo[0], &vars->block);
        load_texture_png("prts\\BrainZ_AMMO_part_2.png", &prt_textures.prt_ammo[1], &vars->block);
        load_texture_png("prts\\BrainZ_AMMO_part_3.png", &prt_textures.prt_ammo[2], &vars->block);
        load_texture_png("prts\\BrainZ_AMMO_part_4.png", &prt_textures.prt_ammo[3], &vars->block);
        load_texture_png("prts\\BrainZ_AMMO_part_5.png", &prt_textures.prt_ammo[4], &vars->block);
        
        load_texture_png("prts\\BrainZ_BLOOD_part_1.png", &prt_textures.prt_blood[0], &vars->block);
        load_texture_png("prts\\BrainZ_BLOOD_part_2.png", &prt_textures.prt_blood[1], &vars->block);
        load_texture_png("prts\\BrainZ_BLOOD_part_3.png", &prt_textures.prt_blood[2], &vars->block);
        load_texture_png("prts\\BrainZ_BLOOD_part_4.png", &prt_textures.prt_blood[3], &vars->block);
        load_texture_png("prts\\BrainZ_BLOOD_part_5.png", &prt_textures.prt_blood[4], &vars->block);
        load_texture_png("prts\\BrainZ_BLOOD_part_6.png", &prt_textures.prt_blood[5], &vars->block);
        
        load_texture_png("prts\\BrainZ_BONE_part_1.png", &prt_textures.prt_bone[0], &vars->block);
        load_texture_png("prts\\BrainZ_BONE_part_2.png", &prt_textures.prt_bone[1], &vars->block);
        load_texture_png("prts\\BrainZ_BONE_part_3.png", &prt_textures.prt_bone[2], &vars->block);
        load_texture_png("prts\\BrainZ_BONE_part_4.png", &prt_textures.prt_bone[3], &vars->block);
        load_texture_png("prts\\BrainZ_BONE_part_5.png", &prt_textures.prt_bone[4], &vars->block);
        load_texture_png("prts\\BrainZ_BONE_part_6.png", &prt_textures.prt_bone[5], &vars->block);
        
        load_texture_png("prts\\BrainZ_BRAIN_part_1.png", &prt_textures.prt_brain[0], &vars->block);
        load_texture_png("prts\\BrainZ_BRAIN_part_2.png", &prt_textures.prt_brain[1], &vars->block);
        load_texture_png("prts\\BrainZ_BRAIN_part_3.png", &prt_textures.prt_brain[2], &vars->block);
        load_texture_png("prts\\BrainZ_BRAIN_part_4.png", &prt_textures.prt_brain[3], &vars->block);
        
        load_texture_png("prts\\BrainZ_HUMAN_part_1.png", &prt_textures.prt_human[0], &vars->block);
        load_texture_png("prts\\BrainZ_HUMAN_part_2.png", &prt_textures.prt_human[1], &vars->block);
        load_texture_png("prts\\BrainZ_HUMAN_part_3.png", &prt_textures.prt_human[2], &vars->block);
        load_texture_png("prts\\BrainZ_HUMAN_part_4.png", &prt_textures.prt_human[3], &vars->block);
        
        load_texture_png("prts\\BrainZ_MUZZLE_part_1.png", &prt_textures.prt_muzzle[0], &vars->block);
        load_texture_png("prts\\BrainZ_MUZZLE_part_2.png", &prt_textures.prt_muzzle[1], &vars->block);
        
        load_texture_png("prts\\BrainZ_TNT_part_1.png", &prt_textures.prt_tnt[0], &vars->block);
        load_texture_png("prts\\BrainZ_TNT_part_2.png", &prt_textures.prt_tnt[1], &vars->block);
        load_texture_png("prts\\BrainZ_TNT_part_3.png", &prt_textures.prt_tnt[2], &vars->block);
        load_texture_png("prts\\BrainZ_TNT_part_4.png", &prt_textures.prt_tnt[3], &vars->block);
        load_texture_png("prts\\BrainZ_TNT_part_5.png", &prt_textures.prt_tnt[4], &vars->block);
        
        load_texture_png("prts\\BrainZ_WALL_part_1.png", &prt_textures.prt_wall[0], &vars->block);
        load_texture_png("prts\\BrainZ_WALL_part_2.png", &prt_textures.prt_wall[1], &vars->block);
        load_texture_png("prts\\BrainZ_WALL_part_3.png", &prt_textures.prt_wall[2], &vars->block);
        load_texture_png("prts\\BrainZ_WALL_part_4.png", &prt_textures.prt_wall[3], &vars->block);
        load_texture_png("prts\\BrainZ_WALL_part_5.png", &prt_textures.prt_wall[4], &vars->block);
        
        load_texture_png("prts\\BrainZ_ZOMBIE_part_1.png", &prt_textures.prt_zombie[0], &vars->block);
        load_texture_png("prts\\BrainZ_ZOMBIE_part_2.png", &prt_textures.prt_zombie[1], &vars->block);
        load_texture_png("prts\\BrainZ_ZOMBIE_part_3.png", &prt_textures.prt_zombie[2], &vars->block);
        load_texture_png("prts\\BrainZ_ZOMBIE_part_4.png", &prt_textures.prt_zombie[3], &vars->block);
        
        load_texture_png("prts\\BrainZ_BULLET_part_1.png", &prt_textures.prt_bullet[0], &vars->block);
        
        i32 prt_tex_count = 0;
        i32 prt_type_count = 0;
        
#define COUNT_PRTS(prt_type) prt_tex_count += ArrayCount(prt_textures.prt_type); ++prt_type_count;
        LIST_PRT_TYPES(COUNT_PRTS);
#undef COUNT_PRTS
        
        vars->prt_type_count = prt_type_count;
        vars->prt_textures = get_mem_array(Render_Texture, &vars->block, prt_tex_count);
        vars->prt_types = get_mem_array(Particle_Type, &vars->block, prt_type_count);
        
        prt_tex_count = 0;
        
        i32 type_i = 0;
#define COUNT_PRTS(prt_type) vars->prt_types[type_i].first_tex_id = prt_tex_count; vars->prt_types[type_i].tex_count = ArrayCount(prt_textures.prt_type); prt_tex_count += ArrayCount(prt_textures.prt_type); ++type_i;
        LIST_PRT_TYPES(COUNT_PRTS);
#undef COUNT_PRTS
        
        Temp_Memory prt_temp = begin_temp_memory(&vars->block);
        Render_Texture **prt_texture_by_type = get_mem_array(Render_Texture*, &vars->block, prt_type_count);
        
        type_i = 0;
#define COUNT_PRTS(prt_type) prt_texture_by_type[type_i++] = prt_textures.prt_type
        LIST_PRT_TYPES(COUNT_PRTS);
#undef COUNT_PRTS
        
        i32 prt_tex_i = 0;
        for (i32 i = 0; i < prt_type_count; ++i){
            i32 count = vars->prt_types[i].tex_count;
            for (i32 j = 0; j < count; ++j){
                vars->prt_textures[prt_tex_i++] = prt_texture_by_type[i][j];
            }
        }
        
        end_temp_memory(prt_temp);
        
        type_i = 0;
#define COUNT_PRTS(t) vars->t##_index = type_i++
        LIST_PRT_TYPES(COUNT_PRTS);
#undef COUNT_PRTS
        
        //load_sound("audtest.wav", &vars->music.sound, target.audio_samples_per_second, &vars->block);
        
        load_sound("SFX\\AmmoFlip_1.wav", &vars->ammo_flip1, target.audio_samples_per_second, &vars->block);
        load_sound("SFX\\AmmoFlip_2.wav", &vars->ammo_flip2, target.audio_samples_per_second, &vars->block);
        load_sound("SFX\\AmmoLand.wav", &vars->ammo_land, target.audio_samples_per_second, &vars->block);
        
        load_sound("SFX\\Brains_1.wav", &vars->brains1, target.audio_samples_per_second, &vars->block);
        load_sound("SFX\\Brains_2.wav", &vars->brains2, target.audio_samples_per_second, &vars->block);
        
        load_sound("SFX\\Explosion.wav", &vars->explosion, target.audio_samples_per_second, &vars->block);
        load_sound("SFX\\GunShot.wav", &vars->gun_shot, target.audio_samples_per_second, &vars->block);
        
        load_sound("SFX\\PersonFlip_1.wav", &vars->person_flip1, target.audio_samples_per_second, &vars->block);
        load_sound("SFX\\PersonFlip_2.wav", &vars->person_flip2, target.audio_samples_per_second, &vars->block);
        load_sound("SFX\\PersonLand.wav", &vars->person_land, target.audio_samples_per_second, &vars->block);
        
        load_sound("SFX\\Reload.wav", &vars->reload, target.audio_samples_per_second, &vars->block);
        
        load_sound("SFX\\SoftFlip_1.wav", &vars->soft_flip1, target.audio_samples_per_second, &vars->block);
        load_sound("SFX\\SoftFlip_2.wav", &vars->soft_flip2, target.audio_samples_per_second, &vars->block);
        load_sound("SFX\\SoftLand_1.wav", &vars->soft_land1, target.audio_samples_per_second, &vars->block);
        load_sound("SFX\\SoftLand_2.wav", &vars->soft_land2, target.audio_samples_per_second, &vars->block);
        
        load_sound("SFX\\WallFlip_1.wav", &vars->wall_flip1, target.audio_samples_per_second, &vars->block);
        load_sound("SFX\\WallFlip_2.wav", &vars->wall_flip2, target.audio_samples_per_second, &vars->block);
        load_sound("SFX\\WallLand.wav", &vars->wall_land, target.audio_samples_per_second, &vars->block);
        
        load_sound("SFX\\ZombieBreak.wav", &vars->zombie_break, target.audio_samples_per_second, &vars->block);
        
        load_sound("SFX\\SplatDeath.wav", &vars->splat_death, target.audio_samples_per_second, &vars->block);
        
        load_sound("SFX\\Score_1.wav", &vars->score1, target.audio_samples_per_second, &vars->block);
        load_sound("SFX\\Score_2.wav", &vars->score2, target.audio_samples_per_second, &vars->block);
        
        load_sound("music\\Menu.wav", &vars->menu_music, target.audio_samples_per_second, &vars->block);
        load_sound("music\\Gameplay_1.wav", &vars->gameplay1, target.audio_samples_per_second, &vars->block);
        load_sound("music\\Gameplay_2.wav", &vars->gameplay2, target.audio_samples_per_second, &vars->block);
        
        load_font("Dimbo Regular.ttf", &vars->font, 36, &vars->block);
        load_font("Dimbo Regular.ttf", &vars->small_font, 32, &vars->block);
        
        game_set_to_new(vars);
        
#if START_WITH_GAME_OVER
        vars->game_over = 1;
        vars->score = 1000000;
#endif
#if SKIP_TITLE_SCREEN
        vars->title_screen = 0;
#endif
    }
    
    // EVERY FRAME
    gj_update(vars->gj);
    
    if (!vars->looked_up_teaser){
        vars->looked_up_teaser = 1;
        
        GJ_Score_Data score_data;
        i32 score_count;
        score_count = gj_get_scores(vars->gj, vars->table_id,
                                    1, &score_data);
        
        if (score_count == 1){
            vars->teaser_score_len = score_data.score_len;
            vars->teaser_name_len = score_data.name_len;
            
            if (vars->teaser_score_len > ArrayCount(vars->teaser_score)){
                vars->teaser_score_len = ArrayCount(vars->teaser_score);
            }
            
            if (vars->teaser_name_len > ArrayCount(vars->teaser_name)){
                vars->teaser_name_len = ArrayCount(vars->teaser_name);
            }
            
            memcpy(vars->teaser_score, score_data.score, vars->teaser_score_len);
            memcpy(vars->teaser_name, score_data.name, vars->teaser_name_len);
            
            gj_free_score(score_data);
        }
        else{
            persist char zero_string[] = "ZERO";
            persist char nobody_string[] = "NOBODY";
            
            vars->teaser_score_len = ArrayCount(zero_string) - 1;
            vars->teaser_name_len = ArrayCount(nobody_string) - 1;
            
            memcpy(vars->teaser_score, nobody_string, vars->teaser_score_len);
            memcpy(vars->teaser_name, nobody_string, vars->teaser_name_len);
        }
    }
    
    if (vars->silence_timer > 0){
        --vars->silence_timer;
    }
    
    if (vars->song_done){
        vars->music = {};
        vars->music.sound = vars->gameplay1;
        vars->music.playing = 1;
        vars->music.looping = 1;
        vars->music.volume = 0.03f;
        vars->music.bend = 1.f;
        vars->song_done = 0;
    }
    
    if (vars->title_screen > 0){
        // DO NOTHING
    }
    else if (vars->controls_screen){
        // DO NOTHING
    }
    else{
        i32 prev_score = vars->score;
        if (!vars->game_over){
            
            // TODO(allen): allow holding but slow it down
            bool32 left = input.digital.left && !vars->prev_input.digital.left;
            bool32 right = input.digital.right && !vars->prev_input.digital.right;
            bool32 rot_left = input.button[1] && !vars->prev_input.button[1];
            bool32 rot_right = input.button[2] && !vars->prev_input.button[2];
            
            for (i32 i = 0; i < vars->entity_count; ++i){
                Entity *entity = vars->entities + i;
                
                if (entity->active){
                    if (left && !right){
                        i32 left = entity->grid_x - 1;
                        if (can_move_to(vars, entity, left, entity->grid_y)){
                            move_entity(vars, entity, left, entity->grid_y);
                        }
                    }
                    else if (right && !left){
                        i32 right = entity->grid_x + 1;
                        if (can_move_to(vars, entity, right, entity->grid_y)){
                            move_entity(vars, entity, right, entity->grid_y);
                        }
                    }
                    
                    bool32 did_rotation = 0;
                    if (rot_left && !rot_right){
                        entity->facing += 1;
                        entity->facing %= 4;
                        did_rotation = 1;
                    }
                    else if (rot_right && !rot_left){
                        entity->facing += 3;
                        entity->facing %= 4;
                        did_rotation = 1;
                    }
                    
                    if (did_rotation){
                        entity->wobble = TAU32 / 10.f;
                        
                        Sound *snds[2];
                        switch (entity->type){
                        case AMMO:
                        {
                            snds[0] = &vars->ammo_flip1;
                            snds[1] = &vars->ammo_flip2;
                        }break;
                        
                        case HUMAN:
                        case ZOMBIE:
                        {
                            snds[0] = &vars->person_flip1;
                            snds[1] = &vars->person_flip2;
                        }break;
                        
                        case BRAIN:
                        {
                            snds[0] = &vars->soft_flip1;
                            snds[1] = &vars->soft_flip2;
                        }break;
                        
                        case WALL:
                        case BOMB:
                        {
                            snds[0] = &vars->wall_flip1;
                            snds[1] = &vars->wall_flip2;
                        }break;
                        }
                        
                        i32 which = (pcg32_random_r(&vars->rnd) % 2);
                        
                        play_sound_effect(vars, *snds[which],
                                          1.f, random_real32(vars, BEND_DOWN, BEND_UP));
                    }
                }
            }
        
            if (!vars->chain_reacting){
                bool32 board_check = 0;
                if (vars->need_to_fill_gaps){
                    if (vars->fill_timer > 0){
                        --vars->fill_timer;
                    }
                    else{
                        // FILL DOWN
                        Temp_Memory temp = begin_temp_memory(&vars->block);
                    
                        Entity_Y_Data *data = get_mem_array(Entity_Y_Data, &vars->block, vars->entity_count);
                        for (i32 i = 0; i < vars->entity_count; ++i){
                            data[i].index = i;
                            data[i].y = vars->entities[i].grid_y;
                        }
                    
                        quick_sort(data, 0, vars->entity_count-1);
                    
                        vars->need_to_fill_gaps = 0;
                        for (i32 i = 0; i < vars->entity_count; ++i){
                            Entity *entity = vars->entities + data[i].index;
                        
                            if (can_move_to(vars, entity, entity->grid_x, entity->grid_y + 1)){
                                move_entity(vars, entity, entity->grid_x, entity->grid_y + 1);
                                vars->need_to_fill_gaps = 1;
                            }
                        }
                    
                        if (!vars->need_to_fill_gaps){
                            board_check = 1;
                        }
                        else{
                            vars->fill_timer = COMBO_TIME;
                        }
                    
                        end_temp_memory(temp);
                    }
                }
                else if (!vars->need_new_block){
                    // FALL TICK
                    if (input.digital.down){
                        if (vars->fall_timer > 4){
                            vars->fall_timer = 4;
                        }
                    }
                    
                    bool32 land_now = 0;
                    --vars->fall_timer;
                    
                    for (i32 i = 0; i < vars->entity_count; ++i){
                        Entity *entity = vars->entities + i;
                        
                        if (entity->active){
                            if (vars->fall_timer <= 0){
                                vars->fall_timer += get_level_fall_tick_time(vars, vars->level);
                                
                                if (can_move_to(vars, entity, entity->grid_x, entity->grid_y + 1)){
                                    move_entity(vars, entity, entity->grid_x, entity->grid_y + 1);
                                }
                                else{
                                    land_now = 1;
                                }
                            }
                            else{
                                if (!can_move_to(vars, entity, entity->grid_x, entity->grid_y + 1) &&
                                    entity->show_x == entity->grid_x && entity->show_y == entity->grid_y){
                                    land_now = 1;
                                }
                            }
                            
                            if (land_now){
                                Sound *snd = 0;
                                switch (entity->type){
                                case AMMO:
                                {
                                    snd = &vars->ammo_land;
                                }break;
                        
                                case HUMAN:
                                case ZOMBIE:
                                {
                                    snd = &vars->person_land;
                                }break;
                        
                                case BRAIN:
                                {
                                    if ((pcg32_random_r(&vars->rnd) % 2) == 0){
                                        snd = &vars->soft_land1;
                                    }
                                    else{
                                        snd = &vars->soft_land2;
                                    }
                                }break;
                                    
                                case WALL:
                                {
                                    if ((pcg32_random_r(&vars->rnd) % 2) == 0){
                                        snd = &vars->wall_land;
                                    }
                                    else{
                                        snd = &vars->wall_flip1;
                                    }
                                }break;
                                    
                                case BOMB:
                                {
                                    snd = &vars->person_flip2;
                                }break;
                                }

                                if (snd){
                                    play_sound_effect(vars, *snd,
                                                      1.f, random_real32(vars, BEND_DOWN, BEND_UP));
                                }
                                    
                                entity->active = 0;
                                entity->wobble = 0;
                                if (entity->grid_y == 0){
                                    vars->game_over = 1;
                                }
                                vars->need_new_block = 1;
                                board_check = 1;
                            }
                        }
                    }
                }
            
                // SPREAD INFECTION
                for (i32 i = 0; i < vars->entity_count; ++i){
                    Entity *entity = vars->entities + i;
                    if (entity->type == HUMAN){
                        Grid_Pos center = get_grid_pos(entity);
                    
                        i32 infection_counter = 0;
                        for (i32 i = 0; i < ArrayCount(von_neumann); ++i){
                            Grid_Pos pos = center + von_neumann[i];
                            Entity *neighbor = get_grid_entity(vars, pos.x, pos.y);
                            if (neighbor){
                                if (neighbor->type == ZOMBIE){
                                    ++infection_counter;
                                }
                            }
                        }
                    
                        if (infection_counter == 0){
                            if (entity->infection_amount > 0){
                                entity->infection_amount -= 1;
                            }
                        }
                        else{
                            entity->infection_amount += infection_counter;
                        }
                    
                        if (entity->infection_amount > ZOMBIE_TURN_THRESHOLD){
                            entity->type = ZOMBIE;
                            entity->facing += 3;
                            entity->facing %= 4;
                            board_check = 1;
                        }
                    }
                }
            
                // BOARD CHECK
                if (board_check){
                    for (i32 i = 0; i < vars->entity_count; ++i){
                        Entity *entity = vars->entities + i;
                    
                        switch (entity->type){
                        case AMMO:
                        {
                            bool32 ammo_used = 0;
                            Grid_Pos center = get_grid_pos(entity);
                            for (i32 i = 0; i < ArrayCount(von_neumann); ++i){
                                Grid_Pos pos = center + von_neumann[i];
                            
                                Entity *neighbor = get_grid_entity(vars, pos.x, pos.y);
                                if (neighbor){
                                    if (neighbor->type == HUMAN){
                                        ammo_used = 1;
                                        neighbor->firing = 1;
                                    }
                                }
                            }
                            
                            if (ammo_used){
                                play_sound_effect(vars, vars->reload,
                                                  0.5f, random_real32(vars, BEND_DOWN, BEND_UP));
                                mark_block_destroyed(vars, entity);
                            }
                        }break;
                    
                        case BRAIN:
                        {
                            bool32 brain_eaten = 0;
                            Grid_Pos center = get_grid_pos(entity);
                            for (i32 i = 0; i < ArrayCount(von_neumann); ++i){
                                Grid_Pos pos = center + von_neumann[i];
                            
                                Entity *neighbor = get_grid_entity(vars, pos.x, pos.y);
                                if (neighbor){
                                    if (neighbor->type == ZOMBIE){
                                        brain_eaten = 1;
                                        neighbor->firing = 1;
                                    }
                                }
                            }
                            
                            if (brain_eaten){
                                mark_block_destroyed(vars, entity);
                            }
                        }break;
                        }
                    }
                }
            }

            // RANDOM BRAINS
            bool32 has_zombie = 0;
            for (i32 i = 0; i < vars->entity_count; ++i){
                Entity *entity = vars->entities + i;
                
                if (entity->type == ZOMBIE){
                    has_zombie = 1;
                    break;
                }
            }
            
            if (has_zombie && (pcg32_random_r(&vars->rnd) % 600) == 0){
                Sound *snd;
                if ((pcg32_random_r(&vars->rnd) % 2) == 0){
                    snd = &vars->brains1;
                }
                else{
                    snd = &vars->brains2;
                }
                
                play_sound_effect(vars, *snd,
                                  1.f, random_real32(vars, BEND_DOWN, BEND_UP));
            }
            
            // FIRE
            if (vars->reaction_timer > 0){
                --vars->reaction_timer;
            }
            else{
                vars->chain_reacting = 0;
                Temp_Memory gun_temp = begin_temp_memory(&vars->block);
                i32 reaction_fire_count = 0;
                i32 reaction_fire_max = WIDTH*HEIGHT;
                Entity **reaction_fire = get_mem_array(Entity*, &vars->block, reaction_fire_max);
                for (i32 i = 0; i < vars->entity_count; ++i){
                    Entity *entity = vars->entities + i;
                    Assert(!(entity->firing && entity->step_forward));
                    if (entity->firing){
                        vars->chain_reacting = 1;
                        vars->reaction_timer = COMBO_TIME;
                        entity->firing = 0;
                        if (entity->type == HUMAN || entity->type == AMMO){
                            i32 step_count = ArrayCount(up_shot);
                            Grid_Pos *shot_pos = right_shot;
                            switch (entity->facing){
                            case UP:
                                shot_pos = up_shot;
                                break;
                            
                            case LEFT:
                                shot_pos = left_shot;
                                break;
                            
                            case DOWN:
                                shot_pos = down_shot;
                                break;
                            }
                        
                            Grid_Pos center = get_grid_pos(entity);
                            for (i32 i = 0; i < step_count; ++i){
                                Grid_Pos pos = center + shot_pos[i];
                            
                                bool32 out_of_grid;
                                Entity *hit_entity = get_grid_entity(vars, pos.x, pos.y, &out_of_grid);
                                if (out_of_grid){
                                    break;
                                }
                                if (hit_entity){
                                    if (hit_entity->type == WALL){
                                        break;
                                    }
                                    else{
                                        switch (hit_entity->type){
                                        case HUMAN: case ZOMBIE: case BRAIN:
                                        {
                                            mark_block_destroyed(vars, hit_entity);
                                        }break;
                                    
                                        case AMMO: case BOMB:
                                        {
                                            Assert(reaction_fire_count < reaction_fire_max);
                                            reaction_fire[reaction_fire_count++] = hit_entity;
                                        }break;
                                        }
                                    }
                                }
                            }
                            
                            play_sound_effect(vars, vars->gun_shot,
                                              0.5f, random_real32(vars, BEND_DOWN, BEND_UP));
                            
                            if (entity->type == AMMO){
                                mark_block_destroyed(vars, entity);
                            }
                            if (entity->type == HUMAN){
                                real32 rotation = 0.f;
                                Vec2 pos = get_screen_pos(entity);
                                persist real32 MUZZLE_HALF_WIDTH = 70.f;
                            
                                switch (entity->facing){
                                case RIGHT:
                                    rotation = 0.f;
                                    pos.x += 43 + MUZZLE_HALF_WIDTH;
                                    pos.y += 36;
                                    break;
                                
                                case UP:
                                    rotation = 270.f;
                                    pos.x += 36;
                                    pos.y += -43 - MUZZLE_HALF_WIDTH;
                                    break;
                                
                                case LEFT:
                                    rotation = 180.f;
                                    pos.x += -43 - MUZZLE_HALF_WIDTH;
                                    pos.y += -36;
                                    break;
                                
                                case DOWN:
                                    rotation = 90.f;
                                    pos.x += -36;
                                    pos.y += 43 + MUZZLE_HALF_WIDTH;
                                    break;
                                }
                            
                                spawn_particles_type_3(vars, vars->prt_muzzle_index, pos, rotation);
                            }
                        }
                    
                        else if (entity->type == BOMB){
                            Grid_Pos center = get_grid_pos(entity);
                            for (i32 i = 0; i < ArrayCount(moore); ++i){
                                Grid_Pos pos = center + moore[i];
                                
                                Entity *hit_entity = get_grid_entity(vars, pos.x, pos.y);
                                if (hit_entity){
                                    switch (hit_entity->type){
                                    case HUMAN: case ZOMBIE: case BRAIN: case WALL:
                                    {
                                        mark_block_destroyed(vars, hit_entity);
                                    }break;
                                    
                                    case AMMO: case BOMB:
                                    {
                                        Assert(reaction_fire_count < reaction_fire_max);
                                        reaction_fire[reaction_fire_count++] = hit_entity;
                                    }break;
                                    }
                                }
                                
                                play_sound_effect(vars, vars->explosion,
                                                  0.03f, random_real32(vars, BEND_DOWN, BEND_UP));
                            }
                            mark_block_destroyed(vars, entity);
                        }
                    
                        else if (entity->type == ZOMBIE){
                            Grid_Pos pos = get_grid_pos(entity);
                            switch (entity->facing){
                            case RIGHT:
                                pos.x += 1;
                                break;
                                
                            case UP:
                                pos.y -= 1;
                                break;
                                
                            case LEFT:
                                pos.x -= 1;
                                break;
                                
                            case DOWN:
                                pos.y += 1;
                                break;
                            }
                            
                            bool32 out_of_grid;
                            Entity *target = get_grid_entity(vars, pos.x, pos.y, &out_of_grid);
                            if (!out_of_grid){
                                if (target){
                                    switch (target->type){
                                    case BOMB: case AMMO:
                                    {
                                        Assert(reaction_fire_count < reaction_fire_max);
                                        reaction_fire[reaction_fire_count++] = target;
                                    }break;
                                
                                    default:
                                    {
                                        mark_block_destroyed(vars, target);
                                    }break;
                                    }
                                }
                                entity->step_forward = 3;
                                play_sound_effect(vars, vars->zombie_break,
                                                  0.1f, random_real32(vars, BEND_DOWN, BEND_UP));
                            }
                        }
                    
                        else{
                            Assert(1);
                        }
                    }
                }
                
                for (i32 i = 0; i < vars->entity_count; ++i){
                    Entity *entity = vars->entities + i;
                    Assert(!(entity->firing && entity->step_forward));
                    if (entity->step_forward){
                        --entity->step_forward;
                        vars->chain_reacting = 1;
                        vars->reaction_timer = COMBO_TIME;
                        Assert(entity->type == ZOMBIE);
                    
                        Grid_Pos pos = get_grid_pos(entity);
                        switch (entity->facing){
                        case RIGHT:
                            pos.x += 1;
                            break;
                        
                        case UP:
                            pos.y -= 1;
                            break;
                        
                        case LEFT:
                            pos.x -= 1;
                            break;
                        
                        case DOWN:
                            pos.y += 1;
                            break;
                        }
                        
                        bool32 out_of_grid;
                        if (can_move_to(vars, entity, pos.x, pos.y, &out_of_grid)){
                            move_entity(vars, entity, pos.x, pos.y);
                            entity->step_forward = 0;
                            
                            Sound *snd;
                            if ((pcg32_random_r(&vars->rnd) % 2) == 0){
                                snd = &vars->brains1;
                            }
                            else{
                                snd = &vars->brains2;
                            }
                            
                            play_sound_effect(vars, *snd,
                                              1.f, random_real32(vars, BEND_DOWN, BEND_UP));
                        }
                        if (out_of_grid){
                            entity->step_forward = 0;
                        }
                    }
                }
            
                for (i32 i = 0; i < reaction_fire_count; ++i){
                    reaction_fire[i]->firing = 1;
                }
            
                end_temp_memory(gun_temp);
            }
        
            // TRY TO SPAWN BLOCK
            if (vars->need_new_block && !vars->need_to_fill_gaps && !vars->chain_reacting){
                make_random_block(vars);
                vars->need_new_block = 0;
            }
        
            // DESTROY OBJECTS
            Assert(vars->free_count <= vars->entity_count);
            quick_sort((void**)vars->to_free, 0, vars->free_count-1);
            for (i32 i = 0; i < vars->free_count; ++i){
                Entity *entity = vars->to_free[i];
                
                if (entity->active){
                    vars->need_new_block = 1;
                }
                
                i32 entity_index = get_entity_index(vars, entity);
                
                bool32 play_splat_death = 0;
                
                switch (entity->type){
                case HUMAN:
                {
                    spawn_particles_type_1(vars, vars->prt_human_index, get_screen_pos(entity), 8);
                    spawn_particles_type_1(vars, vars->prt_bone_index, get_screen_pos(entity), 6);
                    spawn_particles_type_1(vars, vars->prt_blood_index, get_screen_pos(entity), 15);
                    spawn_particles_type_1(vars, vars->prt_brain_index, get_screen_pos(entity), 2);
                    
                    play_splat_death = 1;
                }break;
                
                case ZOMBIE:
                {
                    spawn_particles_type_1(vars, vars->prt_zombie_index, get_screen_pos(entity), 8);
                    spawn_particles_type_1(vars, vars->prt_bone_index, get_screen_pos(entity), 6);
                    spawn_particles_type_1(vars, vars->prt_blood_index, get_screen_pos(entity), 15);
                    spawn_particles_type_1(vars, vars->prt_brain_index, get_screen_pos(entity), 2);
                    
                    play_splat_death = 1;
                }break;
                
                case AMMO:
                {
                    spawn_particles_type_2(vars, vars->prt_ammo_index, get_screen_pos(entity), 12);
                }break;
                
                case BRAIN:
                {
                    spawn_particles_type_1(vars, vars->prt_brain_index, get_screen_pos(entity), 6);
                    spawn_particles_type_1(vars, vars->prt_blood_index, get_screen_pos(entity), 15);
                    
                    play_splat_death = 1;
                }break;
                
                case WALL:
                {
                    spawn_particles_type_1(vars, vars->prt_wall_index, get_screen_pos(entity), 12);
                }break;
                
                case BOMB:
                {
                    spawn_particles_type_1(vars, vars->prt_tnt_index, get_screen_pos(entity), 12);
                }break;
                }
                
                if (play_splat_death){
                    play_sound_effect(vars, vars->splat_death,
                                      1.f, random_real32(vars, BEND_DOWN, BEND_UP));
                }
                
                vars->grid[get_i(entity->grid_x, entity->grid_y)] = 0;
                --vars->entity_count;
                if (vars->entity_count > entity_index){
                    Entity *end_entity = vars->entities + vars->entity_count;
                    vars->grid[get_i(end_entity->grid_x, end_entity->grid_y)] = entity;
                    *entity = *end_entity;
                }
            }
            vars->free_count = 0;
            
            // SPAWN OBJECTS
            for (i32 i = 0; i < vars->spawn_count; ++i){
                Spawn_Request spawn = vars->spawns[i];
                
                i32 x, y;
                Block_Type type;
                i32 facing;
                if (spawn.random){
                    x = pcg32_random_r(&vars->rnd) % WIDTH;
                    y = 0;
                    
#if TEST_ORDER == 0
                    //type = (Block_Type)(ZOMBIE + pcg32_random_r(&vars->rnd) % type_count);
                    type = block_freq_table[pcg32_random_r(&vars->rnd) % ArrayCount(block_freq_table)];
#else
                    Block_Type test_types[] = {
                        AMMO,
                        HUMAN
                    };
                
                    type = test_types[vars->test_type_i++];
                    vars->test_type_i = vars->test_type_i % ArrayCount(test_types);
#endif
                    facing = (i32)(pcg32_random_r(&vars->rnd) % 4);
                }
                else{
                    x = spawn.x;
                    y = spawn.y;
                    type = spawn.type;
                    facing = (i32)(pcg32_random_r(&vars->rnd) % 4);
                }
            
                if (vars->grid[get_i(x, y)] == 0){
                    Entity *entity = spawn_entity(vars);
                    entity->type = type;
                    entity->grid_x = x;
                    entity->grid_y = y;
                    entity->show_x = (real32)x;
                    entity->show_y = (real32)y;
                    entity->active = 1;
                    entity->facing = facing;
                
                    vars->grid[get_i(x, y)] = entity;
                }
                else{
                    vars->game_over = 1;
                }
            }
        
            vars->spawn_count = 0;
        }
    
        // CHECK EFFECTS FROM POINTS
        i32 next_level = vars->level + 1;
        if (vars->score >= get_level_min_score(vars, next_level)){
            vars->level = next_level;
        }
        
        i32 score_increase = vars->score - prev_score;
        
        if (score_increase > 0){
            Sound *snd;
            if (score_increase >= 100){
                snd = &vars->score1;
            }
            else{
                snd = &vars->score2;
            }
            play_sound_effect(vars, *snd,
                              1.1f, random_real32(vars, BEND_DOWN, BEND_UP),
                              15);
        }
    
    }
    
    // RENDERING
    Vec2 screen_center;
    screen_center.x = (real32)target.render.width * 0.5f;
    screen_center.y = (real32)target.render.height * 0.5f;
    
    if (vars->title_screen > 0){
        Vec4 black;
        black = V4(0.f, 0.f, 0.f, 1.f);
        draw_clear(black);

        
        persist i32 PHASE_0 = 120;
        persist i32 PHASE_1 = 150;
        persist i32 PHASE_2 = 210;
        persist i32 PHASE_3 = 330;
        persist i32 PHASE_4 = 400;
        
        if (vars->title_screen < PHASE_4){
            ++vars->title_screen;
        }
        
        if (vars->title_screen < PHASE_0){
            real32 a = (vars->title_screen / (real32)(PHASE_0));
            
            Vec4 color = V4(1.f, 1.f, 1.f, a);
            draw_texture(vars->overreact, screen_center, screen_center, 0.f, color);
        }
        else if (vars->title_screen < PHASE_1){
            Vec4 color = V4(1.f, 1.f, 1.f, 1.f);
            draw_texture(vars->overreact, screen_center, screen_center, 0.f, color);
        }
        else if (vars->title_screen < PHASE_2){
            real32 a = ((PHASE_2 - vars->title_screen) / (real32)(PHASE_2 - PHASE_1));
            
            Vec4 color = V4(1.f, 1.f, 1.f, a);
            draw_texture(vars->overreact, screen_center, screen_center, 0.f, color);
        }
        else{
            real32 a = 1.f;
            if (vars->title_screen < PHASE_3){
                a = ((vars->title_screen - PHASE_2) / (real32)(PHASE_3 - PHASE_2));
            }
            
            Vec4 color = V4(1.f, 1.f, 1.f, a);
            draw_texture(vars->title, screen_center, screen_center, 0.f, color);
            
            if (vars->title_screen >= PHASE_3){
                a = 1.f;
                if (vars->title_screen < PHASE_4){
                    a = ((vars->title_screen - PHASE_3) / (real32)(PHASE_4 - PHASE_3));
                }
                
                if (input.button[0]){
                    vars->title_screen = 0;
                    vars->controls_screen = 1;
                    
                    vars->music = {};
                    vars->music.sound = vars->gameplay2;
                    vars->music.playing = 1;
                    vars->music.volume = 0.03f;
                    vars->music.bend = 1.f;
                }
                
                color = V4(1.f, 1.f, 1.f, a);
                draw_texture(vars->title_button, V2(400.f, 420.f), 0.f, color);
            }
        }
    }
    else{
        draw_texture(vars->background, screen_center, screen_center);
    
        draw_texture(vars->scoreback, V2(110.f, 80.f));
        draw_texture(vars->scorename, V2(110.f, 50.f));
    
        char score_string[16];
        char post_string[16];
        persist char mil_string[] = "MILLION";
    
        i32 i, j;
        i = int_to_string(score_string, vars->score);
        j = int_to_string(post_string, vars->nonsense_score, 3);
        
        real32 text_x, text_y, start_x, end_x;
        text_x = 20.f;
        text_y = 100.f;
        start_x = 20.f;
        end_x = 180.f;
        
        Vec4 white = {1.f, 1.f, 1.f, 1.f};
        AllowLocal(white);
        Vec4 red = {1.f, 0.f, 0.f, 1.f};
    
        draw_text(&target.render, &vars->small_font, &text_x, &text_y, score_string + i, 16 - i, red, start_x, end_x);
        if (vars->score > 0){
            draw_text(&target.render, &vars->small_font, &text_x, &text_y, post_string + j, 16 - j, red, start_x, end_x);
            text_x = start_x;
            text_y += vars->small_font.height;
            draw_text(&target.render, &vars->small_font, &text_x, &text_y, mil_string, ArrayCount(mil_string) - 1, red, start_x, end_x);
        }
        
        text_x = 20.f;
        text_y = 300.f;
        start_x = 20.f;
        end_x = 180.f;
    
        persist char level_label[] = "LEVEL: ";
        draw_text(&target.render, &vars->small_font, &text_x, &text_y, level_label, ArrayCount(level_label) - 1, red, start_x, end_x);
    
        char level_string[16];
        i = int_to_string(level_string, vars->level + 1);
        draw_text(&target.render, &vars->small_font, &text_x, &text_y, level_string + i, 16 - i, red, start_x, end_x);
    
        text_x = 20.f;
        text_y = 475.f;
        start_x = 20.f;
        end_x = 180.f;
    
        persist char best_label[] = "THE MOST SCORE";
        draw_text(&target.render, &vars->small_font, &text_x, &text_y, best_label, ArrayCount(best_label) - 1, red, start_x, end_x);
        text_x = start_x;
        text_y += vars->small_font.height;
        draw_text(&target.render, &vars->small_font, &text_x, &text_y, vars->teaser_score, vars->teaser_score_len, red, start_x, end_x);
        text_x = start_x;
        text_y += vars->small_font.height;
        draw_text(&target.render, &vars->small_font, &text_x, &text_y, vars->teaser_name, vars->teaser_name_len, red, start_x, end_x);
    
        for (i32 i = 0; i < vars->entity_count; ++i){
            Entity *entity = vars->entities + i;
        
            persist real32 BLOCK_LERP_SPEED = 0.5f;
        
            if (entity->show_x != entity->grid_x){
                entity->show_x = lerp(entity->show_x, BLOCK_LERP_SPEED, (real32)entity->grid_x);
                if (abs(entity->show_x - entity->grid_x) < 0.05){
                    entity->show_x = (real32)(entity->grid_x);
                }
            }
        
            if (entity->show_y != entity->grid_y){
                entity->show_y = lerp(entity->show_y, BLOCK_LERP_SPEED, (real32)entity->grid_y);
                if (abs(entity->show_y - entity->grid_y) < 0.05){
                    entity->show_y = (real32)(entity->grid_y);
                }
            }
            
            real32 x, y;
            x = entity->show_x;
            y = entity->show_y;
            
            Render_Texture *texture = 0;
            switch (entity->type){
            case ZOMBIE:
                texture = &vars->zombie[0];
                break;
            
            case HUMAN:
            {
                i32 infection_level = (entity->infection_amount * 4) / ZOMBIE_TURN_THRESHOLD;
                texture = &vars->human[0][infection_level];
            }break;
        
            case BRAIN:
                texture = &vars->brain;
                break;
            
            case AMMO:
                texture = &vars->ammo;
                break;
            
            case BOMB:
                texture = &vars->bomb;
                break;
            
            case WALL:
                texture = &vars->wall;
                break;
            }
        
            Vec2 center = get_screen_pos(entity);
        
            persist real32 SCALE_DOWN = 0.3168316f;
        
            Vec2 halfdim;
            halfdim.x = texture->img_width * .5f * SCALE_DOWN;
            halfdim.y = texture->img_height * .5f * SCALE_DOWN;
        
            real32 rotation = 0.f;
            if (entity->type == ZOMBIE){
                switch (entity->facing){
                case RIGHT:
                    rotation = 0.f;
                    break;
                
                case UP:
                    rotation = 270.f;
                    break;
                
                case LEFT:
                    rotation = 180.f;
                    break;
                
                case DOWN:
                    rotation = 90.f;
                    break;
                }
            }
            
            else{
                switch (entity->facing){
                case RIGHT:
                    rotation = 90.f;
                    break;
                    
                case UP:
                    rotation = 0.f;
                    break;
                
                case LEFT:
                    rotation = 270.f;
                    break;
                
                case DOWN:
                    rotation = 180.f;
                    break;
                }
            }
            
            if (entity->active){
                rotation += 10.f*sinf(entity->wobble);
                if (entity->wobble > 0.f && entity->wobble < TAU32){
                    entity->wobble += TAU32 / 5.f;
                }
                else{
                    entity->wobble = 0;
                }
            }
            
            if (entity->firing){
                switch (entity->type){
                case BOMB:
                {
                    halfdim *= 1.1f;
                    rotation += 10.f*sinf(entity->wobble);
                    entity->wobble += TAU32 / 5.f;
                
                    draw_texture(*texture, center, halfdim, rotation);
                }break;
                
                case AMMO:
                {
                    Vec4 color = V4(1.f, 1.f, 1.f, 1.f);
                    color.g = abs(sinf(entity->wobble))*0.5f + 0.5f;
                    color.b = color.g;
                    entity->wobble += TAU32 / 20.f;
                    
                    draw_texture(*texture, center, halfdim, rotation, color);
                }break;
                
                default:
                {
                    draw_texture(*texture, center, halfdim, rotation);
                }break;
                }
            }
            else if (entity->step_forward){
                halfdim *= (1.f + sinf(entity->wobble)*.15f + .15f);
                entity->wobble += TAU32 / 25.f;
                
                draw_texture(*texture, center, halfdim, rotation);
            }
            else{
                draw_texture(*texture, center, halfdim, rotation);
            }
        }
    
        // PARTICLE RENDER
        Particle *part = vars->particles;
        Particle *end_part = vars->particles + vars->particle_count;
        Render_Texture *part_texs = vars->prt_textures;
        for (; part < end_part;){
            if ((--part->life_counter) > 0){
                part->pos += part->vel;
                part->rot += part->rot_vel;
                part->vel.y += 0.5f;
            
                if (part->pos.y < 700.f){
                    draw_texture(part_texs[part->tex_index], part->pos, part->rot);
                    ++part;
                }
                else{
                    --end_part;
                    *part = *end_part;
                }
            }
            else{
                --end_part;
                *part = *end_part;
            }
        }
        vars->particle_count = (i32)(end_part - vars->particles);
        Assert(vars->particle_count >= 0);
        
        draw_texture(vars->shadow, screen_center, screen_center, 0.f, V4(1.f, 1.f, 1.f, 0.4f));

        // CONTROLS SCREEN RENDER
        if (vars->controls_screen){
            draw_rectangle(0.f, 0.f, screen_center.x*2, screen_center.y*2,
                           V4(0.f, 0.f, 0.f, 0.5f));
            draw_texture(vars->controls, screen_center, screen_center);
            
            i32 field_index = 0;
            if (do_button(field_index++, vars->active_field,
                          input, vars->finish_button,
                          690.f, 500.f)){
                vars->controls_screen = 0;
            }
        
            if (input.digital.up && !vars->prev_input.digital.up){
                vars->active_field += (field_index - 1);
                vars->active_field %= field_index;
            }
        
            if (input.digital.down && !vars->prev_input.digital.down){
                vars->active_field += 1;
                vars->active_field %= field_index;
            }
        }
        
        // GAME OVER RENDER
        else if (vars->game_over){
            draw_rectangle(0.f, 0.f, screen_center.x*2, screen_center.y*2,
                           V4(0.f, 0.f, 0.f, 0.5f));
            draw_texture(vars->gameover, screen_center, screen_center);
        
            i32 field_index = 0;
            bool32 blink_on = (vars->blink_timer < 15);
            vars->blink_timer = (vars->blink_timer + 1) % 30;
        
            bool32 move_down = 0;
        
            if (do_text_field(field_index++, vars->active_field, blink_on,
                              &target.render, input,
                              &vars->font, 305.f, 285.f,
                              &vars->user_name_len, vars->user_name, 15)){
                move_down = 1;
            }
        
            if (do_text_field(field_index++, vars->active_field, blink_on,
                              &target.render, input,
                              &vars->font, 305.f, 342.f,
                              &vars->user_token_len, vars->user_token, 15)){
                move_down = 1;
            }
        
            if (do_button(field_index++, vars->active_field,
                          input, vars->finish_button,
                          690.f, 500.f)){
                if (vars->user_name_len != 0){
                    vars->user_name[vars->user_name_len] = 0;
                    vars->user_token[vars->user_token_len] = 0;
                    
                    char score_string[64];
                    i32 i = 64, j;
                    if (vars->score > 0){
                        score_string[--i] = 0;
                        persist char short_mil_string[] = " MIL";
                        for (i32 k = ArrayCount(short_mil_string) - 2; k >= 0; --k){
                            score_string[--i] = short_mil_string[k];
                        }
                    
                        j = int_to_string(score_string + i - 16, vars->nonsense_score, 3);
                        j = (16 - j);
                        i -= j;
                        
                        j = int_to_string(score_string + i - 16, vars->score);
                        j = (16 - j);
                        i -= j;
                    }
                    else{
                        persist char zero_string[] = "ZERO";
                        score_string[--i] = 0;
                        for (i32 j = ArrayCount(zero_string) - 2; j >= 0; --j){
                            score_string[--i] = zero_string[j];
                        }
                    }
                
                    bool32 guest_score = 0;
                    if (vars->user_token_len != 0){
                        if (gj_login(vars->gj, vars->user_name, vars->user_token)){
                            gj_post_score(vars->gj, vars->table_id, score_string + i, vars->score, "", "");
                            gj_logout(vars->gj);
                        }
                        else{
                            guest_score = 1;
                        }
                    }
                    else{
                        guest_score = 1;
                    }
                
                    if (guest_score){
                        gj_post_score(vars->gj, vars->table_id, score_string + i, vars->score, "", vars->user_name);
                    }
                }
                
                game_set_to_new(vars);
            }
        
            if (input.digital.up && !vars->prev_input.digital.up){
                vars->active_field += (field_index - 1);
                vars->active_field %= field_index;
            }
        
            if ((input.digital.down && !vars->prev_input.digital.down) || move_down){
                vars->active_field += 1;
                vars->active_field %= field_index;
            }
        }
    }
    
    vars->prev_input = input;
    
    return 0;
}

// BOTTOM
