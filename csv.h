#ifndef CSV_H
#define CSV_H

typedef enum {
    EMPTY,
    RAW,
    COMPUTING,
    DONE,
    ERROR
} CellState;

typedef struct {
    char* raw_data;
    CellState state;
    int value;
} Cell;

typedef struct {
    Cell** grid;
    char** col_names;
    int* row_ids;
    int col_count;
    int row_count;
} Table;

Table* read_csv(const char*);
void print_table(Table*);
void free_table(Table*);



void evaluate_cell(Table*, int, int);
void evaluate_all(Table*);
#endif