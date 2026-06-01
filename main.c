#include <stdio.h>
#include "csv.h"

int main (int argc, char* argv[]) {
    if (argc < 2) {
        printf("Missed *.csv file!\n");
        return 0;
    }
    
    printf("Processing file: %s\n", argv[1]);
    Table* t = read_csv(argv[1]);
    if (!t) return 1;

    evaluate_all(t);
    print_table(t);
    free_table(t);
    
    return 0;
}