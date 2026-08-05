#include "fs_render.h"
#include "fs_browser.h"
#include "fonts.h"
#include "stdio.h"
#include "stm32412g_discovery_lcd.h"
#include "csv_render.h" // some fuctions should be moved to general render.h 

FS_Entry entries_buff[MAX_ENTRIES];
extern char current_path[MAX_PATH_LEN];

void display_fs_browser(uint8_t selected_entry) {
    BSP_LCD_SetFont(&Font16);
    if (!fs_browser_mount()) {
        // Handle error if needed
        display_error("Failed to mount SD card");
        return;
    }

    fs_list_dir("/", entries_buff);
    for (uint8_t i = 0; i < MAX_ENTRIES; i++) {
        if (entries_buff[i].name[0] == '\0') {
            break; 
        }

        if (i == selected_entry) {
            BSP_LCD_SetBackColor(LCD_COLOR_DARKGRAY);
        }
        else {
            BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
        }

        char display_str[MAX_PATH_LEN + 2];
        snprintf(display_str, sizeof(display_str), "%s%s", entries_buff[i].name, entries_buff[i].is_dir ? "/" : "");
        BSP_LCD_DisplayStringAtLine(i, (uint8_t*)display_str);
    }
}