#ifndef VFS_H
#define VFS_H

#include "common.h"

#define VFS_TYPE_FILE 1
#define VFS_TYPE_DIR 2

typedef struct vfs_node {
    char name[32];
    uint32_t type;
    uint32_t size;
    uint32_t inode;
    struct vfs_node* parent;
    struct vfs_node* children;
    struct vfs_node* next;
    void* data; // File content for RAM disk
} vfs_node_t;

void vfs_init(void);
vfs_node_t* vfs_get_root(void);
vfs_node_t* vfs_lookup(vfs_node_t* parent, const char* name);
vfs_node_t* vfs_mkdir(vfs_node_t* parent, const char* name);
vfs_node_t* vfs_touch(vfs_node_t* parent, const char* name);
int vfs_write(vfs_node_t* node, const char* data, uint32_t size);
const char* vfs_read(vfs_node_t* node);

#endif
