/*
 * Debug systems
 */

// TOP

void
DBG_expand_partition(System_API *system, Partition *part, i32 min_expand_size){
    i32 new_size = part->rel_part.max*2;
    if (part->rel_part.max < min_expand_size){
        new_size = min_expand_size*2;
    }
    
    void *new_mem = system->DBG_memory_allocate(new_size);
    if (part->base){
        cd_memcpy(new_mem, part->base, part->rel_part.pos);
        system->DBG_memory_free(part->base);
        cd_memset((char*)new_mem + part->rel_part.pos, 0, new_size - part->rel_part.pos);
    }
    
    part->base = new_mem;
    part->rel_part.max = new_size;
}

// BOTTOM
