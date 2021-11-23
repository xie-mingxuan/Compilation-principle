//
// Created by 谢铭轩 on 2021/10/14.
//

#include <sstream>
#include "../headers/grammarAnalysis.h"

return_token word;
FILE *input;
FILE *output;
list<variable_list_elem> variable_list;        // 这是所有定义的变量
extern int register_num;
bool last_token_is_if_or_else = false;        // 标记上一个 token 是不是 if 或 else
bool is_else_if = false;                    // 标记是否为 else if 语句
int code_block_num = 1;                        // 标记当前应该处理第几个代码块
bool can_deal_multiply_stmt = true;            // 如果可以连续处理多句 stmt 语句则为真
int can_deal_stmt_left = 0;                    // 标记当前仍然可以处理多少 stmt 语句
stack<undefined_code_block_stack_elem> undefined_code_block_stack;

void update_can_deal_multiply_stmt();

void exit_() {
	fclose(input);
	fclose(output);
	exit(-1);
}

void FuncDef(FILE *file) {
	FuncType(file);
	Ident(file);
	if (word.type != SYMBOL || word.token != "LPar")
		exit_();
	word = get_symbol(file);
	fprintf(output, "(");

	if (word.type != SYMBOL || word.token != "RPar")
		exit_();
	word = get_symbol(file);
	fprintf(output, ")");

	Block(file);

	word = get_symbol(file);
}

void FuncType(FILE *file) {
	if (word.type != IDENT || word.token != "int")
		exit_();

	word = get_symbol(file);
	fprintf(output, "define i32 ");
}

void Ident(FILE *file) {
	if (word.type != IDENT || word.token != "main")
		exit_();

	word = get_symbol(file);
	fprintf(output, "@main");
}

void Block(FILE *file) {
	if (word.type != SYMBOL || word.token != "LBrace")
		exit_();
	word = get_symbol(file);
	fprintf(output, "{\n");

	while (word.type != SYMBOL || word.token != "RBrace") {
		BlockItem(file);
	}

	if (word.type != SYMBOL || word.token != "RBrace")
		exit_();
	word = get_symbol(file);
	fprintf(output, "}\n");
}

void BlockItem(FILE *file) {
	if (word.type == IDENT && (word.token == "const" || word.token == "int"))
		Decl(file);
	else
		Stmt(file);
}

void Decl(FILE *file) {
	if (word.type == IDENT && word.token == "const")
		ConstDecl(file);
	else if (word.type == IDENT && word.token == "int")
		VarDecl(file);
	else exit_();
}

void ConstDecl(FILE *file) {
	if (word.type != IDENT || word.token != "const")
		exit_();
	word = get_symbol(file);

	if (word.type != IDENT || word.token != "int")
		exit_();
	word = get_symbol(file);

	ConstDef(file);

	while (word.type == SYMBOL && word.token == "Comma") {
		word = get_symbol(file);
		ConstDef(file);
	}

	if (word.type != SYMBOL || word.token != "Semicolon")
		exit_();

	word = get_symbol(file);
}

void ConstDef(FILE *file) {
	if (word.type != IDENT)
		exit_();

	// 如果声明的变量已经被声明过了，就要退出；否则将其加入符号表
	if (list_contains(word))
		exit_();
	variable_list_elem elem;
	elem.token = word;
	elem.is_const = true;
	stringstream stream;
	stream << register_num++;
	elem.saved_pointer = "%" + stream.str();
	variable_list.push_back(elem);
	fprintf(output, "%s = alloca i32\t\t; define const variable_pointer '%s' at %s\n", elem.saved_pointer.c_str(),
			word.token.c_str(), elem.saved_pointer.c_str());

	return_token x = word;
	string i = word.token;

	word = get_symbol(file);

	if (word.type != SYMBOL || word.token != "Assign")
		exit_();
	word = get_symbol(file);

	number_stack_elem res = ConstInitVal(file);
	if (res.is_variable)
		fprintf(output, "store i32 %s, i32* %s\n", res.variable.c_str(), get_pointer(x).c_str());
	else
		fprintf(output, "store i32 %d, i32* %s\n", res.token.num, get_pointer(x).c_str());
	fprintf(output, "%%%d = load i32, i32* %s\t\t; define const variable '%s'\n\n", register_num,
			get_pointer(x).c_str(),
			i.c_str());
	stringstream stream1;
	stream1 << register_num++;
	set_register(x, "%" + stream1.str());

}

