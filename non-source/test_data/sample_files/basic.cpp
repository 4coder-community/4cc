/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Application layer for project codename "4ed"
 *
 */

#if 1 // HELLO
#endif // HELLO
#include <stuff> // HELLO!

int $c = 1;

struct Partitioñ_€ursor{
    u8 *memory_base;
    u8 *memory_cursor;
    i32 max_size;
};

// HERE

internal Partitioñ_€ursor
partitioñ_open(void *memory, i32 size){
    Partitioñ_€ursor partitioñ = {};
    partitioñ.memory_base = partitioñ.memory_cursor = (u8*)memory;
    partitioñ.max_size = size;
    return partitioñ;
}

R"()"
LR"foo()foo"
u8"foo"

u8'a'
R'z'

#   defin X (x)

internal void*
partitioñ_allocate(Partition_€ursor *data, i32 size){
    void *ret = 0;
    if ((data->memory_cursor - data->memory_base) + size <= data->max_size && size > 0){
        ret = data->memory_cursor;
        data->memory_cursor += size;
    }
    return ret;
}

/*
 * Plaform Independent File Name Helpers
 */

struct File_List_Iterator{
    bool32 folder_stage;
    u8 **filename_ptr;
};

internal File_List_Iterator
files_iterator_init(File_List *files){
    File_List_Iterator files_it = {};
    if (files->folder_count > 0){
        files_it.filename_ptr = files->folder_names;
        files_it.folder_stage = 1;
    }
    else if (files->file_count > 0){
        files_it.filename_ptr = files->file_names;
    }
    return files_it;
}

internal void
files_iterator_step(File_List *files, File_List_Iterator *files_it){
    ++files_it->filename_ptr;
    if (files_it->folder_stage){
        if (files_it->filename_ptr >= files->folder_names + files->folder_count){
            if (files->file_count > 0){
                files_it->filename_ptr = files->file_names;
            }
            else{
                files_it->filename_ptr = 0;
            }
            files_it->folder_stage = 0;
        }
    }
    else{
        if (files_it->filename_ptr >= files->file_names + files->file_count){
            files_it->filename_ptr = 0;
        }
    }
}

/*
 * Drawing Functions
 */

R"raw()raw"
internal u32
style_token_color(Editing_Style *style, Cpp_Token_Type type){
    u32 result;
    switch (type){
        case CPP_TOKEN_COMMENT:
        result = style->comment_color;
        break;
        
        case CPP_TOKEN_KEYWORD:
        result = style->keyword_color;
        break;
        
        case CPP_TOKEN_STRING_CONSTANT:
        case CPP_TOKEN_CHARACTER_CONSTANT:
        case CPP_TOKEN_INTEGER_CONSTANT:
        case CPP_TOKEN_FLOATING_CONSTANT:
        case CPP_TOKEN_BOOLEAN_CONSTANT:
        case CPP_TOKEN_INCLUDE_FILE:
        result = style->constant_color;
        break;
        
        default:
        result = style->default_color;
        break;
    }
    return result;
}
R"raw()raw";

internal void
panel_draw(Thread_Context *thread, Render_Target *target, Editing_Panel *panel, bool32 is_active){
    Editing_File *file = panel->file;
    Editing_Style *style = file->style;
    Font *font = style->font;
    
    i32 character_w = (i32)(style_get_character_width(style));
    i32 character_h = (i32)(font->line_skip);
    i32 offset_x = (i32)(panel->x);
    i32 offset_y = (i32)(panel->y);
    i32 max_x = (i32)(panel->w);
    i32 max_y = (i32)(panel->h);
    
    Blit_Rect panel_area;
    panel_area.x_start = offset_x;
    panel_area.y_start = offset_y;
    panel_area.x_end = offset_x + max_x;
    panel_area.y_end = offset_y + max_y;
    
    if (!file || !file->data || file->is_dummy){
        i32 start_x = (panel_area.x_start + panel_area.x_end)/2;
        i32 start_y = (panel_area.y_start + panel_area.y_end)/2;
        persist String null_file_message = make_lit_string("NULL FILE");
        start_x -= (character_w*null_file_message.size)/2;
        start_y -= (character_h)/2;
        
        real32 pos_x = 0;
        real32 pos_y = 0;
        
        for (i32 i = 0; i < null_file_message.size; ++i){
            u8 to_render = null_file_message.str[i];
            
            if (font->glyphs[to_render].data){
                font_draw_glyph_clipped(target, font, to_render, (real32)start_x + pos_x, (real32)start_y + pos_y, style->special_character_color, panel_area);
                pos_x += character_w;
            }
        }
    }
    
    else{
        u32 tab_width = style->tab_width;
        i32 size = (i32)file->size;
        u8 *data = (u8*)file->data;
        
        real32 shift_x = 0;
        shift_x = -panel->scroll_x * character_w;
        
        i32 truncated_start_y = (i32)panel->scroll_y;
        real32 scroll_amount = panel->scroll_y - truncated_start_y;
        
        Panel_Cursor_Data start_cursor;
        start_cursor = panel->scroll_y_cursor;
        start_cursor = panel_compute_cursor_from_xy(panel, 0, truncated_start_y);
        panel->scroll_y_cursor = start_cursor;
        
        i32 start_character = start_cursor.pos;
        
        real32 pos_x = 0;
        real32 pos_y = -character_h*scroll_amount;
        
        Cpp_Token_Stack token_stack = file->token_stack;
        u32 main_color = style->default_color;
        u32 highlight_color = 0x00000000;
        i32 token_i = 0;
        bool32 tokens_exist = file->tokens_exist;
        
        if (tokens_exist){
            // TODO(allen): Use cpp_get_token, it binary searches this shit!
            while (token_i < token_stack.count &&
                   start_character > token_stack.tokens[token_i].start){
                ++token_i;
            }
            if (token_i != 0){
                main_color = style_token_color(style, token_stack.tokens[token_i-1].type);
                if (token_stack.tokens[token_i-1].type == CPP_TOKEN_JUNK){
                    highlight_color = style->highlight_junk_color;
                }
            }
        }
        
        // TODO(allen): Render through NULLS
        for (i32 i = start_character; i < size && data[i]; ++i){
            u8 to_render = data[i];
            
            u32 fade_color = 0xFFFF00FF;
            real32 fade_amount = 0.f;
            if (style->use_paste_color && panel->paste_effect.tick_down > 0 && panel->paste_effect.start <= i && i < panel->paste_effect.end){
                fade_color = style->paste_color;
                fade_amount = (real32) ((panel->paste_effect.tick_down)/panel->paste_effect.tick_max);
            }
            
            if (tokens_exist && token_i < token_stack.count){
                if (i == token_stack.tokens[token_i].start){
                    main_color = style_token_color(style, token_stack.tokens[token_i].type);
                    ++token_i;
                }
                if (token_i > 0 && i >= token_stack.tokens[token_i-1].start + token_stack.tokens[token_i-1].size){
                    main_color = 0xFFFFFFFF;
                }
            }
            
            highlight_color = 0x00000000;
            
            if (tokens_exist && token_i > 0 && token_stack.tokens[token_i-1].type == CPP_TOKEN_JUNK && i >= token_stack.tokens[token_i-1].start && i <= token_stack.tokens[token_i-1].start + token_stack.tokens[token_i-1].size){
                highlight_color = style->highlight_junk_color;
            }
            else if (panel->show_whitespace && character_is_any_whitespace(data[i])){
                highlight_color = style->highlight_white_color;
            }
            
            i32 cursor_mode = 0;
            if (panel->show_temp_highlight){
                if (panel->temp_highlight.pos <= i && i < panel->temp_highlight_end_pos){
                    cursor_mode = 2;
                }
            }
            else{
                if (panel->cursor.pos == i){
                    cursor_mode = 1;
                }
            }
            
            if (!panel->unwrapped_lines && pos_x + character_w > max_x){
                pos_x = 0;
                pos_y += font->line_skip;
            }
            
            if (highlight_color != 0){
                if (file->endline_mode == ENDLINE_RN_COMBINED &&
                    to_render == '\r' &&
                    i + 1 < size &&
                    data[i+1] == '\n'){
                    // DO NOTHING
                }
                else{
                    draw_rectangle_clipped(target,
                                           (i32)shift_x + offset_x + (i32)pos_x,
                                           offset_y + (i32)pos_y,
                                           character_w, font->line_skip,
                                           highlight_color, panel_area);
                }
            }
            
            if (cursor_mode){
                if (is_active){
                    u32 color = 0x00000000;
                    switch (cursor_mode){
                        case 1:
                        color = style->cursor_color;
                        break;
                        case 2:
                        color = style->highlight_color;
                        break;
                    }
                    draw_rectangle_clipped(target,
                                           (i32)shift_x + offset_x + (i32)pos_x,
                                           offset_y + (i32)pos_y,
                                           character_w, font->line_skip,
                                           color, panel_area);
                }
                else{
                    draw_rectangle_outline_clipped(target, (i32)shift_x + offset_x + (i32)pos_x, offset_y + (i32)pos_y,
                                                   character_w, font->line_skip, style->cursor_color,
                                                   panel_area);
                }
            }
            
            if (i == panel->mark){
                draw_rectangle_outline_clipped(target, (i32)shift_x + offset_x + (i32)pos_x, offset_y + (i32)pos_y,
                                               character_w, font->line_skip, style->mark_color,
                                               panel_area);
            }
            
            if (to_render == '\r'){
                switch (file->endline_mode){
                    case ENDLINE_RN_COMBINED:
                    {
                        if (i + 1 < size && data[i+1] == '\n'){
                            // DO NOTHING
                        }
                        else{
                            u32 char_color = style->special_character_color;
                            if (cursor_mode && is_active){
                                switch (cursor_mode){
                                    case 1:
                                    char_color = style->at_cursor_color;
                                    break;
                                    case 2:
                                    char_color = style->at_highlight_color;
                                    break;
                                }
                            }
                            
                            char_color = color_blend(char_color, fade_amount, fade_color);
                            
                            font_draw_glyph_clipped(target, font, '\\',
                                                    shift_x + offset_x + pos_x,
                                                    (real32)offset_y + pos_y,
                                                    char_color, panel_area);
                            
                            pos_x += character_w;
                            draw_rectangle_clipped(target,
                                                   (i32)shift_x + offset_x + (i32)pos_x,
                                                   offset_y + (i32)pos_y,
                                                   character_w, font->line_skip,
                                                   highlight_color, panel_area);
                            
                            font_draw_glyph_clipped(target, font, 'r',
                                                    shift_x + offset_x + pos_x,
                                                    (real32)offset_y + pos_y,
                                                    char_color, panel_area);
                            pos_x += character_w;
                        }
                    }break;
                    
                    case ENDLINE_RN_SEPARATE:
                    {
                        pos_x = 0;
                        pos_y += font->line_skip;
                    }break;
                    
                    case ENDLINE_RN_SHOWALLR:
                    {
                        u32 char_color = style->special_character_color;
                        if (cursor_mode && is_active){
                            switch (cursor_mode){
                                case 1:
                                char_color = style->at_cursor_color;
                                break;
                                case 2:
                                char_color = style->at_highlight_color;
                                break;
                            }
                        }
                        
                        char_color = color_blend(char_color, fade_amount, fade_color);
                        
                        font_draw_glyph_clipped(target, font, '\\',
                                                shift_x + offset_x + pos_x,
                                                (real32)offset_y + pos_y,
                                                char_color, panel_area);
                        
                        pos_x += character_w;
                        draw_rectangle_clipped(target,
                                               (i32)shift_x + offset_x + (i32)pos_x,
                                               offset_y + (i32)pos_y,
                                               character_w, font->line_skip,
                                               highlight_color, panel_area);
                        
                        font_draw_glyph_clipped(target, font, 'r',
                                                shift_x + offset_x + pos_x,
                                                (real32)offset_y + pos_y,
                                                char_color, panel_area);
                        pos_x += character_w;
                    }break;
                }
            }
            
            else if (to_render == '\n'){
                pos_x = 0;
                pos_y += font->line_skip;
            }
            else if (to_render == '\t'){
                if (highlight_color != 0){
                    draw_rectangle_clipped(target,
                                           (i32)shift_x + offset_x + (i32)pos_x + character_w,
                                           offset_y + (i32)pos_y,
                                           character_w*(tab_width-1), font->line_skip,
                                           highlight_color, panel_area);
                }
                pos_x += character_w*tab_width;
            }
            
            else if (font->glyphs[to_render].data){
                u32 char_color = main_color;
                if (cursor_mode && is_active){
                    switch (cursor_mode){
                        case 1:
                        char_color = style->at_cursor_color;
                        break;
                        case 2:
                        char_color = style->at_highlight_color;
                        break;
                    }
                }
                
                char_color = color_blend(char_color, fade_amount, fade_color);
                
                font_draw_glyph_clipped(target, font, to_render,
                                        shift_x + offset_x + pos_x,
                                        (real32)offset_y + pos_y, char_color,
                                        panel_area);
                pos_x += character_w;
            }
            
            else{
                pos_x += character_w;
            }
            
            if (pos_y > max_y){
                break;
            }
        }
    }
}

