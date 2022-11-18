/*

Development tool: Hierarchy editor
-Allen
28.06.2016

*/


// TOP

static void
extend_hierarchy_vars(System_API *system, Partition *part, Hierarchy_Vars *vars, i32 new_assets){
    void *new_mem = push_array(part, Editor_Asset_State, new_assets);
    if (new_mem == 0){
        DBG_expand_partition(system, part, new_assets*sizeof(Editor_Asset_State));
        new_mem = push_array(part, Editor_Asset_State, new_assets);
    }
    Assert(new_mem);
    cd_memset(new_mem, 0, new_assets*sizeof(Editor_Asset_State));
    vars->asset_count += new_assets;
}

static void
make_hierarchy_vars(System_API *system, Partition *part, Hierarchy_Vars *vars, i32 asset_count){
    extend_hierarchy_vars(system, part, vars, asset_count);
    vars->asset_states = (Editor_Asset_State*)part->base;
    vars->backup_timer = BACKUP_FREQUENCY*.5f;
}

static void
clear_details(Slot_Details *details){
    cd_memset(details, 0, sizeof(*details));
}

static void
full_sibling_insert_before(Asset_Node *node, Asset_Node *pos, void *manifest_memory){
    i32 pos_parent_self = pos->parent;
    Asset_Node *pos_parent = to_node_ptr(pos_parent_self, manifest_memory);
    rptr32 pos_self = to_rptr32(pos, manifest_memory);
    
    // NOTE(allen): If we just say "sibling_insert_before" it will
    // not update the "first_child" of pos_parent.  So if we happen
    // to be inserting before first here, we actually want to use
    // an insert under call which can adjust the parent's first child.
    if (pos_parent->first_child == pos_self){
        insert_under(node, pos_parent, manifest_memory, true);
    }
    else{
        sibling_insert_before(node, pos, manifest_memory);
    }
}

static b32
gui_do_text_field(GUI *gui, GUI_State *state, String *str, u32 flags){
    b32 result = false;
    
    if (gui_do_command(gui, gui_make_text_field(str, flags))){
        GUI_id id = guid_1(str);
        if ((flags & TextField_Selectable) &&
            (flags & TextField_StaticString)){
            if (guid_eq(state->selected, id)){
                result = true;
            }
        }
        else{
            if (guid_eq(state->activated, id)){
                result = true;
            }
        }
    }
    
    return(result);
}

static b32
gui_do_image_preview(GUI *gui, GUI_State *state, i32 image_id){
    b32 result = false;
    
    if (gui_do_command(gui, gui_make_image_preview(image_id))){
        result = true;
    }
    
    return(result);
}

static b32
gui_do_deselect(GUI *gui, GUI_State *state){
    b32 result = false;
    
    if (gui_do_command(gui, gui_make_deselect())){
        result = true;
    }
    
    return(result);
}

static String type_name_strs[AssetType_TypeCount];

