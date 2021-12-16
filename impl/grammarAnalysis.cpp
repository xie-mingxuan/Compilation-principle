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
int if_else_code_block_num = 1;                // 标记当前应该处理第几个 if else 代码块
int while_code_block_num = 1;                // 标记当前应该处理第几个 while 代码块
int logic_or_code_block_num = 1;                // 标记当前应该处理第几个 逻辑或 代码块
int logic_and_code_block_num = 1;                // 标记当前应该处理第几个 逻辑与 代码块
bool can_deal_multiply_stmt = true;            // 如果可以连续处理多句 stmt 语句则为真
int can_deal_stmt_left = 0;                    // 标记当前仍然可以处理多少 stmt 语句
bool need_br = true;                        // 标记当前是否还需要 br 跳转语句
int code_block_layer = 0;                    // 标记现在是第几层大括号（局部变量的层数）
bool have_returned = false;                  // 标记函数现在是否已经进行了返回
bool have_print_block_name = false;            // 标记当前已经打印过了块名称
stack<undefined_code_block_stack_elem> undefined_code_block_stack;

void update_can_deal_multiply_stmt();

void exit_() {
	fclose(input);
	fclose(output);
	exit(-1);
}

void FuncDef(FILE *file, bool is_main_func, int function_type, const string &function_name) {
	if (function_type == VOID) {
		fprintf(output, "define void ");
	} else {
		fprintf(output, "define i32 ");
	}
//	Ident(file);
	variable_list_elem function_elem;
	if (is_main_func) {
		fprintf(output, "@main");
		word = get_symbol(file);
	} else {
		function_elem.code_block_layer = 0;
		function_elem.token.type = "Ident";
		function_elem.token.token = function_name;
		function_elem.saved_pointer = "@" + function_name;
		function_elem.is_function = true;
		function_elem.is_array = false;
		function_elem.function_return_type = function_type;
		fprintf(output, "@%s", function_name.c_str());
		register_num = 0;
	}
	if (word.type != SYMBOL || word.token != "LPar")
		exit_();
	word = get_symbol(file);
	fprintf(output, "(");

	int func_param_number = 0;
	while (word.type != SYMBOL || word.token != "RPar") {
		if (word.type != IDENT || word.token != "int")
			exit_();
		word = get_symbol(file);
		variable_list_elem elem;
		elem.token = word;
		stringstream stream;
		stream << register_num++;
		elem.saved_register = "%" + stream.str();
		elem.code_block_layer = 1;

		word = get_symbol(file);
		if (word.type == SYMBOL && (word.token == "Comma" || word.token == "RPar")) {
			elem.variable_type = "i32";
			function_elem.function_param_type[++func_param_number] = "i32";
			fprintf(output, "%s %%%d, ", elem.variable_type.c_str(), register_num - 1);
			variable_list.push_back(elem);
			if (word.token == "RPar")
				break;
			word = get_symbol(file);
			continue;
		} else {
			if (word.type != SYMBOL || word.token != "[")
				exit_();
			word = get_symbol(file);
			if (word.type != SYMBOL || word.token != "]")
				exit_();
			elem.dimension++;
			function_elem.function_param_dimension[++func_param_number]++;
			word = get_symbol(file);
			// 二维数组
			if (word.type == SYMBOL && word.token == "[") {
				word = get_symbol(file);
				elem.dimension++;
				elem.dimension_num[2] = word.num;
				word = get_symbol(file);
				if (word.type != SYMBOL || word.token != "]")
					exit_();
				word = get_symbol(file);
				function_elem.function_param_dimension[func_param_number]++;
			}
			// 数组指针（无论一维还是二维）都是 i32 * 类型
			elem.variable_type = "i32*";
			function_elem.function_param_type[func_param_number] = "i32*";
			elem.is_array = true;
			variable_list.push_back(elem);
		}
		fprintf(output, "%s %%%d, ", elem.variable_type.c_str(), register_num - 1);

		if (word.type == SYMBOL && word.token == "Comma")
			word = get_symbol(file);
	}
	function_elem.function_param_num = func_param_number;

	if (!is_main_func) {
		variable_list.push_back(function_elem);
		if (function_elem.function_param_num != 0)
			fseek(output, -2, SEEK_CUR);
	}
	word = get_symbol(file);
	fprintf(output, ")");
	if (!is_main_func)
		register_num++;

	Block(file, function_type, true);

	register_num = 1;

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

void Block(FILE *file, int function_type, bool is_function_define) {
	have_returned = false;
	if (word.type != SYMBOL || word.token != "LBrace")
		exit_();
	code_block_layer++;
	word = get_symbol(file);
	fprintf(output, "{\n");
	if (!is_function_define)
		print_variable_table();
	else
		reload_param();

	while (word.type != SYMBOL || word.token != "RBrace") {
		BlockItem(file, function_type);
	}
	update_variable_list();
	code_block_layer--;
	update_variable_list();

	//word = get_symbol(file);

	if (!have_returned) {
		if (function_type == INT)
			fprintf(output, "ret i32 0\n");
		else fprintf(output, "ret void\n");
	}

	fprintf(output, "}\n\n");
}

void BlockItem(FILE *file, int function_type) {
	if (word.type == IDENT && (word.token == "const" || word.token == "int"))
		Decl(file);
	else
		Stmt(file, function_type);
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
	return_token x = word;

	variable_list_elem elem;
	elem.code_block_layer = code_block_layer;
	elem.token = word;
	elem.is_const = true;
	stringstream stream;
	stream << register_num++;
	elem.saved_pointer = "%" + stream.str();

	// 判断该变量是否为数组
	word = get_symbol(file);
	bool is_array_define = false;
	int dimension = 0;
	while (word.type == SYMBOL && word.token == "[") {
		dimension++;
		is_array_define = true;
		elem.is_array = true;
		word = get_symbol(input);
		elem.dimension_num[dimension] = calcAntiPoland(input, true, false).token.num;
		if (elem.dimension_num[dimension] <= 0)
			exit_();
		if (word.type != SYMBOL || word.token != "]")
			exit_();
		word = get_symbol(input);
	}
	elem.dimension = dimension;

	if (is_array_define) {
		int total = 1;
		for (int i = 1; i <= elem.dimension; i++)
			total *= elem.dimension_num[i];
		fprintf(output, "%s = alloca [%d x i32]\t\t; 将数组 %s 的指针指定在 %s 的位置\n", elem.saved_pointer.c_str(), total,
				elem.token.token.c_str(), elem.saved_pointer.c_str());
		stringstream stream1;
		stream1 << total;
		elem.variable_type = "[" + stream1.str() + " x i32]";
	} else {
		fprintf(output, "%s = alloca i32\t\t; 将变量 %s 的指针定义在 %s 的位置\n", elem.saved_pointer.c_str(),
				elem.token.token.c_str(), elem.saved_pointer.c_str());
		elem.variable_type = "i32";
	}

	if (word.type != SYMBOL || word.token != "Assign") {
		variable_list.push_back(elem);
		return;
	}

	word = get_symbol(file);

	if (is_array_define) {
		// lab 7 要求可以定义数组元素
		if (word.type != SYMBOL || word.token != "LBrace")
			exit_();
		int current_define_pos[50] = {'\0'};
		word = get_symbol(input);
		if (word.type == SYMBOL && word.token == "RBrace") {
			int total = 1;
			for (int i = 1; i <= elem.dimension; i++)
				total *= elem.dimension_num[i];
			fprintf(output, "%%%d = getelementptr %s, %s* %s, i32 0, i32 0\n",
					register_num++, elem.variable_type.c_str(), elem.variable_type.c_str(), elem.saved_pointer.c_str());
			fprintf(output, "call void @memset(i32* %%%d, i32 0, i32 %d)\n", register_num - 1, total);
		} else
			init_array(elem, current_define_pos, 1, false, false);
	} else {
		number_stack_elem res = ConstExp(file);
		elem.token.num = res.token.num;
		if (res.token.num >= 0) {
			fprintf(output, "%%%d = add i32 0, %d", register_num++, res.token.num);
		} else {
			fprintf(output, "%%%d = sub i32 0, %d", register_num++, abs(res.token.num));
		}
		fprintf(output, "\t\t\t; 将常数 %s 定义在 %%%d 的位置\n", elem.token.token.c_str(), register_num - 1);
	}
	variable_list.push_back(elem);
}

number_stack_elem ConstInitVal(FILE *file, return_token &token) {
	variable_list_elem elem = get_variable(token);
	if (!elem.is_array) {
		number_stack_elem res = ConstExp(file);
		token.num = res.token.num;
		if (res.is_variable)
			fprintf(output, "store i32 %s, i32* %s\n", res.variable.c_str(), get_pointer(token).c_str());
		else
			fprintf(output, "store i32 %d, i32* %s\n", res.token.num, get_pointer(token).c_str());
		fprintf(output, "%%%d = load i32, i32* %s\t\t; define const variable '%s'\n\n", register_num,
				get_pointer(token).c_str(), token.token.c_str());
		stringstream stream1;
		stream1 << register_num++;
		set_register(token, "%" + stream1.str());
	} else {
		// lab 7 要求可以定义数组元素
		word = get_symbol(input);
		if (word.type != SYMBOL || word.token != "LBrace")
			exit_();
		word = get_symbol(input);
		int current_define_pos[50] = {'\0'};
		init_array(elem, current_define_pos, 1, true, false);
	}
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
	return_token x = word;

	variable_list_elem elem;
	elem.code_block_layer = code_block_layer;
	elem.token = word;
	stringstream stream;
	stream << register_num++;
	elem.saved_pointer = "%" + stream.str();

	word = get_symbol(file);
	bool is_array_define = false;
	int dimension = 0;
	while (word.type == SYMBOL && word.token == "[") {
		dimension++;
		is_array_define = true;
		elem.is_array = true;
		word = get_symbol(input);
		elem.dimension_num[dimension] = calcAntiPoland(input, true, false).token.num;
		if (elem.dimension_num[dimension] <= 0)
			exit_();
		if (word.type != SYMBOL || word.token != "]")
			exit_();
		word = get_symbol(input);
	}
	elem.dimension = dimension;

	if (is_array_define) {
		int total = 1;
		for (int i = 1; i <= elem.dimension; i++)
			total *= elem.dimension_num[i];
		fprintf(output, "%s = alloca [%d x i32]\t\t; 将数组 %s 的指针指定在 %s 的位置\n", elem.saved_pointer.c_str(), total,
				elem.token.token.c_str(), elem.saved_pointer.c_str());
		stringstream stream1;
		stream1 << total;
		elem.variable_type = "[" + stream1.str() + " x i32]";
	} else {
		fprintf(output, "%s = alloca i32\t\t; 将变量 %s 的指针定义在 %s 的位置\n", elem.saved_pointer.c_str(),
				elem.token.token.c_str(), elem.saved_pointer.c_str());
		elem.variable_type = "i32";
	}
	variable_list.push_back(elem);

	if (word.type != SYMBOL || word.token != "Assign")
		return;

	word = get_symbol(file);

	if (is_array_define) {
		// lab 7 要求可以定义数组元素
		if (word.type != SYMBOL || word.token != "LBrace")
			exit_();
		int current_define_pos[50] = {'\0'};
		word = get_symbol(input);
		if (word.type == SYMBOL && word.token == "RBrace") {
			int total = 1;
			for (int i = 1; i <= elem.dimension; i++)
				total *= elem.dimension_num[i];
			fprintf(output, "%%%d = getelementptr %s, %s* %s, i32 0, i32 0\n",
					register_num++, elem.variable_type.c_str(), elem.variable_type.c_str(), elem.saved_pointer.c_str());
			fprintf(output, "call void @memset(i32* %%%d, i32 0, i32 %d)\n", register_num - 1, total);
		} else
			init_array(elem, current_define_pos, 1, false, false);
	} else {
		number_stack_elem res = InitVal(file);
		if (res.is_variable)
			fprintf(output, "store i32 %s, i32* %s\n", res.variable.c_str(), get_pointer(x).c_str());
		else
			fprintf(output, "store i32 %d, i32* %s\n", res.token.num, get_pointer(x).c_str());
		fprintf(output, "%%%d = load i32, i32* %s\t\t; define variable '%s'\n", register_num, get_pointer(x).c_str(),
				elem.token.token.c_str());
		stringstream stream1;
		stream1 << register_num++;
		set_register(x, "%" + stream1.str());
	}
}

number_stack_elem InitVal(FILE *file) {
	return Exp(file);
}

number_stack_elem Exp(FILE *file) {
	//fprintf(output, "%s\n", calcAntiPoland(file).c_str());
	return calcAntiPoland(file);
}

void Stmt(FILE *file, int function_type) {
	if (word.type == IDENT) {
		// 返回语句
		if (word.token == "Return") {
			if (!can_deal_multiply_stmt) {
				if (can_deal_stmt_left == 0)
					return;
				can_deal_stmt_left--;
			}

			if (last_token_is_if_or_else && !undefined_code_block_stack.empty()) {
				last_token_is_if_or_else = false;
				undefined_code_block_stack_elem elem = undefined_code_block_stack.top();
				undefined_code_block_stack.pop();
				print_code_block(elem);
			}

			word = get_symbol(file);
			if (function_type == INT) {
				number_stack_elem res = calcAntiPoland(file);
				if (res.is_variable)
					fprintf(output, "ret i32 %s\n", res.variable.c_str());
				else fprintf(output, "ret i32 %d\n", res.token.num);
			} else {
				fprintf(output, "ret void\n");
			}
			need_br = false;
			if (word.type != SYMBOL || word.token != "Semicolon")
				exit_();
			word = get_symbol(file);
			have_returned = true;
			return;
		}
			// 判断语句
		else if (word.token == "If") {
			have_returned = false;
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
			//is_else_if = false;

			if (word.type != SYMBOL || word.token != "RPar")
				exit_();
			word = get_symbol(file);

			bool can_deal_multiply_stmt_temp = can_deal_multiply_stmt;
			int can_deal_stmt_left_temp = can_deal_stmt_left;
			update_can_deal_multiply_stmt();
			last_token_is_if_or_else = true;
			Stmt(file, function_type);

			if (is_else_if)
				return;

			if (word.type == IDENT && word.token == "Else") {
				have_returned = false;
				last_token_is_if_or_else = true;
				word = get_symbol(file);

				// 处理 else-if 语句
				bool else_if_have_else_stmt = false; // 记录 else if 语句是否有最终的 else
				while (word.type == IDENT && word.token == "If") {
					is_else_if = true;
					bool can_deal_multiply_stmt_temp_1 = can_deal_multiply_stmt;
					int can_deal_stmt_left_temp_1 = can_deal_stmt_left;
					update_can_deal_multiply_stmt();
					Stmt(file);
					can_deal_stmt_left = can_deal_stmt_left_temp_1;
					can_deal_multiply_stmt = can_deal_multiply_stmt_temp_1;

					if (word.type != IDENT || word.token != "Else") {
						else_if_have_else_stmt = false;
						break;
					}

					word = get_symbol(input);
					if (word.type != IDENT || word.token != "If") {
						else_if_have_else_stmt = true;
						break;
					}

					undefined_code_block_stack_elem elem = undefined_code_block_stack.top();
					undefined_code_block_stack.pop();
					print_code_block(elem);
					print_variable_table();
				}

				undefined_code_block_stack_elem elem = undefined_code_block_stack.top();
				undefined_code_block_stack.pop();
				print_code_block(elem);
				print_variable_table();
				have_print_block_name = true;
				if (else_if_have_else_stmt || !is_else_if) {
//				if (else_if_have_else_stmt) {
					if (word.type != SYMBOL || word.token != "LBrace")
						Stmt(input);
					else {
						code_block_layer++;
						last_token_is_if_or_else = false;
						int can_deal_multiply_stmt_temp2 = can_deal_multiply_stmt;
						int can_deal_left_temp_2 = can_deal_stmt_left;
						update_can_deal_multiply_stmt();
						word = get_symbol(input);
						while (word.type != SYMBOL || word.token != "RBrace")
							BlockItem(input, function_type);
						code_block_layer--;
						update_variable_list();
						word = get_symbol(input);
						can_deal_stmt_left = can_deal_left_temp_2;
						can_deal_multiply_stmt = can_deal_multiply_stmt_temp2;
					}
				}
				is_else_if = false;
				if (need_br) {
					stack<undefined_code_block_stack_elem> temp;
					undefined_code_block_stack_elem elem1 = undefined_code_block_stack.top();
					while (elem1.block_type != IF_FINAL) {
						temp.push(elem1);
						undefined_code_block_stack.pop();
						elem1 = undefined_code_block_stack.top();
					}
					fprintf(output, "br label %%IF_FINAL_%d\n", elem1.register_num);
					while (!temp.empty()) {
						elem1 = temp.top();
						temp.pop();
						undefined_code_block_stack.push(elem1);
					}
				}
				last_token_is_if_or_else = false;
				need_br = true;
			} else {
				have_returned = false;
				undefined_code_block_stack_elem elem = undefined_code_block_stack.top();
				undefined_code_block_stack.pop();
				undefined_code_block_stack_elem final_elem = undefined_code_block_stack.top();
//				fprintf(output, "\n\n\n%d:\t; 定义缺省的 else 代码块\n", elem.register_num);
				print_code_block(elem);
				print_variable_table();

				fprintf(output, "br label %%IF_FINAL_%d\n", final_elem.register_num);
				need_br = false;
			}
			have_print_block_name = false;

			if (!undefined_code_block_stack.empty()) {
				have_returned = false;
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
			} else {
				while (word.type != SYMBOL || word.token != "RBrace")
					Stmt(file);
			}

			if (!undefined_code_block_stack.empty() && need_br) {
				undefined_code_block_stack_elem elem = undefined_code_block_stack.top();
				stack<undefined_code_block_stack_elem> temp;
				if (elem.block_type == WHILE_FINAL) {
					fprintf(output, "br label %%WHILE_COND_%d\n", elem.register_num);
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
			have_returned = false;
			word = get_symbol(input);
			if (word.type != SYMBOL || word.token != "LPar")
				exit_();
			fprintf(output, "br label %%WHILE_COND_%d\n", while_code_block_num);
			fprintf(output, "\n\n\nWHILE_COND_%d:\t; while 循环的判断条件\n", while_code_block_num);
			print_variable_table();
			word = get_symbol(input);
			Cond(input, false, true);
			if (word.type != SYMBOL || word.token != "RPar")
				exit_();
			word = get_symbol(input);

			if (word.type == SYMBOL && word.token == "Semicolon") {
				word = get_symbol(input);
				undefined_code_block_stack_elem elem = undefined_code_block_stack.top();
				undefined_code_block_stack.pop();
				undefined_code_block_stack.pop();
				fprintf(output, "\n\n\nWHILE_LOOP_%d:\t; while 循环的循环体\n", elem.register_num);
				fprintf(output, "br label %%WHILE_COND_%d\n", elem.register_num);
				fprintf(output, "\n\n\nWHILE_FINAL_%d:\t; while 循环的结束\n", elem.register_num);
				return;
			}

			if (word.type != SYMBOL || word.token != "LBrace")
				exit_();
			code_block_layer++;
			undefined_code_block_stack_elem elem = undefined_code_block_stack.top();
			undefined_code_block_stack.pop();
			fprintf(output, "\n\n\nWHILE_LOOP_%d:\t; while 循环的循环体\n", elem.register_num);
			print_variable_table();
			word = get_symbol(input);
			while (word.type != SYMBOL || word.token != "RBrace")
				BlockItem(input, function_type);
			code_block_layer--;
			update_variable_list();
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
			if (res.is_variable) {
				fprintf(output, "call void @putch(i32 %s)\n", res.variable.c_str());
			} else
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
			// putarray 语句
		else if (word.token == "putarray") {
			word = get_symbol(input);
			if (word.type != SYMBOL || word.token != "LPar")
				exit_();
			word = get_symbol(input);

			number_stack_elem para1 = calcAntiPoland(input);
			if (word.type != SYMBOL && word.token != "Comma")
				exit_();

			word = get_symbol(input);
			if (word.type != IDENT)
				exit_();
			variable_list_elem para2 = get_variable(word);
			if (!para2.is_array)
				exit_();

			fprintf(output, "%%%d = add i32 0, 0\t\t\t; 定义临时变量偏移量 0，用来计算数组元素的位置\n", register_num);
			int offset_register = register_num++;
			for (int i = 1; i < para2.dimension; i++) {
				word = get_symbol(input);
				if (word.type != SYMBOL || word.token != "[")
					exit_();
				word = get_symbol(input);

				int dimension_number = 1;
				for (int j = i + 1; j <= para2.dimension; j++) {
					dimension_number *= para2.dimension_num[j];
				}

				number_stack_elem res = calcAntiPoland(input);
				if (res.is_variable)
					fprintf(output, "%%%d = mul i32 %d, %s\n", register_num, dimension_number, res.variable.c_str());
				else
					fprintf(output, "%%%d = mul i32 %d, %d\n", register_num, dimension_number, res.token.num);
				fprintf(output, "%%%d = add i32 %%%d, %%%d\n", register_num + 1, offset_register, register_num);
				register_num++;
				offset_register = register_num++;

				if (word.type != SYMBOL || word.token != "]")
					exit_();
			}
			if (para2.variable_type == "i32")
				fprintf(output, "%%%d = getelementptr i32, i32* %s, i32 %%%d\t; 获取数组元素对应的指针\n", register_num++,
						para2.saved_pointer.c_str(), offset_register);
			else
				fprintf(output, "%%%d = getelementptr %s, %s* %s, i32 0, i32 %%%d\t; 获取数组元素对应的指针\n", register_num++,
						para2.variable_type.c_str(), para2.variable_type.c_str(), para2.saved_pointer.c_str(),
						offset_register);
			fprintf(output, "call void @putarray(i32 ");
			print_number_stack_elem(para1);
			fprintf(output, ", i32* %%%d)\n", register_num - 1);

			word = get_symbol(input);
			if (word.type != SYMBOL || word.token != "RPar")
				exit_();

			word = get_symbol(input);
			if (word.type != SYMBOL || word.token != "Semicolon")
				exit_();
			word = get_symbol(input);
			return;
		}
			// getarray 语句
		else if (word.token == "getarray") {
			word = get_symbol(input);
			if (word.type != SYMBOL || word.token != "LPar")
				exit_();
			word = get_symbol(input);
			if (!is_variable_list_contains_in_all_layer(word))
				exit_();

			variable_list_elem array = get_variable(word);
			if (!array.is_array)
				exit_();

			int offset_register = register_num;
			fprintf(output, "%%%d = add i32 0, 0\n", register_num++);
			for (int i = 1; i <= array.dimension; i++) {
				word = get_symbol(input);
				if (word.type != SYMBOL || word.token != "[")
					break;
				word = get_symbol(input);
				number_stack_elem param = calcAntiPoland(input);
				if (word.type != SYMBOL || word.token != "]")
					exit_();

				int dimension_num = 1;
				for (int j = i + 1; j <= array.dimension; j++)
					dimension_num *= array.dimension_num[j];

				fprintf(output, "%%%d = mul i32 %d, ", register_num++, dimension_num);
				print_number_stack_elem(param);
				fprintf(output, ")\n");
				fprintf(output, "%%%d = add i32 %%%d, %%%d\n", register_num, offset_register, register_num - 1);
				register_num++;
			}

			if (array.variable_type == "i32")
				fprintf(output, "%%%d = getelementptr i32, i32* %s, i32 %%%d\t; 获取数组元素对应的指针\n", register_num++,
						array.saved_pointer.c_str(), offset_register);
			else
				fprintf(output, "%%%d = getelementptr %s, %s*, %s, i32 0, i32 %%%d\n", register_num++,
						array.variable_type.c_str(), array.variable_type.c_str(), array.saved_pointer.c_str(),
						offset_register);
			fprintf(output, "%%%d = call i32 @getarray(i32* %%%d)\n", register_num, register_num - 1);
			register_num++;

			if (word.type != SYMBOL || word.token != "RPar")
				exit_();
			word = get_symbol(input);
			if (word.type != SYMBOL || word.token != "Semicolon")
				exit_();
			return;
		}
			// 定义语句
		else if (word.token == "int") {
			word = get_symbol(file);
			VarDef(input);
			return;
		}

		if (!is_variable_list_contains_in_all_layer(word)) {
			printf("%s has never been defined!\n", word.token.c_str());
			exit_();
		}

		return_token x = word;
		string left_value_pointer;

		// 如果赋值语句正处在一个 if_else 代码块的下面，则将其作为该代码块的成分
		if (!have_print_block_name && last_token_is_if_or_else) {
			undefined_code_block_stack_elem elem = undefined_code_block_stack.top();
			undefined_code_block_stack.pop();
//			fprintf(output, "\n\n\n%%%d:\t; 定义省略了大括号的赋值语句\n", elem.register_num);
			print_code_block(elem);
			print_variable_table();
		}

		variable_list_elem left_value = get_variable(x);
		if (left_value.is_array) {
			number_stack_elem array_dimension_value[50];
			int offset = register_num;
			fprintf(output, "%%%d = add i32 0, 0\t\t\t; 定义临时变量偏移量 0，用来计算数组元素的位置\n", register_num++);
			for (int i = 1; i <= left_value.dimension; i++) {
				word = get_symbol(input);
				if (word.type != SYMBOL || word.token != "[")
					exit_();
				word = get_symbol(input);
				array_dimension_value[i] = calcAntiPoland(file, false, false);
				if (word.type != SYMBOL || word.token != "]")
					exit_();
				if (i != left_value.dimension) {
					int number = 1; // 计算数组下一维度的元素数量
					for (int j = i + 1; j <= left_value.dimension; j++)
						number *= left_value.dimension_num[j];
					if (array_dimension_value[i].is_variable)
						fprintf(output, "%%%d = mul i32 %d, %s\n", register_num++, number,
								array_dimension_value[i].variable.c_str());
					else
						fprintf(output, "%%%d = mul i32 %d, %d\n", register_num++, number,
								array_dimension_value[i].token.num);
					fprintf(output, "%%%d = add i32 %%%d, %%%d\n", register_num, offset, register_num - 1);
					offset = register_num++;
				} else {
					if (array_dimension_value[i].is_variable)
						fprintf(output, "%%%d = add i32 %%%d, %s\n", register_num, offset,
								array_dimension_value[i].variable.c_str());
					else
						fprintf(output, "%%%d = add i32 %%%d, %d\n", register_num, offset,
								array_dimension_value[i].token.num);
					offset = register_num++;
				}
			}
			if (left_value.variable_type == "i32")
				fprintf(output, "%%%d = getelementptr i32, i32* %s, i32 %%%d\t; 获取数组元素对应的指针\n", register_num++,
						left_value.saved_pointer.c_str(), offset);
			else
				fprintf(output, "%%%d = getelementptr %s, %s* %s, i32 0, i32 %%%d\t; 获取数组元素对应的指针\n",
						register_num++, left_value.variable_type.c_str(), left_value.variable_type.c_str(),
						left_value.saved_pointer.c_str(), offset);
			stringstream stream;
			stream << register_num - 1;
			left_value_pointer = "%" + stream.str();
		}
			// 处理函数
		else if (left_value.is_function) {
			print_function_elem(left_value);
			while (word.type != SYMBOL || word.token != "Semicolon") {
				if (is_variable_list_contains_in_all_layer(word)) {
					variable_list_elem elem = get_variable(word);
					if (elem.is_function)
						print_function_elem(elem);
				}
				word = get_symbol(input);
			}
			word = get_symbol(input);

			if (last_token_is_if_or_else && !undefined_code_block_stack.empty()) {
				fprintf(output, "br label %%IF_FINAL_%d\n", undefined_code_block_stack.top().register_num);
			}
			last_token_is_if_or_else = false;
			return;
		} else {
			left_value_pointer = get_pointer(x);
		}

		word = get_symbol(file);

		// 无意义语句直接跳过
		if (word.type != SYMBOL || word.token != "Assign") {
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

		word = get_symbol(file);
		number_stack_elem res = calcAntiPoland(file);
		if (res.is_variable)
			fprintf(output, "store i32 %s, i32* %s\n", res.variable.c_str(), left_value_pointer.c_str());
		else
			fprintf(output, "store i32 %d, i32* %s\n", res.token.num, left_value_pointer.c_str());
		// 非数组元素需要修改对应的储值寄存器
		if (!left_value.is_array && !left_value.is_global) {
			fprintf(output, "%%%d = load i32, i32* %s\t\t; set variable '%s'\n", register_num,
					left_value_pointer.c_str(), x.token.c_str());
			stringstream stream;
			stream << register_num++;
			set_register(x, "%" + stream.str());
		}

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
		bool is_else_if_temp = is_else_if;
		is_else_if = false;
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
		}
		last_token_is_if_or_else = false;

		word = get_symbol(file);
		while (word.type != SYMBOL || word.token != "RBrace")
			BlockItem(file, function_type);
		code_block_layer--;
		update_variable_list();
		last_token_is_if_or_else = last_token_is_if_or_else_temp;
		is_else_if = is_else_if_temp;

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
	init();
	word = get_symbol(in);
	// 全局变量和函数定义
	while (true) {
		string variable_name;
		variable_list_elem elem;
		elem.is_global = true;
		int function_type;
		if (word.type == IDENT && word.token == "const") {
			word = get_symbol(input);
			elem.is_const = true;
		}
		if (word.type != IDENT || (word.token != "int" && word.token != "void"))
			exit_();
		if (word.token == "int")
			function_type = INT;
		else function_type = VOID;
		word = get_symbol(input);
		if (word.type == IDENT && word.token == "main")
			break;
		string function_name = word.token;
		variable_name = word.token;
		int need_get_symbol = false;
		word = get_symbol(input);
		if (word.type == SYMBOL && word.token == "LPar") {
			FuncDef(input, false, function_type, function_name);
			continue;
		}
		goto label1;
		// 处理一句话中的多重定义
		while (word.type != SYMBOL || word.token != "Semicolon") {
			label1:
			if (need_get_symbol)
				variable_name = word.token;
			elem.token.type = IDENT;
			elem.token.token = variable_name;
			elem.saved_pointer = "@" + variable_name;
			elem.code_block_layer = 0;

			if (need_get_symbol)
				word = get_symbol(input);
			else need_get_symbol = true;

			while (word.type == SYMBOL && word.token == "[") {
				elem.is_array = true;
				elem.dimension++;
				word = get_symbol(input);
				number_stack_elem res = calcAntiPoland(input, true, true);
				elem.dimension_num[elem.dimension] = res.token.num;
				if (word.type != SYMBOL || word.token != "]")
					exit_();
				word = get_symbol(input);
			}

			if (word.type == SYMBOL && (word.token == "Semicolon" || word.token == "Comma")) {
				if (elem.is_array) {
					int total_num = 1;
					for (int i = 1; i <= elem.dimension; i++)
						total_num *= elem.dimension_num[i];
					fprintf(output, "@%s = global [%d x i32] zeroinitializer\t; 定义全局数组并初始化为 0\n", variable_name.c_str(),
							total_num);
					stringstream stream;
					stream << total_num;
					elem.variable_type = "[" + stream.str() + " x i32]";
				} else {
					fprintf(output, "@%s = global i32 0\t; 定义全局变量并初始化 %s = 0\n", variable_name.c_str(),
							variable_name.c_str());
					elem.global_variable_value = 0;
					elem.variable_type = "i32";
				}
				variable_list.push_back(elem);
				// 遇到逗号继续
				if (word.token == "Comma") {
					word = get_symbol(input);
					elem.dimension = 0;
					elem.saved_pointer = "";
					elem.variable_type = "";
					memset(elem.dimension_num, 0, sizeof(int) * 10);
					elem.is_array = false;
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
			// 初始化全局数组
			if (elem.is_array) {
				int total_num = 1;
				for (int i = 1; i <= elem.dimension; i++)
					total_num *= elem.dimension_num[i];
				fprintf(output, "@%s = global [%d x i32] [", variable_name.c_str(), total_num);
				stringstream stream;
				stream << total_num;
				elem.variable_type = "[" + stream.str() + " x i32]";

				word = get_symbol(input);
				if (word.type != SYMBOL || word.token != "LBrace")
					exit_();
				int dimension = 1;
				int defined_dimension_num[50] = {'\0'};
				int number = 0;
				int total_number = 0;
				word = get_symbol(input);
				while (dimension != 0) {
					if (word.type == SYMBOL && word.token == "LBrace")
						dimension++;
					else if (word.type == SYMBOL && word.token == "RBrace") {
						int dimension_total = 1;
						for (int i = dimension; i <= elem.dimension; i++)
							dimension_total *= elem.dimension_num[i];
						if (dimension == 1) {
							for (int i = total_number + 1; i <= dimension_total; i++)
								fprintf(output, "i32 0, ");
						} else {
							for (int i = defined_dimension_num[dimension] + 1; i <= dimension_total; i++) {
								fprintf(output, "i32 0, ");
								total_number++;
							}
						}
						defined_dimension_num[dimension] = 0;
						defined_dimension_num[dimension - 1] += dimension_total;
						dimension--;
					} else if (word.type == SYMBOL && word.token == "Comma") {}
					else {
						number_stack_elem res = calcAntiPoland(input, true, true);
						fprintf(output, "i32 %d, ", res.token.num);
						defined_dimension_num[dimension]++;
						total_number++;
						continue;
					}
					word = get_symbol(input);
				}
				fseek(output, -2, SEEK_CUR);
				fprintf(output, "]\t; 初始化全局数组\n");
			}
				// 初始化全局变量
			else {
				word = get_symbol(input);
				number_stack_elem res = calcAntiPoland(input, true, true);
				fprintf(output, "@%s = global i32 %d\t; 定义全局变量 %s = %d\n", variable_name.c_str(), res.token.num,
						variable_name.c_str(), res.token.num);
				elem.global_variable_value = res.token.num;
				elem.variable_type = "i32";
			}
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
	FuncDef(input, true);
	word = get_symbol(in);
	if (word.type == "Error")
		exit_();
}

void Cond(FILE *file, bool is_else_if_cond = false, bool is_while_cond = false) {
	number_stack_elem res = calcAntiPoland(file);
	bool have_logic_or = false;
	int logic_or_code_num = logic_or_code_block_num;
	int logic_or_num = 1;
	while (word.type == SYMBOL && word.token == "LogicOr") {
		if (!have_logic_or) {
			logic_or_code_block_num++;
			fprintf(output, "%%logic_or_val_%d = alloca i32\n", logic_or_code_num);
			fprintf(output, "br label %%LOGIC_OR_JUDGE_%d_%d\n", logic_or_code_num, logic_or_num);
			fprintf(output, "\n\nLOGIC_OR_JUDGE_%d_%d:\t; 第 %d 个 逻辑或 的 第 %d 个判断\n", logic_or_code_num, logic_or_num,
					logic_or_code_num, logic_or_num);
			have_logic_or = true;
		}
		logic_or_num++;

		fprintf(output, "%%%d = icmp ne i32 0, ", register_num);
		word = get_symbol(input);
		print_number_stack_elem(res);
		fprintf(output, "\nbr i1 %%%d, label %%LOGIC_OR_TRUE_%d, label %%LOGIC_OR_JUDGE_%d_%d\n", register_num++,
				logic_or_code_num, logic_or_code_num, logic_or_num);
		fprintf(output, "\n\nLOGIC_OR_JUDGE_%d_%d:\t; 第 %d 个 逻辑或 的 第 %d 个判断\n", logic_or_code_num, logic_or_num,
				logic_or_code_num, logic_or_num);
		res = calcAntiPoland(file);
	}

	if (have_logic_or) {

		fprintf(output, "%%%d = icmp ne i32 0, ", register_num);
		print_number_stack_elem(res);
		fprintf(output, "\nbr i1 %%%d, label %%LOGIC_OR_TRUE_%d, label %%LOGIC_OR_FALSE_%d\n",
				register_num++, logic_or_code_num, logic_or_code_num);
		fprintf(output, "\n\nLOGIC_OR_TRUE_%d:\n", logic_or_code_num);
		fprintf(output, "store i32 1, i32* %%logic_or_val_%d\n", logic_or_code_num);
		fprintf(output, "br label %%LOGIC_OR_FINAL_%d\n", logic_or_code_num);

		fprintf(output, "\n\nLOGIC_OR_FALSE_%d:\n", logic_or_code_num);
		fprintf(output, "store i32 0, i32* %%logic_or_val_%d\n", logic_or_code_num);
		fprintf(output, "br label %%LOGIC_OR_FINAL_%d\n", logic_or_code_num);

		fprintf(output, "\n\nLOGIC_OR_FINAL_%d:\n", logic_or_code_num);
		fprintf(output, "%%%d = load i32, i32* %%logic_or_val_%d\n", register_num, logic_or_code_num);
		stringstream stream;
		stream << register_num++;
		res.is_variable = true;
		res.is_function = false;
		res.variable = "%" + stream.str();
	}

	fprintf(output, "%%%d = icmp ne i32 ", register_num++);
	if (res.is_variable)
		fprintf(output, "%s", res.variable.c_str());
	else fprintf(output, "%d", res.token.num);
	fprintf(output, ", 0\t; 将 i32 的值转化为 i1 形式，然后进行判断\n");

	// 来自 while 循环语句
	if (is_while_cond) {
		fprintf(output, "br i1 %%%d, label %%WHILE_LOOP_%d, label %%WHILE_FINAL_%d\t; 将 i1 形式的值进行判断，然后选择跳转块\n",
				register_num - 1, while_code_block_num, while_code_block_num);
		// 在来自 while 循环的语句中，需要添加两个新的代码块
		undefined_code_block_stack_elem elem;
		elem.register_num = while_code_block_num++;
		elem.block_type = WHILE_FINAL;
		undefined_code_block_stack.push(elem);
		elem.block_type = WHILE_LOOP;
		undefined_code_block_stack.push(elem);
	}
		// 来自 else if 语句
	else if (is_else_if_cond) {
		fprintf(output, "br i1 %%%d, label %%IF_TRUE_%d, label %%IF_FALSE_%d\t; 将 i1 形式的值进行判断，然后选择跳转块\n",
				register_num - 1, if_else_code_block_num, if_else_code_block_num);
		// 在来自 else if 的语句中，只需要向其中添加两个新的代码块
		undefined_code_block_stack_elem elem;
		elem.register_num = if_else_code_block_num++;
		elem.block_type = IF_FALSE;
		undefined_code_block_stack.push(elem);
		elem.block_type = IF_TRUE;
		undefined_code_block_stack.push(elem);
	}
		//来自普通 if 语句
	else {
		fprintf(output, "br i1 %%%d, label %%IF_TRUE_%d, label %%IF_FALSE_%d\t; 将 i1 形式的值进行判断，然后选择跳转块\n",
				register_num - 1,
				if_else_code_block_num, if_else_code_block_num);
		// 保留下了接下来的三个代码块，分别用于 条件为真、条件为假、条件语句结束 的对应代码块，然后倒序入栈。
		undefined_code_block_stack_elem elem;
		elem.register_num = if_else_code_block_num++;
		elem.block_type = IF_FINAL;
		undefined_code_block_stack.push(elem);
		elem.block_type = IF_FALSE;
		undefined_code_block_stack.push(elem);
		elem.block_type = IF_TRUE;
		undefined_code_block_stack.push(elem);
	}
}

void init() {
	fseek(output, 0, SEEK_SET);
	fprintf(output, "declare i32 @getint()\n"
					"declare void @putint(i32)\n"
					"declare i32 @getch()\n"
					"declare void @putch(i32)\n"
					"declare i32 @getarray(i32*)\n"
					"declare void @putarray(i32, i32*)\n"
					"declare void @memset(i32*, i32, i32)\n\n");
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
				if (variable.code_block_layer == 0) {
					fprintf(output, "%%%d = load %s, %s* %s\t; 代码块中临时调用全局变量 %s\n", register_num,
							variable.variable_type.c_str(), variable.variable_type.c_str(),
							variable.saved_pointer.c_str(), variable.token.token.c_str());
					stringstream stream;
					stream << register_num++;
					variable.saved_register = "%" + stream.str();
				} else if (variable.saved_register.empty()) {
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
		if (i.is_global || i.is_function)
			continue;
		fprintf(output, "%%%d = load %s, %s %s\t; 代码块中重新定义变量 %s\n", register_num,
				i.variable_type.c_str(), (i.variable_type + "*").c_str(), i.saved_pointer.c_str(),
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
	need_br = true;
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
			if (token == i.token && layer == i.code_block_layer)
				return i;
		}
	}
}

void
init_array(const variable_list_elem &array, int *current_pos, int dimension, bool is_const_define,
		   bool is_global_define) {
	while (word.type != SYMBOL || word.token != "RBrace") {
		if (word.type == SYMBOL && word.token == "LBrace") {
			current_pos[dimension + 1] = 0;
			word = get_symbol(input);
			init_array(array, current_pos, dimension + 1, is_const_define, is_global_define);
		} else if (word.token == SYMBOL || word.token == "Comma") {
			word = get_symbol(input);
		} else {
			if (dimension != array.dimension)
				exit(-2);
			int offset = 0;
			for (int i = 1; i < dimension; i++) {
				int base = 1;
				for (int j = i + 1; j <= dimension; j++)
					base *= array.dimension_num[j];
				offset += base * current_pos[i];
			}
			offset += current_pos[dimension];
			int pointer_pos = register_num++;
			if (array.variable_type == "i32")
				fprintf(output, "%%%d = getelementptr i32, i32* %s, i32 %%%d\t; 获取数组元素对应的指针\n", register_num++,
						array.saved_pointer.c_str(), offset);
			else
				fprintf(output, "%%%d = getelementptr %s, %s* %s, i32 0, i32 %d\n", pointer_pos,
						array.variable_type.c_str(),
						array.variable_type.c_str(), array.saved_pointer.c_str(), offset);
			number_stack_elem res = calcAntiPoland(input, is_const_define, is_global_define);
			if (res.is_variable)
				fprintf(output, "store i32 %s, i32* %%%d\n", res.variable.c_str(), pointer_pos);
			else
				fprintf(output, "store i32 %d, i32* %%%d\n", res.token.num, pointer_pos);
			current_pos[dimension]++;
		}
	}
	current_pos[dimension - 1]++;
	word = get_symbol(input);
}

void reload_param() {
	for (auto &variable: variable_list) {
		if (variable.code_block_layer == 1) {
			fprintf(output, "%%%d = alloca %s\n", register_num, variable.variable_type.c_str());
			fprintf(output, "store %s %s, %s* %%%d\n", variable.variable_type.c_str(), variable.saved_register.c_str(),
					variable.variable_type.c_str(), register_num);
			stringstream stream;
			stream << register_num++;
			variable.saved_pointer = "%" + stream.str();
			fprintf(output, "%%%d = load %s, %s* %s\n", register_num++, variable.variable_type.c_str(),
					variable.variable_type.c_str(), variable.saved_pointer.c_str());
			if (variable.variable_type == "i32*") {
				variable.variable_type = "i32";
				stringstream stream1;
				stream1 << register_num - 1;
				variable.saved_pointer = "%" + stream1.str();
			}
		}
	}
}

void print_function_elem(const variable_list_elem &elem) {
	word = get_symbol(input);
	if (word.type != SYMBOL || word.token != "LPar")
		exit_();
	number_stack_elem params[50];
	int i;
	for (i = 1; i <= elem.function_param_num; i++) {
		word = get_symbol(input);
		params[i] = calcAntiPoland(input);
	}
	if (i == 1)
		word = get_symbol(input);
	if (word.type != SYMBOL || word.token != "RPar")
		exit_();
	if (elem.function_return_type == INT)
		fprintf(output, "%%%d = call i32 @%s(", register_num++, elem.token.token.c_str());
	else
		fprintf(output, "call void @%s(", elem.token.token.c_str());
	for (i = 1; i <= elem.function_param_num; i++) {
		fprintf(output, "%s ", elem.function_param_type[i].c_str());
		print_number_stack_elem(params[i]);
		fprintf(output, ", ");
	}
	if (elem.function_param_num != 0)
		fseek(output, -2, SEEK_CUR);
	fprintf(output, ")\n");
	word = get_symbol(input);
}