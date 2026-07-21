/*
 * csv_render.h
 *
 *  Created on: 4 июл. 2026 г.
 *      Author: artem
 */

#ifndef CSV_RENDER_H
#define CSV_RENDER_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "csv.h"

#include "stm32412g_discovery.h"
#include "stm32412g_discovery_lcd.h"

void init_lcd();
void display_error(const char* error_text);
void display_main_menu(uint8_t count, uint8_t selected);
void find_cell_pos(Table* t, int row, int col, int* x, int* y, int start_row, int start_col);
int get_clicked_row(int start_row, int y);
int get_clicked_col(Table* table, int start_col, int start_row, int x);
void render_table_to_lcd(Table*, int, int);
void update_viewport(int selected_row, int selected_col, int* start_row, int* start_col, Table* table, volatile uint8_t* viewport_changed);
int highlight_cell(Table* table, int new_row, int new_col, int start_row, int start_col);
int unhighlight_cell(Table* table, int prev_row, int prev_col, int start_row, int start_col);

uint8_t can_scroll_right(Table* table, int start_row, int start_col);
uint8_t can_scroll_down(Table* table, int start_row);
uint8_t is_cell_visible(Table* table, int row, int col, int s_row, int s_col);

#endif /* CSV_RENDER_H */
