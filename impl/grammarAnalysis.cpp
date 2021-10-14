//
// Created by 谢铭轩 on 2021/10/14.
//

#include "../headers/grammarAnalysis.h"

return_token token;
int num;

void FuncDef(FILE *);

void FuncType(FILE *);

void Ident(FILE *);

void Block(FILE *);

void Stmt(FILE *);

void FuncDef(FILE *file) {
    FuncType(file);
    Ident(file);
    if (token.type != "Symbol" || token.token != "LPar")
        exit(-1);
    token = getSymbol(file);
    if (token.type != "Symbol" || token.token != "RPar")
        exit(-1);
    token = getSymbol(file);
    Block(file);

    token = getSymbol(file);
}

void FuncType(FILE *file) {
    if (token.type != "Ident" || token.token != "int")
        exit(-1);

    token = getSymbol(file);
}

void Ident(FILE *file) {
    if (token.type != "Ident" || token.token != "main")
        exit(-1);

    token = getSymbol(file);
}

void Block(FILE *file) {
    if (token.type != "Symbol" || token.token != "LBrace")
        exit(-1);
    token = getSymbol(file);
    Stmt(file);
    if (token.type != "Symbol" || token.token != "RBrace")
        exit(-1);
    token = getSymbol(file);
}

void Stmt(FILE *file) {
    if (token.type != "Ident" || token.token != "Return")
        exit(-1);
    token = getSymbol(file);
    if (token.type != "Number")
        exit(-1);
    num = token.num;
    token = getSymbol(file);
    if (token.type != "Symbol" || token.token != "Semicolon")
        exit(-1);
}

void CompUnit(FILE *in, FILE *out) {
    FuncDef(in);
    token = getSymbol(in);
    if (token.type == "Error")
        exit(-1);

    fprintf(out, "define dso_local i32 @main(){\n");
    fprintf(out, "    ret i32 %d\n}", num);
}