/*
 * Hot Directory
 */

struct Hot_Directory{
    String string;
    File_List file_list;
};

internal void
hot_directory_init(Hot_Directory *hot_directory){
    hot_directory->string = make_string((u8*)system_get_memory(256, SYSTEM_MEM_SMALL_STRING), 0, 256);
    
    i32 dir_size = system_get_working_directory(hot_directory->string.str,
                                                hot_directory->string.memory_size);
    
    if (dir_size <= 0){
        dir_size = system_get_easy_directory(hot_directory->string.str);
    }
    
    hot_directory->string.size = dir_size;
    
    append(&hot_directory->string, (u8*)"\\");
}

internal void
hot_directory_reload_list(Hot_Directory *hot_directory){
    if (hot_directory->file_list.block){
        system_free_file_list(hot_directory->file_list);
    }
    hot_directory->file_list = system_get_files(hot_directory->string);
}

internal bool32
hot_directory_set(Hot_Directory *hot_directory, String str){
    bool32 did_set = 0;
    if (copy_checked(&hot_directory->string, str)){
        did_set = 1;
        system_free_file_list(hot_directory->file_list);
        hot_directory->file_list = system_get_files(str);
    }
    return did_set;
}

struct Hot_Directory_Match{
    u8 *filename;
    bool32 is_folder;
};

internal Hot_Directory_Match
hot_directory_first_match(Hot_Directory *hot_directory,
                          String directory,
                          bool32 include_files,
                          bool32 exact_match){
    Hot_Directory_Match result = {};
    
    File_List files = hot_directory->file_list;
    File_List_Iterator files_it = files_iterator_init(&files);
    
    while (files_it.filename_ptr &&
           (include_files || files_it.folder_stage)){
        u8 *filename = *files_it.filename_ptr;
        
        bool32 is_match = 0;
        if (exact_match){
            if (match(filename, directory)){
                is_match = 1;
            }
        }
        else{
            if (directory.size == 0 ||
                has_substr_unsensitive(filename, directory)){
                is_match = 1;
            }
        }
        
        if (is_match){
            result.is_folder = files_it.folder_stage;
            result.filename = filename;
            break;
        }
        
        files_iterator_step(&files, &files_it);
    }
    
    return result;
}

/*
* App Structs
*/

struct Command_Data;
typedef void (*Command_Function)(Command_Data *command);

enum Input_Request_Type{
    REQUEST_SYS_FILE = 0,
    REQUEST_LIVE_FILE = 1,
    // never below this
    REQUEST_COUNT = 2
};

struct Input_Request{
    Input_Request_Type type;
    union{
        struct{
            String query;
            String dest;
            bool32 hit_ctrl_newline;
            bool32 fast_folder;
        } sys_file;
        
        struct{
            String query;
            String dest;
            bool32 hit_ctrl_newline;
        } live_file;
    };
};

enum App_State{
    APP_STATE_EDIT,
    APP_STATE_SEARCH,
    APP_STATE_RESIZING,
    // never below this
    APP_STATE_COUNT
};

struct App_State_Incremental_Search{
    String str;
    bool32 reverse;
    i32 pos;
};

struct App_State_Resizing{
    Editing_Panel *left, *right;
};

struct Command_Map{
    Command_Function basic_mode_key[1+sizeof(Key_Codes)/2];
    Command_Function control_ascii[128];
    Command_Function control_key[1+sizeof(Key_Codes)/2];
};

struct App_Vars{
    Command_Map map;
    
    Font font;
    Editing_Style style;
    Interactive_Style command_style;
    
    Editing_Working_Set working_set;
    
    Editing_Layout layout;
    real32 last_click_x, last_click_y;
    
    Hot_Directory hot_directory;
    
    Input_Request request_queue[16];
    i32 request_count, request_filled, request_max;
    
    Command_Function pending_command;
    
    App_State state;
    union{
        App_State_Incremental_Search isearch;
        App_State_Resizing resizing;
    };
};

/*
* Commands
*/

struct Command_Data{
    Editing_Panel *panel;
    Editing_Working_Set *working_set;
    Editing_Layout *layout;
    Editing_Style *style;
    App_Vars *vars;
    
    i32 screen_width, screen_height;
    i32 screen_y_off;
    Key_Event_Data key;
    
    Input_Request *requests;
    i32 request_count, request_filled;
};

internal void
app_clear_request_queue(App_Vars *vars){
    for (i32 i = 0; i < vars->request_count; ++i){
        Input_Request *request = vars->request_queue + i;
        switch (request->type){
            case REQUEST_SYS_FILE:
            {
                system_free_memory(request->sys_file.query.str);
                system_free_memory(request->sys_file.dest.str);
            }break;
            
            case REQUEST_LIVE_FILE:
            {
                system_free_memory(request->live_file.query.str);
                system_free_memory(request->live_file.dest.str);
            }break;
        }
    }
    vars->request_count = 0;
    vars->request_filled = 0;
}

internal void
command_write_character(Command_Data *command){
    Editing_Panel *panel = command->panel;
    panel_write_character(panel, (u8)command->key.character);
    if (panel->unwrapped_lines){
        panel->preferred_x = panel->cursor.unwrapped_x;
    }
    else{
        panel->preferred_x = panel->cursor.wrapped_x;
    }
    panel->file->cursor.pos = panel->cursor.pos;
    
    switch ((u8)command->key.character){
        case '{': case '}':
        case '(': case ')':
        case ';': case ':':
        case '#': case '\n':
        {
            panel_auto_tab(panel, panel->cursor.pos, panel->cursor.pos);
        }break;
    }
    panel_measure_all_wrapped_y(panel);
}

internal void
command_move_left(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Editing_File *file = panel->file;
    u8 *data = (u8*)file->data;
    
    i32 pos = panel->cursor.pos;
    if (pos > 0){
        if (file->endline_mode == ENDLINE_RN_COMBINED){
            pos = pos_adjust_to_left(pos, data);
            if (pos > 0){
                --pos;
            }
            else{
                pos = pos_adjust_to_self(pos, data, file->size);
            }
        }
        else{
            --pos;
        }
    }
    
    panel_cursor_move(panel, pos);
}

internal void
command_move_right(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Editing_File *file = panel->file;
    i32 size = file->size;
    u8* data = (u8*)file->data;
    
    i32 pos = panel->cursor.pos;
    if (pos < size){
        ++pos;
        if (file->endline_mode == ENDLINE_RN_COMBINED){
            pos = pos_adjust_to_self(pos, data, size);
        }
    }
    
    panel_cursor_move(panel, pos);
}

internal void
command_backspace(Command_Data *command){
    Editing_Panel *panel = command->panel;
    i32 cursor_pos = panel->cursor.pos;
    Editing_File *file = panel->file;
    i8 *data = (i8*)file->data;
    
    if (cursor_pos > 0 && cursor_pos <= (i32)file->size){
        
        if (file->endline_mode == ENDLINE_RN_COMBINED){
            i32 target_pos = cursor_pos;
            if (cursor_pos > 1 &&
                data[cursor_pos] == '\n' &&
                data[cursor_pos-1] == '\r'){
                --target_pos;
            }
            
            if (target_pos > 1 &&
                data[target_pos-1] == '\n' &&
                data[target_pos-2] == '\r'){
                
                buffer_delete_range(file, target_pos-2, target_pos);
                if (panel->mark >= cursor_pos){
                    panel->mark -= 2;
                }
                cursor_pos -= 2;
            }
            else{
                if (target_pos > 0){
                    buffer_delete(file, target_pos-1);
                    if (panel->mark >= cursor_pos){
                        --panel->mark;
                    }
                    --cursor_pos;
                }
            }
        }
        else{
            buffer_delete(file, cursor_pos-1);
            if (panel->mark >= cursor_pos){
                --panel->mark;
            }
            --cursor_pos;
        }
        
        panel_measure_all_wrapped_y(panel);
        panel_cursor_move(panel, cursor_pos);
    }
}
internal void
command_delete(Command_Data *command){
    Editing_Panel *panel = command->panel;
    i32 cursor_pos = panel->cursor.pos;
    Editing_File *file = panel->file;
    i8 *data = (i8*)file->data;
    
    if (file->size > 0){
        if (file->endline_mode == ENDLINE_RN_COMBINED){
            if (cursor_pos > 0 &&
                data[cursor_pos-1] == '\r' &&
                data[cursor_pos] == '\n'){
                buffer_delete_range(file, cursor_pos-1, cursor_pos+1);
                if (panel->mark > cursor_pos){
                    panel->mark -= 2;
                }
                cursor_pos -= 1;
            }
            
            else{
                buffer_delete(file, cursor_pos);
                if (panel->mark > cursor_pos){
                    --panel->mark;
                }
            }
        }
        
        else{
            buffer_delete(file, cursor_pos);
            if (panel->mark > cursor_pos){
                --panel->mark;
            }
        }
        
        panel_measure_all_wrapped_y(panel);
        panel_cursor_move(panel, cursor_pos);
    }
}

internal void
command_move_up(Command_Data *command){
    Editing_Panel *panel = command->panel;
    i32 cy = panel_get_cursor_y(panel)-1;
    i32 px = panel->preferred_x;
    if (cy >= 0){
        panel->cursor = panel_compute_cursor_from_xy(panel, px, cy);
        panel->file->cursor.pos = panel->cursor.pos;
    }
}

internal void
command_move_down(Command_Data *command){
    Editing_Panel *panel = command->panel;
    i32 cy = panel_get_cursor_y(panel)+1;
    i32 px = panel->preferred_x;
    
    panel->cursor = panel_compute_cursor_from_xy(panel, px, cy);
    panel->file->cursor.pos = panel->cursor.pos;
}

