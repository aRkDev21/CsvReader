#ifndef CSV_H
#define CSV_H

#define MAX_LEN_LINE 128
#define MAX_LEN_FIELD 32

#include <stdint.h>

typedef enum {
    EMPTY,
    RAW,
    COMPUTING,
    DONE,
    CSV_ERROR
} CellState;

typedef struct {
    char* raw_data;
    CellState state;
    int value;
} Cell;

typedef struct {
    Cell* grid;
    char** col_names;
    int* row_ids;
    int col_count;
    int row_count;
    int* col_widths;
} Table;

typedef struct {
    const char* cur;
    const char* start;
} StringStream;

typedef int (*read_line_fn)(void* ctx, char* buf, int max_len);
typedef void (*rewind_fn)(void* ctx);

int digit_count(int);
Table* read_csv(read_line_fn, rewind_fn, void*);
Table* read_csv_from_file(const char* filename);
Table* read_csv_from_strmem(const char* buffer);
uint8_t save_table(Table* table, const char* filename);
void free_table(Table*);
int len_header(Table*, int, int);
int len_row(Table* table, int row, int i, int j);

uint8_t evaluate_all(Table*);
#endif
