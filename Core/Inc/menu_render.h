#ifndef MENU_RENDER_H
#define MENU_RENDER_H

#include "stdint.h"

#define FONT_SIZE 12
#define OFFSET_LINE 24

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

typedef struct {
  uint8_t selected_table;
  uint8_t total_tables;
} MenuParams;

void init_lcd();
void display_main_menu(MenuParams *menu);
void display_error(const char* error_text);

#endif