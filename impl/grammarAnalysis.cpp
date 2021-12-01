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
bool need_br = true;                        // 标记当前是否还需要 br 跳转语句
int code_block_layer = 0;                    // 标记现在是第几层大括号（局部变量的层数）
stack<undefined_code_block_stack_elem> undefined_code_block_stack;

void update_can_deal_multiply_stmt();

void exit_() {
	fclose(input);
	fclose(output);
	exit(-1);
}

void FuncDef(FILE *file, bool is_main_func) {
	if (!is_main_func) {
		// 修改为主函数不需要判断函数类型了
		FuncType(file);
	} else {
		fprintf(output, "define i32 ");
	}
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
	if (word.type == IDENT || word.token == "int")
		fprintf(output, "define i32 ");
	else if (word.type == IDENT || word.token == "void")
		fprintf(output, "define void ");
	else exit_();
	word = get_symbol(file);

}

void Ident(FILE *file) {
	if (word.type != IDENT || word.token != "main")
		exit_();

	fprintf(output, "@%s", word.token.c_str());
	word = get_symbol(file);
}

void Block(FILE *file) {
	if (word.type != SYMBOL || word.token != "LBrace")
		exit_();
	code_block_layer++;
	word = get_symbol(file);
	fprintf(output, "{\n");
	print_variable_table();

	while (word.type != SYMBOL || word.token != "RBrace") {
		BlockItem(file);
	}
	update_variable_list();
	code_block_layer--;

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
	if (is_variable_list_contains_in_this_layer(word))
		exit_();
	variable_list_elem elem;
	elem.code_block_layer = code_block_layer;
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
	if (is_variable_list_contains_in_this_layer(word))
		exit_();
	variable_list_elem elem;
	elem.code_block_layer = code_block_layer;
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
	fprintf(output, "%%%d = load i32, i32* %s\t\t; define variable '%s'\n", register_num, get_pointer(x).c_str(),
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
			need_br = false;

			if (word.type != SYMBOL || word.token != "Semicolon")
				exit_();
			word = get_symbol(file);
			return;
		}
			// 判断语句
		else if (word.token == "If") {
			if (!can_deal_multiply_stmt) {
				if (can_deal_stmt_left == 0)
					return;
				else can_deal_stmt_left--;
			}
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

			Cond(file, is_else_if, false);
			is_else_if = false;

			if (word.type != SYMBOL || word.token != "RPar")
				exit_();
			word = get_symbol(file);

			bool can_deal_multiply_stmt_temp = can_deal_multiply_stmt;
			int can_deal_stmt_left_temp = can_deal_stmt_left;
			update_can_deal_multiply_stmt();
			last_token_is_if_or_else = true;
			Stmt(file);

			if (word.type == IDENT && word.token == "Else") {
				last_token_is_if_or_else = true;
				word = get_symbol(file);

				// 处理 else-if 语句
				if (word.type == IDENT && word.token == "If")
					is_else_if = true;
				bool can_deal_multiply_stmt_temp_1 = can_deal_multiply_stmt;
				int can_deal_stmt_left_temp_1 = can_deal_stmt_left;
				update_can_deal_multiply_stmt();
				Stmt(file);
				can_deal_stmt_left = can_deal_stmt_left_temp_1;
				can_deal_multiply_stmt = can_deal_multiply_stmt_temp_1;
			} else {
				undefined_code_block_stack_elem elem = undefined_code_block_stack.top();
				undefined_code_block_stack.pop();
				undefined_code_block_stack_elem final_elem = undefined_code_block_stack.top();
//				fprintf(output, "\n\n\n%d:\t; 定义缺省的 else 代码块\n", elem.register_num);
				print_code_block(elem);
				print_variable_table();

				fprintf(output, "br label %%IF_FINAL_%d\n", final_elem.register_num);
				need_br = false;
			}

			if (!undefined_code_block_stack.empty()) {
				undefined_code_block_stack_elem elem = undefined_code_block_stack.top();
				undefined_code_block_stack.pop();
//				fprintf(output, "\n\n\n%d:\t; 定义 if-else 语句之后的代码块\n", elem.register_num);
				print_code_block(elem);
				print_variable_table();
			}

			can_deal_stmt_left = can_deal_stmt_left_temp;
			can_deal_multiply_stmt = can_deal_multiply_stmt_temp;

			if (!can_deal_multiply_stmt) {
				if (can_deal_stmt_left != 0) {
					can_deal_stmt_left--;
					Stmt(file);
				}
			} else
				Stmt(file);

			if (!undefined_code_block_stack.empty()) {
				undefined_code_block_stack_elem elem = undefined_code_block_stack.top();
				stack<undefined_code_block_stack_elem> temp;
				if (elem.block_type == WHILE_FINAL) {
					fprintf(output, "br label %%WHILE_FINAL_%d\n", elem.register_num);
				} else {
					while (elem.block_type != IF_FINAL) {
						undefined_code_block_stack.pop();
						temp.push(elem);
						elem = undefined_code_block_stack.top();
					}
					fprintf(output, "br label %%IF_FINAL_%d\n", elem.register_num);
					while (!temp.empty()) {
						elem = temp.top();
						temp.pop();
						undefined_code_block_stack.push(elem);
					}
				}
			}
			need_br = false;
			return;
		}
			// 循环语句
		else if (word.token == "While") {
			word = get_symbol(input);
			if (word.type != SYMBOL || word.token != "LPar")
				exit_();
			fprintf(output, "br label %%WHILE_COND_%d\n", code_block_num);
			fprintf(output, "\n\n\nWHILE_COND_%d:\t; while 循环的判断条件\n", code_block_num);
			print_variable_table(); // TODO 可以删掉
			word = get_symbol(input);
			Cond(input, false, true);
			if (word.type != SYMBOL || word.token != "RPar")
				exit_();
			word = get_symbol(input);
			if (word.type != SYMBOL || word.token != "LBrace")
				exit_();
			code_block_layer++;
			undefined_code_block_stack_elem elem = undefined_code_block_stack.top();
			undefined_code_block_stack.pop();
			fprintf(output, "\n\n\nWHILE_LOOP_%d:\t; while 循环的循环体\n", elem.register_num);
			print_variable_table(); // TODO can be deleted
			word = get_symbol(input);
			while (word.type != SYMBOL || word.token != "RBrace")
				BlockItem(input);
			code_block_layer--;
			update_variable_list(); // TODO
			if (need_br)
				fprintf(output, "br label %%WHILE_COND_%d\n", elem.register_num);
			else need_br = true;

			elem = undefined_code_block_stack.top();
			undefined_code_block_stack.pop();
			fprintf(output, "\n\n\nWHILE_FINAL_%d:\t; while 循环的结束\n", elem.register_num);
			print_variable_table();
			word = get_symbol(input);
			return;
		}
			// break 语句 或 continue 语句
		else if (word.token == "Continue" || word.token == "Break") {
			undefined_code_block_stack_elem elem = undefined_code_block_stack.top();
			stack<undefined_code_block_stack_elem> temp;
			if (last_token_is_if_or_else) {
				if (elem.block_type == IF_TRUE)
					fprintf(output, "\n\n\nIF_TRUE_%d:\n", elem.register_num);
				else
					fprintf(output, "\n\n\nIF_FALSE_%d:\n", elem.register_num);
				last_token_is_if_or_else = false;
				undefined_code_block_stack.pop();
				elem = undefined_code_block_stack.top();
			}
			while (elem.block_type != WHILE_FINAL) {
				temp.push(elem);
				undefined_code_block_stack.pop();
				elem = undefined_code_block_stack.top();
			}
			if (word.token == "Continue")
				fprintf(output, "br label %%WHILE_COND_%d\n", elem.register_num);
			else
				fprintf(output, "br label %%WHILE_FINAL_%d\n", elem.register_num);
			while (!temp.empty()) {
				elem = temp.top();
				temp.pop();
				undefined_code_block_stack.push(elem);
			}
			word = get_symbol(input);
			if (word.type != SYMBOL || word.token != "Semicolon")
				exit_();
			word = get_symbol(input);
			need_br = false;
			return;
		}
			// putch 语句
		else if (word.token == "putch") {
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
		}
			// putint 语句
		else if (word.token == "putint") {
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

		if (!is_variable_list_contains_in_all_layer(word)) {
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
			fprintf(output, "br label %%IF_FINAL_%d\n", undefined_code_block_stack.top().register_num);
			need_br = false;
			while (!temp.empty()) {
				elem = temp.top();
				temp.pop();
				undefined_code_block_stack.push(elem);
			}
		}
		last_token_is_if_or_else = false;
	} else if (word.type == SYMBOL && word.token == "LBrace") {
		int final_label;
		code_block_layer++;
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
		code_block_layer--;
		update_variable_list();
		last_token_is_if_or_else = last_token_is_if_or_else_temp;

		// 如果仍然有未定义的代码段，跳转到最近的 IF_FINAL 代码段
		if (last_token_is_if_or_else && !undefined_code_block_stack.empty()) {
//			fprintf(output, "br label %%%d\n", final_label);
			if (need_br) {
				fprintf(output, "br label %%IF_FINAL_%d\n", final_label);
			}
			last_token_is_if_or_else = false;
		}
	}
		// 跳过无意义语句
	else {
		while (word.type != SYMBOL || word.token != "Semicolon") {
			if (word.type == SYMBOL && word.token == "RBrace")
				return;
			word = get_symbol(file);
		}
	}
	need_br = true;
	word = get_symbol(file);
}

