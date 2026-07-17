#include "csv.h"
#include "arena.h"
#include "stm32412g_discovery_lcd.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

MemoryArena table_arena;

int digit_count(int v){
    int8_t is_neg = (v<0) ? 1 : 0;
	v = (v>0) ? v:-v;
	if (v == 0) return 1;
	int c = 0;
	while ( v != 0) {
		c++;
		v /= 10;
	}

	return c + is_neg;
}

int count_char(char *str, char target) {
    int count = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == target) {
            count++;
        }
    }

    return count;
}

//
int read_line(const char** file_ptr, char buffer[]) {
    size_t len = 0;

    if (!file_ptr || !*file_ptr || **file_ptr == '\0') return len;
    const char* str = *file_ptr;
    while (*str != '\0' && *str != '\n') {
        if (len + 1 >= MAX_LEN_LINE) {
            return -1;
        }
        buffer[len++] = *str++;
    }

    if (*str == '\n') str++;
    buffer[len] = '\0';
    *file_ptr = str;
    return len;

}

void trim_newline(char* str){
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[--len] = '\0';
    }
}

int len_header(Table* table, int i, int j) {
    int total_length = 0;
    for (int k = i; k < j; k++) {
        total_length += strlen(table->col_names[k]) + LCD_DEFAULT_FONT.Width; // +2 for ", "
    }
    return total_length;
}

int len_row(Table* table, int row, int i, int j) {
    if (row == -1) return len_header(table, i, j);
    int total_length = 0;
    for (int k = i; k < j; k++) {
        Cell* cell = &table->grid[row * table->col_count + k];
        if (k != j-1) {
            total_length += LCD_DEFAULT_FONT.Width; // for " "
        }
        if (cell->state == CSV_ERROR) {
            total_length += strlen("#ERROR");
        } else if (cell->state == EMPTY) {
            total_length += strlen("#EMPTY");
        } else {
            char buffer[MAX_LEN_FIELD + 1];
            sprintf(buffer, "%d", cell->value);
            total_length += strlen(buffer);
        }
    }
    return total_length;
}


int parse_header(char* line, Table* table) {
	// 1 - success, 0 - error
    char* line_ptr = line;
    char* next_comma = strchr(line_ptr, ',');

    if (next_comma) {
        line_ptr = next_comma + 1; // skip first comma

        int col_idx = 0;
        while (line_ptr && *line_ptr != '\0') {
            next_comma = strchr(line_ptr, ',');
            if (next_comma) {
                *next_comma = '\0';
            }

            table->col_names[col_idx] = arena_strdup(&table_arena, line_ptr);
            if (!table->col_names[col_idx]) return 0;

            if (next_comma) {
                line_ptr = next_comma + 1;
            } else {
                line_ptr = NULL;
            }

            col_idx++;
        }
    }
    return 1;
}

int parse_body(char* line, const char* file_cursor, Table* table) {
	// 1 - success, 0 - error
    int current_row = 0;
    while ((read_line(&file_cursor, line)) > 0) {
        trim_newline(line);

        char *line_ptr = line;
        char *next_comma;

        next_comma = strchr(line_ptr, ',');

        if (next_comma) {
            *next_comma = '\0';
            table->row_ids[current_row] = atoi(line_ptr);
            line_ptr = next_comma + 1;
        }

        for (int j = 0; j < table->col_count; j++) {
            next_comma = strchr(line_ptr, ',');

            char *field_value;
            if (next_comma) {
                *next_comma = '\0';
                field_value = line_ptr;
                line_ptr = next_comma + 1;
            } else {
                field_value = line_ptr;
            }

            Cell* cell = &table->grid[current_row * table->col_count + j];
            cell->raw_data = arena_strdup(&table_arena, field_value);
            if (!cell->raw_data) return 0;

            cell->value = 0;
            cell->state = RAW;
        }

        current_row++;
    }
    return 1;
}

Table* read_csv(const char* file) {
    Table* table = (Table*) arena_alloc(&table_arena, sizeof(Table));

    if (!table) return NULL;
    char line[MAX_LEN_LINE];
    const char* file_cursor = file;
    if (read_line(&file_cursor, line) < 0) return NULL;

    trim_newline(line);

    // columns count
    table->col_count = count_char(line, ',');
    table->col_names = (char**) arena_alloc(&table_arena, table->col_count*sizeof(char*));
    if (table->col_names == NULL) return NULL;

    if (!parse_header(line, table)) return NULL;

    // rows count
    int rows = 0;
    const char* current_pos = file_cursor;
    while ((read_line(&file_cursor, line)) > 0) rows++;
    file_cursor = current_pos;
    table->row_count = rows;

    table->row_ids = (int*) arena_alloc(&table_arena, rows * sizeof(int));
    table->grid = (Cell*) arena_alloc(&table_arena, rows * table->col_count * sizeof(Cell));
    if (!table->grid || !table->row_ids) return NULL;

    if (!parse_body(line, file_cursor, table)) return NULL;

    return table;
}

void free_table(Table* table) {
    reset_arena(&table_arena);
}
