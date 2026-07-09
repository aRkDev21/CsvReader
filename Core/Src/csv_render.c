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

void draw_cell(Cell* cell, int *curX, int *curY) {
    if (cell->state == CSV_ERROR) {
        BSP_LCD_SetTextColor(LCD_COLOR_RED);
        BSP_LCD_DisplayStringAt(*curX, *curY, (uint8_t*)"#ERROR", 0);
        *curX += strlen("#ERROR") * LCD_DEFAULT_FONT.Width;
    }

    else if (cell->state == EMPTY) {
        BSP_LCD_SetTextColor(LCD_COLOR_ORANGE);
        BSP_LCD_DisplayStringAt(*curX, *curY, (uint8_t*)"#EMPTY", 0);
        *curX += strlen("#EMPTY") * LCD_DEFAULT_FONT.Width;
    }
    else {
        BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
        char lcd_buffer[MAX_LEN_FIELD + 1];
        sprintf(lcd_buffer, "%d", cell->value);
        BSP_LCD_DisplayStringAt(*curX, *curY, (uint8_t*)lcd_buffer, 0);
        *curX += digit_count(cell->value) * LCD_DEFAULT_FONT.Width;
    }
    BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
}

// print to lcd
void render_table_to_lcd(Table* table, int start_row, int start_col) {
    if (start_row < 0) start_row = 0;
    if (start_col < 0) start_col = 0;
    BSP_LCD_Clear(LCD_COLOR_WHITE);
	
    // print headers
	int curX = FONT_SIZE;
    for (int i = start_col; i < table->col_count; i++) {
        BSP_LCD_DisplayStringAt(curX, 0, (uint8_t*)table->col_names[i], 0);
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
        sprintf(lcd_buffer, "%d", table->row_ids[i]);
        BSP_LCD_DisplayStringAt(curX, curY, (uint8_t*) lcd_buffer, 0);
        
        curX += digit_count(table->row_ids[i]) * LCD_DEFAULT_FONT.Width;
        for (int j = start_col; j < table->col_count; j++) {
            if (curX >= SCREEN_WIDTH) {
                break;
            }
            BSP_LCD_DisplayStringAt(curX, curY, (uint8_t*)", ", 0);
            curX += 2 * LCD_DEFAULT_FONT.Width;
            draw_cell(&table->grid[i * table->col_count + j], &curX, &curY);
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

void find_cell_pos(Table* t, int row, int col, int* x, int* y, int start_row, int start_col) {
    if (row == -1) {
        *x = FONT_SIZE;
        for (int j = start_col; j < col; j++) {
            *x += strlen(t->col_names[j]) * LCD_DEFAULT_FONT.Width + FONT_SIZE;
            *y = 0;
        }
        return;
    } 

    if (col == -1) {
        *x = 0;
        *y = (row - start_row) * OFFSET_LINE + OFFSET_LINE;
        return;
    }

    *x = digit_count(t->row_ids[row]) * LCD_DEFAULT_FONT.Width; // row id width
    int j = start_col;
    while (j != col) {
        *x += 2 * LCD_DEFAULT_FONT.Width; // comma and space
        Cell* cell = &t->grid[row * t->col_count + j];
        if (cell->state == CSV_ERROR || cell->state == EMPTY) {
            *x += strlen("#ERROR") * LCD_DEFAULT_FONT.Width; // #ERROR(5) or #EMPTY(5)
        }
        else {
            *x += digit_count(cell->value) * LCD_DEFAULT_FONT.Width;
        }
        j++;
    }
    *x += 2 * LCD_DEFAULT_FONT.Width; // comma and space
    *y = (row - start_row) * OFFSET_LINE + OFFSET_LINE;
}



void unhighlight_cell(Table* table, int cur_row, int cur_col, int start_row, int start_col) {
    // remove highlight from the previous cell
    if (cur_col < -1) return;
    if (cur_row < -1) return;
    if (cur_col == -1 && cur_row == -1) return;
    int curX = 0, curY = 0;
    find_cell_pos(table, cur_row, cur_col, &curX, &curY, start_row, start_col);

    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    
    if (cur_row == -1) {
        BSP_LCD_DisplayStringAt(curX, 0, (uint8_t*)table->col_names[cur_col], 0);
    }

    else if (cur_col == -1) {
        char lcd_buffer[MAX_LEN_FIELD + 1];
        sprintf(lcd_buffer, "%d", table->row_ids[cur_row]);
        BSP_LCD_DisplayStringAt(curX, curY, (uint8_t*) lcd_buffer, 0);
    } 

    else {
        Cell* cell = &table->grid[cur_row * table->col_count + cur_col];
        draw_cell(cell, &curX, &curY);
    }
}

void highlight_cell(Table* table, int new_row, int new_col, int start_row, int start_col) {
    // highlight the new cell
    if (new_col < -1) return;
    if (new_row < -1) return;
    if (new_col == -1 && new_row == -1) return;

    int curX = 0, curY = 0;
    find_cell_pos(table, new_row, new_col, &curX, &curY, start_row, start_col);

    BSP_LCD_SetBackColor(LCD_COLOR_GREEN);

    if (new_row == -1) {
        BSP_LCD_DisplayStringAt(curX, 0, (uint8_t*)table->col_names[new_col], 0);
    }

    else if (new_col == -1) {
        char lcd_buffer[MAX_LEN_FIELD + 1];
        sprintf(lcd_buffer, "%d", table->row_ids[new_row]);
        BSP_LCD_DisplayStringAt(curX, curY, (uint8_t*) lcd_buffer, 0);
    } 

    else {
        Cell* cell = &table->grid[new_row * table->col_count + new_col];
        draw_cell(cell, &curX, &curY);
    }
    
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
}