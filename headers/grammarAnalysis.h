//
// Created by 谢铭轩 on 2021/10/14.
//

#ifndef LAB1_GRAMMARANALYSIS_H
#define LAB1_GRAMMARANALYSIS_H

#include "lexicalAnalysis.h"
#include "antiPolandExp.h"
#include "cstdio"
#include "cstdlib"
#include "list"

#define IDENT "Ident"
#define SYMBOL "Symbol"

using namespace std;

typedef struct {
	return_token token;
	bool is_const = false;
} variable_list_elem;

/**
 * 判断是 token 否在符号表中
 * @return 在符号表中返回 true
 */
bool list_contains(const return_token &);

/**
 * 判断是否为常量
 * @return 是常量返回 true
 */
bool is_variable_const(const return_token &);

void CompUnit(FILE *in, FILE *out);

#endif //LAB1_GRAMMARANALYSIS_H
