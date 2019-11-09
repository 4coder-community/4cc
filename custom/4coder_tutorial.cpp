/*
4coder_tutorial.cpp - Guided graphical tutorial system.
*/

// TOP

global Tutorial_State tutorial = {};

CUSTOM_COMMAND_SIG(kill_tutorial)
CUSTOM_DOC("If there is an active tutorial, kill it.")
{
    if (!tutorial.in_tutorial){
        return;
    }
    
    tutorial.in_tutorial = false;
    view_close(app, tutorial.view);
}

function void
tutorial_activate(Application_Links *app){
    if (!tutorial.in_tutorial){
        return;
    }
    
    Panel_ID panel = view_get_panel(app, tutorial.view);
    Panel_ID parent = panel_get_parent(app, panel);
    panel_set_split(app, parent, PanelSplitKind_Ratio_Min, 0.5f);
    
    tutorial.is_active = true;
}

function void
tutorial_deactivate(Application_Links *app){
    if (!tutorial.in_tutorial){
        return;
    }
    
    Face_ID face = get_face_id(app, 0);
    Face_Metrics metrics = get_face_metrics(app, face);
    f32 line_height = metrics.line_height;
    
    Panel_ID panel = view_get_panel(app, tutorial.view);
    Panel_ID parent = panel_get_parent(app, panel);
    panel_set_split(app, parent, PanelSplitKind_FixedPixels_Min, line_height*4.f);
    
    tutorial.is_active = false;
}

function void
tutorial_action(Application_Links *app, Tutorial_Action action){
    switch (action){
        case TutorialAction_Prev:
        {
            tutorial.slide_index -= 1;
        }break;
        
        case TutorialAction_Next:
        {
            tutorial.slide_index += 1;
        }break;
        
        case TutorialAction_Exit:
        {
            kill_tutorial(app);
        }break;
        
        case TutorialAction_Restart:
        {
            tutorial.slide_index = 0;
        }break;
    }
}

function void
tutorial_init_title_face(Application_Links *app){
    if (tutorial.face == 0){
        Face_ID face = get_face_id(app, 0);
        Face_Description face_description = get_face_description(app, face);
        face_description.parameters.pt_size *= 2;
        tutorial.face = try_create_new_face(app, &face_description);
        if (tutorial.face == 0){
            tutorial.face = face;
        }
    }
}

