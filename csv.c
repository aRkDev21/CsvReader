#include "csv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* read_line(FILE* file) {
    size_t capacity = 128;
    size_t len = 0;
    char* buffer = malloc(capacity);

    if (!buffer) return NULL;

    int ch;
    while ((ch = fgetc(file)) != EOF && ch != '\n') {
        if (len + 1 >= capacity) {
            capacity *= 2;
            char* new_buffer = realloc(buffer, capacity);
            if (!new_buffer) {
                free(buffer);
                return NULL;
            }
            buffer = new_buffer;
        }
        buffer[len++] = ch;
    }

    if (ch == EOF && len == 0) {
        free(buffer);
        return NULL;
    }

    buffer[len] = '\0';
    return buffer;

}

void trim_newline(char* str){
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[--len] = '\0';
    }
}

Table* read_csv(const char* filename) {
    Table* table = malloc(sizeof(Table));
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Could not open file: %s\n", filename);
        free(table);
        return NULL;
    }

    char* line = read_line(file);
    if (!line) {
        printf("Error reading first line of file: %s\n", filename);
        free(table);
        fclose(file);
        return NULL;
    }

    trim_newline(line);

        table->col_count = 0;
        table->col_names = NULL;

        char* line_ptr = line;
        char* next_comma = strpbrk(line_ptr, ",");
        
        
        if (next_comma) {
            line_ptr = next_comma + 1; 
            
            while (line_ptr && *line_ptr != '\0') {
                next_comma = strpbrk(line_ptr, ",");
                if (next_comma) {
                    *next_comma = '\0';
                }

                table->col_count++;
                table->col_names = realloc(table->col_names, table->col_count * sizeof(char*));
                table->col_names[table->col_count - 1] = strdup(line_ptr);

                if (next_comma) {
                    line_ptr = next_comma + 1;
                } else {
                    line_ptr = NULL;
                }
            }
        }

        free(line);
    
    


    table->row_count = 0;
    table->row_ids = NULL;
    table->grid = NULL;

    while ((line = read_line(file)) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) {
            free(line);
            continue;
        }

        table->row_count++;

        table->row_ids = realloc(table->row_ids, table->row_count * sizeof(int));
        table->grid = realloc(table->grid, table->row_count * sizeof(Cell*));
        table->grid[table->row_count - 1] = malloc(table->col_count * sizeof(Cell));

        char *line_ptr = line;
        char *next_comma;

        next_comma = strpbrk(line_ptr, ",");

        if (next_comma) {
            *next_comma = '\0';
            table->row_ids[table->row_count - 1] = atoi(line_ptr);
            line_ptr = next_comma + 1;
        }

        for (int j = 0; j < table->col_count; j++) {
            next_comma = strpbrk(line_ptr, ",");
            
            char *field_value;
            if (next_comma) {
                *next_comma = '\0';
                field_value = line_ptr;
                line_ptr = next_comma + 1;
            } else {
                field_value = line_ptr;
            }

            Cell* cell = &table->grid[table->row_count - 1][j];
            cell->raw_data = strdup(field_value);
            cell->value = 0;
            cell->state = RAW;
            }
        }
    fclose(file);

    return table;
}

void print_table(Table* table) {
    for (int i = 0; i < table->col_count; i++) {
        printf(",%s", table->col_names[i]);
    }
    printf("\n");

    for (int i = 0; i < table->row_count; i++) {
        printf("%d", table->row_ids[i]);
        for (int j = 0; j < table->col_count; j++) {
            if (table->grid[i][j].state == ERROR) {
                printf(", #ERROR");
            } 
            else if (table->grid[i][j].state == EMPTY) {
                printf(",");
            
            } else {
                printf(",%d", table->grid[i][j].value);
            }
        }
        printf("\n");
    }
}

void free_table(Table* table) {
    if (!table) return;

    for (int i = 0; i < table->row_count; i++) {
        for (int j = 0; j < table->col_count; j++) {
            free(table->grid[i][j].raw_data);
        }
        free(table->grid[i]);
    }

    free(table->grid);
 
    free(table->row_ids);

    for (int i = 0; i < table->col_count; i++) {
        free(table->col_names[i]);
    }
    free(table->col_names);

    free(table);
}
