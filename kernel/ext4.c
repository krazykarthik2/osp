#include "ext4.h"
#include "terminal.h"

// For "Real" feel, we emulate a block device in memory (1 MB)
#define DISK_BLOCKS 1024
#define BLK_SIZE 1024
static uint8_t disk[DISK_BLOCKS * BLK_SIZE];

// In-core mount state
static int current_dir_inode = EXT4_ROOT_INODE;

// --- Helper Functions ---

static void disk_read(uint32_t blk, void* buf) {
    if (blk >= DISK_BLOCKS) return;
    for (int i = 0; i < BLK_SIZE; i++) ((uint8_t*)buf)[i] = disk[blk * BLK_SIZE + i];
}

static void disk_write(uint32_t blk, const void* buf) {
    if (blk >= DISK_BLOCKS) return;
    for (int i = 0; i < BLK_SIZE; i++) disk[blk * BLK_SIZE + i] = ((uint8_t*)buf)[i];
}

static int streq(const char* a, const char* b) {
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return *a == *b;
}

// Simple bit set/test for bitmaps
static void bit_set(uint8_t* bitmap, int bit) { bitmap[bit / 8] |= (1 << (bit % 8)); }
static void bit_clear(uint8_t* bitmap, int bit) { bitmap[bit / 8] &= ~(1 << (bit % 8)); }
static int bit_test(uint8_t* bitmap, int bit) { return (bitmap[bit / 8] & (1 << (bit % 8))) != 0; }

// --- Metadata Management ---

static void get_sb(ext4_super_block_t* sb) { disk_read(1, sb); }
static void set_sb(ext4_super_block_t* sb) { disk_write(1, sb); }

static void get_gd(ext4_group_desc_t* gd) { disk_read(2, gd); }
static void set_gd(ext4_group_desc_t* gd) { disk_write(2, gd); }

static void get_inode(uint32_t ino, ext4_inode_t* inode_buf) {
    ext4_group_desc_t gd;
    get_gd(&gd);
    uint32_t blk = gd.bg_inode_table + ((ino - 1) * EXT4_INODE_SIZE) / BLK_SIZE;
    uint32_t off = ((ino - 1) * EXT4_INODE_SIZE) % BLK_SIZE;
    uint8_t tmp[BLK_SIZE];
    disk_read(blk, tmp);
    for (int i = 0; i < (int)sizeof(ext4_inode_t); i++) ((uint8_t*)inode_buf)[i] = tmp[off + i];
}

static void set_inode(uint32_t ino, ext4_inode_t* inode_buf) {
    ext4_group_desc_t gd;
    get_gd(&gd);
    uint32_t blk = gd.bg_inode_table + ((ino - 1) * EXT4_INODE_SIZE) / BLK_SIZE;
    uint32_t off = ((ino - 1) * EXT4_INODE_SIZE) % BLK_SIZE;
    uint8_t tmp[BLK_SIZE];
    disk_read(blk, tmp);
    for (int i = 0; i < (int)sizeof(ext4_inode_t); i++) tmp[off + i] = ((uint8_t*)inode_buf)[i];
    disk_write(blk, tmp);
}

static uint32_t alloc_block(void) {
    ext4_group_desc_t gd;
    get_gd(&gd);
    uint8_t bitmap[BLK_SIZE];
    disk_read(gd.bg_block_bitmap, bitmap);
    for (int i = 0; i < DISK_BLOCKS; i++) {
        if (!bit_test(bitmap, i)) {
            bit_set(bitmap, i);
            disk_write(gd.bg_block_bitmap, bitmap);
            gd.bg_free_blocks_count--;
            set_gd(&gd);
            return i;
        }
    }
    return 0;
}

static uint32_t alloc_inode(void) {
    ext4_group_desc_t gd;
    get_gd(&gd);
    uint8_t bitmap[BLK_SIZE];
    disk_read(gd.bg_inode_bitmap, bitmap);
    for (int i = 0; i < 128; i++) { // arbitrary limit for inodes
        if (!bit_test(bitmap, i)) {
            bit_set(bitmap, i);
            disk_write(gd.bg_inode_bitmap, bitmap);
            gd.bg_free_inodes_count--;
            set_gd(&gd);
            return i + 1;
        }
    }
    return 0;
}