function void
tutorial_render(Application_Links *app, Frame_Info frame_info, View_ID view_id){
    Face_ID face = get_face_id(app, 0);
    tutorial_init_title_face(app);
    Face_Metrics metrics = get_face_metrics(app, face);
    Face_Metrics tut_metrics = get_face_metrics(app, tutorial.face);
    
    ////////
    
    Scratch_Block scratch(app);
    tutorial.slide_index = clamp(0, tutorial.slide_index, tutorial.slide_count - 1);
    Tutorial_Slide_Function *slide_func = tutorial.slide_func_ptrs[tutorial.slide_index];
    Tutorial_Slide slide = slide_func(app, scratch);
    
    ////////
    
    f32 h0 = get_fancy_line_height(app, 0, &slide.short_details);
    f32 h1 = get_fancy_line_height(app, 0, slide.long_details.first);
    f32 title_height = max(h0, h1);
    
    ////////
    
    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = (active_view == view_id);
    
    FColor margin_color = get_margin_color(is_active_view?UIHighlight_Active:UIHighlight_None);
    Rect_f32 region = draw_background_and_margin(app, view_id, margin_color, margin_color);
    Rect_f32 prev_clip = draw_set_clip(app, region);
    
    f32 panel_y0 = region.y0 - 3.f;
    
    region = rect_inner(region, 3.f);
    draw_rectangle(app, region, 20.f, fcolor_id(Stag_Back));
    region = rect_inner(region, 10.f);
    
    Vec2_f32 title_p = V2f32(region.x0, panel_y0 + (metrics.line_height*2.f) - title_height*0.5f);
    
    if (is_active_view){
        if (!tutorial.is_active){
            view_enqueue_command_function(app, view_id, tutorial_activate);
            }
    }
    else{
        if (tutorial.is_active){
            view_enqueue_command_function(app, view_id, tutorial_deactivate);
        }
    }
    
    tutorial.hover_action = TutorialAction_None;
    if (tutorial.is_active){
        draw_fancy_block(app, 0, fcolor_zero(), &slide.long_details, title_p);
        
        // NOTE(allen): buttons
        Rect_f32_Pair footer_pair = rect_split_top_bottom_neg(region, metrics.line_height*2.f);
        Rect_f32 footer = footer_pair.max;
        footer.x0 += 10.f;
        footer.y0 -= 10.f;
        footer.y1 -= 10.f;
        
        f32 b_width = metrics.normal_advance*10.f;
        Mouse_State mouse = get_mouse_state(app);
        Vec2_f32 m_p = V2f32(mouse.p);
        
        {
            Rect_f32_Pair pair = rect_split_left_right(footer, b_width);
            footer = pair.max;
            footer.x0 += 10.f;
        if (tutorial.slide_index > 0){
            if (draw_button(app, pair.min, m_p, face, string_u8_litexpr("prev"))){
                tutorial.hover_action = TutorialAction_Prev;
            }
        }
}
        
{
            Rect_f32_Pair pair = rect_split_left_right(footer, b_width);
            footer = pair.max;
            footer.x0 += 10.f;
        if (tutorial.slide_index < tutorial.slide_count - 1){
            if (draw_button(app, pair.min, m_p, face, string_u8_litexpr("next"))){
                tutorial.hover_action = TutorialAction_Next;
            }
        }
        }

{
            Rect_f32_Pair pair = rect_split_left_right(footer, b_width);
            footer = pair.max;
            footer.x0 += 10.f;
            Rect_f32 exit_box = pair.min;
            pair = rect_split_left_right(footer, b_width);
            Rect_f32 restart_box = pair.min;
            
        if (tutorial.slide_index == tutorial.slide_count - 1){
            if (draw_button(app, exit_box, m_p, face, string_u8_litexpr("end"))){
                tutorial.hover_action = TutorialAction_Exit;
            }
            
                if (draw_button(app, restart_box, m_p, face, string_u8_litexpr("restart"))){
                tutorial.hover_action = TutorialAction_Restart;
            }
        }
}
    }
    else{
        draw_fancy_line(app, 0, fcolor_zero(), &slide.short_details, title_p);
    }
    
    draw_set_clip(app, prev_clip);
}

function void
tutorial_run_loop(Application_Links *app)
{
    View_ID view = get_this_ctx_view(app, Access_Always);
    View_Context ctx = view_current_context(app, view);
    ctx.render_caller = tutorial_render;
    ctx.hides_buffer = true;
    View_Context_Block ctx_block(app, view, &ctx);
    
    tutorial.in_tutorial = true;
    tutorial.view = view;
    
    for (;;){
        User_Input in = get_next_input(app, EventPropertyGroup_Any, 0);
        if (in.abort){
            break;
        }
        
    b32 handled = true;
        switch (in.event.kind){
            case InputEventKind_MouseButton:
            {
                if (in.event.mouse.code == MouseCode_Left){
                    tutorial.depressed_action = tutorial.hover_action;
                }
            }break;
            
            case InputEventKind_MouseButtonRelease:
            {
                if (in.event.mouse.code == MouseCode_Left){
                    if (tutorial.depressed_action == tutorial.hover_action){
                        tutorial_action(app, tutorial.depressed_action);
                    }
                }
            }break;
            
            case InputEventKind_Core:
            {
                switch (in.event.core.code){
                    case CoreCode_ClickActivateView:
                    {
                        tutorial_activate(app);
                    }break;
                    
                    case CoreCode_ClickDeactivateView:
                    {
                        tutorial_deactivate(app);
                    }break;
                    
                    default:
                    {
                        handled = false;
                    }break;
                }
            }break;
            
            default:
            {
                handled = false;
            }break;
        }

        if (!handled){
            Mapping *mapping = ctx.mapping;
            Command_Map *map = mapping_get_map(mapping, ctx.map_id);
            
            Fallback_Dispatch_Result disp_result =
                fallback_command_dispatch(app, mapping, map, &in);
            if (disp_result.code == FallbackDispatch_DelayedUICall){
                call_after_ctx_shutdown(app, view, disp_result.func);
                break;
            }
            if (disp_result.code == FallbackDispatch_Unhandled){
                leave_current_input_unhandled(app);
            }
        }
    }
    
    tutorial.in_tutorial = false;
}