internal void
command_seek_end_of_line(Command_Data *command){
    Editing_Panel *panel = command->panel;
    
    i32 pos = panel_find_end_of_line(panel, panel->cursor.pos);
    
    panel_cursor_move(panel, pos);
}

internal void
command_seek_beginning_of_line(Command_Data *command){
    Editing_Panel *panel = command->panel;
    
    i32 pos = panel_find_beginning_of_line(panel, panel->cursor.pos);
    
    panel_cursor_move(panel, pos);
}

internal void
command_seek_whitespace_right(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Editing_File *file = panel->file;
    u32 size = file->size;
    u8* data = (u8*)file->data;
    
    u32 pos = panel->cursor.pos;
    while (pos < size && character_is_any_whitespace(data[pos])){
        ++pos;
    }
    
    while (pos < size && !character_is_any_whitespace(data[pos])){
        ++pos;
    }
    
    panel_cursor_move(panel, pos);
}

internal void
command_seek_whitespace_left(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Editing_File *file = panel->file;
    u8* data = (u8*)file->data;
    
    u32 pos = panel->cursor.pos;
    --pos;
    while (pos > 0 && character_is_any_whitespace(data[pos])){
        --pos;
    }
    
    while (pos > 0 && !character_is_any_whitespace(data[pos])){
        --pos;
    }
    
    if (pos != 0){
        ++pos;
    }
    
    panel_cursor_move(panel, pos);
}

internal void
command_seek_whitespace_up(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Editing_File *file = panel->file;
    u8* data = (u8*)file->data;
    
    u32 pos = panel->cursor.pos;
    while (pos > 0 && character_is_any_whitespace(data[pos])){
        --pos;
    }
    
    bool32 no_hard_character = 0;
    while (pos > 0){
        if (starts_new_line(data[pos], file->endline_mode)){
            if (no_hard_character){
                break;
            }
            else{
                no_hard_character = 1;
            }
        }
        else{
            if (!character_is_any_whitespace(data[pos])){
                no_hard_character = 0;
            }
        }
        --pos;
    }
    
    if (pos != 0){
        ++pos;
    }
    
    if (file->endline_mode == ENDLINE_RN_COMBINED){
        pos = pos_adjust_to_self(pos, data, file->size);
    }
    
    panel_cursor_move(panel, pos);
}

internal void
command_seek_whitespace_down(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Editing_File *file = panel->file;
    i32 size = file->size;
    u8* data = (u8*)file->data;
    
    i32 pos = panel->cursor.pos;
    while (pos < size && character_is_any_whitespace(data[pos])){
        ++pos;
    }
    
    bool32 no_hard_character = 0;
    i32 prev_endline = -1;
    while (pos < size){
        if (starts_new_line(data[pos], file->endline_mode)){
            if (no_hard_character){
                break;
            }
            else{
                no_hard_character = 1;
                prev_endline = pos;
            }
        }
        else{
            if (!character_is_any_whitespace(data[pos])){
                no_hard_character = 0;
            }
        }
        ++pos;
    }
    
    if (prev_endline == -1 || prev_endline+1 >= size){
        pos = size-1;
    }
    else{
        pos = prev_endline+1;
    }
    
    panel_cursor_move(panel, pos);
}

internal void
command_seek_token_left(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Editing_File *file = panel->file;
    TentativeAssert(file->tokens_exist);
    i32 current_token;
    
    Cpp_Get_Token_Result get_result = cpp_get_token(&file->token_stack, panel->cursor.pos);
    current_token = get_result.token_index;
    
    // TODO(allen): Make nulltoken?
    if (current_token == -1){
        current_token = 0;
    }
    Cpp_Token *token = &file->token_stack.tokens[current_token];
    if (token->start == panel->cursor.pos && current_token > 0){
        --token;
    }
    
    panel_cursor_move(panel, token->start);
}

internal void
command_seek_token_right(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Editing_File *file = panel->file;
    TentativeAssert(file->tokens_exist);
    // TODO(allen): Make nulltoken?
    i32 current_token;
    
    Cpp_Get_Token_Result get_result = cpp_get_token(&file->token_stack, panel->cursor.pos);
    current_token = get_result.token_index;
    if (get_result.in_whitespace){
        ++current_token;
    }
    
    if (current_token >= file->token_stack.count){
        current_token = file->token_stack.count - 1;
    }
    
    Cpp_Token *token = &file->token_stack.tokens[current_token];
    
    panel_cursor_move(panel, token->start + token->size);
}

internal void
command_begin_search_state(Command_Data *command){
    App_Vars *vars = command->vars;
    vars->state = APP_STATE_SEARCH;
    vars->isearch.str =
        make_string((u8*)system_get_memory(256, SYSTEM_MEM_SMALL_STRING), 0, 256);
    vars->isearch.reverse = 0;
    vars->isearch.pos = command->panel->cursor.pos;
}

internal void
command_begin_rsearch_state(Command_Data *command){
    App_Vars *vars = command->vars;
    vars->state = APP_STATE_SEARCH;
    vars->isearch.str =
        make_string((u8*)system_get_memory(256, SYSTEM_MEM_SMALL_STRING), 0, 256);
    vars->isearch.reverse = 1;
    vars->isearch.pos = command->panel->cursor.pos;
}

internal void
command_set_mark(Command_Data *command){
    Editing_Panel *panel = command->panel;
    panel->mark = (i32)panel->cursor.pos;
}

internal void
command_copy(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Editing_Working_Set *working_set = command->working_set;
    Range range = get_range(panel->cursor.pos, panel->mark);
    if (range.smaller < range.larger){
        u8 *data = (u8*)panel->file->data;
        if (panel->file->endline_mode == ENDLINE_RN_COMBINED){
            range = range_adjust_to_left(range, data);
        }
        clipboard_copy(working_set, data, range);
    }
}

internal void
command_cut(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Editing_Working_Set *working_set = command->working_set;
    Range range = get_range(panel->cursor.pos, panel->mark);
    if (range.smaller < range.larger){
        Editing_File *file = panel->file;
        u8 *data = (u8*)file->data;
        if (file->endline_mode == ENDLINE_RN_COMBINED){
            range = range_adjust_to_left(range, data);
        }
        clipboard_copy(working_set, data, range);
        buffer_delete_range(file, range);
        panel->mark = pos_universal_fix(range.smaller,
                                        file->data, file->size,
                                        file->endline_mode);
        
        panel_measure_all_wrapped_y(panel);
        panel_cursor_move(panel, panel->mark);
    }
}

internal void
command_paste(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Editing_Working_Set *working_set = command->working_set;
    if (working_set->clipboard_size > 0){
        panel->next_mode.rewrite = 1;
        
        Editing_File *file = panel->file;
        
        String *src = working_set_clipboard_head(working_set);
        i32 pos_left = panel->cursor.pos;
        if (file->endline_mode == ENDLINE_RN_COMBINED){
            pos_left = pos_adjust_to_left(pos_left, file->data);
        }
        panel_write_chunk(panel, src, pos_left);
        panel_measure_all_wrapped_y(panel);
        panel->mark = pos_universal_fix(pos_left,
                                        file->data, file->size,
                                        file->endline_mode);
        
        i32 ticks = 20;
        panel->paste_effect.start = pos_left;
        panel->paste_effect.end = pos_left + src->size;
        panel->paste_effect.color = file->style->paste_color;
        panel->paste_effect.tick_down = ticks;
        panel->paste_effect.tick_max = ticks;
    }
}

internal void
command_paste_next(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Editing_Working_Set *working_set = command->working_set;
    if (working_set->clipboard_size > 0 && panel->mode.rewrite){
        panel->next_mode.rewrite = 1;
        
        Range range = get_range(panel->mark, panel->cursor.pos);
        
        if (range.smaller < range.larger){
            Editing_File *file = panel->file;
            if (file->endline_mode == ENDLINE_RN_COMBINED){
                range = range_adjust_to_left(range, file->data);
            }
            
            String *src = working_set_clipboard_roll_down(working_set);
            buffer_replace_range(file, range.smaller, range.larger,
                                 src->str, src->size);
            
            panel->cursor = panel_compute_cursor_from_pos(panel, range.smaller+src->size);
            panel->preferred_x = panel_get_cursor_x(panel);
            panel->file->cursor.pos = panel->cursor.pos;
            
            // TODO(allen): faster way to recompute line measurements afterwards
            buffer_measure_all_lines(file);
            panel_measure_all_wrapped_y(panel);
            panel->mark = pos_universal_fix(range.smaller,
                                            file->data, file->size,
                                            file->endline_mode);
            
            i32 ticks = 20;
            panel->paste_effect.start = range.smaller;
            panel->paste_effect.end = range.smaller + src->size;
            panel->paste_effect.color = file->style->paste_color;
            panel->paste_effect.tick_down = ticks;
            panel->paste_effect.tick_max = ticks;
        }
    }
}

internal void
command_delete_chunk(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Range range = get_range(panel->cursor.pos, panel->mark);
    if (range.smaller < range.larger){
        Editing_File *file = panel->file;
        if (file->endline_mode == ENDLINE_RN_COMBINED){
            range = range_adjust_to_left(range, file->data);
        }
        buffer_delete_range(file, range);
        panel_measure_all_wrapped_y(panel);
        panel->cursor = panel_compute_cursor_from_pos(panel, range.smaller);
        panel->mark = pos_universal_fix(range.smaller,
                                        file->data, file->size,
                                        file->endline_mode);
        panel->file->cursor.pos = panel->cursor.pos;
    }
}

// TODO(allen): Make this preserve order (look at layout_open_panel)
// Make this preserve old ratios or sizes of panels instead of
// resizing them all.
internal void
command_open_panel(Command_Data *command){
    Editing_Layout *layout = command->layout;
    Editing_Working_Set *working_set = command->working_set;
    // TODO(allen): This is wrong.  We should not be passing in the style
    // like this.  The system should be looking it up here.
    Editing_Style *style = command->style;
    // TODO(allen): change the screen info to real32 at the base level?
    real32 screen_full_width = (real32)command->screen_width;
    real32 screen_full_height = (real32)command->screen_height;
    real32 screen_y_off = (real32)command->screen_y_off;
    
    layout_open_panel(layout, working_set->files, style);
    real32 panel_w = ((real32)screen_full_width / layout->panel_count);
    real32 panel_x_pos = 0;
    
    for (i32 panel_i = 0; panel_i < layout->panel_count; ++panel_i){
        Editing_Panel *panel = &layout->panels[panel_i];
        
        panel->full_x = Floor(panel_x_pos);
        panel->full_w = Floor(panel_w);
        panel->full_y = Floor(screen_y_off);
        panel->full_h = Floor(screen_full_height - screen_y_off);
        
        panel_fix_internal_area(panel);
        
        panel_x_pos += panel_w;
    }
}

internal void
command_close_panel(Command_Data *command){
    Editing_Layout *layout = command->layout;
    i32 share_w = layout->panels[layout->active_panel].full_w;
    layout_close_panel(layout, layout->active_panel);
    layout_redistribute_width(layout, share_w);
}

