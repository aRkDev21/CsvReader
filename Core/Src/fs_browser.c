#include "fs_browser.h"
#include "string.h"
#include "ff.h"

extern uint8_t retSD;
extern char SDPath[4];
extern FATFS SDFatFS;

uint8_t fs_browser_mount() {
    if (retSD != 0) {
        return 0;
    }
    
    FRESULT res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 1);
    return (res == FR_OK);
}

uint8_t fs_list_dir(const char* path, FS_Entry* entries) {
    DIR dir;
    FILINFO fno;
    FRESULT res;

    res = f_opendir(&dir, path);
    if (res != FR_OK) {
        return 1;
    }

    uint16_t count = 0;

    while (count < MAX_ENTRIES) {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == '\0') {
            break;
        }

        strncpy(entries[count].name, fno.fname, sizeof(entries[count].name) - 1);
        entries[count].name[sizeof(entries[count].name) - 1] = '\0';

        if (fno.fattrib & AM_DIR) {
            entries[count].is_dir = 1;
        } else {
            entries[count].is_dir = 0;
        }

        count++;
    }

    f_closedir(&dir);
    return 0;
}

void fs_path_append(char* base_path, const char* sub_path) {}
void fs_path_remove_last(char* path) {}