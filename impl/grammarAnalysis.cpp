//
// Created by 谢铭轩 on 2021/10/14.
//

#include "../headers/grammarAnalysis.h"

return_token word;
int num;
FILE *input;
FILE *output;

void FuncDef(FILE *);

void FuncType(FILE *);

void Ident(FILE *);

void Block(FILE *);

void Stmt(FILE *);

void _exit_() {
    fclose(input);
    fclose(output);
    exit(-1);
}

void FuncDef(FILE *file) {
    FuncType(file);
    Ident(file);
    if (word.type != "Symbol" || word.token != "LPar")
        _exit_();
    word = getSymbol(file);
    if (word.type != "Symbol" || word.token != "RPar")
        _exit_();
    word = getSymbol(file);
    Block(file);

    word = getSymbol(file);
}

void FuncType(FILE *file) {
    if (word.type != "Ident" || word.token != "int")
        _exit_();

    word = getSymbol(file);
}

void Ident(FILE *file) {
    if (word.type != "Ident" || word.token != "main")
        _exit_();

    word = getSymbol(file);
}

void Block(FILE *file) {
    if (word.type != "Symbol" || word.token != "LBrace")
        _exit_();
    word = getSymbol(file);
    Stmt(file);
    if (word.type != "Symbol" || word.token != "RBrace")
        _exit_();
    word = getSymbol(file);
}

void Stmt(FILE *file) {
    if (word.type != "Ident" || word.token != "Return")
        _exit_();
    word = getSymbol(file);
    if (word.type != "Number")
        _exit_();
    num = word.num;
    word = getSymbol(file);
    if (word.type != "Symbol" || word.token != "Semicolon")
        _exit_();
}

void CompUnit(FILE *in, FILE *out) {
    input = in;
    output = out;
    FuncDef(in);
//    word = getSymbol(in);
//    if (word.type == "Error")
//        _exit_();

    fprintf(out, "define dso_local i32 @main(){\n");
    fprintf(out, "    ret i32 ");
    fprintf(out, "%d}", num);
}
