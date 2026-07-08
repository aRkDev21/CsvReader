/*
 * csv_render.h
 *
 *  Created on: 4 июл. 2026 г.
 *      Author: artem
 */

#ifndef CSV_RENDER_H
#define CSV_RENDER_H

#include <string.h>

#include "csv.h"

#include "stm32412g_discovery.h"
#include "stm32412g_discovery_lcd.h"

void init_lcd();
void display_error(const char*);
void render_table_to_lcd(Table*, int, int, int, int);

#endif /* CSV_RENDER_H */