static void
hierarchy_editor_step(System_API *system, Partition *part, Asset_Bank *bank,
                      Render_Target *ed_target, Dev_Input_State *dev_input,
                      Hierarchy_Vars *vars, Partition *manifest_part, Partition *edit_part,
                      vec2 start_pos, f32 descend, f32 gui_alpha){
    Asset_Manifest *manifest_memory = (Asset_Manifest*)manifest_part->base;
    
    // TODO(allen): Could stand to metaprogram this
    if (type_name_strs[0].str == 0){
        for (i32 i = 0; i < AssetType_TypeCount; ++i){
            String s = {0};
            
            switch (i){
                case AssetType_GenericFolder: {
                    s = make_lit_string("Generic Folder");
                }break;
                
                case AssetType_Image: {
                    s = make_lit_string("Image");
                }break;
                
                case AssetType_ObstacleType: {
                    s = make_lit_string("Obstacle");
                }break;
            }
            
            type_name_strs[i] = s;
        }
    }
    
    i32 new_asset_count = manifest_memory->asset_node_count - vars->asset_count;
    if (new_asset_count > 0){
        extend_hierarchy_vars(system, edit_part, vars, new_asset_count);
    }
    vars->asset_states = (Editor_Asset_State*)edit_part->base;
    
    f32 indent = 16.f;
    f32 height = 22.f;
    f32 hit_margin = 5.f;
    f32 v_gap = 5.f;
    f32 half_v_gap = 2.5f;
    f32 width = 400.f;
    
    // orange
    vec3 gui_color_1 = v3(1.f, 0.75f, 0.f);  AllowLocal(gui_color_1);
    vec3 gui_color_2 = v3(1.f, 0.5f, 0.f);   AllowLocal(gui_color_2);
    vec3 gui_color_3 = v3(1.f, 0.25f, 0.f);  AllowLocal(gui_color_3);
    // purple
    vec3 gui_color_4 = v3(1.f, 0.f, 0.25f);  AllowLocal(gui_color_4);
    vec3 gui_color_5 = v3(1.f, 0.f, 0.5f);   AllowLocal(gui_color_5);
    vec3 gui_color_6 = v3(1.f, 0.f, 0.75f);  AllowLocal(gui_color_6);
    // indigo
    vec3 gui_color_7 = v3(0.25f, 0.f, 1.f);  AllowLocal(gui_color_7);
    vec3 gui_color_8 = v3(0.5f, 0.f, 1.f);   AllowLocal(gui_color_8);
    vec3 gui_color_9 = v3(0.75f, 0.f, 1.f);  AllowLocal(gui_color_9);
    // cyan
    vec3 gui_color_10 = v3(0.f, 0.25f, 1.f); AllowLocal(gui_color_10);
    vec3 gui_color_11 = v3(0.f, 0.5f, 1.f);  AllowLocal(gui_color_11);
    vec3 gui_color_12 = v3(0.f, 0.75f, 1.f); AllowLocal(gui_color_12);
    // teal
    vec3 gui_color_13 = v3(0.f, 1.f, 0.75f); AllowLocal(gui_color_13);
    vec3 gui_color_14 = v3(0.f, 1.f, 0.5f);  AllowLocal(gui_color_14);
    vec3 gui_color_15 = v3(0.f, 1.f, 0.25f); AllowLocal(gui_color_15);
    // mint
    vec3 gui_color_16 = v3(0.25f, 1.f, 0.f); AllowLocal(gui_color_16);
    vec3 gui_color_17 = v3(0.5f, 1.f, 0.f);  AllowLocal(gui_color_17);
    vec3 gui_color_18 = v3(0.75f, 1.f, 0.f); AllowLocal(gui_color_18);
    
    Temp_Memory temp = begin_temp(part);
    
    i32 op_max = 2;
    i32 op_count = 0;
    Tree_Operation *operations = push_array(part, Tree_Operation, op_max);
    
    vec2 info_pos = start_pos;
    
    info_pos.y -= descend;
    
    b32 has_drop = (dev_input->drop_count > 0);
    i32 mx = dev_input->input.mx;
    i32 my = dev_input->input.my;
    
    f32 drag_offset_x = 0;
    f32 drag_offset_y = 0;
    
    Asset_Walker walker = {0};
    Asset_Node *node = walk_first_asset_node(manifest_memory, &walker);
    for (; node != 0; ){
        Editor_Asset_State *edit_state = &vars->asset_states[node->image_id-1];
        
        i32 level = walker.current_level;
        
        vec2 p0 = info_pos;
        vec2 p1 = {0};
        vec2 pn = info_pos;
        
        pn.y -= height + v_gap;
        p0.x += indent*level;
        
        if (vars->dragged_asset == node->image_id){
            drag_offset_x = mx - vars->dragged_offset_x - p0.x;
            drag_offset_y = my - vars->dragged_offset_y - p0.y;
            vars->dragged_level = level;
        }
        else if (level <= vars->dragged_level){
            drag_offset_x = 0;
            drag_offset_y = 0;
        }
        
        p0.x += drag_offset_x;
        p0.y += drag_offset_y;
        
        p1.x = p0.x + width;
        p1.y = p0.y - height;
        if (p1.y < 0) break;
        
        f32 alpha_mult = 0.5f;
        f32 above_alpha_mult = 0.f;
        f32 below_alpha_mult = 0.f;
        
        f32_rect above_rect = v4(p0.x, p0.y, p1.x, p0.y + v_gap);
        f32_rect slot_rect = v4(p0.x, p1.y, p1.x, p0.y);
        f32_rect below_rect = v4(p0.x, pn.y, p1.x, p1.y);
        
        f32_rect above_target = v4(p0.x, p0.y - hit_margin, p1.x, p0.y + half_v_gap);
        f32_rect hit_target = v4(p0.x, p1.y + hit_margin, p1.x, p0.y - hit_margin);
        f32_rect below_target = v4(p0.x, p1.y - half_v_gap, p1.x, p1.y + hit_margin);
        
        b32 below_is_first_child = false;
        
        if (node->first_child && !edit_state->collapsed){
            below_is_first_child = true;
            below_rect.x0 += indent;
            below_rect.x1 += indent;
        }
        
        enum{
            hit_none,
            hit_above,
            hit_direct,
            hit_below
        };
        
        i32 hit_type = hit_none;
        if (hit_check(mx, my, above_target)){
            above_alpha_mult = 1.f;
            hit_type = hit_above;
        }
        else if (hit_check(mx, my, hit_target)){
            alpha_mult = 1.f;
            hit_type = hit_direct;
        }
        else if (hit_check(mx, my, below_target)){
            below_alpha_mult = 1.f;
            hit_type = hit_below;
        }
        
        i32 pos_id = node->image_id;
        
        if (vars->clicked_asset){
            if (vars->clicked_asset == node->image_id && hit_type == hit_direct){
                if (btn_released(dev_input->input.left_button)){
                    edit_state->collapsed = !edit_state->collapsed;
                }
            }
        }
        
        else if (vars->right_clicked_asset){
            if (vars->right_clicked_asset == node->image_id && hit_type == hit_direct){
                if (btn_released(dev_input->input.right_button)){
                    vars->detail_asset = node->image_id;
                    clear_details(&vars->details);
                }
            }
        }
        
        else if (vars->dragged_asset){
            if (vars->dragged_asset != node->image_id){
                if (!btn_down(dev_input->input.left_button)){
                    if (hit_type != hit_none){
                        Assert(op_count + 2 <= op_max);
                        
                        i32 asset_id = vars->dragged_asset;
                        
                        i32 op_count_original = op_count;
                        operations[op_count++] = make_op(TreeOp_RemoveSubtree, asset_id, 0);
                        switch (hit_type){
                            case hit_above:
                            {
                                if (pos_id == ROOT){
                                    op_count = op_count_original;
                                }
                                else{
                                    operations[op_count++] =
                                        make_op(TreeOp_InsertSiblingSubtreeBefore, asset_id, pos_id);
                                }
                            }break;
                            
                            case hit_direct:
                            {
                                operations[op_count++] =
                                    make_op(TreeOp_InsertLastChildSubtree, asset_id, pos_id);
                            }break;
                            
                            case hit_below:
                            {
                                if (below_is_first_child){
                                    operations[op_count++] =
                                        make_op(TreeOp_InsertFirstChildSubtree, asset_id, pos_id);
                                }
                                else{
                                    if (pos_id == ROOT){
                                        op_count = op_count_original;
                                    }
                                    else{
                                        operations[op_count++] =
                                            make_op(TreeOp_InsertSiblingSubtreeAfter, asset_id, pos_id);
                                    }
                                }
                            }break;
                        }
                    }
                }
            }
        }
        
        else if (has_drop){
            u32 count = dev_input->drop_count; AllowLocal(count);
            Dev_File_Drop *drops = dev_input->drops;
            u32 i = 0;
            
            String name = make_string_slowly(drops[i].name);
            String file_name = front_of_directory(name);
            
            switch (hit_type){
                case hit_direct:
                {
                    String replace_name = make_string_slowly(node->name);
                    
                    b32 do_replace = false;
                    
                    if (match(replace_name, file_name)){
                        asset_file_write(system, name.str, file_name.str);
                        do_replace = true;
                    }
                    else{
                        if (!asset_file_exists(system, file_name.str)){
                            if (asset_file_write(system, name.str, file_name.str)){
                                asset_file_delete(system, replace_name.str);
                                do_replace = true;
                            }
                        }
                    }
                    
                    if (do_replace){
                        bank->replace_bmp(system, bank, part,
                                          drops[i].name, node->image_id);
                        replace_image(manifest_memory, file_name.str, node->image_id);
                    }
                }break;
                
                case hit_above:
                {
                    if (!asset_file_exists(system, file_name.str)){
                        Assert(op_count + 1 <= op_max);
                        Assert(pos_id <= manifest_memory->asset_node_count);
                        operations[op_count++] = make_op(TreeOp_NewNodeBefore, 0, pos_id, file_name.str);
                        asset_file_write(system, name.str, file_name.str);
                    }
                }break;
                
                case hit_below:
                {
                    if (!asset_file_exists(system, file_name.str)){
                        Assert(op_count + 1 <= op_max);
                        if (below_is_first_child){
                            i32 child_id = get_image_id(node->first_child, manifest_memory);
                            Assert(child_id <= manifest_memory->asset_node_count);
                            operations[op_count++] = make_op(TreeOp_NewNodeBefore, 0, child_id, file_name.str);
                            asset_file_write(system, name.str, file_name.str);
                        }
                        else{
                            Assert(pos_id <= manifest_memory->asset_node_count);
                            operations[op_count++] = make_op(TreeOp_NewNodeAfter, 0, pos_id, file_name.str);
                            asset_file_write(system, name.str, file_name.str);
                        }
                    }
                }break;
            }
        }
        
        else if (btn_down(dev_input->input.letter['R'])){
            switch (hit_type){
                case hit_direct:
                {
                    if (btn_pressed(dev_input->input.left_button)){
                        vars->dragged_asset = node->image_id;
                        vars->dragged_offset_y = my - p0.y;
                        vars->dragged_offset_x = mx - p0.x;
                    }
                }break;
            }
        }
        
        else{
            switch (hit_type){
                case hit_direct:
                {
                    if (btn_pressed(dev_input->input.left_button)){
                        vars->clicked_asset = node->image_id;
                    }
                    else if (btn_pressed(dev_input->input.right_button)){
                        vars->right_clicked_asset = node->image_id;
                    }
                }break;
                
                case hit_above:
                {
                    if (btn_pressed(dev_input->input.left_button)){
                        Assert(op_count + 1 <= op_max);
                        Assert(pos_id <= manifest_memory->asset_node_count);
                        operations[op_count++] = make_op(TreeOp_NewNodeBefore, 0, pos_id);
                    }
                }break;
                
                case hit_below:
                {
                    if (btn_pressed(dev_input->input.left_button)){
                        Assert(op_count + 1 <= op_max);
                        if (below_is_first_child){
                            i32 child_id = get_image_id(node->first_child, manifest_memory);
                            Assert(child_id <= manifest_memory->asset_node_count);
                            operations[op_count++] = make_op(TreeOp_NewNodeBefore, 0, child_id);
                        }
                        else{
                            Assert(pos_id <= manifest_memory->asset_node_count);
                            operations[op_count++] = make_op(TreeOp_NewNodeAfter, 0, pos_id);
                        }
                        
                    }
                }break;
            }
        }
        
        if (vars->clicked_asset == node->image_id ||
            vars->right_clicked_asset == node->image_id){
            alpha_mult = clamp_bottom(0.8f, alpha_mult);
        }
        
        switch (node->type){
            case AssetType_GenericFolder:
            {
                do_dbg_rectangle(ed_target, slot_rect, v4(gui_color_17, gui_alpha*alpha_mult));
            }break;
            
            case AssetType_Image:
            {
                do_dbg_rectangle(ed_target, slot_rect, v4(gui_color_2, gui_alpha*alpha_mult));
            }break;
            
            case AssetType_ObstacleType:
            {
                do_dbg_rectangle(ed_target, slot_rect, v4(gui_color_8, gui_alpha*alpha_mult));
            }break;
        }
        do_dbg_rectangle(ed_target, above_rect, v4(gui_color_1, gui_alpha*above_alpha_mult));
        do_dbg_rectangle(ed_target, below_rect, v4(gui_color_1, gui_alpha*below_alpha_mult));
        
        vec2 c = {p0.x + 5.f, p1.y + 3.f};
        
        if (node->first_child){
            if (edit_state->collapsed){
                do_string(ed_target, bank, DBG_FONT, c,
                          "+", 1,
                          white);
            }
            else{
                do_string(ed_target, bank, DBG_FONT, c,
                          "-", 1,
                          white);
            }
            c.x += 10.f;
        }
        
        do_string(ed_target, bank, DBG_FONT, c,
                  node->name, cd_strlen(node->name),
                  white);
        
        info_pos.y -= height + v_gap;
        
        if (edit_state->collapsed){
            node = walk_skip_children_asset_node(manifest_memory, &walker);
        }
        else{
            node = walk_next_asset_node(manifest_memory, &walker);
        }
    }
    
    if (!btn_down(dev_input->input.left_button)){
        vars->clicked_asset = 0;
        vars->dragged_asset = 0;
    }
    
    if (!btn_down(dev_input->input.right_button)){
        vars->right_clicked_asset = 0;
    }
    
    Tree_Operation *op = operations;
    for (i32 i = 0; i < op_count; ++i, ++op){
        i32 image_id = op->id;
        i32 pos_id = op->pos_id;
        Asset_Node *node = get_node(image_id, manifest_memory);
        Asset_Node *pos = get_node(pos_id, manifest_memory);
        char *str = op->str;
        
        switch (op->type){
            case TreeOp_RemoveSubtree:
            {
                tree_remove(node, manifest_memory);
            }break;
            
            case TreeOp_InsertLastChildSubtree:
            {
                insert_under(node, pos, manifest_memory, false);
            }break;
            
            case TreeOp_InsertFirstChildSubtree:
            {
                insert_under(node, pos, manifest_memory, true);
            }break;
            
            case TreeOp_InsertSiblingSubtreeBefore:
            {
                full_sibling_insert_before(node, pos, manifest_memory);
            }break;
            
            case TreeOp_InsertSiblingSubtreeAfter:
            {
                sibling_insert_after(node, pos, manifest_memory);
            }break;
            
            case TreeOp_NewNodeBefore:
            {
                Asset_Node *new_node = get_available_node(system, manifest_part, &manifest_memory);
                // NOTE(allen): We need to recompute pointers after calls to get_available_node because
                // can potentially rebase all the nodes.  Perhaps we should eliminate the Asset_Node*
                // parameters from the public interface to avoid this bug.
                Asset_Node *pos = get_node(pos_id, manifest_memory);
                if (str){
                    replace_image(manifest_memory, str, new_node->image_id);
                }
                full_sibling_insert_before(new_node, pos, manifest_memory);
            }break;
            
            case TreeOp_NewNodeAfter:
            {
                Asset_Node *new_node = get_available_node(system, manifest_part, &manifest_memory);
                // NOTE(allen): We need to recompute pointers after calls to get_available_node because
                // can potentially rebase all the nodes.  Perhaps we should eliminate the Asset_Node*
                // parameters from the public interface to avoid this bug.
                Asset_Node *pos = get_node(pos_id, manifest_memory);
                if (str){
                    replace_image(manifest_memory, str, new_node->image_id);
                }
                sibling_insert_after(new_node, pos, manifest_memory);
            }break;
        }
    }
    
    if (vars->detail_asset){
        Slot_Details *details = &vars->details;
        
        Asset_Node *node = get_node(vars->detail_asset, manifest_memory);
        Editor_Asset_State *edit_state =
            &vars->asset_states[node->image_id-1];
        AllowLocal(edit_state);
        
        if (!details->initialized){
            details->initialized = true;
            cd_memcpy(details->name_field_space, node->name, sizeof(node->name));
            details->name_field = make_string_slowly(details->name_field_space);
            details->name_field.memory_size = sizeof(node->name);
            
            details->type_name = make_fixed_width_string(details->type_name_space);
            copy(&details->type_name, type_name_strs[node->type]);
        }
        
        GUI gui = make_gui(part, 1024);
        GUI_State *state = &details->state;
        
        //
        // GUI declaration
        //
        
        {
            b32 doing_radio_buttons = false;
            if (gui_do_text_field(&gui, state, &details->type_name,
                                  TextField_Selectable | TextField_StaticString)){
                doing_radio_buttons = true;
                vars->do_type_radio_buttons = true;
            }
            
            if (vars->do_type_radio_buttons){
                i32 set_type = AssetType_TypeCount;
                for (i32 i = 0;
                     i < AssetType_TypeCount;
                     ++i){
                    if (gui_do_text_field(&gui, state, &type_name_strs[i], TextField_StaticString)){
                        set_type = i;
                    }
                }
                
                if (set_type < AssetType_TypeCount){
                    vars->do_type_radio_buttons = false;
                    
                    b32 good_to_set = true;
                    
                    if (set_type == AssetType_Image){
                        if (asset_file_exists(system, details->name_field.str)){
                            good_to_set = false;
                        }
                    }
                    
                    if (good_to_set){
                        node->type = set_type;
                        copy(&details->type_name, type_name_strs[set_type]);
                    }
                }
            }
            
            vars->do_type_radio_buttons = doing_radio_buttons;
            
            switch (node->type){
                case AssetType_GenericFolder:
                {
                    if (gui_do_text_field(&gui, state, &details->name_field, TextField_Selectable)){
                        cd_memcpy(node->name, details->name_field_space, sizeof(node->name));
                    }
                }break;
                
                case AssetType_Image:
                {
                    if (gui_do_text_field(&gui, state, &details->name_field, TextField_Selectable)){
                        terminate_with_null(&details->name_field);
                        if (!asset_file_exists(system, details->name_field.str)){
                            String ext = file_extension(details->name_field);
                            
                            if (match(ext, "bmp")){
                                if (asset_file_write(system, node->name, details->name_field.str)){
                                    asset_file_delete(system, node->name);
                                    cd_memcpy(node->name, details->name_field.str, sizeof(node->name));
                                }
                            }
                        }
                    }
                    gui_do_image_preview(&gui, state, node->image_id);
                }break;
                
                case AssetType_ObstacleType:
                {
                    if (gui_do_text_field(&gui, state, &details->name_field, TextField_Selectable)){
                        cd_memcpy(node->name, details->name_field_space, sizeof(node->name));
                    }
                }break;
            }
        }
        
        vec2 p = start_pos + v2(width*1.5f, -height*2);
        
        //
        // GUI execution
        //
        
        {
            GUI_State *state = &details->state;
            b32 reload = false;
            if (btn_pressed(dev_input->input.left_button)){
                state->hot = guid_zero();
            }
            if (btn_released(dev_input->input.left_button)){
                state->selected = guid_zero();
                reload = true;
            }
            
            state->activated = guid_zero();
            
            GUI_Command *command = gui.commands;
            state->activated = guid_zero();
            for (i32 i = 0;
                 i < gui.count;
                 ++i, ++command){
                switch (command->type){
                    case guicom_text_field:
                    {
                        String *edit_str = command->text_field.edit_str;
                        
                        vec2 p0 = p;
                        vec2 p1 = p0 + v2(width, -height);
                        f32_rect rect = v4(p0.x, p1.y, p1.x, p0.y);
                        p.y -= height;
                        
                        vec3 color = gui_color_3;
                        f32 alpha_mult = 0.5f;
                        
                        if (command->text_field.flags == TextField_StaticString){
                            color = gui_color_10;
                        }
                        
                        GUI_id id = guid_1(edit_str);
                        
                        if (hit_check(mx, my, rect)){
                            alpha_mult = 1.f;
                            if (btn_pressed(dev_input->input.left_button)){
                                state->hot = id;
                            }
                            else if (btn_released(dev_input->input.left_button) && guid_eq(state->hot, id)){
                                state->hot = guid_zero();
                                if (command->text_field.flags & TextField_Selectable){
                                    state->selected = id;
                                }
                                else{
                                    state->activated = id;
                                }
                            }
                        }
                        
                        if (guid_eq(state->hot, id)){
                            alpha_mult = clamp_bottom(0.9f, alpha_mult);
                        }
                        
                        if (guid_eq(state->selected, id)){
                            color = gui_color_4;
                            alpha_mult = 1.f;
                            
                            if (!(command->text_field.flags & TextField_StaticString)){
                                i32 count = dev_input->keys.count;
                                for (i32 j = 0;
                                     j < count;
                                     ++j){
                                    char key = dev_input->keys.events[j];
                                    if (key == key_back){
                                        if (edit_str->size > 0){
                                            --edit_str->size;
                                            terminate_with_null(edit_str);
                                        }
                                    }
                                    else if (key == '\n'){
                                        state->activated = id;
                                        state->selected = guid_zero();
                                    }
                                    else if (key >= ' ' && key <= '~'){
                                        if (edit_str->size+1 < edit_str->memory_size){
                                            append(edit_str, key);
                                            terminate_with_null(edit_str);
                                        }
                                    }
                                }
                            }
                        }
                        
                        do_dbg_rectangle(ed_target, rect, v4(color, gui_alpha*alpha_mult));
                        
                        vec2 c = {p0.x + 5.f, p1.y + 3.f};
                        do_string(ed_target, bank, DBG_FONT, c,
                                  edit_str->str, edit_str->size,
                                  white);
                    }break;
                    
                    case guicom_image_preview:
                    {
                        i32 image_id = command->image_preview.image_id;
                        
                        f32 max_w = width;
                        f32 max_h = height*10;
                        
                        f32 w = height;
                        f32 h = height;
                        
                        Image *image = get_image(bank, image_id);
                        if (image){
                            w = image->width;
                            h = image->height;
                            
                            f32 ratio_w = 1.;
                            f32 ratio_h = 1.;
                            
                            if (w > max_w){
                                ratio_w = max_w / w;
                            }
                            if (h > max_h){
                                ratio_h = max_h / h;
                            }
                            
                            f32 ratio = ratio_w;
                            if (ratio > ratio_h){
                                ratio = ratio_h;
                            }
                            
                            w *= ratio;
                            h *= ratio;
                        }
                        
                        vec2 p0 = p;
                        vec2 p1 = p + v2(w, -h);
                        f32_rect rect = v4(p0.x, p1.y, p1.x, p0.y);
                        
                        p = v2(p0.x, p1.y);
                        
                        vec2 x_axis = v2(w*.5f, 0);
                        vec2 y_axis = v2(0, h*.5f);
                        
                        do_dbg_rectangle(ed_target, rect, v4(black3, gui_alpha));
                        do_image_general(ed_target, bank, 0, image_id,
                                         Render_Exact, p0, v2(-1.f, 1.f),
                                         x_axis, y_axis);
                    }break;
                    
                    case guicom_deselect:
                    {
                        state->selected = guid_zero();
                    }break;
                }
            }
            
            if (reload){
                if (guid_eq(state->selected, guid_zero()) &&
                    guid_eq(state->activated, guid_zero())){
                    clear_details(details);
                }
            }
        }
    }
    
    end_temp(temp);
    
    vars->backup_timer -= dev_input->input.dt;
    if (vars->backup_timer <= 0.f){
        vars->backup_timer = BACKUP_FREQUENCY;
        asset_file_backup(system);
    }
}

// BOTTOM


