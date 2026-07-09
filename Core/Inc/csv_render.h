/*
 * csv_render.h
 *
 *  Created on: 4 июл. 2026 г.
 *      Author: artem
 */

#ifndef CSV_RENDER_H
#define CSV_RENDER_H

#include <string.h>
#include <stdbool.h>

#include "csv.h"

#include "stm32412g_discovery.h"
#include "stm32412g_discovery_lcd.h"

void init_lcd();
void display_error(const char* error_text);
void find_cell_pos(Table* t, int row, int col, int* x, int* y, int start_row, int start_col);
void render_table_to_lcd(Table*, int, int);
void update_viewport(int selected_row, int selected_col, int* start_row, int* start_col, Table* table, bool* viewport_changed);
void highlight_cell(Table* table, int new_row, int new_col, int start_row, int start_col);
void unhighlight_cell(Table* table, int prev_row, int prev_col, int start_row, int start_col);

#endif /* CSV_RENDER_H */
