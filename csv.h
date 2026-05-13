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
    int value;
    CellState state;
} Cell;

typedef struct {
    char** col_names;
    int* row_ids;
    Cell** grid;
    int col_count;
    int row_count;
} Table;

Table* read_csv(const char*);
void print_table(Table*);
void free_table(Table*);



int find_col_index(Table*, char*);
int find_row_index(Table*, int);
int get_arg_value(Table*, char*, int*);
void evaluate_cell(Table*, int, int);
void evaluate_all(Table*);
#endif