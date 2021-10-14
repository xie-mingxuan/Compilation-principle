//
// Created by 谢铭轩 on 2021/10/14.
//

#include "../headers/grammarAnalysis.h"

return_token word;
int num;

void FuncDef(FILE *);

void FuncType(FILE *);

void Ident(FILE *);

void Block(FILE *);

void Stmt(FILE *);

void FuncDef(FILE *file) {
    FuncType(file);
    Ident(file);
    if (word.type != "Symbol" || word.token != "LPar")
        exit(-1);
    word = getSymbol(file);
    if (word.type != "Symbol" || word.token != "RPar")
        exit(-1);
    word = getSymbol(file);
    Block(file);

    word = getSymbol(file);
}

void FuncType(FILE *file) {
    if (word.type != "Ident" || word.token != "int")
        exit(-1);

    word = getSymbol(file);
}

void Ident(FILE *file) {
    if (word.type != "Ident" || word.token != "main")
        exit(-1);

    word = getSymbol(file);
}

void Block(FILE *file) {
    if (word.type != "Symbol" || word.token != "LBrace")
        exit(-1);
    word = getSymbol(file);
    Stmt(file);
    if (word.type != "Symbol" || word.token != "RBrace")
        exit(-1);
    word = getSymbol(file);
}

void Stmt(FILE *file) {
    if (word.type != "Ident" || word.token != "Return")
        exit(-1);
    word = getSymbol(file);
    if (word.type != "Number")
        exit(-1);
    num = word.num;
    word = getSymbol(file);
    if (word.type != "Symbol" || word.token != "Semicolon")
        exit(-1);
}

void CompUnit(FILE *in, FILE *out) {
    FuncDef(in);
    word = getSymbol(in);
    if (word.type == "Error")
        exit(-1);

    fprintf(out, "define dso_local i32 @main(){\n");
    fprintf(out, "    ret i32 %d\n}", num);
}
