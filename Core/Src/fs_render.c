#include "fs_render.h"
#include "fs_browser.h"
#include "fonts.h"
#include "stdio.h"
#include "stm32412g_discovery_lcd.h"
#include "csv_render.h" // some fuctions should be moved to general_render.h 
#include <stdint.h>

FS_Entry entries_buff[MAX_ENTRIES];
extern char current_path[MAX_PATH_LEN];

uint16_t display_fs_browser(uint8_t selected_entry) {
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_Clear(LCD_COLOR_WHITE);

	BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);

    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 0, (uint8_t*) "SDCARD", CENTER_MODE); // write device name here
    BSP_LCD_SetFont(&LCD_DEFAULT_FONT);

    uint16_t count = fs_list_dir(current_path, entries_buff);

    for (uint16_t i = 0; i < count; i++) {
        if (entries_buff[i].name[0] == '\0') {
            break;
        }

        if (i == selected_entry) {
            BSP_LCD_SetBackColor((uint16_t)0x74FA);
        }
        else {
            BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
        }

        char display_str[MAX_PATH_LEN + 2];
        snprintf(display_str, sizeof(display_str), "%s%s", entries_buff[i].name, entries_buff[i].is_dir ? "/" : "");
        BSP_LCD_DisplayStringAtLine(i+1, (uint8_t*)display_str);
    }

    return count;
}