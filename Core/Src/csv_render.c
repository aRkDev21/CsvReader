/*
 * csv_render.c
 *
 *  Created on: 4 июл. 2026 г.
 *      Author: artem
 */

#include "csv_render.h"
#include "csv.h"
#include "fonts.h"
#include "stm32412g_discovery_lcd.h"
#include <stdint.h>
#include <stdio.h>

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

void display_main_menu(uint8_t count, uint8_t selected) {
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_Clear(LCD_COLOR_WHITE);

	BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);

    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 0, (uint8_t*) "CSV BARE METAL TOOL", CENTER_MODE);

    uint16_t offsetY = Font16.Height;
    BSP_LCD_SetFont(&LCD_DEFAULT_FONT);
    char buf[32];
    for (int i = 0; i < count; i++) {
        sprintf(buf, "Table %d", i+1);
        if (i == selected) {
            BSP_LCD_SetBackColor(LCD_COLOR_DARKGRAY);
        }
        BSP_LCD_DisplayStringAt(0, offsetY + OFFSET_LINE*i, (uint8_t*)buf, CENTER_MODE);
        BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    }
}
int get_size_cell(Cell* cell) {
    int size = 0;
    if (cell->state == DONE) {
        size = digit_count(cell->value);
    }
    else {
        size = 6; // #ERROR or #EMPTY
    }
    return size;
}

int get_max_col_len(Table* table, int start_row, int col) {
    int mx = 0;
    int cur = 0;
    if (start_row == 0 && col >= 0) {
        mx = strlen(table->col_names[col]);
    }
    for (int i = start_row; i < table->row_count; i++) {
        cur = (col <= -1) ? digit_count(table->row_ids[i]) : get_size_cell(&table->grid[i*table->col_count + col]);
        if (mx < cur) mx = cur;
    }

    return mx;
}

int get_visible_width(Table* table, int s_row, int s_col, int col) {
    int total_w = 0;
    if (s_col == 0)
        total_w += get_max_col_len(table, s_row, -1) * LCD_DEFAULT_FONT.Width + FONT_SIZE;// row id width

    for (int j = s_col; j < col; j++) {
        total_w += get_max_col_len(table, s_row, j) * LCD_DEFAULT_FONT.Width + FONT_SIZE;
    }
    return total_w;
}

uint8_t is_cell_visible(Table* table, int row, int col, int start_row, int start_col) {
    int x, y;
    find_cell_pos(table, row, col, &x, &y, start_row, start_col);

    // Берем ширину и высоту ячейки
    int w = get_max_col_len(table, start_row, col) * LCD_DEFAULT_FONT.Width + FONT_SIZE;
    int h = OFFSET_LINE;

    if ((x + w) > 0 && x < 240 && (y + h) > 0 && y < 240) return 1;
    
    return 0;
}

uint8_t can_scroll_right(Table* table, int start_row, int start_col) {
    if (start_col >= table->col_count - 1) return 0;

    int last_col = table->col_count - 1;
    int last_col_x = get_visible_width(table, start_row, start_col, last_col);
    int last_col_w = get_max_col_len(table, start_row, last_col) * LCD_DEFAULT_FONT.Width + FONT_SIZE;
    
    return (last_col_x + last_col_w > SCREEN_WIDTH) ? 1 : 0;
}


uint8_t can_scroll_down(Table* table, int start_row) {
    if (start_row >= table->row_count - 1) return 0;

    int header_offset = (start_row == 0) ? OFFSET_LINE : 0;
    int total_h = (table->row_count - start_row) * OFFSET_LINE + header_offset;
    
    return (total_h > SCREEN_HEIGHT) ? 1 : 0;
}

uint16_t  get_cell_color(int row, int col) {
    if ( (row + col) % 2 != 0) {
        return LCD_COLOR_LIGHTGRAY;
    }
    return LCD_COLOR_WHITE;
}

