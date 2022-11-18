/*
 * Allen's Fancy Tree Generator
 * 22.08.2015 (dd.mm.yyyy)
 */

// TOP

internal 
split_point(real32 base_radius, real32 branch_radius, i32 branch_n, i32 i){
    real32 branch_dist = branch_radius*2.f;
    Vec2 result = {};
    switch (branch_n){
    case 1:
    {
        if (i == 0) result.x = -base_radius;
        else result.x = base_radius;
    }break;

    case 2:
    {
        real32 a,b,c;
        a = base_radius;
        c = branch_dist;
        a *= a;
        c *= c;
        b = c - a;
        b = SQRT(b);
        switch (i){
        case 0: result.x = -base_radius; result.y = b*.5f; break;
        case 1: result.x = 0; result.y = -b*.5f; break;
        case 2: result.x = base_radius; result.y = b*.5f; break;
        }
    }break;

    case 3:
    {
        real32 a,b,c,aa,cc;
        a = base_radius - branch_dist*.5f;
        c = branch_dist;
        aa = a*a;
        cc = c*c;
        b = cc - aa;
        b = SQRT(b);
        switch (i){
        case 0: result.x = -base_radius; result.y = b*.5f; break;
        case 1: result.x = a-base_radius; result.y = -b*.5f; break;
        case 2: result.x = base_radius-a; result.y = -b*.5f; break;
        case 3: result.x = base_radius; result.y = b*.5f; break;
        }
    }break;
    }

    return result;
}

struct Hot_Spot{
    Vec2 spot;
};

struct Tree_Parameters{
    Vec2 base;
    i32 levels;
    real32 initial_step;
    real32 step_decay;
    real32 initial_thickness;
    real32 thickness_decay;
    real32 dtheta;
    real32 min_weight_threshold;
    real32 center_branch_weight;
    real32 heat_half_distance;

    Hot_Spot *hot_spots;
    i32 hot_spot_count;
};

struct Tree_Skeleton_Vert{
    Vec2 pos;
    i32 child_count;
    i32 children[3];
    real32 theta;
    real32 width;
    Vec4 paint;
};

struct Skeleton_Branch_Point{
    i32 vert_id;
    real32 step;
    real32 theta;
    real32 width;
    i32 depth;
};

internal Skeleton_Branch_Point
SBP(i32 id, real32 step, real32 theta, real32 width, i32 depth){
    Skeleton_Branch_Point result;
    result.vert_id = id;
    result.step = step;
    result.theta = theta;
    result.width = width;
    result.depth = depth;
    return result;
}

struct r32_Range{
    real32 start, end;
};

inline r32_Range
get_range(real32 a, real32 b){
    r32_Range result;
    if (a < b){
        result.start = a;
        result.end = b;
    }
    else{
        result.start = b;
        result.end = a;
    }
    return result;
}

persist Vec4 brown = {.5f, .3333f, 0.f, 1.f};
persist Vec4 silver = {.7f, .7f, .7f, 1.f};
persist Vec4 faded_gold = {.7f, .4f, .2f, 0.5f};
persist Vec4 sky_blue = {0.f, .7f, 1.f, 1.f};

internal bool32
segments_collide(Vec2 a1, Vec2 a2, Vec2 b1, Vec2 b2, Vec2 *p_out = 0){
    bool32 result = 1;

    Vec3 r1, r2;
    if (a2.y != a1.y){
        r1.x = (a2.y - a1.y);
        r1.y = (a1.x - a2.x);
        r1.z = (a1.x*r1.x + a1.y*r1.y);

        r2.x = (b2.y - b1.y);
        r2.y = (b1.x - b2.x);
        r2.z = (b1.x*r2.x + b1.y*r2.y);
    }
    else{
        r1.x = (b2.y - b1.y);
        r1.y = (b1.x - b2.x);
        r1.z = (b1.x*r1.x + b1.y*r1.y);

        r2.x = (a2.y - a1.y);
        r2.y = (a1.x - a2.x);
        r2.z = (a1.x*r2.x + a1.y*r2.y);
    }

    if (r1.x == 0){
        result = (a1.y == b1.y) && ((b1.x <= a1.x && a1.x < b2.x) || (a1.x <= b1.x && b1.x < a2.x));
        return result;
    }

    if (r2.x != 0){
        r2.y /= r2.x;
        r2.z /= r2.x;
        r2.y *= r1.x;
        r2.z *= r1.x;
        r2.y -= r1.y;
        r2.z -= r1.z;
        r2.x = 0;
    }

    if (r2.y == 0){
        result = (r2.z == 0) && ((b1.y <= a1.y && a1.y < b2.y) || (a1.y <= b1.y && b1.y < a2.y));
        return result;
    }

    if (r1.y != 0){
        r1.x /= r1.y;
        r1.z /= r1.y;
        r1.x *= r2.y;
        r1.z *= r2.y;
        r1.x -= r2.x;
        r1.z -= r2.z;
        r1.y = 0;
    }

    assert(!(r1.x == 0 || r2.y == 0));

    r1 *= 1.f / r1.x;
    r2 *= 1.f / r2.y;

    Vec2 cl;
    cl.x = r1.z;
    cl.y = r2.z;
    if (p_out) *p_out = cl;

    real32 epsilon = .0001f;
    real32 neg_epsilon = -epsilon;
    real32 xdif;

    xdif = a1.x - a2.x;
    if (xdif < neg_epsilon && xdif > epsilon){
        r32_Range range = get_range(a1.x, a2.x);
        if (cl.x < range.start || cl.x > range.end){
            result = 0;
        }
    }
    else{
        r32_Range range = get_range(a1.y, a2.y);
        if (cl.y < range.start || cl.y > range.end){
            result = 0;
        }
    }

    xdif = a1.x - a2.x;
    if (xdif < neg_epsilon && xdif > epsilon){
        r32_Range range = get_range(b1.x, b2.x);
        if (cl.x < range.start || cl.x > range.end){
            result = 0;
        }
    }
    else{
        r32_Range range = get_range(b1.y, b2.y);
        if (cl.y < range.start || cl.y > range.end){
            result = 0;
        }
    }

    return result;
}

