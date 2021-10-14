#include <iostream>
#include "headers/grammarAnalysis.h"

int main(int argc, char **argv) {
    if(argc != 3) {
        printf("Args Err!\n");
        return -1;
    }
    FILE *in = fopen(argv[1], "r");
    FILE *out = fopen(argv[2], "w+");
    CompUnit(in, out);
    fclose(in);
    fclose(out);
    return 0;
}
