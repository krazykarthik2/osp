#include "ext4.h"
#include "serial.h"

static uint8_t* disk = (uint8_t*)0x400000;
static int cur_dir_ino = 2;

static int streq(const char* a, const char* b) {
    while (*a && (*a == *b)) { a++; b++; }
    return *a == *b;
}

static void bzero(void* p, int s) {
    uint8_t* b = (uint8_t*)p;
    for(int i=0; i<s; i++) b[i]=0;
}

static uint8_t* get_in_ptr(uint32_t ino) {
    return &disk[5120 + (ino-1)*128];
}

static uint32_t alloc_b() {
    uint8_t* bmap = &disk[3072]; 
    for(int i=12; i<1024; i++) {
        if(!(bmap[i/8] & (1<<(i%8)))) {
            bmap[i/8] |= (1<<(i%8));
            serial_write("EXT4: allocated bid="); 
            serial_putc((i/10)+'0'); serial_putc((i%10)+'0'); serial_write("\n");
            return i;
        }
    }
    return 0;
}

static uint32_t alloc_i() {
    uint8_t* imap = &disk[4096];
    for(int i=3; i<128; i++) {
        if(!(imap[i/8] & (1<<(i%8)))) {
            imap[i/8] |= (1<<(i%8));
            serial_write("EXT4: allocated ino="); 
            serial_putc((i+1)+'0'); serial_write("\n");
            return i+1;
        }
    }
    return 0;
}

void ext4_mount(void) {
    for(int i=0; i<32*1024; i++) disk[i]=0;
    disk[1024 + 56] = 0x53; disk[1024 + 57] = 0xEF;
    uint32_t* gd = (uint32_t*)&disk[2048];
    gd[0] = 3; gd[1] = 4; gd[2] = 5;
    disk[3072] = 0xFF; disk[3072+1] = 0x0F;
    disk[4096] = 0x03;
    uint8_t* in2 = get_in_ptr(2);
    ((uint32_t*)in2)[0] = 0x41ED; ((uint32_t*)in2)[1] = 1024; ((uint32_t*)in2)[10] = 10;
    uint8_t* d10 = &disk[10*1024]; bzero(d10, 1024);
    d10[0]=2; d10[4]=12; d10[6]=1; d10[7]=2; d10[8]='.';
    // .. entry: inode=2, rec_len=1012 (completes 1024), name_len=2, type=2, name=".."
    d10[12]=2; d10[12+4]=(1012&0xFF); d10[12+5]=((1012>>8)&0xFF); d10[12+6]=2; d10[12+7]=2; d10[12+8]='.'; d10[12+9]='.';
    cur_dir_ino = 2;
    serial_write("EXT4: Real FS Mounted\n");
}

static uint32_t find(uint32_t dino, const char* name) {
    uint8_t* in = get_in_ptr(dino);
    uint32_t bid = ((uint32_t*)in)[10];
    uint8_t* blk = &disk[bid*1024];
    uint32_t off = 0;
    while(off < 1024 - 8) {
        uint32_t ino = *(uint32_t*)(blk + off);
        uint16_t rec = *(uint16_t*)(blk + off + 4);
        uint8_t nl = *(uint8_t*)(blk + off + 6);
        if(rec < 8 || off + rec > 1024) break;
        if(ino && nl > 0) {
            char buf[256]; int k; for(k=0; k<nl && k<255; k++) buf[k]=blk[off+8+k]; buf[k]=0;
            if(streq(buf, name)) return ino;
        }
        off += rec;
    }
    return 0;
}

static void add_entry(uint32_t dino, uint32_t cino, const char* name, uint8_t type) {
    serial_write("EXT4: add_entry debug\n");
    uint8_t* in = get_in_ptr(dino);
    uint32_t bid = ((uint32_t*)in)[10];
    uint8_t* blk = &disk[bid*1024];
    uint32_t off = 0;
    while(off < 1024 - 8) {
        uint32_t eino = *(uint32_t*)(blk + off);
        uint16_t erec = *(uint16_t*)(blk + off + 4);
        uint8_t enl = *(uint8_t*)(blk + off + 6);
        uint32_t used = (8 + enl + 3) & ~3;
        
        // Split entry logic
        if(eino != 0 && erec >= used + 12) {
            uint32_t nl_new = 0; while(name[nl_new]) nl_new++;
            uint32_t needed = (8 + nl_new + 3) & ~3;
            if(erec >= used + needed) {
                uint32_t rem = erec - used; *(uint16_t*)(blk+off+4) = used; 
                off += used;
                *(uint32_t*)(blk+off) = cino; *(uint16_t*)(blk+off+4) = rem; blk[off+7]=type;
                int k=0; while(name[k]) { blk[off+8+k]=name[k]; k++; } blk[off+6]=k; return;
            }
        }
        
        // Zero entry logic (hole)
        if(eino == 0 && erec >= 8) {
            uint32_t nl_new = 0; while(name[nl_new]) nl_new++;
            uint32_t needed = (8 + nl_new + 3) & ~3;
            if(erec >= needed) {
                *(uint32_t*)(blk+off) = cino; blk[off+7]=type;
                int k=0; while(name[k]) { blk[off+8+k]=name[k]; k++; } blk[off+6]=k; return;
            }
        }
        
        if(erec < 8) break;
        off += erec;
    }
    serial_write("EXT4: add_entry FAILED (no room!)\n");
}

