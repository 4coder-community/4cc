/*
 * Memory allocator: Partition
 */

// TOP

#ifndef CD_PARTITION_H
#define CD_PARTITION_H

struct Relative_Partition{
    i32 pos, max;
};

struct Partition{
    Relative_Partition rel_part;
    void *base;
};

struct Wrapped_Partition{
    Relative_Partition *rel_part;
    void *base;
};

Partition
make_partition(void *mem, i32 size){
    Partition result;
    result.base = mem;
    result.rel_part.max = size;
    result.rel_part.pos = 0;
    return(result);
}

Relative_Partition
make_relative_partition(i32 size){
    Relative_Partition result;
    result.max = size;
    result.pos = 0;
    return(result);
}

Wrapped_Partition
make_wrapped_partition(Relative_Partition *part, void *base){
    Wrapped_Partition wrapped;
    wrapped.rel_part = part;
    wrapped.base = base;
    return(wrapped);
}

void
clear_partition(Relative_Partition *part){
    part->pos = 0;
}

void*
push_partition(Relative_Partition *part, void *base, i32 size){
    char *ptr = 0;
    if (part->pos + size <= part->max){
        ptr = (char*)base;
        ptr += part->pos;
        part->pos += size;
    }
    return(ptr);
}

void*
push_partition(Partition *part, i32 size){
    return(push_partition(&part->rel_part, part->base, size));
}

void*
push_partition(Wrapped_Partition *part, i32 size){
    return(push_partition(part->rel_part, part->base, size));
}

// NOTE(allen): alignment should always be a power of two.
void
align_partition(Relative_Partition *part, i32 alignment){
    part->pos = (part->pos + (alignment-1)) & (~(alignment-1));
}

void
align_partition(Partition *part, i32 alignment){
    align_partition(&part->rel_part, alignment);
}

void
align_partition(Wrapped_Partition *part, i32 alignment){
    align_partition(part->rel_part, alignment);
}

Relative_Partition
sub_partition(Relative_Partition *part, i32 size){
    Relative_Partition result = {0};
    void *base = 0;
    align_partition(part, 8);
    base = push_partition(part, 0, size);
    if (base){
        result.max = size;
    }
    return(result);
}

Partition
sub_partition(Relative_Partition *part, void *base, i32 size){
    Partition result = {};
    align_partition(part, 8);
    result.base = push_partition(part, base, size);
    if (result.base){
        result.rel_part.max = size;
    }
    return(result);
}

Partition
sub_partition(Partition *part, i32 size){
    return(sub_partition(&part->rel_part, part->base, size));
}

i32
get_pos(Relative_Partition *part){
    return(part->pos);
}

i32
get_pos(Partition *part){
    return(part->rel_part.pos);
}

#define push_block(p, size) push_partition(p, size)
#define push_type(p, type) (type*)push_partition(p, sizeof(type))
#define push_array(p, type, size) (type*)push_partition(p, size*sizeof(type))

struct Temp_Memory{
    Relative_Partition *part;
    i32 pos;
};

Temp_Memory
begin_temp(Relative_Partition *part){
    Temp_Memory temp;
    temp.part = part;
    temp.pos = part->pos;
    return(temp);
}

Temp_Memory
begin_temp(Partition *part){
    Temp_Memory temp = begin_temp(&part->rel_part);
    return(temp);
}

Temp_Memory
begin_temp(Wrapped_Partition *part){
    Temp_Memory temp = begin_temp(part->rel_part);
    return(temp);
}

void
end_temp(Temp_Memory temp){
    temp.part->pos = temp.pos;
}

void*
part_malloc(i32 size, void *user_data){
    void *result = push_partition((Partition*)user_data, size);
    return(result);
}

#define STBTT_malloc(s,u) part_malloc(s,u)
#define STBTT_free(d,u) do{ (void)d; (void)u; }while(0)

#endif

// BOTTOM