void CompUnit(FILE *in, FILE *out) {
	input = in;
	output = out;
	word = get_symbol(in);
	// 全局变量定义
	while (true) {
		bool is_const_define = false;
		string variable_name;
		variable_list_elem elem;
		elem.is_global = true;
		if (word.type == IDENT && word.token == "const") {
			word = get_symbol(input);
			is_const_define = true;
		}
		if (word.type != IDENT || word.token != "int")
			exit_();
		word = get_symbol(input);
		if (word.type == IDENT && word.token == "main")
			break;

		// 处理一句话中的多重定义
		while (word.type != SYMBOL || word.token != "Semicolon") {
			variable_name = word.token;
			elem.token.type = IDENT;
			elem.token.token = variable_name;
			elem.saved_pointer = "@" + variable_name;
			elem.code_block_layer = 0;
			if (is_const_define)
				elem.is_const = true;

			word = get_symbol(input);

			if (word.type == SYMBOL && (word.token == "Semicolon" || word.token == "Comma")) {
				fprintf(output, "@%s = global i32 0\t; 定义全局变量 %s = 0\n", variable_name.c_str(),
						variable_name.c_str());
				elem.global_variable_value = 0;
				variable_list.push_back(elem);
				// 遇到逗号继续
				if (word.token == "Comma") {
					word = get_symbol(input);
					continue;
				}
					// 遇到分号则终止本句话
				else if (word.token == "Semicolon") {
					word = get_symbol(input);
					break;
				}
			}

			if (word.type != SYMBOL || word.token != "Assign")
				exit_();
			word = get_symbol(input);
			number_stack_elem res = calcAntiPoland(input, true, true);
			fprintf(output, "@%s = global i32 %d\t; 定义全局变量 %s = %d\n", variable_name.c_str(), res.token.num,
					variable_name.c_str(), res.token.num);
			elem.global_variable_value = res.token.num;
			variable_list.push_back(elem);
			if (word.type == SYMBOL && (word.token == "Semicolon" || word.token == "Comma")) {
				// 遇到逗号继续
				if (word.token == "Comma") {
					word = get_symbol(input);
					continue;
				}
					// 遇到分号则终止本句话
				else if (word.token == "Semicolon") {
					word = get_symbol(input);
					break;
				}
			} else exit_();
		}
	}
	init();
	FuncDef(input, true);
	word = get_symbol(in);
	if (word.type == "Error")
		exit_();
}

