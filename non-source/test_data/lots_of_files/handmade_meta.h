#if !defined(HANDMADE_META_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

enum meta_type
{
    MetaType_world_chunk,
    MetaType_u32,
    MetaType_b32,
    MetaType_entity_type,
    MetaType_v3,
    MetaType_r32,
    MetaType_sim_entity_collision_volume_group,
    MetaType_s32,
    MetaType_hit_point,
    MetaType_entity_reference,
    MetaType_v2,
    MetaType_world,
    MetaType_world_position,
    MetaType_rectangle2,
    MetaType_rectangle3,
    MetaType_sim_region,
    MetaType_sim_entity,
    MetaType_sim_entity_hash,
    MetaType_sim_entity_collision_volume,
};

enum member_definition_flag
{
    MetaMemberFlag_IsPointer = 0x1,
};
struct member_definition
{
    u32 Flags;
    meta_type Type;
    char *Name;
    u32 Offset;
};

#define HANDMADE_META_H
#endif
