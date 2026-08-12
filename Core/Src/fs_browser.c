#include "fs_browser.h"
#include "string.h"
#include "ff.h"
#include <stddef.h>

extern uint8_t retSD;
extern char SDPath[4];
extern FATFS SDFatFS;
char current_path[MAX_PATH_LEN];

uint8_t fs_browser_mount() {
    if (retSD != 0) {
        return 0;
    }
    
    FRESULT res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 1);
    strcpy(current_path, SDPath);
    return (res == FR_OK);
}

uint16_t fs_list_dir(char* path, FS_Entry* entries) {
    DIR dir;
    FILINFO fno;
    FRESULT res;

    res = f_opendir(&dir, path);
    if (res != FR_OK) {
        return -1;
    }

    strcpy(entries[0].name, "..");
    entries[0].is_dir = 1;

    uint16_t count = 1;

    while (count < MAX_ENTRIES) {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == '\0') {
            entries[count].name[0] = '\0';
            break;
        }

        strcpy(entries[count].name, fno.fname);
        if (fno.fattrib & AM_DIR) {
            entries[count].is_dir = 1;
        } else {
            entries[count].is_dir = 0;
        }

        count++;
    }

    f_closedir(&dir);
    return count;
}

uint8_t fs_path_append(char* base_path, const char* sub_path) {
    size_t base_len = strlen(base_path);
    uint8_t need_slash = (base_len > 0 && base_path[base_len - 1] != '/') ? 1 : 0;
    // base / sub \0
    if (base_len + need_slash + strlen(sub_path) + 1 > MAX_PATH_LEN) {
        return 1;
    }
    if (need_slash) {
        base_path[base_len] = '/';
        base_path[++base_len] = '\0';
    }

    strcpy(&base_path[base_len], sub_path); 
    return 0;
}

void fs_path_remove_last(char* path) {
    size_t len = strlen(path);
    if (len < 3) return;

    long long i = (long long)len - 1;
    while (i > 0 && path[i] != '/') {
        i--;
    }

    if (i == 0 && path[0] == '/') {
        i++;
    }

    path[i] = '\0';
}
