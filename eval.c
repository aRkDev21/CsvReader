#include "csv.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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


int get_arg_value(Table* t, char* arg, int* error) {
    if (isdigit(arg[0]) || (arg[0] == '-' && isdigit(arg[1]))) {
        return atoi(arg);
    }

    char col_part[strlen(arg)+1];
    int i = 0;
    while (arg[i] && !isdigit(arg[i])) {
        col_part[i] = arg[i];
        i++;
    }

    col_part[i] = '\0';
    int row_id = atoi(&arg[i]);

    int col_index = find_col_index(t, col_part);
    int row_index = find_row_index(t, row_id);

    if (col_index == -1 || row_index == -1) {
        *error = 1;
        return 0;
    }

    evaluate_cell(t, row_index, col_index);

    if (t->grid[row_index][col_index].state == ERROR) {
        *error = 1;
        return 0;
    }

    return t->grid[row_index][col_index].value;
}



void evaluate_cell(Table* t, int r, int c) {
    Cell* cell = &t->grid[r][c];

    if (cell->state == DONE || cell->state == ERROR) return;
    if (cell->state == COMPUTING) {
        cell->state = ERROR;
        return;
    }

    if (cell->raw_data[0] != '=') {
        cell->value = atoi(cell->raw_data);
        
        if (strcmp(cell->raw_data, "") == 0) {
            cell->state = EMPTY;
            return;
        }
        cell->state = DONE;
        return;
    }

    cell->state = COMPUTING;
    
    char expr[strlen(cell->raw_data)+1];
    strcpy(expr, cell->raw_data + 1);

    char* op_ptr = strpbrk(expr, "+-*/");
    if (!op_ptr) {
        cell->state = ERROR;
        return;
    }

    char op = *op_ptr;
    *op_ptr = '\0';
    char* arg1 = expr;
    char* arg2 = op_ptr + 1;

    int err = 0;
    int val1 = get_arg_value(t, arg1, &err);
    int val2 = get_arg_value(t, arg2, &err);

    if (err) {
        cell->state = ERROR;
        return;
    }

    if (op == '+') cell->value = val1 + val2;
    else if (op == '-') cell->value = val1 - val2;
    else if (op == '*') cell->value = val1 * val2;
    else if (op == '/') {
        if (val2 == 0) {
            cell->state = ERROR;
            return;
        }
        cell->value = val1 / val2;
    }

    cell->state = DONE;
}

void evaluate_all(Table* t) {
    for (int i = 0; i < t->row_count; i++) {
        for (int j = 0; j < t->col_count; j++) {
            evaluate_cell(t, i, j);
        }
    }
}