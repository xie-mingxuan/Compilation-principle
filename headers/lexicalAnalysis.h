//
// Created by 谢铭轩 on 2021/9/21.
//

#ifndef C_LEXICALANALYSIS_H
#define C_LEXICALANALYSIS_H

#include "vector"
#include "judgeLetter.h"
#include "iostream"

using namespace std;

class lexicalAnalysis {
private:
    char c;
    int num = 0;
    vector<char> token;

    void clearToken();

    void catToken();

    int transNum();

public:
    int getSymbol(FILE *file);
};


#endif //C_LEXICALANALYSIS_H