internal Input_Request*
app_request_sys_file(App_Vars *vars, String query, String dest_init, bool32 fast_folder){
    Input_Request *request = vars->request_queue + (vars->request_count++);
    *request = {};
    request->type = REQUEST_SYS_FILE;
    request->sys_file.fast_folder = 1;
    
    // TODO(allen): Where does this memory come from IRL?  I don't like calling to
    // a system function all the time, knowing it might start doing weird things.
    // Better to put some limitations on the system so we can gaurantee the memory
    // this needs will exist.
    request->sys_file.query =
        make_string((u8*)system_get_memory(256, SYSTEM_MEM_SMALL_STRING), 0, 256);
    request->sys_file.dest =
        make_string((u8*)system_get_memory(256, SYSTEM_MEM_SMALL_STRING), 0, 256);
    
    copy(&request->sys_file.query, query);
    copy(&request->sys_file.dest, dest_init);
    return request;
}


internal Input_Request*
app_request_live_file(App_Vars *vars, String query, String dest_init){
    Input_Request *request = vars->request_queue + (vars->request_count++);
    *request = {};
    request->type = REQUEST_LIVE_FILE;
    
    // TODO(allen): Where does this memory come from IRL?  I don't like calling to
    // a system function all the time, knowing it might start doing weird things.
    // Better to put some limitations on the system so we can gaurantee the memory
    // this needs will exist.
    request->live_file.query =
        make_string((u8*)system_get_memory(256, SYSTEM_MEM_SMALL_STRING), 0, 256);
    request->live_file.dest =
        make_string((u8*)system_get_memory(256, SYSTEM_MEM_SMALL_STRING), 0, 256);
    
    copy(&request->live_file.query, query);
    copy(&request->live_file.dest, dest_init);
    return request;
}

internal void
panel_set_to_new(Editing_Panel *panel){
    panel->cursor = {};
    panel->cursor.pos =
        pos_adjust_to_self(0, panel->file->data, panel->file->size);
    panel->scroll_y = 0;
    panel->target_y = 0;
    panel->vel_y = 1.f;
    panel->scroll_x = 0;
    panel->target_x = 0;
    panel->vel_x = 1.f;
}

internal void
command_reopen(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Editing_File *file = panel->file;
    if (!file->is_dummy){
        Editing_File temp_file;
        if (buffer_load(&temp_file, (u8*)make_c_str(file->source_path))){
            buffer_close(file);
            
            // TODO(allen): Deduplicate!!
            Cpp_File cpp_file;
            cpp_file.data = temp_file.data;
            cpp_file.size = temp_file.size;
            {
                i32 size = cpp_lex_file_token_count(cpp_file);
                size = cpp_get_token_stack_size(size);
                temp_file.token_stack = cpp_make_token_stack(size);
            }
            cpp_lex_file(cpp_file, &temp_file.token_stack);
            temp_file.tokens_complete = 1;
            temp_file.tokens_exist = 1;
            
            *file = temp_file;
            panel_set_to_new(panel);
            panel_measure_all_wrapped_y(panel);
            // TODO(allen): Update all other panels that also view this file.
        }
    }
}

internal void
command_interactive_open(Command_Data *command){
    App_Vars *vars = command->vars;
    if (command->request_count == 0){
        // TODO(allen): Can all of this be simplified?  I don't like having this much
        // complexity for issuing a single request.
        vars->pending_command = command_interactive_open;
        
        Assert(vars->request_count == 0);
        app_request_sys_file(vars, make_lit_string("Open: "), vars->hot_directory.string, 1);
        
        hot_directory_reload_list(&vars->hot_directory);
    }
    else{
        String *string = &command->requests[0].sys_file.dest;
        Editing_File *new_file;
        new_file = buffer_open(&vars->working_set, string->str, command->style);
        
        if (!new_file){
            // TODO(allen): Here's a really tough one.  This crap is now happening twice.
            // Once here and also in the single_file_input function which checks all of it
            // whenever the fast_folder_select option is on to see if the user is selecting
            // a folder.  It would be nice to be able to share that information to here when it
            // is available so we could avoid doing the exact same computations here.
            u8 front_name_space[256];
            String front_name =
                make_string(front_name_space, 0, ArrayCount(front_name_space));
            get_front_of_directory(&front_name, *string);
            
            Hot_Directory_Match match =
                hot_directory_first_match(&vars->hot_directory,
                                          front_name, 1, 0);
            
            if (match.filename){
                // NOTE(allen): is_folder case is handled by the single_file_input function
                // which would only pass control to this command if the user did not match to
                // a folder.
                Assert(!match.is_folder);
                new_file = buffer_open(&vars->working_set, string->str, command->style);
            }
        }
        
        if (new_file){
            Editing_Panel *active_panel = command->panel;
            active_panel->file = new_file;
            panel_set_to_new(active_panel);
            panel_measure_all_wrapped_y(active_panel);
            
            Cpp_File file;
            file.data = new_file->data;
            file.size = new_file->size;
            // TODO(allen): Where should the memory for tokens come from IRL?
            {
                i32 size = cpp_lex_file_token_count(file);
                size = cpp_get_token_stack_size(size);
                new_file->token_stack = cpp_make_token_stack(size);
            }
            cpp_lex_file(file, &new_file->token_stack);
            new_file->tokens_complete = 1;
            new_file->tokens_exist = 1;
        }
    }
}

internal void
command_save(Command_Data *command){
    Editing_Panel *panel = command->panel;
    String *file_path = &panel->file->source_path;
    if (file_path->size > 0){
        buffer_save(panel->file, file_path->str);
    }
}

internal void
command_interactive_save_as(Command_Data *command){
    App_Vars *vars = command->vars;
    if (command->request_count == 0){
        vars->pending_command = command_interactive_save_as;
        
        Assert(vars->request_count == 0);
        app_request_sys_file(vars, make_lit_string("Save As: "), vars->hot_directory.string, 1);
        
        hot_directory_reload_list(&vars->hot_directory);
    }
    else{
        String *string = &command->requests[0].sys_file.dest;
        
        buffer_save_and_set_names(command->panel->file, string->str);
    }
}

internal void
command_change_active_panel(Command_Data *command){
    Editing_Layout *layout = command->layout;
    if (layout->panel_count > 1){
        ++layout->active_panel;
        if (layout->active_panel >= layout->panel_count){
            layout->active_panel = 0;
        }
    }
}

internal void
command_interactive_switch_file(Command_Data *command){
    App_Vars *vars = command->vars;
    if (command->request_count == 0){
        vars->pending_command = command_interactive_switch_file;
        
        Assert(vars->request_count == 0);
        app_request_live_file(vars, make_lit_string("Switch To: "), make_lit_string(""));
    }
    else{
        String *string = &command->requests[0].live_file.dest;
        
        Editing_File *file;
        file = working_set_lookup_file(command->working_set, *string);
        
        if (file){
            Editing_Panel *active_panel = command->panel;
            active_panel->file = file;
            Panel_Cursor_Data cursor_data;
            cursor_data = panel_compute_cursor_from_pos(active_panel, file->cursor.pos);
            active_panel->cursor = cursor_data;
        }
    }
}

internal void
command_kill_file(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Editing_File *file = panel->file;
    if (file && file->max_size != 0){
        buffer_close(file);
        buffer_get_dummy(file, command->style);
    }
}

internal void
command_interactive_kill_file(Command_Data *command){
    App_Vars *vars = command->vars;
    if (command->request_count == 0){
        vars->pending_command = command_interactive_kill_file;
        
        Assert(vars->request_count == 0);
        app_request_live_file(vars, make_lit_string("Kill: "), make_lit_string(""));
    }
    else{
        String *string = &command->requests[0].live_file.dest;
        
        Editing_File *file;
        file = working_set_lookup_file(&vars->working_set, *string);
        
        if (file){
            buffer_close(file);
            buffer_get_dummy(file, command->style);
        }
    }
}

internal void
command_toggle_line_wrap(Command_Data *command){
    Editing_Panel *panel = command->panel;
    if (panel->unwrapped_lines){
        panel->unwrapped_lines = 0;
        panel->target_x = 0;
        panel->cursor =
            panel_compute_cursor_from_pos(panel, panel->cursor.pos);
    }
    else{
        panel->unwrapped_lines = 1;
        panel->cursor =
            panel_compute_cursor_from_pos(panel, panel->cursor.pos);
    }
}

internal void
command_toggle_endline_mode(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Editing_File *file = panel->file;
    switch (file->endline_mode){
        case ENDLINE_RN_COMBINED:
        {
            panel->cursor.pos = pos_adjust_to_left(panel->cursor.pos, panel->file->data);
            file->endline_mode = ENDLINE_RN_SEPARATE;
            panel->cursor =
                panel_compute_cursor_from_pos(panel, panel->cursor.pos);
        }break;
        
        case ENDLINE_RN_SEPARATE:
        {
            file->endline_mode = ENDLINE_RN_SHOWALLR;
            panel->cursor =
                panel_compute_cursor_from_pos(panel, panel->cursor.pos);
        }break;
        
        case ENDLINE_RN_SHOWALLR:
        {
            panel->cursor.pos = pos_adjust_to_self(panel->cursor.pos, panel->file->data,
                                                   panel->file->size);
            file->endline_mode = ENDLINE_RN_COMBINED;
            panel->cursor =
                panel_compute_cursor_from_pos(panel, panel->cursor.pos);
        }break;
    }
}

internal void
command_to_uppercase(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Range range = get_range(panel->cursor.pos, panel->mark);
    if (range.smaller < range.larger){
        Editing_File *file = panel->file;
        u8 *data = file->data;
        for (i32 i = range.smaller; i < range.larger; ++i){
            if (data[i] >= 'a' && data[i] <= 'z'){
                data[i] += (u8)('A' - 'a');
            }
        }
        // TODO(allen): RELEX POINT
        if (file->token_stack.tokens){
            Cpp_File cpp_file;
            cpp_file.size = file->size;
            cpp_file.data = (u8*)file->data;
            cpp_relex_file(cpp_file, &file->token_stack, range.smaller, range.larger, 0);
            file->tokens_complete = 1;
            file->tokens_exist = 1;
        }
    }
}

internal void
command_to_lowercase(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Range range = get_range(panel->cursor.pos, panel->mark);
    if (range.smaller < range.larger){
        Editing_File *file = panel->file;
        u8 *data = file->data;
        for (i32 i = range.smaller; i < range.larger; ++i){
            if (data[i] >= 'A' && data[i] <= 'Z'){
                data[i] -= (u8)('A' - 'a');
            }
        }
        // TODO(allen): RELEX POINT
        if (file->token_stack.tokens){
            Cpp_File cpp_file;
            cpp_file.size = file->size;
            cpp_file.data = (u8*)file->data;
            cpp_relex_file(cpp_file, &file->token_stack, range.smaller, range.larger, 0);
            file->tokens_complete = 1;
            file->tokens_exist = 1;
        }
    }
}

internal void
command_toggle_show_whitespace(Command_Data *command){
    Editing_Panel *panel = command->panel;
    panel->show_whitespace = !panel->show_whitespace;
}

