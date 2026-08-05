#ifndef FS_BROWSER_H
#define FS_BROWSER_H

#include <stdint.h>
#include "fatfs.h"


#define MAX_ENTRIES (32)
#define MAX_PATH_LEN (128)

typedef struct {
    char name[13]; // 8.3 filename format
    uint8_t is_dir;
} FS_Entry;

uint8_t fs_browser_mount();
uint8_t fs_list_dir(const char* path, FS_Entry* entries);

uint8_t fs_path_append(char* base_path, const char* sub_path);
void fs_path_remove_last(char* path);
#endif // FS_BROWSER_H