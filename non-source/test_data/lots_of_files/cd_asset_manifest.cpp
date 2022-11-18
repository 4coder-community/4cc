/*

Asset manifest operations
-Allen
06.06.2016

*/

// TOP

inline rptr32
to_rptr32(void *pos, void *base){
    rptr32 result = (rptr32)((char*)pos - (char*)base);
    return(result);
}

inline void*
to_ptr(rptr32 pos, void *base){
    void *result = ((char*)base + pos);
    return(result);
}

inline Asset_Node*
to_node_ptr(rptr32 pos, void *base){
    Asset_Node *result = (Asset_Node *)((char*)base + pos);
    return(result);
}

inline Asset_Node*
get_node(i32 image_id, Asset_Manifest *manifest){
    Asset_Node *node = to_node_ptr(manifest->asset_nodes, manifest);
    node += image_id - 1;
    return(node);
}

inline i32
get_image_id(rptr32 pos, Asset_Manifest *manifest){
    i32 dist = (pos - manifest->asset_nodes);
    Assert(dist % sizeof(Asset_Node) == 0);
    dist = 1 + (dist/sizeof(Asset_Node));
    return(dist);
}

void
init_sentinel(Asset_Node *node, void *base){
    rptr32 self = to_rptr32(node, base);
    node->next_sibling = self;
    node->prev_sibling = self;
}

void
insert_node(Asset_Node *node, Asset_Node *pos, void *base){
    rptr32 node_self = to_rptr32(node, base);
    rptr32 pos_self = to_rptr32(pos, base);
    rptr32 next_self = pos->next_sibling;
    Asset_Node *next = to_node_ptr(next_self, base);
    
    node->prev_sibling = pos_self;
    node->next_sibling = next_self;
    
    pos->next_sibling = node_self;
    next->prev_sibling = node_self;
}

void
remove_node(Asset_Node *node, void *base){
    rptr32 next_self = node->next_sibling;
    rptr32 prev_self = node->prev_sibling;
    Asset_Node *next = to_node_ptr(next_self, base);
    Asset_Node *prev = to_node_ptr(prev_self, base);
    
    next->prev_sibling = prev_self;
    prev->next_sibling = next_self;
}

void
sibling_insert_before(Asset_Node *node, Asset_Node *pos, void *base){
    rptr32 node_self = to_rptr32(node, base);
    rptr32 pos_self = to_rptr32(pos, base);
    rptr32 prev_self = pos->prev_sibling;
    Asset_Node *prev = to_node_ptr(prev_self, base);
    
    node->parent = pos->parent;
    
    node->next_sibling = pos_self;
    node->prev_sibling = prev_self;
    
    pos->prev_sibling = node_self;
    prev->next_sibling = node_self;
}

void
sibling_insert_after(Asset_Node *node, Asset_Node *pos, void *base){
    rptr32 node_self = to_rptr32(node, base);
    rptr32 pos_self = to_rptr32(pos, base);
    rptr32 next_self = pos->next_sibling;
    Asset_Node *next = to_node_ptr(next_self, base);
    
    node->parent = pos->parent;
    
    node->prev_sibling = pos_self;
    node->next_sibling = next_self;
    
    pos->next_sibling = node_self;
    next->prev_sibling = node_self;
}

void
tree_remove(Asset_Node *node, void *base){
    rptr32 parent_self = node->parent;
    rptr32 node_self = to_rptr32(node, base);
    rptr32 next_self = node->next_sibling;
    rptr32 prev_self = node->prev_sibling;
    Asset_Node *parent = to_node_ptr(parent_self, base);
    Asset_Node *next = to_node_ptr(next_self, base);
    Asset_Node *prev = to_node_ptr(prev_self, base);
    
    next->prev_sibling = prev_self;
    prev->next_sibling = next_self;
    
    if (parent->first_child == node_self){
        if (next_self != node_self){
            parent->first_child = next_self;
        }
        else{
            parent->first_child = 0;
        }
    }
}

void
insert_under(Asset_Node *node, Asset_Node *parent, void *base, b32 insert_as_first){
    if (parent->first_child == 0){
        rptr32 node_self = to_rptr32(node, base);
        rptr32 parent_self = to_rptr32(parent, base);
        
        parent->first_child = node_self;
        node->next_sibling = node_self;
        node->prev_sibling = node_self;
        node->parent = parent_self;
    }
    else{
        Asset_Node *first = to_node_ptr(parent->first_child, base);
        sibling_insert_before(node, first, base);
        
        if (insert_as_first){
            rptr32 node_self = to_rptr32(node, base);
            parent->first_child = node_self;
        }
    }
}

