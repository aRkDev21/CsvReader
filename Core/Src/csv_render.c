/*
 * csv_render.c
 *
 *  Created on: 4 июл. 2026 г.
 *      Author: artem
 */

#include "csv_render.h"

#define FONT_SIZE 12
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

void update_viewport(int selected_row, int selected_col, int* start_row, int* start_col, Table* table) {

    if (selected_row < 0) {
        selected_row++;
    }

    if (selected_col < 0) {
        selected_col++;
    }

    if (selected_row < *start_row) {
        *start_row = selected_row;
    } else if (selected_row >= *start_row + (SCREEN_HEIGHT / OFFSET_LINE)) {
        // if we cant display the selected column 
        *start_row = selected_row - (SCREEN_HEIGHT / OFFSET_LINE) + 1;
    }

    if (selected_col < *start_col ) {
        *start_col = selected_col;
    }
    else {
    // if we cant display the selected row
        while (len_header(table, *start_col, selected_col) * FONT_SIZE >= SCREEN_WIDTH) {
            (*start_col)++;
        }
        
    }
    
}
// print to lcd
void render_table_to_lcd(Table* table, int start_row, int start_col, int selected_row, int selected_col) {
    BSP_LCD_Clear(LCD_COLOR_WHITE);
	// print headers
	int curX = FONT_SIZE;
    for (int i = start_col; i < table->col_count; i++) {
        if (selected_row == -1 && selected_col == i) {
            BSP_LCD_SetBackColor(LCD_COLOR_GREEN);
        }

        BSP_LCD_DisplayStringAt(curX, 0, (uint8_t*)table->col_names[i], 0);
        BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
        curX += strlen(table->col_names[i]) * LCD_DEFAULT_FONT.Width + FONT_SIZE;

        if (curX >= SCREEN_WIDTH) {
            break;
        }
    }

    // print body
    curX = 0;
    int curY = OFFSET_LINE;
    char lcd_buffer[MAX_LEN_FIELD + 1];
    for (int i = start_row; i < table->row_count; i++) {
        // print row id
        if (selected_row == i && selected_col == -1) {
            BSP_LCD_SetBackColor(LCD_COLOR_GREEN);
        }
        sprintf(lcd_buffer, "%d", table->row_ids[i]);
        BSP_LCD_DisplayStringAt(curX, curY, (uint8_t*) lcd_buffer, 0);
        BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
        
        curX += digit_count(table->row_ids[i]) * LCD_DEFAULT_FONT.Width;
        for (int j = start_col; j < table->col_count; j++) {
            if (curX >= SCREEN_WIDTH) {
                break;
            }
            if (table->grid[i * table->col_count + j].state == CSV_ERROR || table->grid[i * table->col_count + j].state == EMPTY) {
                // process error or empty cell
                BSP_LCD_DisplayStringAt(curX, curY, (uint8_t*)", ", 0);
                curX += 2 * LCD_DEFAULT_FONT.Width;

                if (selected_row == i && selected_col == j) {
                    BSP_LCD_SetBackColor(LCD_COLOR_GREEN);
                }

                uint8_t* str = (table->grid[i * table->col_count + j].state == CSV_ERROR) ? (uint8_t*)"#ERROR" : (uint8_t*)"#EMPTY";
                if (str == (uint8_t*)"#ERROR") {
                    BSP_LCD_SetTextColor(LCD_COLOR_RED);
                }
                else {
                    BSP_LCD_SetTextColor(LCD_COLOR_ORANGE);
                }
                BSP_LCD_DisplayStringAt(curX, curY, str, 0);
                curX += strlen((char*)str) * LCD_DEFAULT_FONT.Width;
                BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
                BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
            }
            
            else {
                BSP_LCD_DisplayStringAt(curX, curY, (uint8_t*)",", 0);
                curX += LCD_DEFAULT_FONT.Width*2;
                if (selected_row == i && selected_col == j) {
                    BSP_LCD_SetBackColor(LCD_COLOR_GREEN);
                }

            	sprintf(lcd_buffer, "%d", table->grid[i * table->col_count + j].value);

            	BSP_LCD_DisplayStringAt(curX, curY, (uint8_t *)lcd_buffer, 0);
            	curX += (digit_count(table->grid[i * table->col_count + j].value))* LCD_DEFAULT_FONT.Width;
                BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
            }
        }
        curY += OFFSET_LINE;
        curX = 0;
    }
}


void display_error(const char* error_text) {
	BSP_LCD_SetTextColor(LCD_COLOR_RED);
	BSP_LCD_Clear(LCD_COLOR_WHITE);
	BSP_LCD_DisplayStringAt(0, 
                            SCREEN_HEIGHT/2 - LCD_DEFAULT_FONT.Height/2, 
                            (uint8_t*) error_text, 
                            CENTER_MODE);
    BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
}