#include <cstdio>
#include "headers/lexicalAnalysis.h"

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Error argument count!\n");
        return -2;
    }
    FILE *fp = fopen(argv[1], "r");
    if (fp == nullptr) {
        perror("Open file error!\n");
        return -1;
    }
    lexicalAnalysis analysis;
    analysis.getSymbol(fp);
}