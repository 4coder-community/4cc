/*
Main game code.
*/

// TOP

#include "cd_random.h"
#include "cd_world.h"
#include "cd_colors.h"

#define FTECH_STRING_IMPLEMENTATION
#include "4tech_string.h"

#ifdef DEVELOPER
# include "cd_hierarchy_editor.h"
#endif

#include "cd_world.cpp"

inline i32
relative_lane(i32 total_lanes, i32 half_visible_lanes,
              i32 lane, i32 center){
    i32 result = lane - center;
    if (result >= total_lanes - half_visible_lanes){
        result -= total_lanes;
    }
    else if (result <= -total_lanes + half_visible_lanes){
        result += total_lanes;
    }
    return(result);
}

inline i32
absolute_lane(i32 total_lanes, i32 relative_lane, i32 center){
    i32 result = relative_lane + center;
    if (result >= total_lanes){
        result -= total_lanes;
    }
    if (result < 0){
        result += total_lanes;
    }
    return(result);
}

enum Dev_Mode{
    DevMode_None,
    DevMode_Dbg,
    DevMode_Ed
};

struct App_Vars{
    Partition part;
    b32 initialized;
    Partition trans_part;
    
    Asset_Manifest *manifest_memory;
    
    World world;
    f32 view_y, view_y_vel;
    Random_Series random;
};

struct Dev_Vars{
    Partition part;
    b32 initialized;
    
    i32 dev_mode;
    
    Hierarchy_Vars hierarchy_vars;
    Partition manifest_part;
    Partition edit_state_part;
};

void
establish_target(Render_Target *target, Partition *part){
    target->push_part = sub_partition(part, Kbytes(256));
    cd_memset(target->push_part.base, 0, Kbytes(256));
    
    target->memory_size = Kbytes(256);
    target->memory = push_block(part, target->memory_size);
    cd_memset(target->memory, 0, target->memory_size);
}

void
establish_bank(Asset_Bank *bank, Partition *part, i32 max_fonts, i32 max_images){
    bank->font_max_id = max_fonts;
    if (max_fonts > 0){
        bank->fonts = push_array(part, Font, bank->font_max_id);
    }
    else{
        bank->fonts = 0;
    }
    
    bank->image_max_id = max_images;
    if (max_images > 0){
        bank->images = push_array(part, Image, bank->image_max_id);
    }
    else{
        bank->images = 0;
    }
}

inline b32
btn_pressed(Key_State key){
    b32 result = (key.down && !key.prev_down);
    return(result);
}

inline b32
btn_down(Key_State key){
    b32 result = (key.down);
    return(result);
}

inline b32
btn_released(Key_State key){
    b32 result = (!key.down && key.prev_down);
    return(result);
}

inline b32
btn_up(Key_State key){
    b32 result = (!key.down);
    return(result);
}

#ifdef DEVELOPER

// TODO(allen): Rewrite these as an asset archive system
// for compression and mod/extension packing.

static void
asset_file_backup(System_API *system){
    int len = 0;
    
    char space[1024];
    String script = make_fixed_width_string(space);
    
    append(&script, "backup.bat \"");
    
    
    len = system->DBG_working_path(script.str + script.size,
                                   script.memory_size - script.size);
    script.size += len;
    
    append(&script, "\" ");
    
    append_int_to_str(&script, MAX_BACKUP_COUNT);
    
    terminate_with_null(&script);
    
    system->DBG_call_script(script.str);
}

static void
asset_file_delete(System_API *system, char *name){
    system->DBG_dump_out(name, 0, 0);
}

static b32
asset_file_write(System_API *system, char *source, char *name){
    b32 result = false;
    
    if (system->DBG_copy(source, name)){
        result = true;
    }
    
    return(result);
}

static b32
asset_file_exists(System_API *system, char *name){
    b32 result = false;
    File_Dump dump = system->DBG_dump_begin(name);
    
    if (dump.size > 0){
        result = true;
        system->DBG_dump_end(dump, 0);
    }
    
    return(result);
}

# include "cd_hierarchy_editor.cpp"
#endif