// --- Directory Operations ---

static uint32_t find_in_dir(uint32_t dir_ino, const char* name) {
    ext4_inode_t inode;
    get_inode(dir_ino, &inode);
    if (!(inode.i_mode & 0x4000)) return 0; // Not a dir

    uint8_t block[BLK_SIZE];
    for (int i = 0; i < 12 && inode.i_block[i]; i++) {
        disk_read(inode.i_block[i], block);
        ext4_dir_entry_t* entry = (ext4_dir_entry_t*)block;
        uint32_t offset = 0;
        while (offset < BLK_SIZE && entry->rec_len > 0) {
            char tmp_name[256];
            for (int k = 0; k < entry->name_len; k++) tmp_name[k] = entry->name[k];
            tmp_name[entry->name_len] = 0;
            if (streq(tmp_name, name)) return entry->inode;
            offset += entry->rec_len;
            entry = (ext4_dir_entry_t*)(block + offset);
        }
    }
    return 0;
}

static void add_to_dir(uint32_t dir_ino, uint32_t child_ino, const char* name, uint8_t type) {
    ext4_inode_t inode;
    get_inode(dir_ino, &inode);
    uint8_t block[BLK_SIZE];
    uint32_t last_blk_idx = 0;
    while (last_blk_idx < 11 && inode.i_block[last_blk_idx + 1]) last_blk_idx++;

    if (inode.i_block[last_blk_idx] == 0) {
        inode.i_block[last_blk_idx] = alloc_block();
        set_inode(dir_ino, &inode);
        for (int k = 0; k < BLK_SIZE; k++) block[k] = 0;
    } else {
        disk_read(inode.i_block[last_blk_idx], block);
    }

    ext4_dir_entry_t* entry = (ext4_dir_entry_t*)block;
    uint32_t offset = 0;

    // Search for a gap or the end
    while (offset < BLK_SIZE) {
        if (entry->inode == 0 && (offset == 0 || entry->rec_len == 0)) {
            // Empty block or first entry
            entry->inode = child_ino;
            entry->rec_len = BLK_SIZE - offset;
            entry->file_type = type;
            int nlen = 0; while (name[nlen]) { entry->name[nlen] = name[nlen]; nlen++; }
            entry->name_len = nlen;
            disk_write(inode.i_block[last_blk_idx], block);
            return;
        }

        uint32_t actual_used = (8 + entry->name_len + 3) & ~3;
        if (entry->rec_len > actual_used + 12) { // enough space for a new entry (min ~12 bytes)
            uint32_t old_rec_len = entry->rec_len;
            entry->rec_len = actual_used;
            
            offset += actual_used;
            entry = (ext4_dir_entry_t*)(block + offset);
            entry->inode = child_ino;
            entry->rec_len = old_rec_len - actual_used;
            entry->file_type = type;
            int nlen = 0; while (name[nlen]) { entry->name[nlen] = name[nlen]; nlen++; }
            entry->name_len = nlen;
            disk_write(inode.i_block[last_blk_idx], block);
            return;
        }
        
        offset += entry->rec_len;
        if (offset >= BLK_SIZE) break;
        entry = (ext4_dir_entry_t*)(block + offset);
    }
    // If no space, one could allocate a new block, but we simplify
}

// --- Main API ---

