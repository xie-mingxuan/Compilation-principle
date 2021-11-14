//
// Created by 谢铭轩 on 2021/9/21.
//

#ifndef C_LEXICALANALYSIS_H
#define C_LEXICALANALYSIS_H

#include "vector"
#include "judgeLetter.h"
#include "cstring"
#include "iostream"

using namespace std;

typedef struct x {
    string type;
    string token;
    int num = 0;

    bool operator==(const x &b) const {
        return this->type == b.type && this->token == b.token && this->num == b.num;
    }

    bool operator!=(const x &b) const {
        return !this->operator==(b);
    }
} return_token;

return_token getSymbol(FILE *file);


#endif //C_LEXICALANALYSIS_H
