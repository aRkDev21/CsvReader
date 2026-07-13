/*
 * csv_render.c
 *
 *  Created on: 4 июл. 2026 г.
 *      Author: artem
 */

#include "csv_render.h"
#include "csv.h"
#include "stm32412g_discovery_lcd.h"
#include <stdint.h>

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

int get_size_cell(Cell* cell) {
    int size = 0;
    if (cell->state == DONE) {
        size = digit_count(cell->value);
    }
    else {
        size = 5; // #ERROR or #EMPTY
    }
    return size;
}

int get_max_col_len(Table* table, int start_row, int col) {
    int mx = 0;
    int cur = 0;
    if (start_row == 0) {
        mx = strlen(table->col_names[col]);
    }
    for (int i = start_row; i < table->row_count; i++) {
        cur = (col == -1) ? digit_count(table->row_ids[i]) : get_size_cell(&table->grid[i*table->col_count + col]);
        if (mx < cur) mx = cur;
    }

    return mx;
}

void update_viewport(int selected_row, int selected_col, int* start_row, int* start_col, Table* table, bool* viewport_changed) {
    *viewport_changed = false;

    if (selected_row < *start_row && selected_row != -1) {
        *start_row = selected_row;
        *viewport_changed = true;
    } else if (selected_row >= *start_row + (SCREEN_HEIGHT / OFFSET_LINE)) {
        // if we cant display the selected column 
        *start_row = selected_row - (SCREEN_HEIGHT / OFFSET_LINE) + 1;
        *viewport_changed = true;
    }

    if (selected_col < *start_col  && selected_col != -1) {
        *start_col = selected_col;
        *viewport_changed = true;
    }
    else {
    // if we cant display the selected row
        while (len_row(table, selected_row, *start_col, selected_col) * FONT_SIZE >= SCREEN_WIDTH) {
            (*start_col)++;
            *viewport_changed = true;
        }   
        
    }

    if (*start_row == -1) *start_row = 0;
    if (*start_col == -1) *start_col = 0;
    
}

void draw_cell_value(Cell* cell, int *curX, int *curY) {
    if (cell->state == CSV_ERROR) {
        BSP_LCD_SetTextColor(LCD_COLOR_RED);
        BSP_LCD_DisplayStringAt(*curX, *curY, (uint8_t*)"#ERROR", 0);
        //*curX += strlen("#ERROR") * LCD_DEFAULT_FONT.Width;
    }

    else if (cell->state == EMPTY) {
        BSP_LCD_SetTextColor(LCD_COLOR_ORANGE);
        BSP_LCD_DisplayStringAt(*curX, *curY, (uint8_t*)"#EMPTY", 0);
        //*curX += strlen("#EMPTY") * LCD_DEFAULT_FONT.Width;
    }
    else {
        BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
        char lcd_buffer[MAX_LEN_FIELD + 1];
        sprintf(lcd_buffer, "%d", cell->value);
        BSP_LCD_DisplayStringAt(*curX, *curY, (uint8_t*)lcd_buffer, 0);
        //*curX += digit_count(cell->value) * LCD_DEFAULT_FONT.Width;
    }
    BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
}

void find_cell_pos(Table* t, int row, int col, int* x, int* y, int start_row, int start_col) {
    *x = 0;
    if (row == -1) {
        *y = 0;
    } 
    else {
        *y = (row - start_row) * OFFSET_LINE + OFFSET_LINE;
    }

    if (col == -1) {
        *x = 0;
        return;
    }

    if (start_col == 0)
        *x += get_max_col_len(t, start_row, -1) * LCD_DEFAULT_FONT.Width + FONT_SIZE;// row id width
    // else 
    //     *x = -LCD_DEFAULT_FONT.Width; // lol it works

    // int j = start_col;
    // while (j != col) {
    //     *x += LCD_DEFAULT_FONT.Width; // space
    //     Cell* cell = &t->grid[row * t->col_count + j];
    //     if (cell->state == CSV_ERROR || cell->state == EMPTY) {
    //         *x += strlen("#ERROR") * LCD_DEFAULT_FONT.Width; // #ERROR(5) or #EMPTY(5)
    //     }
    //     else {
    //         *x += digit_count(cell->value) * LCD_DEFAULT_FONT.Width;
    //     }
    //     j++;
    // }
    // *x += LCD_DEFAULT_FONT.Width; // space
    // *y = (row - start_row) * OFFSET_LINE + OFFSET_LINE;

    for (int j = start_col; j < col; j++) {
        *x += get_max_col_len(t, start_row, j) * LCD_DEFAULT_FONT.Width + FONT_SIZE;
    }
}

int draw_cell(Table* table, int row, int col, int *curX, int *curY, int start_row, int start_col) {
    find_cell_pos(table, row, col, curX, curY, start_row, start_col);
    if (*curX >= SCREEN_WIDTH) return 0;
    
    if (row == -1) {
        BSP_LCD_DisplayStringAt(*curX, *curY, (uint8_t*)table->col_names[col], 0);
        //*curX += strlen(table->col_names[col]) * LCD_DEFAULT_FONT.Width + FONT_SIZE;
    }

    else if (col == -1) {
        char lcd_buffer[MAX_LEN_FIELD + 1];
        sprintf(lcd_buffer, "%d", table->row_ids[row]);
        BSP_LCD_DisplayStringAt(*curX, *curY, (uint8_t*) lcd_buffer, 0);
        //*curX += digit_count(table->row_ids[row]) * LCD_DEFAULT_FONT.Width;
    } 

    else {
        Cell* cell = &table->grid[row * table->col_count + col];
        draw_cell_value(cell, curX, curY);
    }
    return 1;
}

// print to lcd
void render_table_to_lcd(Table* table, int start_row, int start_col) {
    BSP_LCD_Clear(LCD_COLOR_WHITE);
    // print headers
	int curX = 0;
    int curY = 0;
    BSP_LCD_SetBackColor(LCD_COLOR_DARKCYAN);
    for (int i = start_col; i < table->col_count; i++) {
        if (!draw_cell(table, -1, i, &curX, &curY, start_row, start_col)) {
            break;
        }
    }

    // print body
    curX = 0;
    curY = OFFSET_LINE;
    for (int i = start_row; i < table->row_count; i++) {
        // print row id
        if (start_col == 0){
            draw_cell(table, i, -1, &curX, &curY, start_row, start_col);
        }
        for (int j = start_col; j < table->col_count; j++) {
            if (!draw_cell(table, i, j, &curX, &curY, start_row, start_col)) {
                break;
            }
            //curX += LCD_DEFAULT_FONT.Width;
        }
        curY += OFFSET_LINE;
        curX = 0;
    }

    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
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


void unhighlight_cell(Table* table, int cur_row, int cur_col, int start_row, int start_col) {
    // remove highlight from the previous cell
    int curX = 0, curY = 0;
    BSP_LCD_SetBackColor(LCD_COLOR_DARKCYAN);
    draw_cell(table, cur_row, cur_col, &curX, &curY, start_row, start_col);
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
}

void highlight_cell(Table* table, int new_row, int new_col, int start_row, int start_col) {
    // highlight the new cell
    int curX = 0, curY = 0;
    BSP_LCD_SetBackColor(LCD_COLOR_GREEN);
    draw_cell(table, new_row, new_col, &curX, &curY, start_row, start_col);
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
}