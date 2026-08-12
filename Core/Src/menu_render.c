#include "menu_render.h"
#include "stm32412g_discovery_lcd.h"

void init_lcd() {
    BSP_LCD_Init();
	BSP_LCD_SetFont(&LCD_DEFAULT_FONT);

	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_Clear(LCD_COLOR_WHITE);

	BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
}

void display_main_menu(MenuParams *menu) {
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_Clear(LCD_COLOR_WHITE);

	BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);

    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 0, (uint8_t*) "CSV BARE METAL TOOL", CENTER_MODE);

    uint16_t offsetY = Font16.Height;
    BSP_LCD_SetFont(&LCD_DEFAULT_FONT);

    if (menu->total_tables+2 > SCREEN_HEIGHT / OFFSET_LINE) {
        menu->total_tables = SCREEN_HEIGHT / OFFSET_LINE - 2;
    }
    char buf[16];
    for (int i = 0; i < menu->total_tables; i++) {
        sprintf(buf, "Table %d", i+1);
        if (i == menu->selected_table) {
            BSP_LCD_SetBackColor((uint16_t)0x74FA);
        }
        BSP_LCD_DisplayStringAt(0, offsetY + OFFSET_LINE*i, (uint8_t*)buf, CENTER_MODE);
        BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    }

    if (menu->selected_table == menu->total_tables) {
        BSP_LCD_SetBackColor((uint16_t)0x74FA);
    }
    else {
        BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    }

    BSP_LCD_DisplayStringAt(0, offsetY + OFFSET_LINE*(menu->total_tables), (uint8_t*)"Load SD card", CENTER_MODE);
}

void display_error(const char* error_text) {
    BSP_LCD_Clear(LCD_COLOR_WHITE);
	BSP_LCD_SetTextColor(LCD_COLOR_RED);
	BSP_LCD_DisplayStringAt(0, 
                            0, 
                            (uint8_t*) error_text, 
                            CENTER_MODE);
    BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
}
