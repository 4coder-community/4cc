/*
 * 4tech experiments
 *   by Allen Webster
 * 20.08.2015 (dd.mm.yyyy)
 *
 */

// TOP

global Vec4 black = {0.f, 0.f, 0.f, 1.f};
global Vec4 white = {1.f, 1.f, 1.f, 1.f};
global Vec4 gray = {0.5f, 0.5f, 0.5f, 1.f};
global Vec4 red = {1.f, 0.f, 0.f, 1.f};

struct real32_Rect{
    real32 x0,y0,x1,y1;
};

inline real32_Rect
rect_around_point(Vec2 p, real32 r){
    real32_Rect result;
    result.x0 = p.x - r;
    result.x1 = p.x + r;
    result.y0 = p.y - r;
    result.y1 = p.y + r;
    return result;
}

internal void
mygl_clear(Vec4 color){
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

internal void
mygl_color(Vec4 color){
    glColor4f(color.r, color.g, color.b, color.a);
}

internal void
mygl_rectangle(real32_Rect rect, Vec4 color){
    mygl_color(color);
    glBegin(GL_QUADS);
    {
        glVertex2f(rect.x0, rect.y0);
        glVertex2f(rect.x1, rect.y0);
        glVertex2f(rect.x1, rect.y1);
        glVertex2f(rect.x0, rect.y1);
    }
    glEnd();
}

internal void
mygl_rectangle_h_grad(real32_Rect rect, Vec4 left, Vec4 right){
    glBegin(GL_QUADS);
    {
        mygl_color(left);
        glVertex2f(rect.x0, rect.y0);
        glVertex2f(rect.x1, rect.y0);

        mygl_color(right);
        glVertex2f(rect.x1, rect.y1);
        glVertex2f(rect.x0, rect.y1);
    }
    glEnd();
}

internal void
mygl_polygon_outline(Vec2 *vertices, i32 vertex_count, Vec4 color){
    mygl_color(color);
    glBegin(GL_LINE_LOOP);
    {
        for (i32 i = 0; i < vertex_count; ++i){
            glVertex2f(vertices[i].x, vertices[i].y);
        }
    }
    glEnd();
}

internal void
mygl_line(Vec2 a, Vec2 b, Vec4 color){
    mygl_color(color);
    glBegin(GL_LINES);
    {
        glVertex2f(a.x, a.y);
        glVertex2f(b.x, b.y);
    }
    glEnd();
}

struct Skeleton_Draw_Point{
    i32 id;
    i32 next_child;
};

internal void
mygl_skeleton(Tree_Skeleton_Vert *vertices, i32 count, i32 highlight_index){
    Vec4 highlight_color = {0.f, 0.f, 1.f, 1.f};
    Skeleton_Draw_Point stack[16];
    i32 top = 0;
    stack[top++] = {0, 0};
    glBegin(GL_LINES);
    while (top > 0){
        Skeleton_Draw_Point *pt = stack + top - 1;
        Tree_Skeleton_Vert *vert = vertices + pt->id;

        bool32 done = 0;
        if (pt->next_child >= 3){
            done = 1;
        }
        else{
            i32 child = vert->children[pt->next_child];
            if (child){
                Tree_Skeleton_Vert *child_vert = vertices + child;
                if (child_vert->paint.r == 0.f && child_vert->paint.g == 0.f &&
                        child_vert->paint.b == 0.f){
                    int x = 1; AllowLocal(x);
                }
                if (highlight_index == child){
                    mygl_color(highlight_color);
                }
                else{
                    mygl_color(child_vert->paint);
                }
                glVertex2f(vert->pos.x, vert->pos.y);
                glVertex2f(child_vert->pos.x, child_vert->pos.y);
                ++pt->next_child;
                stack[top++] = {child, 0};
            }
            else{
                done = 1;
            }
        }

        if (done){
            --top;
        }
    }
    glEnd();
}

internal void
mygl_branch_info(real32 x, real32 y, Branch_Info info){
    Vec4 color = {.6f, 0.f, .4f, 1.f};
    Vec2 a, b;
    real32 x_step = 3.f;
    i32 i;

    a.y = y;

    a.x = b.x = x;
    b.y = y + info.length;
    mygl_line(a, b, color);

    x += x_step;
    a.x = b.x = x;
    b.y = y + info.step;
    mygl_line(a, b, color);

    for (i = 0; i < ArrayCount(info.contribution_list); ++i){
        x += x_step;
        a.x = b.x = x;
        b.y = y + info.contribution_list[i];
        mygl_line(a, b, color);
    }
}

struct Super_Color{
    Vec4 hsla;
    Vec4 rgba;
    u32 packed;
};

struct Memory_Part{
    char *base;
    i32 pos, max;
};

internal void*
mempart_get(Memory_Part *part, i32 size){
    _Assert(part->pos + size < part->max);
    void *result = part->base + part->pos;
    part->pos += size;
    return result;
}

#define mempart_get_array(part,T,size) (T*)mempart_get(part, sizeof(T)*size);

struct Temp_Memory{
    Memory_Part *part;
    i32 start_pos;
};

internal Temp_Memory
tempmem_begin(Memory_Part *part){
    Temp_Memory temp;
    temp.part = part;
    temp.start_pos = part->pos;
    return temp;
}

internal void
tempmem_end(Temp_Memory temp){
    temp.part->pos = temp.start_pos;
}

struct App_Vars{
    bool32 initialized;
    Memory_Part part;
    Tree_Parameters params;
    i32 hot_spot_max;
    bool32 show_skeleton;
    bool32 show_ghost_skeleton;
    bool32 show_tree;
    bool32 show_hot_spots;

    i32 skeleton_limit;
    i32 p_moving;
    Hot_Spot *p;

    i32 inspecto_branch;
};

internal void
show_vec_stats(Vec4 v, real32 x, real32 y){
    real32_Rect h,s,l,f;
    h = {x, y, 10.f + x, y};
    s = h;
    s.x0 += 20.f;
    s.x1 += 20.f;
    l = s;
    l.x0 += 20.f;
    l.x1 += 20.f;

    h.y1 += 100.f*v.x;
    s.y1 += 100.f*v.y;
    l.y1 += 100.f*v.z;

    f.x0 = h.x0 - 5.f;
    f.x1 = l.x1 + 5.f;
    f.y0 = h.y0 - 5.f;
    f.y1 = h.y0 + 105.f;

    mygl_rectangle(f, gray);
    mygl_rectangle(h, white);
    mygl_rectangle(s, white);
    mygl_rectangle(l, white);
}

internal bool32
hit_check(i32 x, i32 y, real32_Rect r){
    return (x >= r.x0 && x < r.x1 && y >= r.y0 && y < r.y1);
}

internal void
do_adjuster(i32 id, Mouse_State *mouse, real32 *v, real32 low, real32 high, real32_Rect rect, bool32 *mouse_consumed){
    mygl_rectangle(rect, V4(.3f, .3f, .3f, 1.f));

    real32 x, scale;
    if (mouse->l && hit_check(mouse->x, mouse->y, rect)){
        x = (real32)mouse->x;
        scale = (x - rect.x0) / (rect.x1 - rect.x0);
        *v = lerp(low, scale, high);
        *mouse_consumed = 1;
    }
    else{
        scale = (*v - low) / (high - low);
        x = lerp(rect.x0, scale, rect.x1);
    }

    rect.x0 = x - 2.f;
    rect.x1 = x + 3.f;
    mygl_rectangle(rect, V4(.6f, .6f, .6f, 1.f));
}

internal void
do_adjuster(i32 id, Mouse_State *mouse, i32 *v, i32 low, i32 high, real32_Rect rect, bool32 *mouse_consumed){
    mygl_rectangle(rect, V4(.3f, .3f, .3f, 1.f));

    real32 x, scale;
    if (mouse->l && hit_check(mouse->x, mouse->y, rect)){
        x = (real32)mouse->x;
        scale = (x - rect.x0) / (rect.x1 - rect.x0);
        *v = FLOOR32(lerp((real32)low, scale, (real32)high + 1));
        if (*v > high) *v = high;
        *mouse_consumed = 1;
    }

    scale = (real32)(*v - low) / (real32)(high - low);
    x = lerp(rect.x0, scale, rect.x1);

    rect.x0 = x - 2.f;
    rect.x1 = x + 3.f;
    mygl_rectangle(rect, V4(.6f, .6f, .6f, 1.f));
}

internal bool32
do_button(i32 id, Mouse_State *mouse, bool32 active, real32_Rect rect, bool32 *mouse_consumed){
    bool32 result = 0;
    if (active) mygl_rectangle(rect, V4(.6f, .6f, .6f, 1.f));
    else mygl_rectangle(rect, V4(.3f, .3f, .3f, 1.f));
    if (mouse->pl && hit_check(mouse->x, mouse->y, rect)){
        result = 1;
        *mouse_consumed = 1;
    }
    return result;
}

internal bool32
do_draggable(i32 id, Mouse_State *mouse, bool32 active, real32_Rect rect, bool32 *mouse_consumed){
    bool32 result = 0;
    if (!*mouse_consumed){
        if (active) mygl_rectangle(rect, V4(.6f, .6f, .6f, 1.f));
        else mygl_rectangle(rect, V4(.3f, .3f, .3f, 1.f));
        if (mouse->l && hit_check(mouse->x, mouse->y, rect)){
            result = 1;
            *mouse_consumed = 1;
        }
    }
    return result;
}

internal bool32
do_right_click(i32 id, Mouse_State *mouse, real32_Rect rect, bool32 *mouse_consumed){
    bool32 result = 0;
    if (!*mouse_consumed){
        if (mouse->pr && hit_check(mouse->x, mouse->y, rect)){
            result = 1;
            *mouse_consumed = 1;
        }
    }
    return result;
}

APP_STEP_SIG(app_step){
    _Assert(sizeof(App_Vars) < step->memory.size);
    App_Vars *vars = (App_Vars*)step->memory.mem;
    if (!vars->initialized){
        vars->initialized = 1;
        vars->part.base = (char*)step->memory.mem;
        vars->part.max = step->memory.size;
        vars->part.pos = sizeof(App_Vars);

        Tree_Parameters params;
        params.base = V2(500.f, 590.f);
        params.levels = 6;
        params.initial_step = 70.f;
        params.step_decay = .8f;
        params.initial_thickness = 10.f;
        params.thickness_decay = .75f;
        params.dtheta = 72.f;
        params.min_weight_threshold = 1.f;
        params.center_branch_weight = 1.f;
        params.heat_half_distance = 30.f;

        vars->hot_spot_max = 10;
        params.hot_spot_count = 3;
        params.hot_spots = mempart_get_array(&vars->part, Hot_Spot, vars->hot_spot_max);

        params.hot_spots[0].spot.x = 500.f;
        params.hot_spots[0].spot.y = 100.f;

        params.hot_spots[1].spot.x = 850.f;
        params.hot_spots[1].spot.y = 400.f;

        params.hot_spots[2].spot.x = 300.f;
        params.hot_spots[2].spot.y = 150.f;

        vars->params = params;

        vars->show_skeleton = 1;
        vars->show_tree = 1;
        vars->show_hot_spots = 1;

        vars->p_moving = -1;
        vars->p = 0;

        vars->skeleton_limit = 0;
    }

    mygl_clear(black);

    Mouse_State *mouse = &step->mouse;
    bool32 mouse_consumed = 0;
    i32 id = 1;
    real32_Rect rect;
    real32 low_point;

    Hot_Spot *hot_spot, *hot_spot_array;
    i32 ind, i;
    hot_spot_array = vars->params.hot_spots;
    hot_spot = hot_spot_array;
    for (i = 0; i < vars->params.hot_spot_count; ++i, ++hot_spot){
        ind = i + (1 << 16);

        rect = rect_around_point(hot_spot->spot, 4);
        if (do_draggable(ind, mouse, (ind == vars->p_moving), rect, &mouse_consumed)){
            vars->p_moving = ind;
            vars->p = hot_spot;
        }

        if (do_right_click(ind, mouse, rect, &mouse_consumed)){
            vars->p_moving = -1;
            --vars->params.hot_spot_count;
            *hot_spot = hot_spot_array[vars->params.hot_spot_count];
            break;
        }
    }

    if (!mouse_consumed && mouse->pr){
        if (vars->params.hot_spot_count < vars->hot_spot_max){
            hot_spot_array[vars->params.hot_spot_count++].spot = V2((f32)mouse->x, (f32)mouse->y);
        }
    }
    // TODO(allen): If the mouse isn't consumed yet, and it's a right click, add a point

    if (vars->p_moving != -1 && mouse->l){
        Hot_Spot *p = vars->p;
        if (p != 0){
            p->spot.x = (real32)mouse->x;
            p->spot.y = (real32)mouse->y;
        }
    }

    if (!mouse->l){
        vars->p_moving = -1;
    }

    rect.x0 = 10.f;
    rect.x1 = 210.f;
    rect.y0 = 10.f;
    rect.y1 = 30.f;
    do_adjuster(id++, mouse, &vars->params.initial_step, 0.f, 200.f, rect, &mouse_consumed);
    rect.y0 += 30.f;
    rect.y1 += 30.f;
    do_adjuster(id++, mouse, &vars->params.step_decay, 0.f, 1.f, rect, &mouse_consumed);
    rect.y0 += 30.f;
    rect.y1 += 30.f;
    do_adjuster(id++, mouse, &vars->params.initial_thickness, 1.f, 50.f, rect, &mouse_consumed);
    rect.y0 += 30.f;
    rect.y1 += 30.f;
    do_adjuster(id++, mouse, &vars->params.thickness_decay, 0.f, 1.f, rect, &mouse_consumed);
    rect.y0 += 30.f;
    rect.y1 += 30.f;
    do_adjuster(id++, mouse, &vars->params.dtheta, 90.f, 0.f, rect, &mouse_consumed);
    rect.y0 += 30.f;
    rect.y1 += 30.f;
    do_adjuster(id++, mouse, &vars->params.levels, 1, 7, rect, &mouse_consumed);
    rect.y0 += 30.f;
    rect.y1 += 30.f;
    do_adjuster(id++, mouse, &vars->params.min_weight_threshold, 1.f, 100.f, rect, &mouse_consumed);
    rect.y0 += 30.f;
    rect.y1 += 30.f;
    do_adjuster(id++, mouse, &vars->params.center_branch_weight, 0.5f, 2.f, rect, &mouse_consumed);
    rect.y0 += 30.f;
    rect.y1 += 30.f;
    do_adjuster(id++, mouse, &vars->params.heat_half_distance, 10.f, 100.f, rect, &mouse_consumed);
    rect.y0 += 30.f;
    rect.y1 += 30.f;
    do_adjuster(id++, mouse, &vars->inspecto_branch, 1, vars->skeleton_limit, rect, &mouse_consumed);
    low_point = rect.y1;

    rect.x0 = rect.x1 + 10.f;
    rect.x1 = rect.x0 + 20.f;
    rect.y0 = 10.f;
    rect.y1 = 30.f;
    if (do_button(id++, mouse, vars->show_skeleton, rect, &mouse_consumed)){
        vars->show_skeleton = !vars->show_skeleton;
    }
    rect.y0 += 30.f;
    rect.y1 += 30.f;
    if (do_button(id++, mouse, vars->show_ghost_skeleton, rect, &mouse_consumed)){
        vars->show_ghost_skeleton = !vars->show_ghost_skeleton;
    }
    rect.y0 += 30.f;
    rect.y1 += 30.f;
    if (do_button(id++, mouse, vars->show_tree, rect, &mouse_consumed)){
        vars->show_tree = !vars->show_tree;
    }
    rect.y0 += 30.f;
    rect.y1 += 30.f;
    if (do_button(id++, mouse, vars->show_hot_spots, rect, &mouse_consumed)){
        vars->show_hot_spots = !vars->show_hot_spots;
    }
    rect.y0 += 30.f;
    rect.y1 += 30.f;
    if (do_button(id++, mouse, 0, rect, &mouse_consumed)){
        vars->skeleton_limit = 0;
    }
    if (low_point < rect.y1) low_point = rect.y1;

    Temp_Memory temp;
    i32 init_skeleton_count, skeleton_count;
    Tree_Skeleton_Vert *skeleton;
    Branch_Info branch_info;
    i32 natural_limit;

    temp = tempmem_begin(&vars->part);
    init_skeleton_count = 2260;
    skeleton = mempart_get_array(&vars->part, Tree_Skeleton_Vert, init_skeleton_count);
    memset(skeleton, 0, sizeof(Tree_Skeleton_Vert)*init_skeleton_count);
    ++vars->skeleton_limit;
    branch_info = {};
    branch_info.which_branch = vars->inspecto_branch;
    natural_limit =
        tree_skeleton_gen(skeleton, init_skeleton_count, &skeleton_count, vars->params, vars->skeleton_limit, 0,
                                       &branch_info, 1);
    if (vars->skeleton_limit > natural_limit) vars->skeleton_limit = natural_limit;

    Tree_Skeleton_Vert *ghost_skeleton = 0;
    i32 ghost_skeleton_count = 0;
    if (vars->show_ghost_skeleton){
        ghost_skeleton = mempart_get_array(&vars->part, Tree_Skeleton_Vert, init_skeleton_count);
        memset(ghost_skeleton, 0, sizeof(Tree_Skeleton_Vert)*init_skeleton_count);
        tree_skeleton_gen(ghost_skeleton, init_skeleton_count, &ghost_skeleton_count,
            vars->params, vars->skeleton_limit, 1, 0, 0);
    }

    i32 init_vertex_count = 2260, vertex_count;
    Vec2 *vertices = mempart_get_array(&vars->part, Vec2, init_vertex_count);
    tree_gen(vertices, init_vertex_count, &vertex_count, vars->params, skeleton);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (vars->show_ghost_skeleton)
        mygl_skeleton(ghost_skeleton, ghost_skeleton_count, -1);

    if (vars->show_skeleton)
        mygl_skeleton(skeleton, skeleton_count, vars->inspecto_branch);

    if (vars->show_tree)
        mygl_polygon_outline(vertices, vertex_count, brown);

    if (vars->show_hot_spots){
        for (i32 i = 0; i < vars->params.hot_spot_count; ++i){
            mygl_rectangle(rect_around_point(vars->params.hot_spots[i].spot, 2), V4(1.f, 1.f, 0.f, 1.f));
        }
    }

    mygl_branch_info(5.f, low_point + 5.f, branch_info);

    tempmem_end(temp);

    out->mouse_cursor = APP_MOUSE_CURSOR_ARROW;
}

// BOTTOM

