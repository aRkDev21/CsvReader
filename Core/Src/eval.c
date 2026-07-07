#include "csv.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_EXPR_LEN 20

int find_col_index(Table* table, char* col_name) {
    for (int i = 0; i < table->col_count; i++) {
        if (strcmp(table->col_names[i], col_name) == 0) {
            return i;
        }
    }
    return -1;
}

int find_row_index(Table* table, int row_id) {
    for (int i = 0; i < table->row_count; i++) {
        if (table->row_ids[i] == row_id) {
            return i;
        }
    }
    return -1;
}

/* get value without recursion
 * true - if cell can be calculated right now
 * false - in other case
 */
bool try_get_arg_value(Table* t, char* arg, int* out_value, bool* out_error) {
    if (isdigit(arg[0]) || (arg[0] == '-' && isdigit(arg[1]))) {
        *out_value = atoi(arg);
        *out_error = false;
        return true;
    }

    char col_part[MAX_EXPR_LEN];
    int i = 0;
    while (arg[i] && !isdigit(arg[i])) {
        if (i < MAX_EXPR_LEN - 1) {
            col_part[i] = arg[i];
            i++;
        } else {
            break;
        }

    }

    col_part[i] = '\0';
    int row_id = atoi(&arg[i]);

    int col_index = find_col_index(t, col_part);
    int row_index = find_row_index(t, row_id);

    if (col_index == -1 || row_index == -1) {
        *out_error = true;
        return true;
    }

    Cell* dep_cell = &t->grid[row_index*t->col_count + col_index];

    if (dep_cell->state == CSV_ERROR) {
        *out_error = true;
        return true;
    }

    if (dep_cell->state == RAW || dep_cell->state == COMPUTING) {
        return false;
    }

    *out_value = dep_cell->value;
    *out_error = false;
    return true;
}


// return true if made some progress
bool try_evaluate_cell(Table* t, int r, int c) {
    Cell* cell = &t->grid[r*t->col_count + c];

    // skip done / error / empty cells
    if (cell->state == DONE || cell->state == CSV_ERROR || cell->state == EMPTY) {
        return false;
    }

    // not formula
    if (cell->raw_data[0] != '=') {
        if (strcmp(cell->raw_data, "") == 0) {
            cell->value = 0;
            cell->state = EMPTY;
        }
        else {
            cell->value = atoi(cell->raw_data);
            cell->state = DONE;
        }
        return true;
    }

    cell->state = COMPUTING;

    char expr[MAX_EXPR_LEN];
    strncpy(expr, cell->raw_data + 1, MAX_EXPR_LEN - 1);
    expr[MAX_EXPR_LEN - 1] = '\0';

    char* op_ptr = strpbrk(expr, "+-*/");
    if (!op_ptr) {
        cell->state = CSV_ERROR;
        return true;
    }

    char op = *op_ptr;
    *op_ptr = '\0';
    char* arg1 = expr;
    char* arg2 = op_ptr + 1;

    int val1 = 0, val2 = 0;
    bool err1 = false, err2 = false;
    bool ready1 = try_get_arg_value(t, arg1, &val1, &err1);
    bool ready2 = try_get_arg_value(t, arg2, &val2, &err2);

    if (!ready1 || !ready2) {
        cell->state = COMPUTING;
        return false;
    }

    if (err1 || err2) {
        cell->state = CSV_ERROR;
        return true;
    }

    if (op == '+') cell->value = val1 + val2;
    else if (op == '-') cell->value = val1 - val2;
    else if (op == '*') cell->value = val1 * val2;
    else if (op == '/') {
        if (val2 == 0) {
            cell->state = CSV_ERROR;
            return true;
        }
        cell->value = val1 / val2;
    }

    cell->state = DONE;
    return true;
}

void evaluate_all(Table* t) {
    bool progress;

    do {
        progress = false;
        for (int i = 0; i < t->row_count; i++) {
            for (int j = 0; j < t->col_count; j++) {
                if (try_evaluate_cell(t, i, j)) {
                    progress = true;
                }
            }
        }
    } while (progress);

    for (int i = 0; i < t->row_count; i++) {
        for (int j = 0; j < t->col_count; j++) {
            Cell* cell = &t->grid[i*t->col_count + j];
            if (cell->state == RAW || cell->state == COMPUTING) {
                cell->state = CSV_ERROR;
            }
        }
    }
}
