/*
 * csv_render.c
 *
 *  Created on: 4 июл. 2026 г.
 *      Author: artem
 */

#include "csv_render.h"

#define OFFSET_WORD 5
#define OFFSET_LINE 25

void init_lcd() {
    BSP_LCD_Init();
	BSP_LCD_SetFont(&LCD_DEFAULT_FONT);

	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_Clear(LCD_COLOR_WHITE);

	BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
}

// print to lcd
void render_table_to_lcd(Table* table) {

	// print headers
	int cur = 0;
    for (int i = 0; i < table->col_count; i++) {
        BSP_LCD_DisplayStringAt(cur, 2, (uint8_t*)table->col_names[i], LEFT_MODE);
        cur += strlen(table->col_names[i]) * LCD_DEFAULT_FONT.Width + OFFSET_WORD; // 5 - OFFSET
    }

    // print body
    int curX = 0;
    int curY = OFFSET_LINE;
    char lcd_buffer[MAX_LEN_LINE];
    for (int i = 0; i < table->row_count; i++) {
        sprintf(lcd_buffer, "%d", table->row_ids[i]);
        BSP_LCD_DisplayStringAt(curX, curY, (uint8_t*) lcd_buffer, LEFT_MODE);
        curX += digit_count(table->row_ids[i]) * LCD_DEFAULT_FONT.Width + OFFSET_WORD;
        for (int j = 0; j < table->col_count; j++) {
            if (table->grid[i * table->col_count + j].state == CSV_ERROR) {
                BSP_LCD_DisplayStringAt(curX, curY, (uint8_t*)", #ERROR", LEFT_MODE);
                curX += strlen(", #ERROR") * LCD_DEFAULT_FONT.Width + OFFSET_WORD;
            }
            else if (table->grid[i * table->col_count + j].state == EMPTY) {
            	BSP_LCD_DisplayStringAt(curX, curY, (uint8_t*)",", LEFT_MODE);
            	curX += LCD_DEFAULT_FONT.Width + OFFSET_WORD;

            } else {
            	sprintf(lcd_buffer, ", %d", table->grid[i * table->col_count + j].value);

            	BSP_LCD_DisplayStringAt(curX, curY, (uint8_t *)lcd_buffer, LEFT_MODE);
            	curX += (2+digit_count(table->grid[i * table->col_count + j].value))* LCD_DEFAULT_FONT.Width + OFFSET_WORD;
            }
        }
        curY += OFFSET_LINE;
        curX = 0;
    }
}


void display_error(const char* error_text) {
	BSP_LCD_SetTextColor(LCD_COLOR_RED);
	BSP_LCD_Clear(LCD_COLOR_WHITE);
	BSP_LCD_DisplayStringAt(0, 2, (uint8_t*) error_text, CENTER_MODE);
}