function void
run_tutorial(Application_Links *app, Tutorial_Slide_Function **slides, i32 slide_count){
    if (slide_count > 0){
        kill_tutorial(app);
        Panel_ID root_panel = panel_get_root(app);
        if (panel_split(app, root_panel, Dimension_Y)){
            panel_swap_children(app, root_panel);
            Panel_ID tutorial_panel = panel_get_child(app, root_panel, Side_Min);
            tutorial.view = panel_get_view(app, tutorial_panel, Access_Always);
            view_set_passive(app, tutorial.view, true);
            tutorial_activate(app);
            tutorial.slide_index = 0;
            tutorial.slide_func_ptrs = slides;
            tutorial.slide_count = slide_count;
            view_enqueue_command_function(app, tutorial.view, tutorial_run_loop);
        }
    }
}

////////////////////////////////

global String_Const_u8 hms_title = string_u8_litexpr("Handmade Seattle Demo");

function void
hms_demo_tutorial_short_details(Application_Links *app, Arena *arena, Fancy_Line *short_details){
    Face_ID face = get_face_id(app, 0);
    push_fancy_string(arena, short_details, tutorial.face, fcolor_id(Stag_Pop1), hms_title);
    push_fancy_string(arena, short_details, face, fcolor_id(Stag_Default), 8.f, 8.f, string_u8_litexpr("Welcome to Handmade Seattle and to this 4coder demo!"));
    push_fancy_string(arena, short_details, face, fcolor_id(Stag_Pop2), string_u8_litexpr("Click here to see more."));
}

function void
hms_demo_tutorial_long_start(Application_Links *app, Arena *arena, Fancy_Block *long_details){
    Fancy_Line *line = push_fancy_line(arena, long_details, tutorial.face, fcolor_id(Stag_Pop1), hms_title);
    
    Face_ID face = get_face_id(app, 0);
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
#define M "If there are any features you'd like to know about or try out, "
    push_fancy_string(arena, line, string_u8_litexpr(M));
#undef M
    push_fancy_string(arena, line, fcolor_id(Stag_Pop2), string_u8_litexpr("ask!"));
    
    //
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Pop1), string_u8_litexpr(""));
}

function void
hms_demo_tutorial_binding_line(Application_Links *app, Arena *arena, Fancy_Block *long_details, Face_ID face, char *modifiers, char *key, char *description){
    String_Const_u8 m = SCu8(modifiers);
    String_Const_u8 k = SCu8(key);
    
    f32 fill_size = (f32)k.size;
    if (m.size > 0){
        fill_size += (f32)m.size + 0.5f;
    }
    f32 pad_size = 0.f;
    if (fill_size < 40.f){
    pad_size = 40.f - fill_size;
    }
    
    Fancy_Line *line = line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, pad_size, 0.5f, "<");
    if (m.size > 0){
        push_fancy_stringf(arena, line, fcolor_id(Stag_Keyword), 0.f, 0.5f, "%s", modifiers);
    }
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2), "%s", key);
    push_fancy_stringf(arena, line, 0.5f, 1.f, ">");
    push_fancy_stringf(arena, line, "%s", description);
}

