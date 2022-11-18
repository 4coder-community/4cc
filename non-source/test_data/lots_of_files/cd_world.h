/*

Game World data structures.

*/

#ifndef CD_WORLD_H
#define CD_WORLD_H

enum Entity_Type{
    ET_Null,
    ET_Player,
    ET_Obstacle,
    ET_Obstacle_Spawner,
    ET_Count
};


struct Entity{
    u32 type;
    
    f32 x;             // position in coordinates relative to player
    f32 speed;         // speed in absolute coordinates
    i32 lane;          // lane in absolute coordinates
    f32 lane_lerp;     // [-1, 1] motion to a new lane
    f32 lane_lerp_dir; // -1, 0, 1 indicates current lane motion
    
    f32 spawn_clock;
    f32 second_per_spawn;
    
    u32 seed;
    u16 entity_id;
    u16 hash_slot;
};

struct Entity_Hash{
    u16 entity_id;
    u16 entity_index;
};

struct World{
    Entity_Hash hash_slots[1031];
    Entity entities[512];
    u16 count;
    
    u16 kill_list[512];
    u16 kill_count;
    
    Random_Series entity_seed_generator;
    u16 next_id;
};

#endif