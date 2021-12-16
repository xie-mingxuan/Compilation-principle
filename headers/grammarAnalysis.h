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
#define NUMBER "Number"
#define IF_TRUE 1
#define IF_FALSE 2
#define IF_FINAL 3
#define WHILE_COND 4
#define WHILE_LOOP 5
#define WHILE_FINAL 6
#define INT 7
#define VOID 8

using namespace std;

typedef struct {
	return_token token;                // 当前 token
	bool is_const = false;            // 标记是否为 常量；默认值为 false
	bool is_global = false;            // 标记是否为 全局变量；默认为 false
	bool is_array = false;            // 标记是否为 数组；默认为 false
	bool is_function = false;        // 标记是否为 函数；默认为 false
	string saved_pointer;            // 记录当前变量的指针地址
	string saved_register;            // 记录当前变量的数值存储地址
	string variable_type;            // 记录变量的数据类型
	int code_block_layer;            // 记录当前变量的作用域
	int global_variable_value;        // 如果是全局变量，记录该变量的值
	int dimension = 0;                // 如果是数组，记录数组的维度，默认值为 0
	int dimension_num[50] = {'\0'};    // 如果是数组，记录数组的维度大小，默认值为 0
	int function_return_type = INT;        // 如果是函数，记录函数的返回值形式；默认是 int 类型的返回值
	int function_param_num = 0;          // 如果是函数，记录函数的参数个数
	string function_param_type[50];        // 如果是函数，记录函数参数的类型
	int function_param_dimension[50] = {'\0'};        // 如果是函数，记录参数的维度
} variable_list_elem;

typedef struct {
	int register_num;
	int block_type;
} undefined_code_block_stack_elem;

void exit_();

/**
 * 判断是 token 否在符号表当前层中定义过
 * @return 在符号表中返回 true
 */
bool is_variable_list_contains_in_this_layer(const return_token &);

/**
 * 判断是 token 否在符号表中定义过
 * @return 在符号表中返回 true
 */
bool is_variable_list_contains_in_all_layer(const return_token &);

/**
 * 更新符号表，将所有当前代码块的局部变量删掉
 */
void update_variable_list();

/**
 * 判断是否为常量
 * @return 是常量返回 true
 */
bool is_variable_const(const return_token &);

void CompUnit(FILE *in, FILE *out);

/**
 * 设置符号表中的变量的储存位置
 */
void set_register(const return_token &, const string &save_register);

/**
 * 获取当前 token 的寄存器位置
 */
string get_register(const return_token &);

/**
 * 获取当前 token 的指针寄存器位置
 */
string get_pointer(const return_token &);

/**
 * 在代码块中刷新符号表
 */
void print_variable_table();

/**
 * 打印下一个代码块
 */
void print_code_block(undefined_code_block_stack_elem);

variable_list_elem get_variable(const return_token &);

void Decl(FILE *);

void ConstDecl(FILE *);

void BType(FILE *);

void ConstDef(FILE *);

number_stack_elem ConstInitVal(FILE *input, const return_token &token);

number_stack_elem ConstExp(FILE *);

void VarDecl(FILE *);

void VarDef(FILE *);

number_stack_elem InitVal(FILE *);

void FuncDef(FILE *, bool is_main_func = false, int function_type = INT, const string &function_name = "main");

void FuncType(FILE *);

void Block(FILE *input, int function_type = INT, bool is_function_define = false);

void BlockItem(FILE *input, int function_type = INT);

void Stmt(FILE *input, int function_type = INT);

number_stack_elem Exp(FILE *);

void Cond(FILE *, bool, bool);

void LVal(FILE *);

void PrimaryExp(FILE *);

void AddExp(FILE *);

void MulExp(FILE *);

void UnaryExp(FILE *);

void FuncRParams(FILE *);

void Ident(FILE *);

void UnaryOp(FILE *);

void RelExp(FILE *);

void EqExp(FILE *);

void LAndExp(FILE *);

void LOrExp(FILE *);

void
init_array(const variable_list_elem &array, int *current_pos, int dimension, bool is_const_define = false,
		   bool is_global_define = false);

void init();

void reload_param();

void print_function_elem(const variable_list_elem &elem);

#endif //LAB1_GRAMMARANALYSIS_H
