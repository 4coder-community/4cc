/*

Development tool: Hierarchy editor
-Allen
28.06.2016

*/


// TOP

#ifndef CD_HIERARCHY_EDITOR_H
#define CD_HIERARCHY_EDITOR_H

//
// GUI
//

struct GUI_id{
    u64 id[2];
};

inline GUI_id
guid_zero(){
    GUI_id id = {0};
    return(id);
}

inline GUI_id
guid_1(u64 x){
    GUI_id id = {x, 0};
    return(id);
}

inline GUI_id
guid_1(void *p){
    GUI_id id = {(u64)p, 0};
    return(id);
}

inline GUI_id
guid_2(u64 x, u64 y){
    GUI_id id = {x, y};
    return(id);
}

inline GUI_id
guid_2(u64 x, void *y){
    GUI_id id = {x, (u64)y};
    return(id);
}

inline GUI_id
guid_2(void *x, u64 y){
    GUI_id id = {(u64)x, y};
    return(id);
}

inline GUI_id
guid_2(void *x, void *y){
    GUI_id id = {(u64)x, (u64)y};
    return(id);
}

inline b32
guid_eq(GUI_id a, GUI_id b){
    b32 result = false;
    if (a.id[0] == b.id[0] &&
        a.id[1] == b.id[1]){
        result = true;
    }
    return(result);
}

enum{
    guicom_text_field,
    guicom_image_preview,
    guicom_deselect
};

enum{
    TextField_StaticString = 0x1,
    TextField_Selectable   = 0x2
};

struct GUI_Command{
    i32 type;
    union{
        struct{
            String *edit_str;
            u32 flags;
        } text_field;
        struct{
            i32 image_id;
        } image_preview;
    };
};

struct GUI{
    GUI_Command *commands;
    i32 count, max;
};

struct GUI_State{
    GUI_id activated;
    GUI_id selected;
    GUI_id hot;
};

inline GUI
make_gui(Partition *part, i32 max){
    GUI gui;
    gui.commands = push_array(part, GUI_Command, max);
    gui.count = 0;
    gui.max = max;
    return(gui);
}

inline b32
gui_do_command(GUI *gui, GUI_Command command){
    b32 result = false;
    if (gui->count < gui->max){
        gui->commands[gui->count++] = command;
        result = true;
    }
    return(result);
}

inline GUI_Command
gui_make_text_field(String *str, u32 flags){
    GUI_Command command = {0};
    command.type = guicom_text_field;
    command.text_field.edit_str = str;
    command.text_field.flags = flags;
    return(command);
}

inline GUI_Command
gui_make_image_preview(i32 image_id){
    GUI_Command command = {0};
    command.type = guicom_image_preview;
    command.image_preview.image_id = image_id;
    return(command);
}

inline GUI_Command
gui_make_deselect(){
    GUI_Command command = {0};
    command.type = guicom_deselect;
    return(command);
}


//
// Control Variables
//

struct Slot_Details{
    b32 initialized;
    
    char name_field_space[ASSET_MAX_NAME];
    String name_field;
    
    char type_name_space[ASSET_MAX_NAME];
    String type_name;
    
    GUI_State state;
};

struct Editor_Asset_State{
    b32 collapsed;
};

struct Hierarchy_Vars{
    Editor_Asset_State *asset_states;
    i32 asset_count;
    
    i32 clicked_asset;
    
    i32 dragged_asset;
    f32 dragged_offset_x;
    f32 dragged_offset_y;
    i32 dragged_level;
    
    i32 right_clicked_asset;
    i32 detail_asset;
    
    f32 backup_timer;
    
    Slot_Details details;
    b32 do_type_radio_buttons;
};

//
// Tree Operations
//

struct Tree_Operation{
    i32 type;
    i32 id;
    i32 pos_id;
    char *str;
};

enum Tree_Operation_Type{
    TreeOp_RemoveSubtree,
    TreeOp_InsertLastChildSubtree,
    TreeOp_InsertFirstChildSubtree,
    TreeOp_InsertSiblingSubtreeBefore,
    TreeOp_InsertSiblingSubtreeAfter,
    TreeOp_NewNodeBefore,
    TreeOp_NewNodeAfter
};


inline Tree_Operation
make_op(i32 type, i32 id, i32 pos_id){
    Tree_Operation op = {type, id, pos_id, 0};
    return(op);
}

inline Tree_Operation
make_op(i32 type, i32 id, i32 pos_id, char *str){
    Tree_Operation op = {type, id, pos_id, str};
    return(op);
}

#endif

// BOTTOM

