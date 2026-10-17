

CUSTOM_COMMAND_SIG(open_dropped_files)
CUSTOM_DOC("Opens new bufers to all files dropped from another application")
{
    User_Input input = get_current_input(app);
    for (View_ID cur_view = get_view_next(app,0,Access_Always); cur_view != 0; cur_view = get_view_next(app,cur_view,Access_Always)) {
        Vec2_f32 p = V2f32(get_mouse_state(app).p);
        Rect_f32 view_rect = view_get_screen_rect(app, cur_view);
        if (rect_contains_point(view_rect, p)) {
            List_String_Const_u8 files = input.event.core.dropfiles;
            if (files.first) {
                view_open_file(app, cur_view, files.first->string, true);
            }
            for (Node_String_Const_u8 *file = files.first->next; file != NULL; file = file->next) {
                Buffer_ID out_buffer; // we don't care about this 
                open_file(app, &out_buffer, file->string, false, true);
            }
        }
    }
}