number_stack_elem ConstInitVal(FILE *file) {
	return ConstExp(file);
}

/**
 * 这个函数用来计算表达式的值，最后返回表达式的值或对应的储存元素
 */
number_stack_elem ConstExp(FILE *file) {
	return calcAntiPoland(file, true);
}

void VarDecl(FILE *file) {
	if (word.type != IDENT || word.token != "int")
		exit_();
	word = get_symbol(file);
	VarDef(file);

	while (word.type == SYMBOL && word.token == "Comma") {
		word = get_symbol(file);
		VarDef(file);
	}

	if (word.type != SYMBOL && word.token != "Semicolon")
		exit_();

	word = get_symbol(file);
}

void VarDef(FILE *file) {
	if (word.type != IDENT)
		exit_();

	// 如果声明的变量已经被声明过了，就要退出；否则将其加入符号表
	if (list_contains(word))
		exit_();
	variable_list_elem elem;
	elem.token = word;
	stringstream stream;
	stream << register_num++;
	elem.saved_pointer = "%" + stream.str();
	variable_list.push_back(elem);
	fprintf(output, "%s = alloca i32\t\t; define variable_point '%s' at %s\n", elem.saved_pointer.c_str(),
			word.token.c_str(), elem.saved_pointer.c_str());

	return_token x = word;
	string i = word.token;

	word = get_symbol(file);

	if (word.type != SYMBOL || word.token != "Assign")
		return;

	word = get_symbol(file);
	number_stack_elem res = InitVal(file);
	if (res.is_variable)
		fprintf(output, "store i32 %s, i32* %s\n", res.variable.c_str(), get_pointer(x).c_str());
	else
		fprintf(output, "store i32 %d, i32* %s\n", res.token.num, get_pointer(x).c_str());
	fprintf(output, "%%%d = load i32, i32* %s\t\t; define variable '%s'\n\n", register_num, get_pointer(x).c_str(),
			i.c_str());
	stringstream stream1;
	stream1 << register_num++;
	set_register(x, "%" + stream1.str());
}

number_stack_elem InitVal(FILE *file) {
	return Exp(file);
}

number_stack_elem Exp(FILE *file) {
	//fprintf(output, "%s\n", calcAntiPoland(file).c_str());
	return calcAntiPoland(file);
}