internal bool32
no_collisions(Vec2 base, Vec2 new_pos, Tree_Skeleton_Vert *vertices){
    bool32 result = 1;
    i32 vert_id_stack[16];
    i32 top = 0;
    vert_id_stack[top++] = 0;

    while (top > 0){
        Tree_Skeleton_Vert *vert = vertices + vert_id_stack[--top];
        i32 count = vert->child_count;
        i32 *children = vert->children;
        Vec2 c = vert->pos;
        for (i32 i = 0; i < count; ++i){
            i32 child_id = children[i];
            Tree_Skeleton_Vert *other_vert = vertices + child_id;
            Vec2 d = other_vert->pos;
            bool32 exempt = (base.x == d.x) && (base.y == d.y);
            if (!exempt) exempt = (base.x == c.x) && (base.y == c.y);
            if (!exempt && segments_collide(base, new_pos, c, d)){
                other_vert->paint = sky_blue;
                result = 0;
                goto no_collisions_end;
            }
            else{
                vert_id_stack[top++] = child_id; 
            }
        }
    }
    no_collisions_end:
    return result;
}

struct Branch_Info{
    i32 which_branch;

    real32 step;
    real32 length;
    real32 contribution_list[3];
    i32 contribution_count;
};

enum Info_To_Post{
    Step_Info,
    Length_Info,
    Contribution_Info,
    Count_Info
};

internal void
post_branch_info(Branch_Info *info, i32 count, i32 index, Info_To_Post type, real32 x){
    i32 i;
    for (i = 0; i < count; ++i, ++info){
        if (index == info->which_branch){
            switch (type){
                case Step_Info: info->step = x; break;
                case Length_Info: info->length = x; break;
                case Contribution_Info:
                if (info->contribution_count < ArrayCount(info->contribution_list)){
                    info->contribution_list[info->contribution_count++] = x;
                }break;
            }
            break;
        }
    }
}