void ext4_mount(void) {
    // 1. Clear disk
    for (int i = 0; i < DISK_BLOCKS * BLK_SIZE; i++) disk[i] = 0;

    // 2. Superblock (Block 1)
    ext4_super_block_t sb = {0};
    sb.s_magic = EXT4_SB_MAGIC;
    sb.s_inodes_count = 128;
    sb.s_blocks_count = DISK_BLOCKS;
    sb.s_free_blocks_count = DISK_BLOCKS - 10; // Reserve some
    sb.s_free_inodes_count = 127;
    sb.s_first_data_block = 1;
    sb.s_log_block_size = 0; // 1024 << 0 = 1024
    sb.s_blocks_per_group = 8192;
    sb.s_inodes_per_group = 128;
    set_sb(&sb);

    // 3. Group Descriptor (Block 2)
    ext4_group_desc_t gd = {0};
    gd.bg_block_bitmap = 3;
    gd.bg_inode_bitmap = 4;
    gd.bg_inode_table = 5;
    gd.bg_free_blocks_count = sb.s_free_blocks_count;
    gd.bg_free_inodes_count = sb.s_free_inodes_count; 
    set_gd(&gd);

    // 4. Mark reserved blocks in bitmap
    uint8_t bmap[BLK_SIZE] = {0};
    for(int i=0; i<10; i++) bit_set(bmap, i);
    disk_write(3, bmap);
    uint8_t imap[BLK_SIZE] = {0};
    bit_set(imap, 0); // reserved
    bit_set(imap, 1); // root inode 2 (index 1)
    disk_write(4, imap);

    // 5. Create Root Inode
    ext4_inode_t root = {0};
    root.i_mode = 0x4000 | 0755; // DIR
    root.i_size = BLK_SIZE;
    root.i_links_count = 2;
    root.i_block[0] = alloc_block();
    set_inode(EXT4_ROOT_INODE, &root);

    // 6. Dot and DotDot for root
    add_to_dir(EXT4_ROOT_INODE, EXT4_ROOT_INODE, ".", 2);
    add_to_dir(EXT4_ROOT_INODE, EXT4_ROOT_INODE, "..", 2);

    current_dir_inode = EXT4_ROOT_INODE;
    terminal_writeln("EXT4: Real-structured FS mounted (RAM disk)");
}

void ext4_mkdir(const char* name) {
    uint32_t ino = alloc_inode();
    ext4_inode_t node = {0};
    node.i_mode = 0x4000 | 0755;
    node.i_size = BLK_SIZE;
    node.i_links_count = 2;
    node.i_block[0] = alloc_block();
    set_inode(ino, &node);
    
    add_to_dir(current_dir_inode, ino, name, 2);
    add_to_dir(ino, ino, ".", 2);
    add_to_dir(ino, current_dir_inode, "..", 2);
}

void ext4_write(const char* name, const char* data, int append) {
    uint32_t ino = find_in_dir(current_dir_inode, name);
    ext4_inode_t node;
    if (!ino) {
        ino = alloc_inode();
        node.i_mode = 0x8000 | 0644;
        node.i_size = 0;
        node.i_block[0] = alloc_block();
        add_to_dir(current_dir_inode, ino, name, 1);
    } else {
        get_inode(ino, &node);
        if (node.i_mode & 0x4000) return; // Is dir
    }

    uint8_t blk_data[BLK_SIZE];
    uint32_t start = append ? node.i_size : 0;
    int len = 0; while(data[len]) len++;

    disk_read(node.i_block[0], blk_data);
    for (int i = 0; i < len && (start+i) < BLK_SIZE; i++) {
        blk_data[start + i] = data[i];
    }
    node.i_size = start + len;
    disk_write(node.i_block[0], blk_data);
    set_inode(ino, &node);
}

void ext4_ls(void) {
    ext4_inode_t inode;
    get_inode(current_dir_inode, &inode);
    uint8_t block[BLK_SIZE];
    for (int i = 0; i < 12 && inode.i_block[i]; i++) {
        disk_read(inode.i_block[i], block);
        ext4_dir_entry_t* entry = (ext4_dir_entry_t*)block;
        uint32_t offset = 0;
        while (offset < BLK_SIZE && entry->rec_len > 0) {
            if (entry->inode != 0) {
                for (int k = 0; k < entry->name_len; k++) terminal_putc(entry->name[k]);
                if (entry->file_type == 2) terminal_putc('/');
                terminal_write("  ");
            }
            offset += entry->rec_len;
            entry = (ext4_dir_entry_t*)(block + offset);
        }
    }
    terminal_putc('\n');
}