void
add_free_nodes(Asset_Manifest *manifest, Wrapped_Partition *part){
    i32 node_count = (manifest->part.max - manifest->part.pos) / sizeof(Asset_Node);
    
    if (node_count > 0){
        Asset_Node *nodes = push_array(part, Asset_Node, node_count);
        
        i32 j = manifest->asset_node_count + 1;
        
        if (manifest->asset_nodes == 0){
            manifest->asset_nodes = to_rptr32(nodes, manifest);
        }
        manifest->asset_node_count += node_count;
        manifest->asset_free_count += node_count;
        
        Asset_Node *node = nodes;
        for (i32 i = 0; i < node_count; ++i, ++j, ++node){
            insert_node(node, &manifest->free_sentinel, manifest);
            node->image_id = j;
            node->name[0] = 0;
        }
    }
}

void
initialize_empty_manifest(void *manifest_memory, i32 size){
    Asset_Manifest *manifest = (Asset_Manifest*)manifest_memory;
    manifest->version = ASSET_MANIFEST_VERSION;
    
    manifest->part = make_relative_partition(size);
    
    Wrapped_Partition part_ = make_wrapped_partition(&manifest->part, manifest_memory);
    Wrapped_Partition *part = &part_;
    
    push_type(part, Asset_Manifest);
    
    init_sentinel(&manifest->free_sentinel, manifest_memory);
    
    manifest->asset_node_count = 0;
    manifest->asset_free_count = 0;
    
    add_free_nodes(manifest, part);
}

#define MANIFEST_NAME "CDmanifest"

Asset_Manifest*
manifest_load(System_API *system, Partition *part){
    void *result = 0;
    
    Temp_Memory temp = begin_temp(part);
    File_Dump dump = system->DBG_dump_begin(MANIFEST_NAME);
    
    if (dump.size > 0){
        result = push_block(part, dump.size);
        
        if (result == 0){
#ifdef DEVELOPER
            DBG_expand_partition(system, part, dump.size);
            result = push_block(part, dump.size);
#else
            InvalidCodePath;
#endif
        }
        
        if (!system->DBG_dump_end(dump, result)){
            end_temp(temp);
            result = 0;
        }
    }
    
    return((Asset_Manifest*)result);
}

#ifdef DEVELOPER
void
manifest_dump(System_API *system, void *base){
    Asset_Manifest *manifest = (Asset_Manifest*)base;
    i32 memory_size = manifest->part.max;
    system->DBG_dump_out(MANIFEST_NAME, base, memory_size);
}
#endif

void
allocate_node(Asset_Manifest *manifest,
              Asset_Node *node,
              char *filename, i32 len){
    --manifest->asset_free_count;
    
    remove_node(node, manifest);
    cd_memcpy(node->name, filename, len+1);
    
    node->first_child = 0;
    node->next_sibling = 0;
    node->prev_sibling = 0;
}

void
free_node(Asset_Manifest *manifest,
          Asset_Node *node){
    
    ++manifest->asset_free_count;
    
    insert_node(node, &manifest->free_sentinel, manifest);
    cd_memcpy(node->name, "", 0);
}

rptr32
declare_image(void *manifest_memory, char *filename, i32 image_id, i32 expect_empty){
    rptr32 result = 0;
    i32 len = 0;
    
    len = cd_strlen(filename);
    
    if (len != 0 && len < ASSET_MAX_NAME){
        Asset_Manifest *manifest = (Asset_Manifest*)manifest_memory;
        Asset_Node *nodes = (Asset_Node*)
            to_ptr(manifest->asset_nodes, manifest_memory);
        Asset_Node *node = nodes + (image_id - 1);
        
        if (node->name[0] == 0 && expect_empty){
            allocate_node(manifest, node, filename, len);
            result = to_rptr32(node, manifest_memory);
        }
        else if (node->name[0] != 0 && !expect_empty){
            cd_memcpy(node->name, filename, len+1);
            result = to_rptr32(node, manifest_memory);
        }
    }
    
    return(result);
}

rptr32
bind_image(void *manifest_memory, Manifest_Setup *setup, char *filename, i32 image_id){
    rptr32 result = declare_image(manifest_memory, filename, image_id, true);
    
    if (setup->count > 0){
        Asset_Node *node = to_node_ptr(result, manifest_memory);
        Asset_Node *parent = to_node_ptr(setup->parents[setup->count-1], manifest_memory);
        
        insert_under(node, parent, manifest_memory, false);
        node->type = AssetType_Image;
    }
    
    return(result);
}

rptr32
replace_image(void *manifest_memory, char *filename, i32 image_id){
    return(declare_image(manifest_memory, filename, image_id, false));
}

void
begin_folder(void *manifest_memory, Manifest_Setup *setup, char *name, i32 image_id){
    Assert(setup->count < ArrayCount(setup->parents));
    
    rptr32 node_self = bind_image(manifest_memory, setup, name, image_id);
    setup->parents[setup->count++] = node_self;
    
    Asset_Node *node = to_node_ptr(node_self, manifest_memory);
    node->type = AssetType_GenericFolder;
}