void update_viewport(int selected_row, int selected_col, int* start_row, int* start_col, Table* table, volatile uint8_t* viewport_changed) {
    *viewport_changed = 0;

    if (selected_row < *start_row && selected_row != -1) {
        *start_row = selected_row;
        *viewport_changed = 1;
    } else if (selected_row > ( (*start_row==0) ? 8 : (*start_row + 9) )) {
        // if we cant display the selected column 
        *start_row = selected_row - 9;
        if (*start_row == 0) *start_row = 1;
        *viewport_changed = 1;
    }

    if (selected_col < *start_col  && selected_col != -1) {
        *start_col = selected_col;
        *viewport_changed = 1;
    }
    else {
    // if we cant display the selected column
        int x = 0, y = 0;
        find_cell_pos(table, selected_row, selected_col, &x, &y, *start_row, *start_col);
        x += get_max_col_len(table, *start_row, selected_col) * LCD_DEFAULT_FONT.Width + FONT_SIZE;
        while (x >= SCREEN_WIDTH) {
            (*start_col)++;
            find_cell_pos(table, selected_row, selected_col, &x, &y, *start_row, *start_col);
            x += get_max_col_len(table, *start_row, selected_col) * LCD_DEFAULT_FONT.Width + FONT_SIZE;
            *viewport_changed = 1;
        }  
    }

    int total_w = get_visible_width(table, *start_row, *start_col, selected_col);
    while (total_w < SCREEN_WIDTH && *start_col > 0) {
        int next_start_col = *start_col - 1;

        int x = 0, y = 0;
        find_cell_pos(table, selected_row, selected_col, &x, &y, *start_row, next_start_col);
        int cell_w = get_max_col_len(table, *start_row, selected_col) * LCD_DEFAULT_FONT.Width + FONT_SIZE;
        
        if (x + cell_w <= SCREEN_WIDTH) {
            *start_col = next_start_col;
            total_w = get_visible_width(table, *start_row, *start_col, selected_col);
            *viewport_changed = 1;
            if (*start_col >= selected_col) {
                break;
            }
        } else {
            break;
        }
    }

    int total_h = (table->row_count - *start_row) * OFFSET_LINE;
    if (*start_row == 0) {
        total_h += OFFSET_LINE;
    }
    while (total_h >= SCREEN_HEIGHT && *start_row > 0) {
        int next_start_row = *start_row - 1;
        
        int cell_y = (selected_row - next_start_row) * OFFSET_LINE;
        if (next_start_row == 0) {
            cell_y += OFFSET_LINE;
        }
        
        if (cell_y + OFFSET_LINE <= SCREEN_HEIGHT) {
            *start_row = next_start_row;
            total_h = (table->row_count - *start_row) * OFFSET_LINE;
            if (*start_row == 0) {
                total_h += OFFSET_LINE;
            }
            *viewport_changed = 1;
        } else {
            break; 
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
        *y = (row - start_row) * OFFSET_LINE + ( (start_row == 0) ? OFFSET_LINE : 0 );
    }

    int id_w = get_max_col_len(t, start_row, -1) * LCD_DEFAULT_FONT.Width + FONT_SIZE;

    if (col == -1) {
        *x = ((start_col == 0) ? 0 : -id_w);
    }
    else {
        *x = get_visible_width(t, start_row, start_col, col);
        if (start_col > 0) *x -= id_w;
    }
}

int draw_cell(Table* table, int row, int col, int *curX, int *curY, int start_row, int start_col, uint16_t color) {
    find_cell_pos(table, row, col , curX, curY, start_row, start_col);
    if (*curY >= SCREEN_HEIGHT || *curX >= SCREEN_WIDTH) return 0;
    int cell_w, cell_h;
    cell_h = OFFSET_LINE;
    cell_w = get_max_col_len(table, start_row, col) * LCD_DEFAULT_FONT.Width + FONT_SIZE;
    if (*curX + cell_w >= SCREEN_WIDTH) cell_w = SCREEN_WIDTH - *curX;
    int copyX = *curX;
    *curX = (*curX < 0) ? 0 : *curX;
    BSP_LCD_SetTextColor(color);
    BSP_LCD_SetBackColor(color);
    if (row == -1 && col == 0 && 0) {
        // cell_w = get_max_col_len(table, start_row, -1) * LCD_DEFAULT_FONT.Width;
        // BSP_LCD_FillRect(0, *curY, cell_w, cell_h);
    }
    else {
        BSP_LCD_FillRect(*curX, *curY, cell_w, cell_h);
    }

    BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);

    int textX = *curX;
    int textY = *curY + (FONT_SIZE)/2;
    if (row == -1) {
        textX += (cell_w - (strlen(table->col_names[col]) * LCD_DEFAULT_FONT.Width)) / 2;
        if (textX <= *curX) textX = *curX;
        BSP_LCD_DisplayStringAt(textX, textY, (uint8_t*)table->col_names[col], 0);
        //*curX += strlen(table->col_names[col]) * LCD_DEFAULT_FONT.Width + FONT_SIZE;
    }

    else if (col == -1) {
        char lcd_buffer[MAX_LEN_FIELD + 1];
        sprintf(lcd_buffer, "%d", table->row_ids[row]);
        textX += (cell_w - digit_count(table->row_ids[row]) * LCD_DEFAULT_FONT.Width ) / 2;
        if (textX <= *curX) textX = *curX;
        BSP_LCD_DisplayStringAt(textX, textY, (uint8_t*) lcd_buffer, 0);
        //*curX += digit_count(table->row_ids[row]) * LCD_DEFAULT_FONT.Width;
    } 

    else {
        Cell* cell = &table->grid[row * table->col_count + col];
        textX += (cell_w - (get_size_cell(cell)*LCD_DEFAULT_FONT.Width)) / 2;
        if (textX <= *curX) textX = *curX;
        draw_cell_value(cell, &textX, &textY);
    }
    *curX = copyX;
    return 1;
}