extern "C"
App_Step_Sig(app_step){
    App_Vars *vars = (App_Vars*)memory.logical;
    
    if (!vars->initialized){
        vars->initialized = 1;
        vars->part = make_partition(memory.logical, memory.logical_size);
        push_type(&vars->part, App_Vars);
        
        vars->trans_part = make_partition(memory.transient, memory.transient_size);
        
        establish_target(main_target, &vars->trans_part);
        establish_bank(bank, &vars->trans_part, DBG_FONT_COUNT, IMAGE_COUNT);
        main_target->me_to_pixel_transform = unit_to_pixel_transform;
        
#ifdef DEVELOPER
        {
            Dev_Vars *dev_vars = (Dev_Vars*)memory.developer;
            vars->manifest_memory = (Asset_Manifest*)
                manifest_load(system, &dev_vars->manifest_part);
        }
#else
        vars->manifest_memory = (Asset_Manifest*)
            manifest_load(system, &vars->trans_part);
#endif
        
        if (vars->manifest_memory){
            Assert(ASSET_MANIFEST_VERSION == vars->manifest_memory->version);
            
            Asset_Walker walker = {0};
            
            Temp_Memory temp = begin_temp(&vars->trans_part);
            Asset_Bank_Setup setup =
                bank->begin_setup(bank, &vars->trans_part, system);
            
            bank->bind_font(&setup, "LiberationSans-Regular.ttf", DBG_FONT);
            bank->bind_font(&setup, "liberation-mono.ttf", DBG_MONO_FONT);
            
            for (Asset_Node *node = walk_first_asset_node(vars->manifest_memory, &walker);
                 node != 0;
                 node = walk_next_asset_node(vars->manifest_memory, &walker)){
                switch (node->type){
                    case AssetType_Image:
                    bank->bind_bmp(&setup, node->name, node->image_id);
                    break;
                }
            }
            
            bank->end_setup(&setup);
            end_temp(temp);
        }
        else{
            i32 manifest_size = Kbytes(8);
#ifdef DEVELOPER
            {
                Dev_Vars *dev_vars = (Dev_Vars*)memory.developer;
                DBG_expand_partition(system, &dev_vars->manifest_part, manifest_size);
                vars->manifest_memory = (Asset_Manifest*)
                    push_block(&dev_vars->manifest_part, manifest_size);
            }
#else
            vars->manifest_memory = (Asset_Manifest*)
                push_block(&vars->trans_part, manifest_size);
#endif
            initialize_empty_manifest(vars->manifest_memory, manifest_size);
            
            {
                void *manifest = vars->manifest_memory;
                Temp_Memory temp = begin_temp(&vars->trans_part);
                Asset_Bank_Setup setup =
                    bank->begin_setup(bank, &vars->trans_part, system);
                
                bank->bind_font(&setup, "LiberationSans-Regular.ttf", DBG_FONT);
                bank->bind_font(&setup, "liberation-mono.ttf", DBG_MONO_FONT);
                
                Manifest_Setup man_setup = {0};
                
                begin_folder(manifest, &man_setup, "game", ROOT);
                {
                    bank->bind_bmp(&setup, "covaris.bmp", BACKGROUND);
                    bind_image(manifest, &man_setup, "covaris.bmp", BACKGROUND);
                    
                    bank->bind_bmp(&setup, "Sheila.bmp", SHEILA);
                    bind_image(manifest, &man_setup, "Sheila.bmp", SHEILA);
                    
                    bank->bind_bmp(&setup, "HUD_Deck-Box.bmp", HUD_DECK);
                    bank->bind_bmp(&setup, "HUD_Distance_meter_ly.bmp", HUD_DISTANCE_METER_LY);
                    bank->bind_bmp(&setup, "HUD_Distance_meter_pc.bmp", HUD_DISTANCE_METER_PC);
                    bank->bind_bmp(&setup, "HUD_Fuel_Velocity_gauge.bmp", HUD_FUEL_VELOCITY);
                    bank->bind_bmp(&setup, "HUD_Indicator_off.bmp", HUD_INDICATOR_OFF);
                    bank->bind_bmp(&setup, "HUD_Indicator_on.bmp", HUD_INDICATOR_ON);
                    bank->bind_bmp(&setup, "HUD_Numbers.bmp", HUD_NUMBERS);
                    bank->bind_bmp(&setup, "HUD_Slider.bmp", HUD_SLIDER);
                    
                    begin_folder(manifest, &man_setup, "HUD", HUD_FOLDER);
                    bind_image(manifest, &man_setup, "HUD_Deck-Box.bmp", HUD_DECK);
                    bind_image(manifest, &man_setup, "HUD_Distance_meter_ly.bmp", HUD_DISTANCE_METER_LY);
                    bind_image(manifest, &man_setup, "HUD_Distance_meter_pc.bmp", HUD_DISTANCE_METER_PC);
                    bind_image(manifest, &man_setup, "HUD_Fuel_Velocity_gauge.bmp", HUD_FUEL_VELOCITY);
                    bind_image(manifest, &man_setup, "HUD_Indicator_off.bmp", HUD_INDICATOR_OFF);
                    bind_image(manifest, &man_setup, "HUD_Indicator_on.bmp", HUD_INDICATOR_ON);
                    bind_image(manifest, &man_setup, "HUD_Numbers.bmp", HUD_NUMBERS);
                    bind_image(manifest, &man_setup, "HUD_Slider.bmp", HUD_SLIDER);
                    end_folder(manifest, &man_setup);
                    
                    bank->bind_bmp(&setup, "Asteriod_1.bmp", ASTEROID1);
                    bank->bind_bmp(&setup, "Asteriod_2.bmp", ASTEROID2);
                    bank->bind_bmp(&setup, "Asteriod_3.bmp", ASTEROID3);
                    bank->bind_bmp(&setup, "Asteriod_16.bmp", ASTEROID4);
                    bank->bind_bmp(&setup, "Asteriod_17.bmp", ASTEROID5);
                    bank->bind_bmp(&setup, "Asteriod_18.bmp", ASTEROID6);
                    
                    begin_folder(manifest, &man_setup, "ass tree odds", ASS_TREE_ODD);
                    bind_image(manifest, &man_setup, "Asteriod_1.bmp", ASTEROID1);
                    bind_image(manifest, &man_setup, "Asteriod_2.bmp", ASTEROID2);
                    bind_image(manifest, &man_setup, "Asteriod_3.bmp", ASTEROID3);
                    bind_image(manifest, &man_setup, "Asteriod_16.bmp", ASTEROID4);
                    bind_image(manifest, &man_setup, "Asteriod_17.bmp", ASTEROID5);
                    bind_image(manifest, &man_setup, "Asteriod_18.bmp", ASTEROID6);
                    end_folder(manifest, &man_setup);
                }
                end_folder(manifest, &man_setup);
                
                bank->end_setup(&setup);
                end_temp(temp);
            }
        }
        
        add_player(&vars->world);
        add_asteroid_spawner(&vars->world, 0.5f);
    }
    
    clear_target(main_target);
    clear_target(dev_target);
    
    Render_Target *dbg_target = 0;
    Render_Target *ed_target = 0;
    
    
    //
    // Developer Logic
    //
    
#ifdef DEVELOPER
    Dev_Vars *dev_vars = (Dev_Vars*)memory.developer;
    if (!dev_vars->initialized){
        dev_vars->initialized = 1;
        dev_vars->part = make_partition(memory.developer, memory.developer_size);
        push_type(&dev_vars->part, Dev_Vars);
        
        // TODO(allen): Don't use game's actual trans_part for dev code!
        // Set aside another dev_trans_part instead if you must.
        establish_target(dev_target, &vars->trans_part);
        dev_target->me_to_pixel_transform = identity_transform;
        
        i32 asset_count = vars->manifest_memory->asset_node_count;
        make_hierarchy_vars(system, &dev_vars->edit_state_part, &dev_vars->hierarchy_vars, asset_count);
    }
    
    if (btn_pressed(dev_input->fkeys[1])){
        if (dev_vars->dev_mode != DevMode_Dbg){
            dev_vars->dev_mode = DevMode_Dbg;
        }
        else{
            dev_vars->dev_mode = DevMode_None;
        }
    }
    else if (btn_pressed(dev_input->fkeys[2])){
        if (dev_vars->dev_mode != DevMode_Ed){
            dev_vars->dev_mode = DevMode_Ed;
        }
        else{
            dev_vars->dev_mode = DevMode_None;
        }
    }
    
    switch (dev_vars->dev_mode){
        case DevMode_None: break;
        case DevMode_Dbg: dbg_target = dev_target; break;
        case DevMode_Ed: ed_target = dev_target; break;
    }
#endif
    
    
    //
    // Execute Game
    //
    
    target_transform(main_target, TF_Pixels);
    target_transform(dev_target, TF_Pixels);
    
    do_image_general(main_target, bank, 0, BACKGROUND,
                     Render_Exact,
                     main_target->dim*.5f,
                     v2(0,0),
                     v2(main_target->dim.x*.5f, 0),
                     v2(0, main_target->dim.y*.5f));
    
    target_transform(main_target, TF_Units);
    target_transform(dev_target, TF_Units);
    
    
    //
    // Establish Lanes
    //
    
    f32 lane_height = visible_top / total_visible_lanes;
    Entity *player_entity = get_player(&vars->world);
    i32 player_lane = player_entity->lane;
    
    {
        i32 half_rendered_lanes = half_visible_lanes+2;
        
        i32 i = -half_rendered_lanes;
        i32 lane_index = player_lane + i;
        if (lane_index < 0) lane_index += total_lanes;
        
        f32 lane_y = -lane_height*half_rendered_lanes;
        
        for (; i <= half_rendered_lanes; ++i){
            set_lane(main_target, dbg_target, lane_index, lane_y - vars->view_y);
            lane_index = (lane_index + 1) % total_lanes;
            lane_y += lane_height;
        }
    }
    
    
    //
    // Move Camera
    //
    
    f32 dt = input->dt;
    {
        f32 lane_shift = lerp(0.f, player_entity->lane_lerp, lane_height);
        f32 view_accel = 10.f;
        f32 damping = 2.f;
        
        if (vars->view_y != lane_shift){
            f32 a = view_accel;
            f32 v = vars->view_y_vel;
            f32 y = vars->view_y;
            
            a = view_accel*(lane_shift - vars->view_y) + damping*(0 - v);
            
            y = 0.5f*dt*dt*a + dt*v + y;
            v = dt*a + v;
            
            vars->view_y_vel = v;
            vars->view_y = y;
        }
    }
    
    
    //
    // Run Simulation
    //
    
    world_begin_simulation(&vars->world);
    
    f32 player_speed = 10.f;
    
    f32 lane_change_per_second = 3.f;
    
    Entity *entity = vars->world.entities;
    u32 entity_count = vars->world.count;
    
    for (u32 entity_index = 0;
         entity_index < entity_count;
         ++entity_index, ++entity){
        
        if (entity->lane_lerp_dir == 0 &&
            entity->lane_lerp != 0){
            if (entity->lane_lerp > 0){
                entity->lane_lerp_dir = 1;
            }
            else{
                entity->lane_lerp_dir = -1;
            }
        }
        
        switch (entity->type){
            case ET_Player:
            {
                if (btn_pressed(input->down)){
                    entity->lane_lerp_dir = -1;
                }
                
                if (btn_pressed(input->up)){
                    entity->lane_lerp_dir = 1;
                }
                
                if (entity->lane_lerp_dir > 0){
                    if (entity->lane_lerp >= 0){
                        entity->lane_lerp += entity->lane_lerp_dir * lane_change_per_second * dt;
                        if (entity->lane_lerp >= 1.f){
                            entity->lane += 1;
                            entity->lane_lerp = 0;
                            entity->lane_lerp_dir = 0;
                            vars->view_y -= lane_height;
                        }
                    }
                    else{
                        entity->lane_lerp += entity->lane_lerp_dir * lane_change_per_second * dt;
                        if (entity->lane_lerp >= 0.f){
                            entity->lane_lerp = 0;
                            entity->lane_lerp_dir = 0;
                        }
                    }
                }
                else if (entity->lane_lerp_dir < 0){
                    if (entity->lane_lerp <= 0){
                        entity->lane_lerp += entity->lane_lerp_dir * lane_change_per_second * dt;
                        if (entity->lane_lerp <= -1.f){
                            entity->lane -= 1;
                            entity->lane_lerp = 0;
                            entity->lane_lerp_dir = 0;
                            vars->view_y += lane_height;
                        }
                    }
                    else{
                        entity->lane_lerp += entity->lane_lerp_dir * lane_change_per_second * dt;
                        if (entity->lane_lerp <= 0.f){
                            entity->lane_lerp = 0;
                            entity->lane_lerp_dir = 0;
                        }
                    }
                }
                
                if (entity->lane < 0) entity->lane += total_lanes;
                if (entity->lane >= total_lanes) entity->lane -= total_lanes;
                
                vec2 pos;
                pos.x = 0.f;
                if (entity->lane_lerp >= 0){
                    pos.y = slerp(0.f, entity->lane_lerp, lane_height) - vars->view_y;
                }
                else{
                    pos.y = slerp(0.f, -entity->lane_lerp, -lane_height) - vars->view_y;
                }
                
                do_image(main_target, bank, dbg_target, SHEILA, pos, 0.2f);
            }break;
            
            case ET_Obstacle:
            {
                f32 speed = entity->speed + player_speed;
                entity->x -= speed * dt;
                
                if (entity->x < -25.f){
                    add_to_kill_list(&vars->world, entity);
                }
                
                do_obstacle(main_target, dbg_target,
                            ET_Obstacle, entity->seed,
                            entity->x, entity->lane);
            }break;
            
            case ET_Obstacle_Spawner:
            {
                f32 second_per_spawn = entity->second_per_spawn;
                entity->spawn_clock += dt;
                
                while (entity->spawn_clock > second_per_spawn){
                    i32 lane = absolute_lane(total_lanes,
                                             random_between(&vars->random,
                                                            -half_visible_lanes*2,
                                                            half_visible_lanes*2),
                                             player_lane);
                    add_asteroid(&vars->world, lane);
                    
                    entity->spawn_clock -= second_per_spawn;
                }
            }break;
            
            case ET_Null:
            case ET_Count:
            default:
            InvalidCodePath;
        }
    }
    
    world_end_simulation(&vars->world);
    
    
    //
    // Render HUD
    //
    
    target_transform(main_target, TF_Pixels);
    target_transform(dev_target, TF_Pixels);
    
    f32 pos = main_target->dim.y;
    
    f32 gauge_width = main_target->dim.x/8.f;
    f32 gauge_height = gauge_width/3.f;
    
    f32 deck_height = gauge_height;
    f32 deck_width = gauge_width;
    
    f32 distance_height = gauge_height;
    f32 distance_width = gauge_width;
    
    f32 indicator_width = 25.f;
    f32 indicator_height = 25.f;
    
    f32 side_margin = 10.f;
    f32 slider_width = 10.f;
    f32 slider_height = 12.f;
    f32 slider_offset = slider_height + 8.f;
    
    f32 extra_deck_width = 50.f;
    
    deck_width += extra_deck_width;
    deck_height = deck_width/3.f;
    
    f32 deck_y_shift = 30.f;
    
    {
        f32 fuel_amount = 0.5f;
        f32 speed_amount = 0.5f;
        
        do_image_general(main_target, bank, 0,
                         HUD_FUEL_VELOCITY, Render_Exact,
                         v2(side_margin, pos),
                         v2(-1.f, 1.f),
                         v2(gauge_width, 0.f),
                         v2(0.f, gauge_height));
        
        do_image_general(main_target, bank, 0,
                         HUD_SLIDER, Render_Exact,
                         v2(side_margin + gauge_width*fuel_amount*2.f,
                            pos - slider_offset),
                         v2(0.f, 1.f),
                         v2(slider_width, 0.f),
                         v2(0.f, -slider_height));
        
        do_image_general(main_target, bank, 0,
                         HUD_SLIDER, Render_Exact,
                         v2(side_margin + gauge_width*speed_amount*2.f,
                            pos - gauge_height*2 + slider_offset + 1),
                         v2(0.f, 1.f),
                         v2(slider_width, 0.f),
                         v2(0.f, slider_height));
    }
    
    {
        do_image_general(main_target, bank, 0,
                         HUD_DECK, Render_Exact,
                         v2(main_target->dim.x*.5f, pos+deck_y_shift),
                         v2(0.f, 1.f),
                         v2(deck_width, 0.f),
                         v2(0.f, deck_height));
    }
    
    {
        do_image_general(main_target, bank, 0,
                         HUD_DISTANCE_METER_LY, Render_Exact,
                         v2(main_target->dim.x - side_margin, pos),
                         v2(1.f, 1.f),
                         v2(distance_width, 0.f),
                         v2(0.f, distance_height));
    }
    
    {
        do_image_general(main_target, bank, 0,
                         HUD_INDICATOR_OFF, Render_Exact,
                         v2(1010.f, 760.f),
                         v2(1.f, 1.f),
                         v2(indicator_width, 0.f),
                         v2(0.f, indicator_height));
        
        do_image_general(main_target, bank, 0,
                         HUD_INDICATOR_OFF, Render_Exact,
                         v2(1010.f, 760.f - indicator_height*2.f),
                         v2(1.f, 1.f),
                         v2(indicator_width, 0.f),
                         v2(0.f, indicator_height));
    }
    
    null_render(main_target);
    main_target->execute(main_target, bank);
    
#ifdef DEVELOPER
    //
    // Loop Info
    //
    
    target_transform(dev_target, TF_Pixels);
    
    vec2 status_pos = v2(25.f, dev_target->dim.y - 25.f);
    
    f32 descend = 20.f;
    f32 mouse_alpha = 0.5f;
    f32 gui_alpha = 0.75f;
    
    if (loop_mode < 3){
        switch (loop_mode){
            case 1:
            do_string(dev_target, bank, DBG_FONT, status_pos,
                      "Recording Loop", sizeof("Recording Loop") - 1,
                      white);
            break;
            
            case 2:
            do_dbg_rectangle(dev_target,
                             v4(dev_input->input.mx - 5.f, dev_input->input.my - 5.f,
                                dev_input->input.mx + 5.f, dev_input->input.my + 5.f),
                             v4(1.f, 1.f, 0.f, mouse_alpha));
            
            do_string(dev_target, bank, DBG_FONT, status_pos,
                      "Playing Loop", sizeof("Playing Loop") - 1,
                      white);
            break;
        }
    }
    else{
        do_dbg_rectangle(dev_target,
                         v4(dev_input->input.mx - 5.f, dev_input->input.my - 5.f,
                            dev_input->input.mx + 5.f, dev_input->input.my + 5.f),
                         v4(1.f, 1.f, 0.f, mouse_alpha));
        
        do_string(dev_target, bank, DBG_FONT, status_pos,
                  "Stopped", sizeof("Stopped") - 1,
                  white);
    }
    
    
    //
    // Execute Debug
    //
    
    if (dev_vars->dev_mode == DevMode_Dbg){
        do_dbg_rectangle(dbg_target,
                         v4(input->mx - 5.f, input->my - 5.f,
                            input->mx + 5.f, input->my + 5.f),
                         v4(0.f, 1.f, 0.f, mouse_alpha));
        
        vec2 info_pos = status_pos;
        info_pos.y -= descend;
        
        char space[64];
        String string = make_fixed_width_string(space);
        
        append(&string, "mouse = (");
        append_int_to_str(&string, dev_input->input.mx);
        append(&string, ",");
        append_int_to_str(&string, dev_input->input.my);
        append(&string, ")");
        
        do_string(dbg_target, bank, DBG_FONT, info_pos,
                  string.str, string.size,
                  white);
    }
    
    
    //
    // Execute Editor
    //
    
    if (dev_vars->dev_mode == DevMode_Ed){
        do_dbg_rectangle(ed_target,
                         v4(input->mx - 5.f, input->my - 5.f,
                            input->mx + 5.f, input->my + 5.f),
                         v4(0.f, 1.f, 0.f, mouse_alpha));
        
        // TODO(allen): Need dev_trans_part
        hierarchy_editor_step(system, &vars->trans_part, bank,
                              ed_target, dev_input,
                              &dev_vars->hierarchy_vars,
                              &dev_vars->manifest_part,
                              &dev_vars->edit_state_part,
                              status_pos, descend, gui_alpha);
        vars->manifest_memory = (Asset_Manifest*)dev_vars->manifest_part.base;
        
        manifest_dump(system, vars->manifest_memory);
    }
#endif
    
    null_render(dev_target);
    dev_target->execute(dev_target, bank);
}

// BOTTOM

