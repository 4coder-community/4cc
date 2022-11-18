#if !defined(HANDMADE_SIM_REGION_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

struct move_spec
{
    bool32 UnitMaxAccelVector;
    real32 Speed;
    real32 Drag;
};

enum entity_type
{
    EntityType_Null,
    
    EntityType_Space,
    
    EntityType_Hero,
    EntityType_Wall,
    EntityType_Familiar,
    EntityType_Monstar,
    EntityType_Sword,
    EntityType_Stairwell,
};

#define HIT_POINT_SUB_COUNT 4
struct hit_point
{
    // TODO(casey): Bake this down into one variable
    uint8 Flags;
    uint8 FilledAmount;
};

// TODO(casey): Rename sim_entity to entity!
struct sim_entity;
union entity_reference
{
    sim_entity *Ptr;
    uint32 Index;
};

enum sim_entity_flags
{
    // TODO(casey): Does it make more sense to have the flag be for _non_ colliding entities?
    // TODO(casey): Collides and ZSupported probably can be removed now/soon
    EntityFlag_Collides = (1 << 0),
    EntityFlag_Nonspatial = (1 << 1),
    EntityFlag_Moveable = (1 << 2),
    EntityFlag_ZSupported = (1 << 3),
    EntityFlag_Traversable = (1 << 4),

    EntityFlag_Simming = (1 << 30),
};

introspect(category:"sim_region") struct sim_entity_collision_volume
{
    v3 OffsetP;
    v3 Dim;
};

introspect(category:"sim_region") struct sim_entity_collision_volume_group
{
    sim_entity_collision_volume TotalVolume;

    // TODO(casey): VolumeCount is always expected to be greater than 0 if the entity
    // has any volume... in the future, this could be compressed if necessary to say
    // that the VolumeCount can be 0 if the TotalVolume should be used as the only
    // collision volume for the entity.
    u32 VolumeCount;
    sim_entity_collision_volume *Volumes; 
};

introspect(category:"regular butter") struct sim_entity
{
    // NOTE(casey): These are only for the sim region
    world_chunk *OldChunk;
    u32 StorageIndex;
    b32 Updatable;

    //
    
    entity_type Type;
    u32 Flags;
    
    v3 P;
    v3 dP;
    
    r32 DistanceLimit;

    sim_entity_collision_volume_group *Collision;

    r32 FacingDirection;
    r32 tBob;

    s32 dAbsTileZ;

    // TODO(casey): Should hitpoints themselves be entities?
    u32 HitPointMax;
    hit_point HitPoint[16];

    entity_reference Sword;

    // TODO(casey): Only for stairwells!
    v2 WalkableDim;
    r32 WalkableHeight;
    
    // TODO(casey): Generation index so we know how "up to date" this entity is.
};

struct sim_entity_hash
{
    sim_entity *Ptr;
    uint32 Index;
};

introspect(category:"regular butter") struct sim_region
{
    // TODO(casey): Need a hash table here to map stored entity indices
    // to sim entities!
    
    world *World;
    r32 MaxEntityRadius;
    r32 MaxEntityVelocity;

    world_position Origin;
    rectangle3 Bounds;
    rectangle3 UpdatableBounds;
    
    u32 MaxEntityCount;
    u32 EntityCount;
    sim_entity *Entities;
    
    // TODO(casey): Do I really want a hash for this??
    // NOTE(casey): Must be a power of two!
    sim_entity_hash Hash[4096];
};

#define HANDMADE_SIM_REGION_H
#endif
