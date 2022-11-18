/*

Game World simulation management
Entity types

*/

// TOP

// TODO(allen): Check more carefully if this id is safe to use!
inline u16
get_next_entity_id(u16 counter){
    u16 next;
    switch(counter){
        case max_u16: case 0: next = 1; break;
        default: next = counter+1; break;
    }
    return(next);
}

#define freed_slot max_u16

static Entity*
add_entity(World *world){
    Entity *entity = 0;
    u16 entity_index = 0;
    
    if (world->count < ArrayCount(world->entities)){
        entity_index = world->count++;
        entity = &world->entities[entity_index];
        memset(entity, 0, sizeof(*entity));
        
        entity->entity_id = world->next_id;
        world->next_id = get_next_entity_id(world->next_id);
        
        {
            Entity_Hash *table = world->hash_slots;
            u16 max = ArrayCount(world->hash_slots);
            u16 entity_id = entity->entity_id;
            u16 hash = entity_id % max;
            u16 hash_slot = hash;
            
            while (table[hash_slot].entity_id != 0 &&
                   table[hash_slot].entity_id != freed_slot){
                ++hash_slot;
                if (hash_slot == max) hash_slot = 0;
                Assert(hash_slot != hash);
            }
            
            table[hash_slot].entity_id = entity_id;
            table[hash_slot].entity_index = entity_index;
            entity->hash_slot = hash_slot;
        }
        
        entity->seed = random_next_u32(&world->entity_seed_generator);
    }
    
    return(entity);
}

static void
update_entity_slot(World *world, u16 entity_id, u16 new_entity_index){
    Entity_Hash *table = world->hash_slots;
    u16 max = ArrayCount(world->hash_slots);
    u16 hash = entity_id % max;
    u16 hash_slot = hash;
    
    while (table[hash_slot].entity_id != entity_id){
        ++hash_slot;
        if (hash_slot == max) hash_slot = 0;
        Assert(hash_slot != hash);
    }
    
    table[hash_slot].entity_index = new_entity_index;
}

static void
kill_entity(World *world, u16 entity_id){
    Entity_Hash *table = world->hash_slots;
    u16 max = ArrayCount(world->hash_slots);
    u16 hash = entity_id % max;
    u16 hash_slot = hash;
    
    while (table[hash_slot].entity_id != entity_id){
        ++hash_slot;
        if (hash_slot == max) hash_slot = 0;
        Assert(hash_slot != hash);
    }
    
    table[hash_slot].entity_id = freed_slot;
    
    {
        Entity *entities = world->entities;
        u16 entity_index = table[hash_slot].entity_index;
        u16 new_entity_count = world->count-1;
        
        world->count = new_entity_count;
        if (entity_index < new_entity_count){
            entities[entity_index] = entities[new_entity_count];
            update_entity_slot(world, entities[entity_index].entity_id, entity_index);
        }
    }
}

inline void
world_begin_simulation(World *world){
    world->kill_count = 0;
}

inline void
world_end_simulation(World *world){
    Entity *entities = world->entities;
    u16 kill_count = world->kill_count;
    u16 *kill_list = world->kill_list;
    u16 kill_index = 0;
    
    for (u16 kill_i = 0;
         kill_i < kill_count;
         ++kill_i){
        kill_index = kill_list[kill_i];
        kill_entity(world, entities[kill_index].entity_id);
    }
}

inline void
add_to_kill_list(World *world, Entity *entity){
    u16 slot_index = (u16)(entity - world->entities);
    Assert(world->kill_count < ArrayCount(world->kill_list));
    world->kill_list[world->kill_count++] = slot_index;
}



static Entity*
add_player(World *world){
    Entity *result = add_entity(world);
    if (result){
        result->type = ET_Player;
    }
    return(result);
}

static Entity*
add_asteroid(World *world, i32 lane){
    Entity *result = add_entity(world);
    if (result){
        result->type = ET_Obstacle;
        result->x = 25.f;
        result->speed = 0.f;
        result->lane = lane;
    }
    return(result);
}

static Entity*
add_asteroid_spawner(World *world, f32 second_per_spawn){
    Entity *result = add_entity(world);
    if (result){
        result->type = ET_Obstacle_Spawner;
        result->second_per_spawn = second_per_spawn;
    }
    return(result);
}

static Entity*
get_player(World *world){
    Entity *result = 0;
    
    Entity *entity = world->entities;
    u32 count = world->count;
    for (u32 i = 0;
         i < count;
         ++i, ++entity){
        if (entity->type == ET_Player){
            result = entity;
            break;
        }
    }
    
    return(result);
}

// BOTTOM

