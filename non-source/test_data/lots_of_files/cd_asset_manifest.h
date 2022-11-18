/*

Asset manifest data format definition
-Allen
06.06.2016

*/

// TOP

#ifndef CD_ASSET_MANIFEST_H
#define CD_ASSET_MANIFEST_H

#define ASSET_MANIFEST_VERSION 1

#define ASSET_MAX_NAME 32

// IMPORTANT(allen):
//  If Asset_Node or Asset_Manifest are edited at all it will invalidate
// all CDmanifest files.
//
//  If Asset_Flag or Asset_Type are rearranged it will invalidate all
// CDmanifest files.
//
//  If we _have_ to invalidate CDmanifest files we will have to write a
// one time convert which is time consuming, so usually finding a method
// other that does not invalidate our data format will be best.
//
//  If this turns into a big issue, I'll look into making a way to write
// these one time converts more quickly or to support a more flexible
// data format.
//
//  Note we can insert enumeration values in Asset_Type right above
// AssetType_TypeCount safely, as that value is just meant to count how many
// asset types we have and should not actually be stored in CDmanifest.
// 

struct Asset_Node{
    rptr32 first_child;
    rptr32 parent;
    rptr32 next_sibling;
    rptr32 prev_sibling;
    
    char name[ASSET_MAX_NAME];
    
    i32 image_id;
    i32 type;
    u64 flags;
    
    char expansion_space[32];
};

enum Asset_Flag{
    AssetFlag_None = 0x0,
};

enum Asset_Type{
    AssetType_GenericFolder,
    AssetType_Image,
    AssetType_ObstacleType,
    
    // this must be at the end
    AssetType_TypeCount
};

struct Asset_Manifest{
    Relative_Partition part;
    i32 version;
    
    Asset_Node free_sentinel;
    rptr32 asset_nodes;
    i32 asset_node_count;
    i32 asset_free_count;
    
    char expansion_space[1008];
};

struct Manifest_Setup{
    rptr32 parents[32];
    i32 count;
};

struct Asset_Walker_Entry{
    rptr32 first, current;
    i32 level;
};

struct Asset_Walker{
    Asset_Walker_Entry stack[32];
    i32 top;
    
    i32 current_level;
};

#endif

// BOTTOM
