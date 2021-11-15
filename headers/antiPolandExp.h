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

typedef struct {
	bool is_variable;
	return_token token;
	string variable;
} number_stack_elem;

/**
 * 这个函数读到最后一个右括号或分号就会返回，此时 word 仍然保持为右括号或分号
 */
number_stack_elem calcAntiPoland(FILE *, bool = false);

#endif //C_ANTIPOLANDEXP_H