void Stmt(FILE *file) {
	if (word.type == IDENT) {
		// 返回语句
		if (word.token == "Return") {
			if (!can_deal_multiply_stmt) {
				if (can_deal_stmt_left == 0)
					return;
				can_deal_stmt_left--;
			}
			word = get_symbol(file);
			number_stack_elem res = calcAntiPoland(file);
			if (res.is_variable)
				fprintf(output, "ret i32 %s\n", res.variable.c_str());
			else fprintf(output, "ret i32 %d\n", res.token.num);

			if (word.type != SYMBOL || word.token != "Semicolon")
				exit_();
			word = get_symbol(file);
			return;
		}
			// 判断语句
		else if (word.token == "If") {
			// 如果是 判断语句下的 判断语句，则需要率先写出上层代码块的定义
			if (last_token_is_if_or_else) {
				undefined_code_block_stack_elem elem = undefined_code_block_stack.top();
				undefined_code_block_stack.pop();
				print_code_block(elem);
				print_variable_table();
			}


			word = get_symbol(file);
			if (word.type != SYMBOL || word.token != "LPar")
				exit_();
			word = get_symbol(file);

			Cond(file, is_else_if);
			is_else_if = false;

			if (word.type != SYMBOL || word.token != "RPar")
				exit_();
			word = get_symbol(file);

			bool can_deal_multiply_stmt_temp = can_deal_multiply_stmt;
			int can_deal_stmt_left_temp = can_deal_stmt_left;
			update_can_deal_multiply_stmt();
			last_token_is_if_or_else = true;
			Stmt(file);
			can_deal_stmt_left = can_deal_stmt_left_temp;
			can_deal_multiply_stmt = can_deal_multiply_stmt_temp;

			if (word.type == IDENT && word.token == "Else") {
				last_token_is_if_or_else = true;
				word = get_symbol(file);
				if (word.type == IDENT && word.token == "If")
					is_else_if = true;
				can_deal_multiply_stmt_temp = can_deal_multiply_stmt;
				can_deal_stmt_left_temp = can_deal_stmt_left;
				update_can_deal_multiply_stmt();
				Stmt(file);
				can_deal_stmt_left = can_deal_stmt_left_temp;
				can_deal_multiply_stmt = can_deal_multiply_stmt_temp;
				if()
			} else {
				undefined_code_block_stack_elem elem = undefined_code_block_stack.top();
				undefined_code_block_stack.pop();
				undefined_code_block_stack_elem final_elem = undefined_code_block_stack.top();
//				fprintf(output, "\n\n\n%d:\t; 定义缺省的 else 代码块\n", elem.register_num);
				print_code_block(elem);

				fprintf(output, "br label %%IF_FINAL_%d\n", final_elem.register_num);
			}

			if (!undefined_code_block_stack.empty()) {
				undefined_code_block_stack_elem elem = undefined_code_block_stack.top();
				undefined_code_block_stack.pop();
//				fprintf(output, "\n\n\n%d:\t; 定义 if-else 语句之后的代码块\n", elem.register_num);
				print_code_block(elem);
				print_variable_table();
			}

			if (!can_deal_multiply_stmt) {
				if (can_deal_stmt_left != 0) {
					can_deal_stmt_left--;
					Stmt(file);
				}
			} else
				Stmt(file);

			// TODO
//			if (!undefined_code_block_stack.empty()) {
//				undefined_code_block_stack_elem elem;
//				stack<undefined_code_block_stack_elem> temp;
//				while (elem.block_type != IF_FINAL) {
//					elem = undefined_code_block_stack.top();
//					undefined_code_block_stack.pop();
//					temp.push(elem);
//				}
//				fprintf(output, "br label %%IF_FINAL_%d\n", elem.register_num);
//				while (!temp.empty()) {
//					elem = temp.top();
//					temp.pop();
//					undefined_code_block_stack.push(elem);
//				}
//			}
			return;
		} else if (word.token == "putch") {
			if (!can_deal_multiply_stmt) {
				if (can_deal_stmt_left == 0)
					return;
				can_deal_stmt_left--;
			}
			word = get_symbol(file);
			if (word.type != SYMBOL || word.token != "LPar")
				exit(-1);

			number_stack_elem res = calcAntiPoland(file);
			if (res.is_variable)
				fprintf(output, "call void @putch(i32 %s)\n", res.variable.c_str());
			else
				fprintf(output, "call void @putch(i32 %d)\n", res.token.num);

			if (word.type != SYMBOL || word.token != "Semicolon")
				exit(-1);

			word = get_symbol(file);
			return;
		} else if (word.token == "putint") {
			if (!can_deal_multiply_stmt) {
				if (can_deal_stmt_left == 0)
					return;
				can_deal_stmt_left--;
			}
			word = get_symbol(file);
			if (word.type != SYMBOL || word.token != "LPar")
				exit(-1);

			number_stack_elem res = calcAntiPoland(file);
			if (res.is_variable)
				fprintf(output, "call void @putint(i32 %s)\n", res.variable.c_str());
			else
				fprintf(output, "call void @putint(i32 %d)\n", res.token.num);

			if (word.type != SYMBOL || word.token != "Semicolon")
				exit(-1);

			word = get_symbol(file);
			return;
		}

		if (!list_contains(word)) {
			printf("%s has never been defined!\n", word.token.c_str());
			exit_();
		}

		// 无意义语句直接跳过
		return_token x = word;
		word = get_symbol(file);
		if (word.type != SYMBOL && word.token != "Assign") {
			while (word.type != SYMBOL || word.token != "Semicolon")
				word = get_symbol(file);
			word = get_symbol(file);
			return;
		}

		// 赋值语句先判断是否是常量，然后再进行赋值
		if (is_variable_const(x)) {
			printf("%s is a const, cannot be assigned!\n", x.token.c_str());
			exit_();
		}

		if (!can_deal_multiply_stmt) {
			if (can_deal_stmt_left == 0)
				return;
			can_deal_stmt_left--;
		}

		// 如果赋值语句正处在一个 if_else 代码块的下面，则将其作为该代码块的成分
		if (last_token_is_if_or_else) {
			undefined_code_block_stack_elem elem = undefined_code_block_stack.top();
			undefined_code_block_stack.pop();
//			fprintf(output, "\n\n\n%%%d:\t; 定义省略了大括号的赋值语句\n", elem.register_num);
			print_code_block(elem);
			print_variable_table();
		}
		word = get_symbol(file);
		number_stack_elem res = calcAntiPoland(file);
		if (res.is_variable)
			fprintf(output, "store i32 %s, i32* %s\n", res.variable.c_str(), get_pointer(x).c_str());
		else
			fprintf(output, "store i32 %d, i32* %s\n", res.token.num, get_pointer(x).c_str());
		fprintf(output, "%%%d = load i32, i32* %s\t\t; set variable '%s'\n", register_num, get_pointer(x).c_str(),
				x.token.c_str());
		stringstream stream;
		stream << register_num++;
		set_register(x, "%" + stream.str());

		if (word.type != SYMBOL || word.token != "Semicolon")
			exit_();

		// 同样，要将其后面添加上跳转代码块
		if (last_token_is_if_or_else) {
			stack<undefined_code_block_stack_elem> temp;
			undefined_code_block_stack_elem elem;
			while (undefined_code_block_stack.top().block_type != IF_FINAL) {
				elem = undefined_code_block_stack.top();
				undefined_code_block_stack.pop();
				temp.push(elem);
			}
			fprintf(output, "br label %%IF_FINAL_%%%d\n", undefined_code_block_stack.top().register_num);
			while (!temp.empty()) {
				elem = temp.top();
				temp.pop();
				undefined_code_block_stack.push(elem);
			}
		}
		word = get_symbol(file);
		return;
	} else if (word.type == SYMBOL && word.token == "LBrace") {
		int final_label;
		bool last_token_is_if_or_else_temp = last_token_is_if_or_else;
		if (last_token_is_if_or_else) {
			last_token_is_if_or_else = false;
			undefined_code_block_stack_elem elem = undefined_code_block_stack.top();
			undefined_code_block_stack.pop();
			if (elem.block_type != IF_FINAL) {
				// 找到最近的一个 IF_FINAL 代码段
				stack<undefined_code_block_stack_elem> temp;
				while (undefined_code_block_stack.top().block_type != IF_FINAL) {
					undefined_code_block_stack_elem e = undefined_code_block_stack.top();
					undefined_code_block_stack.pop();
					temp.push(e);
				}
				final_label = undefined_code_block_stack.top().register_num;
				while (!temp.empty()) {
					undefined_code_block_stack_elem e = temp.top();
					temp.pop();
					undefined_code_block_stack.push(e);
				}
			} else final_label = elem.register_num;
//			fprintf(output, "\n\n\n%d:\t; 定义代码块 %d\n", elem.register_num, elem.register_num);
			print_code_block(elem);
			print_variable_table();
			last_token_is_if_or_else = false;
		}

		word = get_symbol(file);
		while (word.type != SYMBOL || word.token != "RBrace")
			BlockItem(file);

		last_token_is_if_or_else = last_token_is_if_or_else_temp;

		// 如果仍然有未定义的代码段，跳转到最近的 IF_FINAL 代码段
		if (last_token_is_if_or_else && !undefined_code_block_stack.empty()) {
//			fprintf(output, "br label %%%d\n", final_label);
			fprintf(output, "br label %%IF_FINAL_%d\n", final_label);
			last_token_is_if_or_else = false;
		}
	}
		// 跳过无意义语句
	else {
		while (word.type != SYMBOL || word.token != "Semicolon")
			if (word.type == SYMBOL && word.token == "RBrace")
				return;
		word = get_symbol(file);
	}
	word = get_symbol(file);
}