function Tutorial_Slide
hms_demo_tutorial_slide_1(Application_Links *app, Arena *arena){
    Tutorial_Slide result = {};
    
    Face_ID face = get_face_id(app, 0);
    tutorial_init_title_face(app);
    
    hms_demo_tutorial_short_details(app, arena, &result.short_details);
    
    Fancy_Block *long_details = &result.long_details;
    hms_demo_tutorial_long_start(app, arena, long_details);
    
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Pop1), string_u8_litexpr("Let's start with a few navigation commands:"));
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Control", "Comma", "change active panel");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "", "*AnyArrow*", "move cursor one character or line");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Control", "*AnyArrow*", "move cursor by 'chunks'");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "", "Home/End", "move cursor to the first/last character of the line");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "", "PageUp/PageDown", "move cursor by full pages up/down");
    
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Pop1), string_u8_litexpr("Available in code files:"));
    
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Alt", "LeftBracket", "move cursor and mark to surrounding scope");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Alt", "RightBracket", "move cursor and mark to previous scope");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Alt", "Quote", "move cursor and mark to next scope");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Alt Shift", "RightBracket", "move cursor and mark to previous top-level scope");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Alt Shift", "Quote", "move cursor and mark to next scope at the same level as the current scope");
    
    return(result);
}

function Tutorial_Slide
hms_demo_tutorial_slide_2(Application_Links *app, Arena *arena){
    Tutorial_Slide result = {};
    
    Face_ID face = get_face_id(app, 0);
    tutorial_init_title_face(app);
    
    hms_demo_tutorial_short_details(app, arena, &result.short_details);
    
    Fancy_Block *long_details = &result.long_details;
    hms_demo_tutorial_long_start(app, arena, long_details);
    
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Pop1), string_u8_litexpr("Now a look at basic editing:"));
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "", "TextInsert", "non-modal text insertion works in any user-writable buffers at the cursor");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "", "Backspace/Delete", "delete the previous/next character from the cursor");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Control", "Z", "undo the last edit");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Control", "Y", "redo the last undone edit");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Control", "L", "duplicate the current line");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Alt", "Up/Down", "move the current line");
    
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Pop1), string_u8_litexpr("Range commands based on a cursor and mark (emacs style):"));
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Control", "Space", "moves the mark to the cursor cursor");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Control", "D", "delete the range");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Control", "C", "copy the range");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Control", "X", "cut the range");
    
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Pop1), string_u8_litexpr("Paste options with a multi-stage clipboard:"));
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Control", "V", "paste the clipboard to the buffer");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Control Shift", "V", "paste the clipboard to the buffer cycling through the clipboard's 'clips'");
    
    return(result);
}

function Tutorial_Slide
hms_demo_tutorial_slide_3(Application_Links *app, Arena *arena){
    Tutorial_Slide result = {};
    
    Face_ID face = get_face_id(app, 0);
    tutorial_init_title_face(app);
    
    hms_demo_tutorial_short_details(app, arena, &result.short_details);
    
    Fancy_Block *long_details = &result.long_details;
    hms_demo_tutorial_long_start(app, arena, long_details);
    
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Pop1), string_u8_litexpr("Now try beginning a file lister:"));
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Control", "N", "begin a file lister for exploring the file system - always creating a new file");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Control", "O", "begin a file lister for exploring the file system - ultimiately opening or creating a file");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Alt", "O", "same as previous option but opens in the other panel");
    
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Pop1), string_u8_litexpr("Inside a file lister:"));
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "", "TextInsert", "narrows the lister down to options with substrings matching the text field");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "", "Backspace", "backspace the end of the text field");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "", "Up/Down", "move the highlighted option up/down");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "", "PageUp/PageDown", "move the highlighted option up/down by a 'large chunk'");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "", "Return/Tab", "select the highlighted option;  when a folder is highlighted it is opened in the lister");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "", "Escape", "cancel the operation");
    
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Pop1), string_u8_litexpr("There are also buffer listers for operations on buffers that are already open:"));
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Control", "I", "begin a buffer lister and switch to the selected buffer");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Control", "K", "begin a buffer lister and try to kill the selected buffer");
    
    return(result);
}

