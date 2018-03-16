/*

Meta Asset data format definition
-Allen
15.07.2016

*/

// TOP

#ifndef CD_META_ASSET_BANK_H
#define CD_META_ASSET_BANK_H

#define METAASSET_VERSION 1

struct Proto_Obstacle{
    in32_t image_bank;
};

struct Meta_Asset_Bank{
    Proto_Obstacle *proto_obstacles;
    i32 proto_obstacle_max_id;
};

struct Meta_Asset_Setup{
    Meta_Asset_Bank *bank;
    Partition *trans_part;
    System_API *system;
};

#endif

// BOTTOM