void CompUnit(FILE *in, FILE *out) {
	input = in;
	output = out;
	word = get_symbol(in);
	init();
	FuncDef(in);
	word = get_symbol(in);
	if (word.type == "Error")
		exit_();
}

void Cond(FILE *file, bool is_else_if = false) {
	number_stack_elem res = calcAntiPoland(file);
	fprintf(output, "%%%d = icmp eq i32 ", register_num++);
	if (res.is_variable)
		fprintf(output, "%s", res.variable.c_str());
	else fprintf(output, "%d", res.token.num);
	fprintf(output, ", 0\t; 将 i32 的值转化为 i1 形式，然后进行判断\n");

	// 来自 else if 语句
	if (is_else_if) {
		fprintf(output, "br i1 %%%d, label %%IF_TRUE_%d, label %%IF_FALSE_%d\t; 将 i1 形式的值进行判断，然后选择跳转块\n",
				register_num - 1,
				code_block_num, code_block_num);
		// 在来自 else if 的语句中，只需要向其中添加两个新的代码块
		undefined_code_block_stack_elem elem;
		elem.register_num = code_block_num++;
		elem.block_type = IF_FALSE;
		undefined_code_block_stack.push(elem);
		elem.block_type = IF_TRUE;
		undefined_code_block_stack.push(elem);
	}

		//来自普通 if 语句
	else {
		fprintf(output, "br i1 %%%d, label %%IF_TRUE_%d, label %%IF_FALSE_%d\t; 将 i1 形式的值进行判断，然后选择跳转块\n",
				register_num - 1,
				code_block_num, code_block_num);
		// 保留下了接下来的三个代码块，分别用于 条件为真、条件为假、条件语句结束 的对应代码块，然后倒序入栈。
		undefined_code_block_stack_elem elem;
		elem.register_num = code_block_num++;
		elem.block_type = IF_FINAL;
		undefined_code_block_stack.push(elem);
		elem.block_type = IF_FALSE;
		undefined_code_block_stack.push(elem);
		elem.block_type = IF_TRUE;
		undefined_code_block_stack.push(elem);
	}
}