internal void
command_clean_line(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Editing_File *file = panel->file;
    u8 *data = (u8*)file->data;
    
    i32 pos = panel_find_beginning_of_line(panel, panel->cursor.pos);
    
    i32 last_hard_start = pos-1;
    i32 last_hard = last_hard_start;
    while (pos < file->size && data[pos] != '\n'){
        if (!character_is_any_whitespace(data[pos])){
            last_hard = pos;
        }
        ++pos;
    }
    
    if (last_hard != last_hard_start){
        pos = pos_adjust_to_left(pos, data);
        
        if (last_hard + 1 < pos){
            buffer_replace_range(file, last_hard+1, pos, 0, 0, REP_WHITESPACE);
            panel_measure_all_wrapped_y(panel);
            
            if (panel->cursor.pos > last_hard){
                panel->cursor = panel_compute_cursor_from_pos(panel, last_hard + 1);
            }
            if (panel->mark > last_hard && panel->mark <= pos){
                panel->mark = pos_adjust_to_self(last_hard+1, file->data, file->size);
            }
            else if (panel->mark > pos){
                panel->mark -= pos - (last_hard + 1);
            }
        }
    }
    else{
        panel_measure_all_wrapped_y(panel);
        panel_auto_tab(panel, pos, pos);
    }
}

internal void
command_clean_all_lines(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Editing_File *file = panel->file;
    u8 *data = (u8*)file->data;
    
    i32 cursor_pos = panel->cursor.pos;
    i32 pos = 0;
    i32 last_hard = -1;
    bool32 is_all_white = 1;
    while (pos <= file->size){
        if (pos == file->size || data[pos] == '\n'){
            i32 line_pos = pos;
            if (pos < file->size && data[pos] == '\n'){
                line_pos = pos_adjust_to_left(pos, data);
            }
            
            // TODO(allen): This should be optimized by either keeping track of the nesting level
            // in this funciton, or by at least having a nesting hint that is used and updated
            // every time an auto tab happens.  Also auto tab should take a nesting hint.
            if (is_all_white){
                panel_auto_tab(panel, pos, pos);
            }
            else{
                if (last_hard + 1 < line_pos){
                    buffer_replace_range(file, last_hard+1, line_pos, 0, 0, REP_WHITESPACE);
                    
                    if (cursor_pos > last_hard && cursor_pos <= pos){
                        cursor_pos = pos_adjust_to_self(last_hard+1, file->data, file->size);
                    }
                    else if (cursor_pos > pos){
                        cursor_pos -= line_pos - (last_hard + 1);
                    }
                    if (panel->mark > last_hard && panel->mark <= pos){
                        panel->mark = pos_adjust_to_self(last_hard+1, file->data, file->size);
                    }
                    else if (panel->mark > pos){
                        panel->mark -= line_pos - (last_hard + 1);
                    }
                    pos -= line_pos - (last_hard + 1);
                }
            }
            
            last_hard = pos;
            is_all_white = 1;
        }
        else if (!character_is_any_whitespace(data[pos])){
            last_hard = pos;
            is_all_white = 0;
        }
        ++pos;
    }
    panel_measure_all_wrapped_y(panel);
    panel->cursor = panel_compute_cursor_from_pos(panel, cursor_pos);
}

internal void
command_eol_dosify(Command_Data *command){
    Editing_Panel *panel = command->panel;
    panel_endline_convert(panel, ENDLINE_RN, ENDLINE_ERASE, ENDLINE_RN);
    panel_measure_all_wrapped_y(panel);
}

internal void
command_eol_nixify(Command_Data *command){
    Editing_Panel *panel = command->panel;
    panel_endline_convert(panel, ENDLINE_N, ENDLINE_ERASE, ENDLINE_N);
    panel_measure_all_wrapped_y(panel);
}

internal void
command_auto_tab(Command_Data *command){
    Editing_Panel *panel = command->panel;
    Range range = get_range(panel->cursor.pos, panel->mark);
    panel_auto_tab(panel, range.smaller, range.larger);
    panel_measure_all_wrapped_y(panel);
}

internal void
setup_commands(Command_Map *commands, Key_Codes *codes){
    commands->basic_mode_key[codes->left] = command_move_left;
    commands->basic_mode_key[codes->right] = command_move_right;
    commands->basic_mode_key[codes->del] = command_delete;
    commands->basic_mode_key[codes->back] = command_backspace;
    commands->basic_mode_key[codes->up] = command_move_up;
    commands->basic_mode_key[codes->down] = command_move_down;
    commands->basic_mode_key[codes->end] = command_seek_end_of_line;
    commands->basic_mode_key[codes->home] = command_seek_beginning_of_line;
    
#if 0
    commands->control_key[codes->right] = command_seek_token_right;
    commands->control_ascii['m'] = command_seek_token_right;
    commands->control_key[codes->left] = command_seek_token_left;
    commands->control_ascii['n'] = command_seek_token_left;
#else
    commands->control_key[codes->right] = command_seek_whitespace_right;
    commands->control_ascii['m'] = command_seek_whitespace_right;
    commands->control_key[codes->left] = command_seek_whitespace_left;
    commands->control_ascii['n'] = command_seek_whitespace_left;
#endif
    
    commands->control_key[codes->up] = command_seek_whitespace_up;
    commands->control_ascii['y'] = command_seek_whitespace_up;
    commands->control_key[codes->down] = command_seek_whitespace_down;
    commands->control_ascii['h'] = command_seek_whitespace_down;
    
    commands->control_ascii['\t'] = command_auto_tab;
    
    commands->control_ascii[' '] = command_set_mark;
    commands->control_ascii['c'] = command_copy;
    commands->control_ascii['x'] = command_cut;
    commands->control_ascii['v'] = command_paste;
    commands->control_ascii['V'] = command_paste_next;
    commands->control_ascii['d'] = command_delete_chunk;
    commands->control_ascii['p'] = command_open_panel;
    commands->control_ascii['P'] = command_close_panel;
    commands->control_ascii['o'] = command_interactive_open;
    commands->control_ascii['O'] = command_reopen;
    commands->control_ascii['s'] = command_save;
    commands->control_ascii['w'] = command_interactive_save_as;
    commands->control_ascii[','] = command_change_active_panel;
    commands->control_ascii['i'] = command_interactive_switch_file;
    commands->control_ascii['k'] = command_interactive_kill_file;
    commands->control_ascii['K'] = command_kill_file;
    commands->control_ascii['l'] = command_toggle_line_wrap;
    commands->control_ascii['L'] = command_toggle_endline_mode;
    commands->control_ascii['u'] = command_to_uppercase;
    commands->control_ascii['j'] = command_to_lowercase;
    commands->control_ascii['?'] = command_toggle_show_whitespace;
    commands->control_ascii['`'] = command_clean_line;
    commands->control_ascii['~'] = command_clean_all_lines;
    commands->control_ascii['1'] = command_eol_dosify;
    commands->control_ascii['!'] = command_eol_nixify;
    commands->control_ascii['f'] = command_begin_search_state;
    commands->control_ascii['r'] = command_begin_rsearch_state;
}

/*
* Interactive Bar
*/

internal void
intbar_draw_string(Render_Target *target,
                   Interactive_Bar *bar, u8 *str,
                   u32 char_color){
    i32 char_w = font_get_character_width(bar->style.font);
    for (i32 i = 0; str[i]; ++i){
        font_draw_glyph(target, bar->style.font, str[i],
                        (real32)bar->pos_x, (real32)bar->pos_y, char_color);
        bar->pos_x += char_w;
    }
}

internal void
intbar_draw_string(Render_Target *target,
                   Interactive_Bar *bar, String str,
                   u32 char_color){
    i32 char_w = font_get_character_width(bar->style.font);
    for (i32 i = 0; i < str.size; ++i){
        font_draw_glyph(target, bar->style.font, str.str[i],
                        (real32)bar->pos_x, (real32)bar->pos_y, char_color);
        bar->pos_x += char_w;
    }
}

internal void
hot_directory_draw_helper(Render_Target *target,
                          Hot_Directory *hot_directory,
                          Interactive_Bar *bar, String *string,
                          bool32 include_files){
    persist u8 str_open_bracket[] = " {";
    persist u8 str_close_bracket[] = "}";
    persist u8 str_comma[] = ", ";
    
    intbar_draw_string(target, bar, *string, bar->style.pop1_color);
    
    u8 front_name_space[256];
    String front_name =
        make_string(front_name_space, 0, ArrayCount(front_name_space));
    
    get_front_of_directory(&front_name, *string);
    
    intbar_draw_string(target, bar, str_open_bracket, bar->style.base_color);
    
    bool32 is_first_string = 1;
    
    File_List files = hot_directory->file_list;
    File_List_Iterator files_it = files_iterator_init(&files);
    
    while (files_it.filename_ptr &&
           (include_files || files_it.folder_stage)){
        u8 *filename = *files_it.filename_ptr;
        
        if (front_name.size == 0 || has_substr_unsensitive(filename, front_name)){
            if (is_first_string){
                is_first_string = 0;
            }
            else{
                intbar_draw_string(target, bar, str_comma, bar->style.base_color);
            }
            if (files_it.folder_stage){
                intbar_draw_string(target, bar, filename, bar->style.pop1_color);
                intbar_draw_string(target, bar, (u8*)"/", bar->style.pop1_color);
            }
            else{
                intbar_draw_string(target, bar, filename, bar->style.base_color);
            }
        }
        
        files_iterator_step(&files, &files_it);
    }
    intbar_draw_string(target, bar, str_close_bracket, bar->style.base_color);
}

internal void
live_file_draw_helper(Render_Target *target,
                      Editing_Working_Set *working_set,
                      Interactive_Bar *bar, String *string){
    persist u8 str_open_bracket[] = " {";
    persist u8 str_close_bracket[] = "}";
    persist u8 str_comma[] = ", ";
    
    intbar_draw_string(target, bar, *string, bar->style.base_color);
    
    intbar_draw_string(target, bar, str_open_bracket, bar->style.base_color);
    
    bool32 is_first_string = 1;
    for (i32 file_i = 0;
         file_i < working_set->file_index_count;
         ++file_i){
        Editing_File *file = &working_set->files[file_i];
        if (file->live_name.str &&
            (string->size == 0 || has_substr_unsensitive(file->live_name, *string))){
            if (is_first_string){
                is_first_string = 0;
            }
            else{
                intbar_draw_string(target, bar, str_comma, bar->style.base_color);
            }
            intbar_draw_string(target, bar, file->live_name, bar->style.base_color);
        }
    }
    intbar_draw_string(target, bar, str_close_bracket, bar->style.base_color);
}

/*
* App Functions
*/

