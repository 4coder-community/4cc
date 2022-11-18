member_definition MembersOf_sim_entity_collision_volume[] = 
{
   {0, MetaType_v3, "OffsetP", (u32)&((sim_entity_collision_volume *)0)->OffsetP},
   {0, MetaType_v3, "Dim", (u32)&((sim_entity_collision_volume *)0)->Dim},
};
member_definition MembersOf_sim_entity_collision_volume_group[] = 
{
   {0, MetaType_sim_entity_collision_volume, "TotalVolume", (u32)&((sim_entity_collision_volume_group *)0)->TotalVolume},
   {0, MetaType_u32, "VolumeCount", (u32)&((sim_entity_collision_volume_group *)0)->VolumeCount},
   {MetaMemberFlag_IsPointer, MetaType_sim_entity_collision_volume, "Volumes", (u32)&((sim_entity_collision_volume_group *)0)->Volumes},
};
member_definition MembersOf_sim_entity[] = 
{
   {MetaMemberFlag_IsPointer, MetaType_world_chunk, "OldChunk", (u32)&((sim_entity *)0)->OldChunk},
   {0, MetaType_u32, "StorageIndex", (u32)&((sim_entity *)0)->StorageIndex},
   {0, MetaType_b32, "Updatable", (u32)&((sim_entity *)0)->Updatable},
   {0, MetaType_entity_type, "Type", (u32)&((sim_entity *)0)->Type},
   {0, MetaType_u32, "Flags", (u32)&((sim_entity *)0)->Flags},
   {0, MetaType_v3, "P", (u32)&((sim_entity *)0)->P},
   {0, MetaType_v3, "dP", (u32)&((sim_entity *)0)->dP},
   {0, MetaType_r32, "DistanceLimit", (u32)&((sim_entity *)0)->DistanceLimit},
   {MetaMemberFlag_IsPointer, MetaType_sim_entity_collision_volume_group, "Collision", (u32)&((sim_entity *)0)->Collision},
   {0, MetaType_r32, "FacingDirection", (u32)&((sim_entity *)0)->FacingDirection},
   {0, MetaType_r32, "tBob", (u32)&((sim_entity *)0)->tBob},
   {0, MetaType_s32, "dAbsTileZ", (u32)&((sim_entity *)0)->dAbsTileZ},
   {0, MetaType_u32, "HitPointMax", (u32)&((sim_entity *)0)->HitPointMax},
   {0, MetaType_hit_point, "HitPoint", (u32)&((sim_entity *)0)->HitPoint},
   {0, MetaType_entity_reference, "Sword", (u32)&((sim_entity *)0)->Sword},
   {0, MetaType_v2, "WalkableDim", (u32)&((sim_entity *)0)->WalkableDim},
   {0, MetaType_r32, "WalkableHeight", (u32)&((sim_entity *)0)->WalkableHeight},
};
member_definition MembersOf_sim_region[] = 
{
   {MetaMemberFlag_IsPointer, MetaType_world, "World", (u32)&((sim_region *)0)->World},
   {0, MetaType_r32, "MaxEntityRadius", (u32)&((sim_region *)0)->MaxEntityRadius},
   {0, MetaType_r32, "MaxEntityVelocity", (u32)&((sim_region *)0)->MaxEntityVelocity},
   {0, MetaType_world_position, "Origin", (u32)&((sim_region *)0)->Origin},
   {0, MetaType_rectangle3, "Bounds", (u32)&((sim_region *)0)->Bounds},
   {0, MetaType_rectangle3, "UpdatableBounds", (u32)&((sim_region *)0)->UpdatableBounds},
   {0, MetaType_u32, "MaxEntityCount", (u32)&((sim_region *)0)->MaxEntityCount},
   {0, MetaType_u32, "EntityCount", (u32)&((sim_region *)0)->EntityCount},
   {MetaMemberFlag_IsPointer, MetaType_sim_entity, "Entities", (u32)&((sim_region *)0)->Entities},
   {0, MetaType_sim_entity_hash, "Hash", (u32)&((sim_region *)0)->Hash},
};
member_definition MembersOf_rectangle2[] = 
{
   {0, MetaType_v2, "Min", (u32)&((rectangle2 *)0)->Min},
   {0, MetaType_v2, "Max", (u32)&((rectangle2 *)0)->Max},
};
member_definition MembersOf_rectangle3[] = 
{
   {0, MetaType_v3, "Min", (u32)&((rectangle3 *)0)->Min},
   {0, MetaType_v3, "Max", (u32)&((rectangle3 *)0)->Max},
};
member_definition MembersOf_world_position[] = 
{
   {0, MetaType_s32, "ChunkX", (u32)&((world_position *)0)->ChunkX},
   {0, MetaType_s32, "ChunkY", (u32)&((world_position *)0)->ChunkY},
   {0, MetaType_s32, "ChunkZ", (u32)&((world_position *)0)->ChunkZ},
   {0, MetaType_v3, "Offset_", (u32)&((world_position *)0)->Offset_},
};
#define META_HANDLE_TYPE_DUMP(MemberPtr, NextIndentLevel) \
    case MetaType_world_position: {DEBUGTextLine(Member->Name); DEBUGDumpStruct(ArrayCount(MembersOf_world_position), MembersOf_world_position, MemberPtr, (NextIndentLevel));} break; \
    case MetaType_rectangle3: {DEBUGTextLine(Member->Name); DEBUGDumpStruct(ArrayCount(MembersOf_rectangle3), MembersOf_rectangle3, MemberPtr, (NextIndentLevel));} break; \
    case MetaType_rectangle2: {DEBUGTextLine(Member->Name); DEBUGDumpStruct(ArrayCount(MembersOf_rectangle2), MembersOf_rectangle2, MemberPtr, (NextIndentLevel));} break; \
    case MetaType_sim_region: {DEBUGTextLine(Member->Name); DEBUGDumpStruct(ArrayCount(MembersOf_sim_region), MembersOf_sim_region, MemberPtr, (NextIndentLevel));} break; \
    case MetaType_sim_entity: {DEBUGTextLine(Member->Name); DEBUGDumpStruct(ArrayCount(MembersOf_sim_entity), MembersOf_sim_entity, MemberPtr, (NextIndentLevel));} break; \
    case MetaType_sim_entity_collision_volume_group: {DEBUGTextLine(Member->Name); DEBUGDumpStruct(ArrayCount(MembersOf_sim_entity_collision_volume_group), MembersOf_sim_entity_collision_volume_group, MemberPtr, (NextIndentLevel));} break; \
    case MetaType_sim_entity_collision_volume: {DEBUGTextLine(Member->Name); DEBUGDumpStruct(ArrayCount(MembersOf_sim_entity_collision_volume), MembersOf_sim_entity_collision_volume, MemberPtr, (NextIndentLevel));} break; 