void ext4_touch(const char* name, const char* content) {
    uint32_t ino = alloc_inode();
    ext4_inode_t node = {0};
    node.i_mode = 0x8000 | 0644; // FILE
    node.i_block[0] = alloc_block();
    int len = 0;
    while(content[len]) len++;
    node.i_size = len;
    set_inode(ino, &node);
    
    uint8_t data[BLK_SIZE] = {0};
    for(int i=0; i<len && i<BLK_SIZE; i++) data[i] = content[i];
    disk_write(node.i_block[0], data);
    
    add_to_dir(current_dir_inode, ino, name, 1);
}

void ext4_cat(const char* name) {
    uint32_t ino = find_in_dir(current_dir_inode, name);
    if (!ino) { terminal_writeln("Error: not found"); return; }
    ext4_inode_t node;
    get_inode(ino, &node);
    if (node.i_mode & 0x4000) { terminal_writeln("Error: is dir"); return; }
    
    uint8_t data[BLK_SIZE];
    uint32_t remaining = node.i_size;
    for (int i = 0; i < 12 && node.i_block[i] && remaining > 0; i++) {
        disk_read(node.i_block[i], data);
        uint32_t to_print = remaining > BLK_SIZE ? BLK_SIZE : remaining;
        for (uint32_t k = 0; k < to_print; k++) terminal_putc(data[k]);
        remaining -= to_print;
    }
    terminal_putc('\n');
}

int ext4_cd(const char* path) {
    if (path[0] == '/') { current_dir_inode = EXT4_ROOT_INODE; path++; }
    if (!*path) return 0;

    // Naive split-and-step
    char part[32];
    int p = 0;
    while (*path) {
        p = 0;
        while (*path && *path != '/') part[p++] = *path++;
        part[p] = 0;
        if (p > 0) {
            uint32_t next = find_in_dir(current_dir_inode, part);
            if (!next) return -1;
            ext4_inode_t node;
            get_inode(next, &node);
            if (!(node.i_mode & 0x4000)) return -1;
            current_dir_inode = next;
        }
        if (*path == '/') path++;
    }
    return 0;
}

void ext4_rm(const char* name) {
    uint32_t ino = find_in_dir(current_dir_inode, name);
    if (!ino) return;
    
    // Simplification for RM: find entry in parent and zero out its inode
    ext4_inode_t pnode;
    get_inode(current_dir_inode, &pnode);
    uint8_t block[BLK_SIZE];
    for (int i = 0; i < 12 && pnode.i_block[i]; i++) {
        disk_read(pnode.i_block[i], block);
        ext4_dir_entry_t* entry = (ext4_dir_entry_t*)block;
        uint32_t offset = 0;
        while (offset < BLK_SIZE && entry->rec_len > 0) {
            if (entry->inode == ino) {
                entry->inode = 0;
                disk_write(pnode.i_block[i], block);
                return;
            }
            offset += entry->rec_len;
            entry = (ext4_dir_entry_t*)(block + offset);
        }
    }
}

static void tree_recursive(uint32_t dir_ino, int depth) {
    ext4_inode_t inode;
    get_inode(dir_ino, &inode);
    uint8_t block[BLK_SIZE];
    for (int i = 0; i < 12 && inode.i_block[i]; i++) {
        disk_read(inode.i_block[i], block);
        ext4_dir_entry_t* entry = (ext4_dir_entry_t*)block;
        uint32_t offset = 0;
        while (offset < BLK_SIZE && entry->rec_len > 0) {
            char name[256];
            for (int k = 0; k < entry->name_len; k++) name[k] = entry->name[k];
            name[entry->name_len] = 0;
            
            if (entry->inode != 0 && !streq(name, ".") && !streq(name, "..")) {
                for (int d = 0; d < depth; d++) terminal_write("  ");
                terminal_write("|- ");
                terminal_write(name);
                terminal_putc('\n');
                if (entry->file_type == 2) {
                    tree_recursive(entry->inode, depth + 1);
                }
            }
            offset += entry->rec_len;
            entry = (ext4_dir_entry_t*)(block + offset);
        }
    }
}

void ext4_tree(void) {
    terminal_writeln(".");
    tree_recursive(current_dir_inode, 0);
}