// print to lcd
void render_table_to_lcd(Table* table, int start_row, int start_col) {
    BSP_LCD_Clear(LCD_COLOR_WHITE);
    // print headers
	int curX = 0;
    int curY = 0;
    uint16_t bg_color;
    if (start_row == 0) { 
        for (int i = start_col; i < table->col_count; i++) {
            bg_color = get_cell_color(-1, i);
            if (!draw_cell(table, -1, i, &curX, &curY, start_row, start_col, bg_color)) {
                break;
            }
        }
    }

    // print body
    curX = 0;
    curY = (start_row == 0) ? OFFSET_LINE : 0;
    for (int i = start_row; i < table->row_count; i++) {
        bg_color = get_cell_color(i, -1);
        // print row id
        if (start_col == 0){
            draw_cell(table, i, -1, &curX, &curY, start_row, start_col, bg_color);
        }
        for (int j = start_col; j < table->col_count; j++) {
            bg_color = get_cell_color(i, j);
            if (!draw_cell(table, i, j, &curX, &curY, start_row, start_col, bg_color)) {
                break;
            }
            //curX += LCD_DEFAULT_FONT.Width;
        }
        curY += OFFSET_LINE;
        curX = 0;
    }

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


int unhighlight_cell(Table* table, int cur_row, int cur_col, int start_row, int start_col) {
    // remove highlight from the previous cell
    if (!is_cell_visible(table, cur_row, cur_col, start_row, start_col)) return 0;
    int curX = 0, curY = 0;
    return draw_cell(table, cur_row, cur_col, &curX, &curY, start_row, start_col, get_cell_color(cur_row, cur_col));
}

int highlight_cell(Table* table, int new_row, int new_col, int start_row, int start_col) {
    // highlight the new cell
    if (!is_cell_visible(table, new_row, new_col, start_row, start_col)) return 0;
    int curX = 0, curY = 0;
    return draw_cell(table, new_row, new_col, &curX, &curY, start_row, start_col, LCD_COLOR_DARKGRAY);
}

int get_clicked_row(int start_row, int y) {
    if (y < OFFSET_LINE) {
        return start_row - 1;
    }

    return start_row + ((y-OFFSET_LINE) / OFFSET_LINE);
}

int get_clicked_col(Table *table, int start_col, int start_row, int x) {
    int curX = ((start_col == 0) ? get_max_col_len(table, start_row, -1) * LCD_DEFAULT_FONT.Width + FONT_SIZE : 0);
    if (start_col == 0 && x < curX) {
        return -1;
    }
    for (int col = start_col; col < table->col_count; col++) {
        int col_w = get_max_col_len(table, start_row, col) * LCD_DEFAULT_FONT.Width + FONT_SIZE;
        if (x >= curX && x < (curX + col_w)) {
            return col;
        }

        curX += col_w;
        if (curX >= SCREEN_WIDTH) break;
    }

   return -2;
}