internal i32
tree_skeleton_gen(Tree_Skeleton_Vert *vertices, i32 vertex_count, i32 *vertex_count_out,
                               Tree_Parameters params, i32 limit, bool32 allow_colliding,
                               Branch_Info *opt_output, i32 opt_output_count){

    Skeleton_Branch_Point queue[730];
    i32 queue_wrap = ArrayCount(queue);
    i32 write_pos = 0;
    i32 read_pos = 0;
    i32 k = 0;

    vertices[k].pos = params.base;
    queue[write_pos++] =
        SBP(k, params.initial_step, 270.f, params.initial_thickness, 0);
    if (opt_output){
        post_branch_info(opt_output, opt_output_count, k, Step_Info, params.initial_step);
        post_branch_info(opt_output, opt_output_count, k, Length_Info, params.initial_step);
    }
    ++k;

    i32 I = 0;
    for (; read_pos != write_pos; ){
        Skeleton_Branch_Point *bp = queue + read_pos;
        ++read_pos;
        read_pos = read_pos % queue_wrap;

        Tree_Skeleton_Vert *vert = vertices + bp->vert_id;
        vert->theta = bp->theta;
        vert->width = bp->width;
        if (vert->paint.r == 0 && vert->paint.g == 0 && vert->paint.b == 0){
            if (allow_colliding) vert->paint = faded_gold;
            else vert->paint = silver;
        }

        i32 *index_order = 0;
        persist i32 index_order_case_1[] = {0};
        persist i32 index_order_case_3[] = {1, 0, 2};

        i32 branch_count;
        if (bp->depth == 0){
            branch_count = 1;
            index_order = index_order_case_1;
        }else if (bp->depth < params.levels){
            branch_count = 3;
            index_order = index_order_case_3;
        }else{
            branch_count = 0;
        }

        Vec2 p = vert->pos;
        real32 theta_base = bp->theta - (branch_count - 1) * params.dtheta * .5f;
        i32 actual_branch_count = 0;
        _Assert(vert->children[0] == 0);
        _Assert(vert->children[1] == 0);
        _Assert(vert->children[2] == 0);

        bool32 block_others = 0;
        for (i32 ii = 0; ii < branch_count && !block_others; ++ii){
            i32 i = index_order[ii];

            real32 theta = theta_base + params.dtheta * i;
            real32 default_step = bp->step;
#if 0
            Vec2 towards_new_p = polar_to_cartesian(theta, default_step);
            f32 weighted_theta = theta;
            f32 weight = 0.f;
            i32 contribution_count = 0;
            for (i32 j = 0; j < params.hot_spot_count; ++j){
                Vec2 towards_hot_spot = params.hot_spots[j] - p;
                real32 projection = vector_projection(towards_new_p, towards_hot_spot);
                if (projection > 0.f){
                    weight += projection;
                    ++contribution_count;
                }
                if (opt_output){
                    post_branch_info(opt_output, opt_output_count, k, Contribution_Info, weight);
                }
            }
            if (theta >= bp->theta - 0.001f && theta <= bp->theta + 0.001f){
                //weight *= params.center_branch_weight;
            }
            if (contribution_count != 0){
                weight /= contribution_count;
            }

#elif 1
            f32 weight = default_step;
            f32 weighted_theta = theta;

            f32 average_arg = 0.f, total_weight = 0.f;

            Vec2 d_vec;
            f32 arg, d, local_weight;
            Hot_Spot *spot = params.hot_spots;
            for (i32 j = 0; j < params.hot_spot_count; ++j, ++spot){
                d_vec = spot->spot - p;
                arg = argument_degrees(d_vec);
                d = SQRT(dot(d_vec, d_vec));
                d -= default_step;
                if (d > 0 && d < params.heat_half_distance * 5.f){
                    local_weight = POW(2.f, -d / params.heat_half_distance);
                    total_weight += local_weight;
                    average_arg += (arg*local_weight);
                }
            }

            if (total_weight > 0){
                average_arg /= total_weight;
                weighted_theta = average_arg;

                if (weighted_theta < 0) weighted_theta += 360.f;

                f32 max_theta, min_theta;
                max_theta = theta + 20.f;
                min_theta = theta - 20.f;

                if (min_theta < 0) min_theta += 360.f;
                if (max_theta < min_theta){
                    max_theta += 360.f;
                }
                weighted_theta = Max(min_theta, Min(max_theta, weighted_theta));

                if (weighted_theta >= 360.f) weighted_theta -= 360.f;
            }

#else
            f32 weight = default_step;
            f32 weighted_theta = theta;
#endif

            if (weight > params.min_weight_threshold){
                Vec2 new_p = p + polar_to_cartesian(weighted_theta, weight);

                if (I < limit && (allow_colliding || no_collisions(p, new_p, vertices))){
                    ++I;

                    _Assert(k < vertex_count);
                    vert->children[i] = k;
                    ++actual_branch_count;
                    vertices[k].pos = new_p;
                    // TODO(allen): should the new length be based on bp->step or on this branch's specific weight?
                    queue[write_pos++] =
                        SBP(k, bp->step * params.step_decay, theta, bp->width * params.thickness_decay, bp->depth + 1);
                    write_pos = write_pos % queue_wrap;
                    _Assert(write_pos != read_pos);

                    if (opt_output){
                        post_branch_info(opt_output, opt_output_count, k, Step_Info, bp->step);
                        post_branch_info(opt_output, opt_output_count, k, Length_Info, weight);
                    }

                    ++k;
                }
                else{
                    if (ii == 0) block_others = 1;
                }
            }
            else{
                if (ii == 0) block_others = 1;
            }
        }
        vert->child_count = actual_branch_count;
        i32 i, j;
        for (i = 0, j = 0; i < 3; ++i){
            if (vert->children[i] != 0){
                vert->children[j] = vert->children[i];
                ++j;
            }
        }
        _Assert(j == actual_branch_count);
        for (; j < 3; ++j){
            vert->children[j] = 0;
        }
    }
    *vertex_count_out = k;

    return I;
}

struct Thickness_Point{
    i32 id;
    i32 next_child;
};

internal Thickness_Point
TP(i32 id){
    Thickness_Point result;
    result.id = id;
    result.next_child = 0;
    return result;
}

internal void
tree_gen(Vec2 *vertices, i32 vertex_count, i32 *vertex_count_out, Tree_Parameters params,
                Tree_Skeleton_Vert *skeleton){
    Thickness_Point stack[16];
    i32 top = 0;
    i32 k = 0;

    stack[top++] = TP(0);

    while (top > 0){
        Thickness_Point *tp = stack + top - 1;

        Tree_Skeleton_Vert *vert = skeleton + tp->id;

        {
            Vec2 p = vert->pos;
            Vec2 q = split_point(vert->width, vert->width * params.thickness_decay, vert->child_count, tp->next_child);
            vertices[k++] = p + rotate(q, vert->theta - 270.f);
        }

        bool32 done = 0;
        if (tp->next_child == 3){
            done = 1;
        }
        else{
            i32 child = vert->children[tp->next_child];
            if (child){
                stack[top++] = TP(child);
                ++tp->next_child;
            }
            else{
                done = 1;
            }
        }
        if (done){
            --top;
        }
    }
    *vertex_count_out = k;
}

// BOTTOM

