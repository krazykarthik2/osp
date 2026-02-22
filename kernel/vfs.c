#include "vfs.h"
#include "terminal.h"

#define VFS_MAX_NODES 32
static vfs_node_t nodes[VFS_MAX_NODES];
static int next_node = 0;
static vfs_node_t* root = 0;

static vfs_node_t* alloc_node(const char* name, uint32_t type) {
    if (next_node >= VFS_MAX_NODES) return 0;
    vfs_node_t* node = &nodes[next_node++];
    
    int i = 0;
    while (i < 31 && name[i]) {
        node->name[i] = name[i];
        i++;
    }
    node->name[i] = 0;

    node->type = type;
    node->size = 0;
    node->inode = next_node;
    node->parent = 0;
    node->children = 0;
    node->next = 0;
    node->data = 0;
    return node;
}

void vfs_init(void) {
    next_node = 0;
    root = alloc_node("/", VFS_TYPE_DIR);
}

vfs_node_t* vfs_get_root(void) { return root; }

vfs_node_t* vfs_lookup(vfs_node_t* parent, const char* name) {
    if (!parent || parent->type != VFS_TYPE_DIR) return 0;
    vfs_node_t* child = parent->children;
    while (child) {
        const char* s1 = (const char*)child->name;
        const char* s2 = name;
        while (*s1 && *s1 == *s2) { s1++; s2++; }
        if (*s1 == 0 && *s2 == 0) return child;
        child = child->next;
    }
    return 0;
}

vfs_node_t* vfs_mkdir(vfs_node_t* parent, const char* name) {
    if (!parent || parent->type != VFS_TYPE_DIR) return 0;
    if (vfs_lookup(parent, name)) return 0;
    vfs_node_t* node = alloc_node(name, VFS_TYPE_DIR);
    if (!node) return 0;
    node->parent = parent;
    node->next = parent->children;
    parent->children = node;
    return node;
}

vfs_node_t* vfs_touch(vfs_node_t* parent, const char* name) {
    if (!parent || parent->type != VFS_TYPE_DIR) return 0;
    if (vfs_lookup(parent, name)) return 0;
    vfs_node_t* node = alloc_node(name, VFS_TYPE_FILE);
    if (!node) return 0;
    node->parent = parent;
    node->next = parent->children;
    parent->children = node;
    return node;
}

#define MAX_DATA_POOL 4096
static char data_pool[MAX_DATA_POOL];
static uint32_t data_cursor = 0;

int vfs_write(vfs_node_t* node, const char* data, uint32_t size) {
    if (!node || node->type != VFS_TYPE_FILE) return -1;
    if (data_cursor + size + 1 > MAX_DATA_POOL) return -1;
    char* target = &data_pool[data_cursor];
    uint32_t i = 0;
    while (i < size) { target[i] = data[i]; i++; }
    target[size] = 0;
    node->data = target;
    node->size = size;
    data_cursor += size + 1;
    return 0;
}

const char* vfs_read(vfs_node_t* node) {
    if (!node || node->type != VFS_TYPE_FILE) return 0;
    return (const char*)node->data;
}
