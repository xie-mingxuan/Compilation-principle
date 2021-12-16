//
// Created by 谢铭轩 on 2021/11/1.
//

#include "cstdio"
#include "cstdlib"
#include "lexicalAnalysis.h"
#include "grammarAnalysis.h"
#include "cassert"
#include "stack"
#include "queue"

using namespace std;

#ifndef C_ANTIPOLANDEXP_H
#define C_ANTIPOLANDEXP_H

typedef struct {
	bool is_variable;
	bool is_function;
	return_token token;
	string variable;
	int array_dimension;
} number_stack_elem;

/**
 * 这个函数读到最后一个右括号或分号就会返回，此时 word 仍然保持为右括号或分号
 */
number_stack_elem calcAntiPoland(FILE *input, bool is_const_define = false, bool is_global_define = false);

void print_number_stack_elem(const number_stack_elem &);
#endif //C_ANTIPOLANDEXP_H