internal bool32
app_init(Thread_Context *thread, Application_Memory *memory,
         Key_Codes *loose_codes, Clipboard_Contents clipboard){
    Partition_€ursor partition =
        partition_open(memory->vars_memory, memory->vars_memory_size);
    
    App_Vars *vars = (App_Vars*)
        partition_allocate(&partition, sizeof(App_Vars));
    
    Assert(vars);
    *vars = {};
    
    u32 panel_max_count = vars->layout.panel_max_count = 4;
    u32 panel_count = vars->layout.panel_count = 1;
    Assert(panel_max_count >= 1);
    Editing_Panel *panels = vars->layout.panels = (Editing_Panel*)
        partition_allocate(&partition, sizeof(Editing_Panel)*panel_max_count);
    
    Assert(panels);
    // TODO(allen): improved Assert
    
    // NOTE(allen): command map setup
    setup_commands(&vars->map, loose_codes);
    
    // NOTE(allen): font setup
    if (font_init() != 0){
        FatalError("Error initializing fonts");
        return 0;
    }
    
    i32 memory_used = 0;
    if (font_load(&vars->font, 15, memory->font_memory,
                  font_predict_size(15), &memory_used) != 0){
        FatalError("Could not find any fonts");
    }
    
    // NOTE(allen): file setup
    vars->working_set.file_index_count = 1;
    vars->working_set.file_max_count = 29;
    
    vars->working_set.files = (Editing_File*)
        partition_allocate(&partition,
                           sizeof(Editing_File)*vars->working_set.file_max_count);
    
    buffer_get_dummy(&vars->working_set.files[0], &vars->style);
    
    // NOTE(allen): clipboard setup
    vars->working_set.clipboard_max_size = ArrayCount(vars->working_set.clipboards);
    vars->working_set.clipboard_size = 0;
    vars->working_set.clipboard_current = 0;
    vars->working_set.clipboard_rolling = 0;
    
    if (clipboard.str){
        String *dest = working_set_next_clipboard_string(&vars->working_set, clipboard.size);
        copy(dest, make_string(clipboard.str, clipboard.size));
    }
    
    // TODO(allen): more robust allocation solution for the clipboard?
    
    // NOTE(allen): style setup
    // TODO(allen): style_set_font function
    vars->style.font = &vars->font;
    vars->style.font_metrics.character_width = vars->font.glyphs[' '].advance;
    vars->style.font_metrics.line_skip = vars->font.line_skip;
    
    vars->style.back_color = 0xFF0C0C0C;
    vars->style.margin_color = 0xFF181818;
    vars->style.cursor_color = 0xFF00EE00;
    vars->style.highlight_color = 0xFFDDEE00;
    vars->style.mark_color = 0xFF494949;
    vars->style.default_color = 0xFF90B080;
    vars->style.at_cursor_color = vars->style.back_color;
    vars->style.at_highlight_color = 0xFFFF44DD;
    vars->style.comment_color = 0xFF2090F0;
    vars->style.keyword_color = 0xFFD08F20;
    vars->style.constant_color = 0xFF50FF30;
    vars->style.special_character_color = 0xFFFF0000;
    
    vars->style.use_paste_color = 1;
    vars->style.paste_color = 0xFFDDEE00;
    
    vars->style.highlight_junk_color = 0x44FF0000;
    vars->style.highlight_white_color = 0x1100FFFF;
    vars->style.tab_width = 4;
    vars->style.margin_width = 5;
    
    Interactive_Style file_info_style;
    file_info_style.font = &vars->font;
    file_info_style.bar_color = 0xFF888888;
    file_info_style.base_color = 0xFF000000;
    file_info_style.pop1_color = 0xFF4444AA;
    file_info_style.pop2_color = 0xFFFF0000;
    file_info_style.height = vars->style.font->line_skip + 2;
    
    vars->style.file_info_style = file_info_style;
    
    Interactive_Style command_style;
    command_style.font = &vars->font;
    command_style.bar_color = 0xFF0C0C0C;
    command_style.base_color = 0xFFDDDDBB;
    command_style.pop1_color = 0xFF4444AA;
    command_style.pop2_color = 0xFF44FF44;
    command_style.height = vars->style.font->line_skip + 2;
    
    vars->command_style = command_style;
    
    
    // NOTE(allen): panel setup
    AllowLocal(panel_count);
    panel_init(&panels[0], &vars->working_set.files[0]);
    
    // NOTE(allen): hot directory setup
    hot_directory_init(&vars->hot_directory);
    
    // NOTE(allen): request stack setup
    vars->request_count = 0;
    vars->request_filled = 0;
    vars->request_max = ArrayCount(vars->request_queue);
    
    return 1;
}

struct Single_Line_Input_Step{
    bool32 hit_newline;
    bool32 hit_ctrl_newline;
    bool32 hit_a_character;
    bool32 hit_backspace;
    bool32 hit_esc;
    bool32 made_a_change;
    bool32 did_command;
};

enum Single_Line_Input_Type{
    SINGLE_LINE_STRING,
    SINGLE_LINE_FILE
};

struct Single_Line_Mode{
    Single_Line_Input_Type type;
    String *string;
    Hot_Directory *hot_directory;
    bool32 fast_folder_select;
};

internal Single_Line_Input_Step
app_single_line_input_core(Key_Codes *codes,
                           Key_Input_Data *input,
                           Single_Line_Mode mode){
    Single_Line_Input_Step result = {};
    
    if (input->has_press){
        if (input->press.keycode == codes->back){
            result.hit_backspace = 1;
            switch (mode.type){
                case SINGLE_LINE_STRING:
                {
                    // TODO(allen): Is this still okay?  It looks like it's keeping a NULL terminator
                    // which is at least unecessary given the new protocol.
                    if (mode.string->size > 0){
                        --mode.string->size;
                        mode.string->str[mode.string->size] = 0;
                        result.made_a_change = 1;
                    }
                }break;
                
                case SINGLE_LINE_FILE:
                {
                    if (mode.string->size > 0){
                        --mode.string->size;
                        i8 end_character = mode.string->str[mode.string->size];
                        if (character_is_slash(end_character)){
                            mode.string->size = reverse_seek_slash(*mode.string) + 1;
                            // TODO(allen): Is this still okay?  It looks like it's keeping a NULL terminator
                            // which is at least unecessary given the new protocol.
                            // TODO(allen): What to do when the string becomes empty though?
                            mode.string->str[mode.string->size] = 0;
                            hot_directory_set(mode.hot_directory, *mode.string);
                        }
                        else{
                            // TODO(allen): Is this still okay?  It looks like it's keeping a NULL terminator
                            // which is at least unecessary given the new protocol.
                            mode.string->str[mode.string->size] = 0;
                        }
                        result.made_a_change = 1;
                    }
                }break;
            }
        }
        
        else if (input->press.keycode == codes->newline){
            if (input->control_keys[CONTROL_KEY_CONTROL]){
                result.hit_ctrl_newline = 1;
                result.made_a_change = 1;
            }
            
            else{
                result.made_a_change = 1;
                if (mode.fast_folder_select){
                    u8 front_name_space[256];
                    String front_name =
                        make_string(front_name_space, 0, ArrayCount(front_name_space));
                    get_front_of_directory(&front_name, *mode.string);
                    Hot_Directory_Match match;
                    match = hot_directory_first_match(mode.hot_directory, front_name, 1, 1);
                    
                    if (!match.filename){
                        match = hot_directory_first_match(mode.hot_directory, front_name, 1, 0);
                    }
                    
                    if (match.filename){
                        if (match.is_folder){
                            set_last_folder(mode.string, match.filename);
                            hot_directory_set(mode.hot_directory, *mode.string);
                        }
                        else{
                            mode.string->size = reverse_seek_slash(*mode.string) + 1;
                            append(mode.string, match.filename);
                            result.hit_newline = 1;
                        }
                    }
                    else{
                        result.hit_newline = 1;
                    }
                }
                else{
                    result.hit_newline = 1;
                }
            }
        }
        
        else if (input->press.keycode == codes->esc){
            result.hit_esc = 1;
            result.made_a_change = 1;
        }
        
        else if (input->press.character){
            result.hit_a_character = 1;
            if (!input->control_keys[CONTROL_KEY_CONTROL]){
                switch (mode.type){
                    case SINGLE_LINE_STRING:
                    {
                        if (mode.string->size+1 < mode.string->memory_size){
                            mode.string->str[mode.string->size] = (i8)input->press.character;
                            mode.string->size++;
                            // TODO(allen): More of this keeping a NULL terminator business...
                            // I want out of this business for the String struct.
                            mode.string->str[mode.string->size] = 0;
                            result.made_a_change = 1;
                        }
                    }break;
                    
                    case SINGLE_LINE_FILE:
                    {
                        if (mode.string->size+1 < mode.string->memory_size){
                            i8 new_character = (i8)input->press.character;
                            mode.string->str[mode.string->size] = new_character;
                            mode.string->size++;
                            // TODO(allen): More of this keeping a NULL terminator business...
                            // I want out of this business for the String struct.
                            mode.string->str[mode.string->size] = 0;
                            
                            if (character_is_slash(new_character)){
                                hot_directory_set(mode.hot_directory, *mode.string);
                            }
                            result.made_a_change = 1;
                        }
                    }break;
                }
            }
            
            else{
                result.did_command = 1;
                result.made_a_change = 1;
            }
        }
    }
    
    return result;
}

inline internal Single_Line_Input_Step
app_single_line_input_step(Key_Codes *codes, Key_Input_Data *input, String *string){
    Single_Line_Mode mode = {};
    mode.type = SINGLE_LINE_STRING;
    mode.string = string;
    return app_single_line_input_core(codes, input, mode);
}

inline internal Single_Line_Input_Step
app_single_file_input_step(Key_Codes *codes, Key_Input_Data *input,
                           String *string, Hot_Directory *hot_directory,
                           bool32 fast_folder_select){
    Single_Line_Mode mode = {};
    mode.type = SINGLE_LINE_FILE;
    mode.string = string;
    mode.hot_directory = hot_directory;
    mode.fast_folder_select = fast_folder_select;
    return app_single_line_input_core(codes, input, mode);
}

inline internal Command_Function
app_get_command(App_Vars *vars, Key_Input_Data *input, Key_Event_Data key){
    Command_Function function = 0;
    if (input->control_keys[CONTROL_KEY_CONTROL]){
        if (key.character_no_caps_lock){
            function = vars->map.control_ascii[key.character_no_caps_lock];
        }
        else{
            function = vars->map.control_key[key.keycode];
        }
    }
    else if (!input->control_keys[CONTROL_KEY_ALT]){
        if (key.character != 0){
            function = command_write_character;
        }
        else{
            function = vars->map.basic_mode_key[key.keycode];
        }
    }
    return function;
}

internal bool32
smooth_camera_step(real32 *target, real32 *current, real32 *vel, real32 S, real32 T){
    bool32 result = 0;
    real32 targ = *target;
    real32 curr = *current;
    real32 v = *vel;
    if (curr != targ){
        real32 L = lerp(curr, T, targ);
        
        i32 sign = (targ > curr) - (targ < curr);
        real32 V = curr + sign*v;
        
        if (sign > 0){
            curr = Min(L, V);
        }
        else{
            curr = Max(L, V);
        }
        
        if (curr == V){
            v *= S;
        }
        
        if (curr > targ - .0001f && curr < targ + .0001f){
            curr = targ;
            v = 1.f;
        }
        *target = targ;
        *current = curr;
        *vel = v;
        result = 1;
    }
    return result;
}

#if FRED_INTERNAL
Application_Memory *GLOBAL;
#endif