void Cond(FILE *file, bool is_else_if_cond = false, bool is_while_cond = false) {
	number_stack_elem res = calcAntiPoland(file);
	fprintf(output, "%%%d = icmp ne i32 ", register_num++);
	if (res.is_variable)
		fprintf(output, "%s", res.variable.c_str());
	else fprintf(output, "%d", res.token.num);
	fprintf(output, ", 0\t; 将 i32 的值转化为 i1 形式，然后进行判断\n");

	// 来自 while 循环语句
	if (is_while_cond) {
		fprintf(output, "br i1 %%%d, label %%WHILE_LOOP_%d, label %%WHILE_FINAL_%d\t; 将 i1 形式的值进行判断，然后选择跳转块\n",
				register_num - 1, code_block_num, code_block_num);
		// 在来自 while 循环的语句中，需要添加两个新的代码块
		undefined_code_block_stack_elem elem;
		elem.register_num = code_block_num++;
		elem.block_type = WHILE_FINAL;
		undefined_code_block_stack.push(elem);
		elem.block_type = WHILE_LOOP;
		undefined_code_block_stack.push(elem);
	}
		// 来自 else if 语句
	else if (is_else_if_cond) {
		fprintf(output, "br i1 %%%d, label %%IF_TRUE_%d, label %%IF_FALSE_%d\t; 将 i1 形式的值进行判断，然后选择跳转块\n",
				register_num - 1, code_block_num, code_block_num);
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

bool is_variable_list_contains_in_this_layer(const return_token &token) {
	if (variable_list.empty())
		return false;
	for (const auto &variable: variable_list) {
		if (variable.code_block_layer == code_block_layer && variable.token == token)
			return true;
	}
	return false;
}

bool is_variable_list_contains_in_all_layer(const return_token &token) {
	if (variable_list.empty())
		return false;
	for (const auto &variable: variable_list) {
		if (variable.code_block_layer <= code_block_layer && variable.token == token)
			return true;
	}
	return false;
}

void update_variable_list() {
	list<variable_list_elem> new_list;
	for (const auto &variable: variable_list) {
		if (variable.code_block_layer <= code_block_layer)
			new_list.push_back(variable);
	}
	variable_list = new_list;
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
	for (int layer = code_block_layer; layer >= 0; layer--) {
		for (auto &variable: variable_list) {
			if (variable.code_block_layer == layer && variable.token == token) {
				variable.saved_register = save_register;
				return;
			}
		}
	}
}

string get_register(const return_token &token) {
	for (int layer = code_block_layer; layer >= 0; layer--) {
		for (auto &variable: variable_list) {
			if (variable.code_block_layer == layer && variable.token == token) {
				if (variable.saved_register.empty()) {
					printf("%s has never been initialized!\n", token.token.c_str());
					exit(-1);
				}
				return variable.saved_register;
			}
		}
	}
	return "";
}

string get_pointer(const return_token &token) {
	for (int layer = code_block_layer; layer >= 0; layer--) {
		for (auto &variable: variable_list) {
			if (variable.code_block_layer == layer && variable.token == token)
				return variable.saved_pointer;
		}
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

variable_list_elem get_variable(const return_token &token) {
	int layer = code_block_layer;
	for (; layer >= 0; layer--) {
		for (auto &i: variable_list) {
			if (token == i.token)
				return i;
		}
	}
}