void ext4_mkdir(const char* n) {
    uint32_t i = alloc_i(); uint8_t* in = get_in_ptr(i); bzero(in, 128);
    ((uint32_t*)in)[0] = 0x41ED; ((uint32_t*)in)[1] = 1024;
    uint32_t b = alloc_b(); ((uint32_t*)in)[10] = b;
    add_entry(cur_dir_ino, i, n, 2);
    uint8_t* d = &disk[b*1024]; bzero(d, 1024);
    d[0]=i&0xFF; d[1]=(i>>8)&0xFF; d[4]=12; d[6]=1; d[7]=2; d[8]='.';
    d[12]=cur_dir_ino&0xFF; d[13]=(cur_dir_ino>>8)&0xFF; d[12+4]=232; d[12+5]=3; d[12+6]=2; d[12+7]=2; d[12+8]='.'; d[12+9]='.';
}

void ext4_write(const char* n, const char* data, int append) {
    serial_write("EXT4: write file="); serial_write(n ? n : "NULL"); serial_write("\n");
    if(!n) return;
    uint32_t i = find(cur_dir_ino, n);
    if(!i) {
        serial_write("EXT4: allocating inode\n");
        i = alloc_i(); 
        serial_write("EXT4: clearing inode\n");
        uint8_t* in = get_in_ptr(i); bzero(in, 128);
        ((uint32_t*)in)[0] = 0x81A4; 
        serial_write("EXT4: allocating data block\n");
        ((uint32_t*)in)[10] = alloc_b();
        serial_write("EXT4: adding dir entry\n");
        add_entry(cur_dir_ino, i, (char*)n, 1);
        serial_write("EXT4: add_entry finished\n");
    }
    uint8_t* in = get_in_ptr(i);
    uint32_t bid = ((uint32_t*)in)[10]; uint32_t size = ((uint32_t*)in)[1];
    int start = append ? size : 0;
    serial_write("EXT4: writing data to bid="); 
    serial_putc((bid/10)+'0'); serial_putc((bid%10)+'0'); serial_write("\n");
    int k=0; while(data[k] && k < 4000) { 
        if(bid*1024 + start + k < 1024*1024) disk[bid*1024 + start + k]=data[k]; 
        k++; 
    }
    ((uint32_t*)in)[1] = start + k;
    serial_write("EXT4: write finished\n");
}

void ext4_ls(void) {
    uint8_t* in = get_in_ptr(cur_dir_ino); uint32_t bid = ((uint32_t*)in)[10];
    uint8_t* blk = &disk[bid*1024]; uint32_t off = 0;
    while(off < 1024) {
        uint32_t ino = *(uint32_t*)(blk + off); uint16_t rec = *(uint16_t*)(blk + off + 4);
        uint8_t nl = *(uint8_t*)(blk + off + 6);
        if(rec < 8) break;
        if(ino) { for(int k=0; k<nl; k++) serial_putc(blk[off+8+k]); serial_write("  "); }
        off += rec;
    }
    serial_write("\n");
}

void ext4_cat(const char* n) {
    uint32_t i = find(cur_dir_ino, n); if(!i) { serial_write("File not found\n"); return; }
    uint8_t* in = get_in_ptr(i); uint32_t size = ((uint32_t*)in)[1];
    uint32_t bid = ((uint32_t*)in)[10];
    uint8_t* blk = &disk[bid*1024];
    for(uint32_t k=0; k<size; k++) serial_putc(blk[k]);
    serial_write("\n");
}

int ext4_cd(const char* p) {
    if(streq(p, "/")) { cur_dir_ino = 2; return 0; }
    if(streq(p, "..")) { uint32_t i = find(cur_dir_ino, ".."); if(i) cur_dir_ino=i; return 0; }
    uint32_t i = find(cur_dir_ino, p); if(!i) return -1;
    uint8_t* in = get_in_ptr(i); if(!(((uint32_t*)in)[0] & 0x4000)) return -1;
    cur_dir_ino = i; return 0;
}

void ext4_rm(const char* n) {
    uint8_t* in = get_in_ptr(cur_dir_ino); uint32_t bid = ((uint32_t*)in)[10];
    uint8_t* blk = &disk[bid*1024]; uint32_t off = 0;
    while(off < 1024) {
        uint32_t ino = *(uint32_t*)(blk + off); uint16_t rec = *(uint16_t*)(blk + off + 4);
        uint8_t nl = *(uint8_t*)(blk + off + 6);
        if(rec < 8) break;
        char buf[256]; int k; for(k=0; k<nl && k<255; k++) buf[k]=blk[off+8+k]; buf[k]=0;
        if(streq(buf, n)) { *(uint32_t*)(blk+off) = 0; return; }
        off += rec;
    }
}
void ext4_tree(void) { ext4_ls(); }
void ext4_touch(const char* n, const char* c) { ext4_write(n, c, 0); }
