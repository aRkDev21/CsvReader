#ifndef CSV_H
#define CSV_H

#define MAX_LEN_LINE 64

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
} Table;

int digit_count(int);
Table* read_csv(const char*);
void free_table(Table*);

void evaluate_all(Table*);
#endif
