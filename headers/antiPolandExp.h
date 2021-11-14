//
// Created by 谢铭轩 on 2021/11/1.
//

#include "cstdio"
#include "cstdlib"
#include "lexicalAnalysis.h"
#include "grammarAnalysis.h"
#include "stack"
#include "queue"

using namespace std;

#ifndef C_ANTIPOLANDEXP_H
#define C_ANTIPOLANDEXP_H

/**
 * 这个函数读到最后一个右括号或分号就会返回，此时 word 仍然保持为右括号或分号
 */
string calcAntiPoland(FILE *);

#endif //C_ANTIPOLANDEXP_H