void init() {
	fprintf(output, "declare i32 @getint()\n"
					"declare void @putint(i32)\n"
					"declare i32 @getch()\n"
					"declare void @putch(i32)\n\n");
}

bool list_contains(const return_token &token) {
	if (variable_list.empty())
		return false;
	for (const auto &saved_token: variable_list) {
		if (saved_token.token == token)
			return true;
	}
	return false;
}

bool is_variable_const(const return_token &token) {
	if (variable_list.empty())
		return false;
	for (const auto &saved_token: variable_list) {
		if (saved_token.token == token)
			return saved_token.is_const;
	}
	return false;
}

void set_register(const return_token &token, const string &save_register) {
	for (auto &saved_token: variable_list) {
		if (saved_token.token == token) {
			saved_token.saved_register = save_register;
			return;
		}
	}
}

string get_register(const return_token &token) {
	for (auto &saved_token: variable_list) {
		if (saved_token.token == token) {
			if (saved_token.saved_register.empty()) {
				printf("%s has never been initialized!\n", token.token.c_str());
				exit(-1);
			}
			return saved_token.saved_register;
		}
	}
	return "";
}

string get_pointer(const return_token &token) {
	for (auto &saved_token: variable_list) {
		if (saved_token.token == token)
			return saved_token.saved_pointer;
	}
	return "";
}

void print_variable_table() {
	for (auto &i: variable_list) {
		fprintf(output, "%%%d = load i32, i32* %s\t; 代码块中重新定义变量 %s\n", register_num,
				i.saved_pointer.c_str(),
				i.token.token.c_str());
		stringstream stream;
		stream << register_num++;
		i.saved_register = "%" + stream.str();
	}
}

void print_code_block(undefined_code_block_stack_elem elem) {
	fprintf(output, "\n\n\n");
	if (elem.block_type == IF_TRUE)
		fprintf(output, "IF_TRUE_");
	else if (elem.block_type == IF_FALSE)
		fprintf(output, "IF_FALSE_");
	else
		fprintf(output, "IF_FINAL_");
	fprintf(output, "%d:\n", elem.register_num);
}

void update_can_deal_multiply_stmt() {
	// 只有在下一个符号就是大括号的时候，才能多条处理 stmt 语句
	if (word.type == SYMBOL && word.token == "LBrace") {
		can_deal_multiply_stmt = true;
	} else {
		can_deal_multiply_stmt = false;
		can_deal_stmt_left = 1;
	}
}