internal Application_Step_Result
app_step(Thread_Context *thread, Key_Codes *codes,
         Key_Input_Data *input, Mouse_State *mouse,
         bool32 time_step, Render_Target *target,
         Application_Memory *memory,
         Clipboard_Contents clipboard,
         bool32 first_step){
    
#if FRED_INTERNAL
    GLOBAL = memory;
#endif
    
    ProfileStart(app_step);
    ProfileSection(thread, app_step, "start");
    
    Application_Step_Result app_result = {};
    app_result.redraw = 1;
    
    App_Vars *vars = (App_Vars*)memory->vars_memory;
    
    // TODO(allen): Let's find another way to do first_step
    // so we can get it out of app_step and the platform layer.
    if (first_step || !time_step){
        app_result.redraw = 1;
    }
    
    Editing_Panel *panels = vars->layout.panels;
    Editing_Panel *active_panel = &panels[vars->layout.active_panel];
    ProfileSection(thread, app_step, "setup");
    
    // NOTE(allen): OS clipboard event handling
    if (clipboard.str){
        String *dest = working_set_next_clipboard_string(&vars->working_set, clipboard.size);
        copy(dest, make_string(clipboard.str, clipboard.size));
    }
    
    // NOTE(allen): check files are up to date
    for (i32 i = 0; i < vars->working_set.file_index_count; ++i){
        Editing_File *file = vars->working_set.files + i;
        
        if (!file->is_dummy){
            Time_Stamp time_stamp;
            time_stamp = system_file_time_stamp((u8*)make_c_str(file->source_path));
            
            if (time_stamp.success){
                file->last_sys_write_time = time_stamp.time;
                if (file->last_sys_write_time != file->last_4ed_write_time){
                    app_result.redraw = 1;
                }
            }
        }
    }
    ProfileSection(thread, app_step, "OS syncing");
    
    // NOTE(allen): keyboard input handling
    if (time_step){
        switch (vars->state){
            case APP_STATE_EDIT:
            {
                Command_Data command_data;
                command_data.panel = active_panel;
                command_data.working_set = &vars->working_set;
                command_data.layout = &vars->layout;
                command_data.style = &vars->style;
                command_data.vars = vars;
                command_data.screen_width = target->width;
                command_data.screen_height = target->height;
                command_data.screen_y_off = vars->command_style.height;
                command_data.requests = 0;
                command_data.request_count = 0;
                command_data.request_filled = 0;
                
                if (vars->request_count == 0){
                    Command_Function function = 0;
                    if (input->has_press){
                        command_data.key = input->press;
                        function = app_get_command(vars, input, command_data.key);
                    }
                    
                    else if (input->has_hold){
                        command_data.key = input->hold;
                        function = app_get_command(vars, input, command_data.key);
                        
                    }
                    
                    if (function){
                        command_data.panel->next_mode = {};
                        function(&command_data);
                        app_result.redraw = 1;
                        command_data.panel->mode = command_data.panel->next_mode;
                    }
                }
                
                else{
                    
                    Input_Request *active_request = vars->request_queue + vars->request_filled;
                    bool32 killed_command = 0;
                    switch (active_request->type){
                        case REQUEST_SYS_FILE:
                        {
                            String *string = &active_request->sys_file.dest;
                            Single_Line_Input_Step result =
                                app_single_file_input_step(codes, input, string, &vars->hot_directory, 1);
                            
                            if (result.made_a_change){
                                app_result.redraw = 1;
                            }
                            
                            if (result.hit_ctrl_newline){
                                active_request->sys_file.hit_ctrl_newline = 1;
                            }
                            
                            if (result.hit_newline || result.hit_ctrl_newline){
                                ++vars->request_filled;
                            }
                            
                            if (result.hit_esc){
                                app_clear_request_queue(vars);
                                killed_command = 1;
                            }
                        }break;
                        
                        case REQUEST_LIVE_FILE:
                        {
                            String *string = &active_request->live_file.dest;
                            Single_Line_Input_Step result =
                                app_single_line_input_step(codes, input, string);
                            
                            if (result.made_a_change){
                                app_result.redraw = 1;
                            }
                            
                            if (result.hit_ctrl_newline){
                                active_request->live_file.hit_ctrl_newline = 1;
                            }
                            
                            if (result.hit_newline || result.hit_ctrl_newline){
                                ++vars->request_filled;
                            }
                            
                            if (result.hit_esc){
                                app_clear_request_queue(vars);
                                killed_command = 1;
                            }
                        }break;
                        
                    }
                    
                    if (vars->request_filled == vars->request_count && !killed_command){
                        command_data.requests = vars->request_queue;
                        command_data.request_count = vars->request_count;
                        command_data.request_filled = vars->request_filled;
                        vars->pending_command(&command_data);
                        app_clear_request_queue(vars);
                    }
                }
                
            }break;
            
            case APP_STATE_SEARCH:
            {
                String *string = &vars->isearch.str;
                Single_Line_Input_Step result =
                    app_single_line_input_step(codes, input, string);
                
                if (result.made_a_change){
                    app_result.redraw = 1;
                    
                    Editing_File *file = active_panel->file;
                    
                    bool32 step_forward = 0;
                    bool32 step_backward = 0;
                    
                    if (input->has_press){
                        Key_Event_Data key = input->press;
                        Command_Function function = app_get_command(vars, input, key);
                        if (function == command_begin_search_state){
                            step_forward = 1;
                        }
                        if (function == command_begin_rsearch_state){
                            step_backward = 1;
                        }
                    }
                    
                    if (!step_forward && !step_backward &&
                        !input->has_press && input->has_hold){
                        Key_Event_Data key = input->hold;
                        Command_Function function = app_get_command(vars, input, key);
                        if (function == command_begin_search_state){
                            step_forward = 1;
                        }
                        if (function == command_begin_rsearch_state){
                            step_backward = 1;
                        }
                    }
                    
                    i32 start_pos = vars->isearch.pos;
                    if (step_forward){
                        if (vars->isearch.reverse){
                            start_pos = active_panel->temp_highlight.pos - 1;
                            vars->isearch.pos = start_pos;
                            vars->isearch.reverse = 0;
                        }
                    }
                    if (step_backward){
                        if (!vars->isearch.reverse){
                            start_pos = active_panel->temp_highlight.pos + 1;
                            vars->isearch.pos = start_pos;
                            vars->isearch.reverse = 1;
                        }
                    }
                    
                    String file_string = make_string(file->data, file->size);
                    
                    i32 pos;
                    
                    if (vars->isearch.reverse){
                        if (result.hit_backspace){
                            start_pos = active_panel->temp_highlight.pos + 1;
                            vars->isearch.pos = start_pos;
                        }
                        else{
                            pos = rfind_substr(file_string, start_pos - 1, *string);
                            
                            if (pos >= 0){
                                if (step_backward){
                                    vars->isearch.pos = pos;
                                    start_pos = pos;
                                    pos = rfind_substr(file_string, start_pos - 1, *string);
                                    if (pos == -1){
                                        pos = start_pos;
                                    }
                                }
                                
                                panel_set_temp_highlight(active_panel, pos, pos+string->size);
                            }
                        }
                    }
                    else{
                        if (result.hit_backspace){
                            start_pos = active_panel->temp_highlight.pos - 1;
                            vars->isearch.pos = start_pos;
                        }
                        else{
                            pos = find_substr(file_string, start_pos + 1, *string);
                            
                            if (pos < file->size){
                                if (step_forward){
                                    vars->isearch.pos = pos;
                                    start_pos = pos;
                                    pos = find_substr(file_string, start_pos + 1, *string);
                                    if (pos == file->size){
                                        pos = start_pos;
                                    }
                                }
                                
                                panel_set_temp_highlight(active_panel, pos, pos+string->size);
                            }
                        }
                    }
                }
                
                if (result.hit_newline || result.hit_ctrl_newline){
                    panel_cursor_move(active_panel, active_panel->temp_highlight);
                    
                    system_free_memory(vars->isearch.str.str);
                    vars->state = APP_STATE_EDIT;
                }
                
                if (result.hit_esc){
                    active_panel->show_temp_highlight = 0;
                    
                    system_free_memory(vars->isearch.str.str);
                    vars->state = APP_STATE_EDIT;
                }
                
            }break;
            
            case APP_STATE_RESIZING:
            {
                if (input->has_press){
                    vars->state = APP_STATE_EDIT;
                }
            }break;
        }
    }
    ProfileSection(thread, app_step, "keyboard input");
    
    // NOTE(allen): reorganizing panels on screen
    i32 prev_width = vars->layout.full_width;
    i32 prev_height = vars->layout.full_height;
    i32 full_width = vars->layout.full_width = target->width;
    i32 full_height = vars->layout.full_height = target->height;
    
    if (prev_width != full_width || prev_height != full_height){
        layout_refit(&vars->layout, 0, vars->command_style.height,
                     full_width, full_height,
                     prev_width, prev_height);
        
        for (i32 panel_i = 0;
             panel_i < vars->layout.panel_count &&
             vars->layout.panel_count > 1;
             ++panel_i){
            Editing_Panel *panel = &panels[panel_i];
            
            Editing_Style *style = panel->file->style;
            i32 character_w = style_get_character_width(style);
            i32 margin_width = style->margin_width;
            if (panel->full_w < character_w*6 + margin_width*2){
                i32 share_w = panel->full_w;
                layout_close_panel(&vars->layout, panel_i);
                layout_redistribute_width(&vars->layout, share_w);
                panel_i = 0;
            }
        }
        
        for (i32 panel_i = 0; panel_i < vars->layout.panel_count; ++panel_i){
            Editing_Panel *panel = panels + panel_i;
            if (!panel->file->is_dummy){
                panel->cursor = panel_compute_cursor_from_pos(panel, panel->cursor.pos);
            }
        }
        
        app_result.redraw = 1;
    }
    ProfileSection(thread, app_step, "reorganizing panels");
    
    // NOTE(allen): mouse input handling
    if (time_step && !mouse->out_of_window){
        
        i32 mx = mouse->x;
        i32 my = mouse->y;
        bool32 mouse_press_event = 0;
        
        if (mouse->left_button && !mouse->left_button_prev){
            // TODO(allen):
            // This means any mouse use will be impossible, I guess it will have
            // to depend on the type of the request seen at the top???
            app_clear_request_queue(vars);
            mouse_press_event = 1;
            
            switch (vars->state){
                case APP_STATE_SEARCH:
                {
                    active_panel->show_temp_highlight = 0;
                    
                    system_free_memory(vars->isearch.str.str);
                    vars->state = APP_STATE_EDIT;
                }break;
            }
        }
        
        switch (vars->state){
            case APP_STATE_EDIT:
            {
                for (i32 panel_i = 0; panel_i < vars->layout.panel_count; ++panel_i){
                    
                    Editing_Panel *panel = &panels[panel_i];
                    
                    if (mx >= panel->x &&
                        mx < panel->w + panel->x &&
                        my >= panel->y &&
                        my < panel->h + panel->y){
                        
                        app_result.mouse_cursor_type = APP_MOUSE_CURSOR_IBEAM;
                        
                        if (mouse_press_event){
                            if (!panel->file->is_dummy){
                                app_result.redraw = 1;
                                
                                Font *font = &vars->font;
                                
                                i32 character_w = style_get_character_width(panel->file->style);
                                i32 character_h = font->line_skip;
                                i32 max_line_length = panel_compute_max_line_length(panel);
                                i32 max_lines = panel_compute_max_lines(panel);
                                
                                real32 grid_x = ((real32)mx - panel->x) / character_w;
                                real32 grid_y = ((real32)my - panel->y) / character_h;
                                
                                vars->last_click_x = grid_x;
                                vars->last_click_y = grid_y;
                                
                                if (grid_x >= 0 && grid_x <= max_line_length &&
                                    grid_y >= 0 && grid_y < max_lines){
                                    
                                    i32 pos_x = (i32)(grid_x + active_panel->scroll_x);
                                    i32 pos_y = (i32)(grid_y + active_panel->scroll_y);
                                    
                                    panel_cursor_move(active_panel, pos_x, pos_y);
                                }
                            }
                            vars->layout.active_panel = panel_i;
                            active_panel = panel;
                        }
                        else{
                            // NOTE(allen): mouse inside editing area but no click
                        }
                    }
                    
                    else if (mx >= panel->full_x &&
                             mx < panel->full_w + panel->full_x &&
                             my >= panel->full_y &&
                             my < panel->full_h + panel->full_y){
                        // NOTE(allen): not inside the editing area but within the margins
                        bool32 resize_area = 0;
                        app_result.mouse_cursor_type = APP_MOUSE_CURSOR_ARROW;
                        if (mx >= (panel->full_x+panel->full_w+panel->x+panel->w)/2){
                            if (panel_i != vars->layout.panel_count-1){
                                app_result.mouse_cursor_type = APP_MOUSE_CURSOR_LEFTRIGHT;
                                resize_area = 1;
                            }
                        }
                        else if (mx <= (panel->full_x+panel->x)/2){
                            if (panel_i != 0){
                                app_result.mouse_cursor_type = APP_MOUSE_CURSOR_LEFTRIGHT;
                                resize_area = -1;
                            }
                        }
                        
                        if (resize_area != 0 && mouse_press_event){
                            vars->state = APP_STATE_RESIZING;
                            if (resize_area == 1){
                                vars->resizing.left = panel;
                                vars->resizing.right = panel+1;
                            }
                            else if (resize_area == -1){
                                vars->resizing.left = panel-1;
                                vars->resizing.right = panel;
                            }
                        }
                    }
                }
                
                i32 cursor_y = panel_get_cursor_y(active_panel);
                real32 target_y = active_panel->target_y;
                i32 max_lines = panel_compute_max_lines(active_panel);
                
                bool32 wheel_used;
                
                real32 delta_target_y = Max(1, max_lines / 3.f);
                delta_target_y *= -mouse->wheel;
                
                target_y += delta_target_y;
                
                if (target_y < 0){
                    target_y = 0;
                }
                
                if (mouse->wheel == 0){
                    wheel_used = 0;
                }
                else{
                    wheel_used = 1;
                    
                    if (cursor_y >= target_y + max_lines){
                        cursor_y = (i32)target_y + max_lines - 1;
                    }
                    if (cursor_y < target_y){
                        cursor_y = (i32)target_y + 1;
                    }
                }
                
                active_panel->target_y = target_y;
                if (cursor_y != panel_get_cursor_y(active_panel)){
                    active_panel->cursor =
                        panel_compute_cursor_from_xy(active_panel,
                                                     active_panel->preferred_x,
                                                     cursor_y);
                }
                
                if (wheel_used){
                    app_result.redraw = 1;
                }
            }break;
            
            case APP_STATE_RESIZING:
            {
                if (mouse->left_button){
                    Editing_Panel *left = vars->resizing.left;
                    Editing_Panel *right = vars->resizing.right;
                    i32 left_x = left->full_x;
                    i32 left_w = left->full_w;
                    i32 right_x = right->full_x;
                    i32 right_w = right->full_w;
                    
                    AllowLocal(right_x);
                    
                    i32 new_left_x = left_x;
                    i32 new_left_w = mx - left_x;
                    i32 new_right_w = right_w - (new_left_w - left_w);
                    i32 new_right_x = mx;
                    
                    if (left_w != new_left_w){
                        app_result.redraw = 1;
                        
                        Editing_Style *left_style = left->file->style;
                        Editing_Style *right_style = right->file->style;
                        
                        i32 left_character_w = style_get_character_width(left_style);
                        i32 right_character_w = style_get_character_width(right_style);
                        
                        i32 left_margin_width = left_style->margin_width;
                        i32 right_margin_width = right_style->margin_width;
                        if (new_left_w > left_margin_width*2 + left_character_w*6 &&
                            new_right_w > right_margin_width*2 + right_character_w*6){
                            left->full_x = new_left_x;
                            left->full_w = new_left_w;
                            right->full_x = new_right_x;
                            right->full_w = new_right_w;
                            
                            panel_fix_internal_area(left);
                            panel_fix_internal_area(right);
                        }
                    }
                }
                else{
                    app_result.redraw = 1;
                    vars->state = APP_STATE_EDIT;
                }
            }break;
        }
    }
    ProfileSection(thread, app_step, "mouse input");
    
    // NOTE(allen): fix scrolling on all panels
    for (i32 panel_i = 0; panel_i < vars->layout.panel_count; ++panel_i){
        Editing_Panel *panel = &panels[panel_i];
        i32 cursor_y;
        if (panel->show_temp_highlight){
            if (panel->unwrapped_lines){
                cursor_y = panel->temp_highlight.unwrapped_y;
            }
            else{
                cursor_y = panel->temp_highlight.wrapped_y;
            }
        }
        else{
            if (panel->unwrapped_lines){
                cursor_y = panel->cursor.unwrapped_y;
            }
            else{
                cursor_y = panel->cursor.wrapped_y;
            }
        }
        real32 target_y = panel->target_y;
        real32 original_target_y = target_y;
        i32 max_lines = panel_compute_max_lines(panel);
        
        while (cursor_y >= Floor(target_y) + max_lines){
            target_y += 3.f;
        }
        
        while (cursor_y < target_y){
            target_y -= 3.f;
        }
        
        if (target_y < 0){
            target_y = 0;
        }
        
        panel->target_y = target_y;
        
        i32 cursor_x = panel_get_cursor_x(panel);
        real32 target_x = panel->target_x;
        real32 original_target_x = target_x;
        i32 max_x = panel_compute_max_line_length(panel);
        if (cursor_x < target_x){
            target_x = (real32)Max(0, cursor_x - max_x/2);
        }
        else if (cursor_x >= target_x + max_x){
            target_x = (real32)(cursor_x - max_x/2);
        }
        
        panel->target_x = target_x;
        
        if (original_target_y != panel->target_y ||
            original_target_x != panel->target_x){
            app_result.redraw;
        }
    }
    ProfileSection(thread, app_step, "fix scrolling");
    
    // NOTE(allen): execute animations
    for (i32 i = 0; i < vars->layout.panel_count; ++i){
        Editing_Panel *panel = vars->layout.panels + i;
        
        // TODO(allen): Scrolling parameterization in style?
        if (smooth_camera_step(&panel->target_y, &panel->scroll_y, &panel->vel_y, 2.f, 1.f/9.f)){
            app_result.redraw = 1;
        }
        
        if (smooth_camera_step(&panel->target_x, &panel->scroll_x, &panel->vel_x, 2.f, 1.f/6.f)){
            app_result.redraw = 1;
        }
        
        if (panel->paste_effect.tick_down > 0){
            --panel->paste_effect.tick_down;
            app_result.redraw = 1;
        }
        
    }
    ProfileSection(thread, app_step, "execute animations");
    
    if (app_result.redraw){
        // NOTE(allen): render the panels
        for (i32 panel_i = 0; panel_i < vars->layout.panel_count; ++panel_i){
            Editing_Panel *panel = &panels[panel_i];
            Editing_Style *style = panel->file->style;
            
            i32 full_left = panel->full_x;
            i32 full_top = panel->full_y;
            i32 full_right = full_left + panel->full_w;
            i32 full_bottom = full_top + panel->full_h;
            
            u32 back_color = style->back_color;
            draw_rectangle_2corner(target, full_left, full_top, full_right, full_bottom, back_color);
            
            u32 side_margin_color = style->margin_color;
            
            panel_draw(thread, target, panel, vars->layout.active_panel == panel_i);
            
            // NOTE(allen): file info bar
            {
                Interactive_Bar bar;
                bar.style = style->file_info_style;
                bar.pos_x = panel->x;
                bar.pos_y = full_top;
                
                draw_rectangle(target, full_left, bar.pos_y,
                               panel->full_w, bar.style.height,
                               bar.style.bar_color);
                
                Editing_File *file = panel->file;
                if (!file->is_dummy){
                    intbar_draw_string(target, &bar, panel->file->live_name, bar.style.base_color);
                    intbar_draw_string(target, &bar, make_lit_string(" - "), bar.style.base_color);
                    
                    u8 line_number_space[30];
                    String line_number = make_string(line_number_space, 0, 30);
                    append(&line_number, (u8*)"L#");
                    append_int_to_str(panel->cursor.line, &line_number);
                    
                    intbar_draw_string(target, &bar, line_number, bar.style.base_color);
                    
                    if (file->last_4ed_write_time != file->last_sys_write_time){
                        persist String out_of_sync = make_lit_string(" FILE SYNC");
                        intbar_draw_string(target, &bar, out_of_sync, bar.style.pop2_color);
                    }
                }
            }
            
            // L
            draw_rectangle_2corner(target, full_left, panel->y,
                                   panel->x, full_bottom, side_margin_color);
            
            // R
            draw_rectangle_2corner(target, panel->x + panel->w, panel->y,
                                   full_right, full_bottom, side_margin_color);
            
            // B
            draw_rectangle_2corner(target, full_left, panel->y + panel->h,
                                   full_right, full_bottom, side_margin_color);
            
            if (panel_i != 0){
                draw_rectangle_2corner(target, panel->full_x-1, panel->full_y,
                                       panel->full_x+1, panel->full_y+panel->full_h,
                                       0xFFFFFFFF);
            }
        }
        ProfileSection(thread, app_step, "render files");
        
        // NOTE (allen): command bar
        {
            Interactive_Bar bar;
            bar.style = vars->command_style;
            bar.pos_x = 0;
            bar.pos_y = 0;
            
            draw_rectangle(target, 0, 0,
                           target->width, bar.style.height,
                           bar.style.bar_color);
            
            switch (vars->state){
                case APP_STATE_EDIT:
                {
                    if (vars->request_count > 0){
                        Input_Request *request = vars->request_queue + vars->request_filled;
                        
                        switch (request->type){
                            case REQUEST_SYS_FILE:
                            {
                                intbar_draw_string(target, &bar, request->sys_file.query, bar.style.pop2_color);
                                
                                String *string = &request->sys_file.dest;
                                hot_directory_draw_helper(target, &vars->hot_directory, &bar, string, 1);
                            }break;
                            
                            case REQUEST_LIVE_FILE:
                            {
                                intbar_draw_string(target, &bar, request->sys_file.query, bar.style.pop2_color);
                                
                                String *string = &request->sys_file.dest;
                                live_file_draw_helper(target, &vars->working_set, &bar, string);
                            }break;
                            
                        }
                    }
                }break;
                
                case APP_STATE_SEARCH:
                {
                    persist String search_str = make_lit_string("I-Search: ");
                    persist String rsearch_str = make_lit_string("Reverse-I-Search: ");
                    if (vars->isearch.reverse){
                        intbar_draw_string(target, &bar, rsearch_str, bar.style.pop2_color);
                    }
                    else{
                        intbar_draw_string(target, &bar, search_str, bar.style.pop2_color);
                    }
                    
                    intbar_draw_string(target, &bar, vars->isearch.str, bar.style.base_color);
                }break;
                
                case APP_STATE_RESIZING:
                {
                    intbar_draw_string(target, &bar, make_lit_string("Resizing!"), bar.style.pop2_color);
                }break;
            }
        }
        ProfileSection(thread, app_step, "interaction bar");
        
    }
    
    ProfileEnd(thread, app_step, "total");
    
    return(app_result);
}

// BOTTOM

