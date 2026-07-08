/*
 * csv_render.c
 *
 *  Created on: 4 июл. 2026 г.
 *      Author: artem
 */

#include "csv_render.h"

#define OFFSET_WORD 12
#define OFFSET_LINE 24

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

void init_lcd() {
    BSP_LCD_Init();
	BSP_LCD_SetFont(&LCD_DEFAULT_FONT);

	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_Clear(LCD_COLOR_WHITE);

	BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
}

// print to lcd
void render_table_to_lcd(Table* table, int start_row, int start_col, int selected_row, int selected_col) {
    BSP_LCD_Clear(LCD_COLOR_WHITE);
	// print headers
	int curX = OFFSET_WORD;
    for (int i = start_col; i < table->col_count; i++) {
        if (selected_row == -1 && selected_col == i) {
            BSP_LCD_SetBackColor(LCD_COLOR_GREEN);
        }

        BSP_LCD_DisplayStringAt(curX, 0, (uint8_t*)table->col_names[i], 0);
        BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
        curX += strlen(table->col_names[i]) * LCD_DEFAULT_FONT.Width + OFFSET_WORD;
    }

    // print body
    curX = 0;
    int curY = OFFSET_LINE;
    char lcd_buffer[MAX_LEN_LINE];
    for (int i = start_row; i < table->row_count; i++) {
        sprintf(lcd_buffer, "%d", table->row_ids[i]);
        BSP_LCD_DisplayStringAt(curX, curY, (uint8_t*) lcd_buffer, 0);
        curX += digit_count(table->row_ids[i]) * LCD_DEFAULT_FONT.Width;
        for (int j = start_col; j < table->col_count; j++) {
            if (selected_row == i && selected_col == j) {
                BSP_LCD_SetBackColor(LCD_COLOR_GREEN);
            }
            else {
                BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
            }

            if (table->grid[i * table->col_count + j].state == CSV_ERROR) {
                BSP_LCD_DisplayStringAt(curX, curY, (uint8_t*)", #ERROR", 0);
                curX += strlen(", #ERROR") * LCD_DEFAULT_FONT.Width + OFFSET_WORD;
            }
            else if (table->grid[i * table->col_count + j].state == EMPTY) {
            	BSP_LCD_DisplayStringAt(curX, curY, (uint8_t*)",", 0);
            	curX += LCD_DEFAULT_FONT.Width + OFFSET_WORD;

            } else {
            	sprintf(lcd_buffer, ", %d", table->grid[i * table->col_count + j].value);

            	BSP_LCD_DisplayStringAt(curX, curY, (uint8_t *)lcd_buffer, 0);
            	curX += (2+digit_count(table->grid[i * table->col_count + j].value))* LCD_DEFAULT_FONT.Width;
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
    BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
}