void
end_folder(void *manifest_memory, Manifest_Setup *setup){
    Assert(setup->count > 0);
    --setup->count;
}

int
new_image(void *manifest_memory, char *filename){
    i32 result = 0;
    
    i32 len = cd_strlen(filename);
    
    if (len != 0 && len < ASSET_MAX_NAME){
        Asset_Manifest *manifest = (Asset_Manifest*)manifest_memory;
        Asset_Node *node =
            to_node_ptr(manifest->free_sentinel.next_sibling, manifest_memory);
        if (node != &manifest->free_sentinel){
            Assert(node->name[0] == 0);
            allocate_node(manifest, node, filename, len);
            result = 1;
        }
    }
    
    return(result);
}

void
delete_image(void *manifest_memory, i32 image_id){
    Asset_Manifest *manifest = (Asset_Manifest*)manifest_memory;
    Asset_Node *nodes = to_node_ptr(manifest->asset_nodes, manifest_memory);
    Asset_Node *node = nodes + (image_id - 1);
    
    if (node->name[0] != 0){
        free_node(manifest, node);
    }
}

void
move_manifest_to_bigger_block(void *manifest_memory, void *new_memory, i32 size){
    Asset_Manifest *manifest = (Asset_Manifest*)manifest_memory;
    
    Assert(manifest->part.max < size);
    
    if (new_memory != manifest_memory){
        cd_memcpy(new_memory, manifest_memory, manifest->part.max);
    }
    manifest = (Asset_Manifest*)new_memory;
    manifest->part.max = size;
    
    Wrapped_Partition part = make_wrapped_partition(&manifest->part, manifest_memory);
    add_free_nodes(manifest, &part);
}

#ifdef DEVELOPER
Asset_Node*
get_available_node(System_API *system, Partition *manifest_part, Asset_Manifest **manifest_memory_ptr){
    Asset_Manifest *manifest = (Asset_Manifest*)manifest_part->base;
    
    rptr32 node_self = manifest->free_sentinel.next_sibling;
    Asset_Node *result = to_node_ptr(node_self, manifest);
    
    if (result == &manifest->free_sentinel){
        i32 pos = manifest_part->rel_part.pos;
        Asset_Manifest *new_manifest = (Asset_Manifest*)push_block(manifest_part, pos);
        if (new_manifest == 0){
            DBG_expand_partition(system, manifest_part, pos);
            new_manifest = (Asset_Manifest*)push_block(manifest_part, pos);
            Assert(new_manifest);
            manifest = (Asset_Manifest*)manifest_part->base;
            *manifest_memory_ptr = (Asset_Manifest*)manifest_part->base;
        }
        move_manifest_to_bigger_block(manifest, manifest, manifest_part->rel_part.pos);
        
        node_self = manifest->free_sentinel.next_sibling;
        result = to_node_ptr(node_self, manifest);
        Assert(result != &manifest->free_sentinel);
    }
    
    allocate_node(manifest, result, "New", 3);
    
    return(result);
}
#endif

Asset_Walker_Entry
new_walker_entry(rptr32 first, i32 level){
    Asset_Walker_Entry result;
    result.first = first;
    result.current = first;
    result.level = level;
    return(result);
}

Asset_Node*
walk_first_asset_node(void *manifest_memory, Asset_Walker *walker){
    Asset_Manifest *manifest = (Asset_Manifest*)manifest_memory;
    Asset_Node *nodes = to_node_ptr(manifest->asset_nodes, manifest_memory);
    Asset_Node *node = nodes + (ROOT - 1);
    
    walker->current_level = 0;
    
    if (node->first_child){
        walker->stack[walker->top++] = new_walker_entry(node->first_child, 1);
    }
    
    return(node);
}

Asset_Node*
walk_next_asset_node(void *manifest_memory, Asset_Walker *walker){
    Asset_Node *node = 0;
    
    if (walker->top != 0){
        walker->current_level = walker->stack[walker->top-1].level;
    }
    
    if (walker->top > 0){
        Asset_Walker_Entry *top = walker->stack + (walker->top - 1);
        
        node = to_node_ptr(top->current, manifest_memory);
        
        top->current = node->next_sibling;
        
        if (top->current == top->first){
            --walker->top;
        }
        
        if (node->first_child){
            walker->stack[walker->top++] =
                new_walker_entry(node->first_child, top->level + 1);
        }
    }
    
    return(node);
}

Asset_Node*
walk_skip_children_asset_node(void *manifest_memory, Asset_Walker *walker){
    Asset_Node *node = 0;
    
    if (walker->top > 0){
        Asset_Walker_Entry *top = walker->stack + (walker->top - 1);
        
        while (top->level > walker->current_level){
            --walker->top;
            if (walker->top > 0){
                top = walker->stack + (walker->top - 1);
            }
            else{
                break;
            }
        }
        
        node = walk_next_asset_node(manifest_memory, walker);
    }
    
    return(node);
}

// BOTTOM
