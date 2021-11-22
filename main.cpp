#include <iostream>
#include "headers/grammarAnalysis.h"

int main(int argc, char **argv) {
	if (argc != 3) {
		printf("Args Err!\n");
		return -1;
	}
	FILE *in = fopen(argv[1], "r");
	FILE *out = fopen(argv[2], "w+");
	fseek(in, SEEK_SET, 0);
	fseek(out, SEEK_SET, 0);
	CompUnit(in, out);
	fclose(in);
	fclose(out);
//	out = fopen(argv[2], "r");
//	char c;
//	while ((c = fgetc(out)) != EOF) {
//		putchar(c);
//	}
	return 0;
}
