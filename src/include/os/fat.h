#pragma once

#include <os/ata.h>
#include <ustdio.h>
#include <cstring>

typedef uint16_t ucs2char_t;

typedef struct __attribute__ ((packed)) {
    uint8_t jump[3];
    uint8_t oemstring[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t directory_entries;
    uint16_t total_sectors;
    uint8_t media_desc;
    uint16_t sectors_per_fat_16;
    uint16_t sectors_per_track;
    uint16_t head_count;
    uint32_t hidden_sectors;
    uint32_t media_sectors;
    union {
        struct __attribute__ ((packed)) {
            uint8_t drive_num;
            uint8_t nt_flags;
            uint8_t signature;
            uint32_t serial;
            uint8_t label[11];
            uint8_t sys_ident[8];
        } fat16_ebpb;
        struct __attribute__ ((packed)) {
            uint32_t sectors_per_fat;
            uint16_t flags;
            uint16_t fat_version;
            uint32_t root_cluster;
            uint16_t fsinfo_sector;
            uint16_t backup_boot_sector;
            uint8_t reserved[12];
            uint8_t drive_num;
            uint8_t nt_flags;
            uint8_t signature;
            uint32_t serial;
            uint8_t label[11];
            uint8_t sys_ident[8];
        } fat32_ebpb;
    };
} bpb;

typedef struct __attribute__ ((packed)) {
    uint8_t hour : 5;
    uint8_t minutes : 6;
    uint8_t seconds : 5;
} fat_time;

typedef struct __attribute__ ((packed)) {
    uint8_t year : 7;
    uint8_t month : 4;
    uint8_t day : 5;
} fat_date;
    
typedef union {
    struct __attribute__ ((packed)) {
        uint8_t file_name[11];
        uint8_t attributes;
        uint8_t nt_reserved;
        uint8_t dafuq;
        fat_time creation_time;
        fat_date creation_date;
        fat_date access_date;
        uint16_t cluster_hi;
        fat_time modification_time;
        fat_date modification_date;
        uint16_t cluster_lo;
        uint32_t file_size;
    } std;
    struct __attribute__ ((packed)) {
        uint8_t order;
        ucs2char_t name_p1[5];
        uint8_t attribute;
        uint8_t long_type;
        uint8_t checksum;
        ucs2char_t name_p2[6];
        uint16_t zero;
        ucs2char_t name_p3[2];
    } lfn;
} dir_entry;

enum class entry_type {
    file, directory, deleted
};

typedef struct {
    ucs2char_t actual_file_name[256];
    dir_entry dirent;
} parsed_dir_entry;

#define FAT_USE_MEMORY 0x10000
#define FAT_BLOCK_COUNT (FAT_USE_MEMORY / 0x200)
#define FAT_ENTRIES_IN_BLOCK (0x200 / sizeof(uint32_t))

typedef struct {
    uint32_t popularity;
    uint32_t index;
    uint32_t data[FAT_ENTRIES_IN_BLOCK];
} fat_block;

class fat {
private:
    uint8_t m_cluster_buffer[32768];
    fat_block m_fat[FAT_BLOCK_COUNT];
    bpb m_bpb;
    
public:
    fat() {
        uint8_t sec_buffer[512];
        ata::read_sectors(ata_bus::primary, ata_drive::master, sec_buffer, 1, 4096);
        m_bpb = *reinterpret_cast<bpb*>(sec_buffer);
        
        memset(m_fat, 0, FAT_BLOCK_COUNT * sizeof(fat_block));
    }
    
    int get_next_cluster(int cluster) {
        uint32_t fat_index = cluster / FAT_ENTRIES_IN_BLOCK + 1;
        
        uint32_t min_pop = 0xffffffff;
        int min_pop_blk = 0;
        for (int i = 0; i < FAT_BLOCK_COUNT; i++) {
            if (m_fat[i].index == fat_index) {
                m_fat[i].popularity++;
                return m_fat[i].data[cluster % FAT_ENTRIES_IN_BLOCK];
            }
            if (min_pop_blk < 0) continue;
            if (m_fat[i].index) {
                if (min_pop > m_fat[i].popularity) {
                    min_pop = m_fat[i].popularity;
                    min_pop_blk = i;
                }
            } else min_pop_blk = -i - 1;
        }
        if (min_pop_blk < 0) min_pop_blk = -min_pop_blk - 1;
        
        m_fat[min_pop_blk].index = fat_index;
        m_fat[min_pop_blk].popularity = 0;
        ata::read_sectors(ata_bus::primary, ata_drive::master, 
                          reinterpret_cast<uint8_t*>(m_fat[min_pop_blk].data), 
                          1, m_bpb.reserved_sectors + 4096 + fat_index - 1);
        return m_fat[min_pop_blk].data[cluster % FAT_ENTRIES_IN_BLOCK];
    }
    
    void read_file(int cluster, uint32_t size, uint8_t *buffer) {
        uint32_t cluster_size = m_bpb.sectors_per_cluster * 0x200;
        
        // stdio::printf("going cluster %d, writing to %x\n", cluster, (void*) buffer);
        if (cluster >= 0x0FFFFFF8) return;
        
        uint32_t data_offs = m_bpb.reserved_sectors + (m_bpb.fat_count * m_bpb.fat32_ebpb.sectors_per_fat);
        uint32_t sec = data_offs + (cluster - 2) * m_bpb.sectors_per_cluster + 4096;
        // stdio::printf("reading from sector %x\n", sec);
        
        ata::read_sectors(ata_bus::primary, ata_drive::master, m_cluster_buffer, m_bpb.sectors_per_cluster, sec);
        
        int cp_size = size > cluster_size ? cluster_size : size;
        
        // stdio::printf("copying %d bytes\n", cp_size);
        
        memcpy(buffer, m_cluster_buffer, cp_size);
        
        if (size > cluster_size) {
            size -= cluster_size;
            
            read_file(get_next_cluster(cluster), size, buffer + cluster_size);
        }
    }
    
    static bool namecmp(const char *path_name, const ucs2char_t *real_name, int name_len) {
        for (int i = 0; i < name_len; i++) {
            if (path_name[i] != real_name[i]) return false;
        }
        return !real_name[name_len];
    }
    
    void enumerate_directory(dir_entry *from, bool (*func)(parsed_dir_entry*)) {
        uint32_t cluster = from ? (from->std.cluster_hi << 16) | from->std.cluster_lo :
            m_bpb.fat32_ebpb.root_cluster;
        
        uint32_t data_offs = m_bpb.reserved_sectors + (m_bpb.fat_count * m_bpb.fat32_ebpb.sectors_per_fat);
        uint32_t root_dir_sec = data_offs + (cluster - 2) * m_bpb.sectors_per_cluster + 4096;
        
        ata::read_sectors(ata_bus::primary, ata_drive::master, m_cluster_buffer, m_bpb.sectors_per_cluster, root_dir_sec);
        
        dir_entry *entries = new dir_entry[m_bpb.sectors_per_cluster * 0x10];
        memcpy(entries, m_cluster_buffer, m_bpb.sectors_per_cluster * 0x200);
        
        ucs2char_t name_buf[256];
        bool has_lfn = false;
                
        for (int i = 0; i < m_bpb.sectors_per_cluster * 0x10; i++) {
            if (entries[i].std.attributes == 0x0f) {
                if (entries[i].lfn.order == 0xe5) continue;
                has_lfn = true;
                int ofs = (entries[i].lfn.order & 0x3f) - 1;
                memcpy(name_buf + ofs * 13, entries[i].lfn.name_p1, 10);
                memcpy(name_buf + ofs * 13 + 5, entries[i].lfn.name_p2, 12);
                memcpy(name_buf + ofs * 13 + 11, entries[i].lfn.name_p3, 4);
                continue;
            }
            if (entries[i].std.file_name[0] == 0) break;
            if (entries[i].std.file_name[0] == 0xe5 || entries[i].std.file_name[0] == 0x2e) {
                has_lfn = false;
                continue;
            }
            
            parsed_dir_entry pde;
            pde.dirent = entries[i];
            if (!has_lfn) {
                ucs2char_t *pname = pde.actual_file_name;
                for (int j = 0; (j < 8) && (entries[i].std.file_name[j] != ' '); j++)
                    *pname++ = entries[i].std.file_name[j];
                *pname++ = '.'; 
                for (int j = 0; (j < 3) && (entries[i].std.file_name[8 + j] != ' '); j++)
                    *pname++ = entries[i].std.file_name[8 + j];
                if (func(&pde)) break;
            } else {
                memcpy(pde.actual_file_name, name_buf, sizeof(pde.actual_file_name));
                if (func(&pde)) break;
            }
            
            has_lfn = false;
        }
        
        delete[] entries;
    }
    
    static const char *m_path;
    static parsed_dir_entry found_entry;
    
    static bool traverse_func(parsed_dir_entry* ent) {
        // stdio::printf("entry %s\n", ent->actual_file_name); 
        int len = 0;
        const char *path = m_path;
        while (*path && *path != '/') path++, len++;
        
        if (namecmp(m_path, ent->actual_file_name, len)) {
            found_entry = *ent;
            return true;
        }
        return false;
    }
    
    parsed_dir_entry traverse_directory(const char *path, parsed_dir_entry *traverse_from) {
        // stdio::printf("traversing path %s\n", path); 
        m_path = path;
        enumerate_directory(traverse_from ? &traverse_from->dirent : 0, traverse_func);
        
        while (*path && *path != '/') path++;
        if (*path) {
            path++;
            return traverse_directory(path, &found_entry);
        } else return found_entry;
    }
    
    uint32_t get_file_size(const char *file_name) {
        parsed_dir_entry entry = traverse_directory(file_name, 0);
        return entry.dirent.std.file_size;
    }
    
    void get_file(const char *file_name, uint8_t *buffer) {
        parsed_dir_entry entry = traverse_directory(file_name, 0);
        read_file((entry.dirent.std.cluster_hi << 16) | entry.dirent.std.cluster_lo, entry.dirent.std.file_size, buffer);
    }
};