function Tutorial_Slide
hms_demo_tutorial_slide_4(Application_Links *app, Arena *arena){
    Tutorial_Slide result = {};
    
    Face_ID face = get_face_id(app, 0);
    tutorial_init_title_face(app);
    
    hms_demo_tutorial_short_details(app, arena, &result.short_details);
    
    Fancy_Block *long_details = &result.long_details;
    hms_demo_tutorial_long_start(app, arena, long_details);
    
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Pop1), string_u8_litexpr("The command lister makes all commands available in one place:"));
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Alt", "X", "a lister of all commands");
    
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Pop1), string_u8_litexpr("Try some of these commands from the command lister:"));
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "", "toggle_filebar", "toggle the panel's filebar");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "", "toggle_line_numbers", "toggle the panel's line number");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "", "set_eol_mode_*", "change how a buffer prefers to fixup it's line endings");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "", "close_panel", "close the current panel");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "", "open_panel_vsplit", "create a vertical split in the current panel");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "", "open_panel_hsplit", "create a horizontal split in the current panel");
    
    return(result);
}

function Tutorial_Slide
hms_demo_tutorial_slide_5(Application_Links *app, Arena *arena){
    Tutorial_Slide result = {};
    
    Face_ID face = get_face_id(app, 0);
    tutorial_init_title_face(app);
    
    hms_demo_tutorial_short_details(app, arena, &result.short_details);
    
    Fancy_Block *long_details = &result.long_details;
    hms_demo_tutorial_long_start(app, arena, long_details);
    
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Pop1), string_u8_litexpr("Fast navigation by jump lists:"));
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Alt", "F", "after the user answers a query for a string generate a jump list of matches");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Control Shift", "T", "extract an identifier from the text under cursor and generate a jump list of matches");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Control Shift", "I", "parse the current buffer as a C/C++ source and generate a jump list of functions");
    
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Pop1), string_u8_litexpr("After generating a jump list it is bound as the active jump list enabling these commands:"));
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Alt", "N", "jump to the next jump location");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Alt Shift", "N", "jump to the previous jump location");
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Alt Shift", "M", "jump to the first jump");
    
    return(result);
}

function Tutorial_Slide
hms_demo_tutorial_slide_6(Application_Links *app, Arena *arena){
    Tutorial_Slide result = {};
    
    Face_ID face = get_face_id(app, 0);
    tutorial_init_title_face(app);
    
    hms_demo_tutorial_short_details(app, arena, &result.short_details);
    
    Fancy_Block *long_details = &result.long_details;
    hms_demo_tutorial_long_start(app, arena, long_details);
    
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Pop1), string_u8_litexpr("Virtual whitespace:"));
    
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default),
                    string_u8_litexpr("\tBuffers that are indexed with information about nest structures can be equiped with the virtual whitespace layout algorithm."));
    
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default),
                    string_u8_litexpr("\tTry inserting new scopes and parenthetical sections in a code file."));
    
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default),
                    string_u8_litexpr("\tObserve that indentation is updated automatically."));
    
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default),
                    string_u8_litexpr("\tTry creating a line that is long enough to wrap around the edge."));
    
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default),
                    string_u8_litexpr("\tObserve that wrapped lines obey the same indentation rules as literal lines."));
    
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default),
                    string_u8_litexpr("\tThe on screen layout of text is independent of the actual whitespace contents of the underlying text."));
    
    {
        Fancy_Line *line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
        push_fancy_stringf(arena, line, "\tUse the command ");
        push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2), "toggle_virtual_whitespace");
        push_fancy_stringf(arena, line, " to turn this feature on and off");
    }
    
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Pop1), string_u8_litexpr("Auto Indentation:"));
    
    hms_demo_tutorial_binding_line(app, arena, long_details, face,
                                   "Control", "Tab", "auto indent the lines marked by the cursor-mark range; this command only observable when virtual whitespace is off.");
    
    return(result);
}

CUSTOM_COMMAND_SIG(hms_demo_tutorial)
CUSTOM_DOC("Tutorial for built in 4coder bindings and features.")
{
    local_persist Tutorial_Slide_Function *slides[] = {
        // basic navigation
        hms_demo_tutorial_slide_1,
        // basic editing
        hms_demo_tutorial_slide_2,
        // file and buffer lister
        hms_demo_tutorial_slide_3,
        // command lister
        hms_demo_tutorial_slide_4,
        // advanced navigation
        hms_demo_tutorial_slide_5,
        // virtual whitespace
        hms_demo_tutorial_slide_6,
    };
    run_tutorial(app, slides, ArrayCount(slides));
}

// BOTTOM
