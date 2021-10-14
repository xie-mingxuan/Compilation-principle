//
// Created by 谢铭轩 on 2021/9/21.
//

#ifndef C_LEXICALANALYSIS_H
#define C_LEXICALANALYSIS_H

#include "vector"
#include "judgeLetter.h"
#include "iostream"

using namespace std;

typedef struct return_token {
    string type;
    string token;
    int num;
} return_token;

return_token getSymbol(FILE *file);


#endif //C_LEXICALANALYSIS_H
