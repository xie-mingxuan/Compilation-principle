//
// Created by 谢铭轩 on 2021/11/1.
//

#include "../headers/antiPolandExp.h"
#include "sstream"

extern return_token word;
extern FILE *input;
extern FILE *output;
extern int code_block_layer;
extern int while_code_block_num;
extern int logic_and_code_block_num;
extern int logic_or_code_block_num;
int register_num = 1;

int priority(const return_token &c) {
	if (c.token == "Plus" || c.token == "Minus")
		return 4;
	if (c.token == "Mult" || c.token == "Div" || c.token == "Mod")
		return 3;
	if (c.token == "Gt" || c.token == "Ge" || c.token == "Lt" || c.token == "Le")
		return 6;
	if (c.token == "Eq" || c.token == "NotEq")
		return 7;
	if (c.token == "Not")
		return 2;
	if (c.token == "LogicAnd")
		return 11;
	if (c.token == "LogicOr")
		return 12;
	return 16;
}

void pop_and_print(stack<number_stack_elem> &number_stack, stack<return_token> &operator_stack) {
	bool is_icmp_calc = false;
	number_stack_elem x1, x2;
	return_token op = operator_stack.top();
	operator_stack.pop();

	// 非运算是单目运算符，要单独考虑
	if (op.token == "Not") {
		x1 = number_stack.top();
		number_stack.pop();

		if (x1.is_function) {
			// TODO do sth to call func
		}

		fprintf(output, "%%%d = icmp eq i32 ", register_num++);
		print_number_stack_elem(x1);
		fprintf(output, ", 0\n%%%d = zext i1 %%%d to i32\n", register_num, register_num - 1);

		number_stack_elem res;
		res.is_variable = true;
		stringstream stream;
		stream << register_num++;
		res.variable = "%" + stream.str();
		number_stack.push(res);
		return;
	}

	if (!number_stack.empty()) {
		x2 = number_stack.top();
		number_stack.pop();
	} else exit(-1);
	if (!number_stack.empty()) {
		x1 = number_stack.top();
		number_stack.pop();
	} else exit(-1);

	if (x1.is_function) {
		// TODO do sth to call func
	}

	fprintf(output, "%%%d = ", register_num);
	if (op.token == "Plus")
		fprintf(output, "add i32 ");
	else if (op.token == "Minus")
		fprintf(output, "sub i32 ");
	else if (op.token == "Mult")
		fprintf(output, "mul i32 ");
	else if (op.token == "Div")
		fprintf(output, "sdiv i32 ");
	else if (op.token == "Mod")
		fprintf(output, "sdiv i32 ");
//	else if (op.token == "LogicAnd") {
//		stringstream stream;
//		fprintf(output, "icmp ne i32 0, ");
//		print_number_stack_elem(x1);
//		stream << register_num++;
//		x1.is_variable = true;
//		x1.variable = "%" + stream.str();
//
//		// 添加 逻辑与 短路求值代码
//		fprintf(output, "\nbr label %%LOGIC_AND_JUDGE_LEFT_%d\n", logic_and_code_block_num);
//		fprintf(output, "%%logic_and_val_%d = alloca i32\n", logic_and_code_block_num);
//		fprintf(output, "\n\nLOGIC_AND_JUDGE_LEFT_%d:\t; 第 %d 个 逻辑与 的左值判断\n", logic_and_code_block_num,
//				logic_and_code_block_num);
//		fprintf(output, "%%%d = icmp ne i1 0, %s\n", register_num, x1.variable.c_str());
//		fprintf(output, "br i1 %%%d, label %%LOGIC_AND_JUDGE_RIGHT_%d, label %%LOGIC_AND_FALSE_%d\n", register_num++,
//				logic_and_code_block_num, logic_and_code_block_num);
//
//
//		fprintf(output, "\n\nLOGIC_AND_JUDGE_RIGHT_%d:\t; 第 %d 个 逻辑与 的右值判断\n", logic_and_code_block_num,
//				logic_and_code_block_num);
//		stringstream stream2;
//		if (x2.is_function) {
//			// TODO do sth to call func
//		}
//		fprintf(output, "\n%%%d = icmp ne i32 0, ", register_num);
//		print_number_stack_elem(x2);
//		stream2 << register_num++;
//		x2.is_variable = true;
//		x2.variable = "%" + stream2.str();
//		fprintf(output, "%%%d = icmp ne i1 0, %s\n", register_num, x2.variable.c_str());
//		fprintf(output, "br i1 %%%d, label %%LOGIC_AND_TRUE_%d, label %%LOGIC_AND_FALSE_%d\n", register_num++,
//				logic_and_code_block_num, logic_and_code_block_num);
//
//		fprintf(output, "\n\n%%LOGIC_AND_TRUE_%d:\n", logic_and_code_block_num);
//		fprintf(output, "%%%d = add i32 0, 1\n", register_num);
//		fprintf(output, "store i32 %%%d, i32* %%logic_and_val_%d", register_num++, logic_and_code_block_num);
//		fprintf(output, "br label %%LOGIC_AND_FINAL_%d:\n", logic_and_code_block_num);
//
//		fprintf(output, "\n\n%%LOGIC_AND_FALSE_%d:\n", logic_and_code_block_num);
//		fprintf(output, "%%%d = add i32 0, 0\n", register_num);
//		fprintf(output, "store i32 %%%d, i32* %%logic_and_val_%d", register_num++, logic_and_code_block_num);
//		fprintf(output, "br label %%LOGIC_AND_FINAL_%d:\n", logic_and_code_block_num);
//
//		fprintf(output, "\n\n%%LOGIC_AND_FINAL_%d:\n", logic_and_code_block_num++);
//		fprintf(output, "%%%d = load i32, i32* %%logic_and_val_%d\n", register_num, register_num - 1);
//		number_stack_elem res;
//		res.is_variable = true;
//		res.is_function = false;
//		stringstream stream1;
//		stream1 << register_num++;
//		res.variable = "%" + stream1.str();
//		number_stack.push(res);
//		return;
//	}
	else {
		if (op.token == "Eq")
			fprintf(output, "icmp eq i32 ");
		else if (op.token == "NotEq")
			fprintf(output, "icmp ne i32 ");
		else if (op.token == "Le")
			fprintf(output, "icmp sle i32 ");
		else if (op.token == "Lt")
			fprintf(output, "icmp slt i32 ");
		else if (op.token == "Ge")
			fprintf(output, "icmp sge i32 ");
		else if (op.token == "Gt")
			fprintf(output, "icmp sgt i32 ");
		else exit_();
		is_icmp_calc = true;
	}

	print_number_stack_elem(x1);
	fprintf(output, ", ");
	print_number_stack_elem(x2);
	fprintf(output, "\n");

	if (op.token == "Mod") {
		fprintf(output, "%%%d = mul i32 ", register_num + 1);
		if (!x2.is_variable)
			fprintf(output, "%d, ", x2.token.num);
		else
			fprintf(output, "%s, ", x2.variable.c_str());
		fprintf(output, "%%%d\n", register_num);

		fprintf(output, "%%%d = sub i32 ", register_num + 2);
		if (!x1.is_variable)
			fprintf(output, "%d, ", x1.token.num);
		else
			fprintf(output, "%s, ", x1.variable.c_str());
		fprintf(output, "%%%d\n", register_num + 1);
		register_num = register_num + 2;
	}

	if (is_icmp_calc) {
		stringstream s3;
		s3 << register_num++;
		string i1_register = "%" + s3.str();
		fprintf(output, "%%%d = zext i1 %s to i32\t; 比较运算后还需要将结果转化为 i32 格式，储存在寄存器 %%%d 中\n", register_num,
				i1_register.c_str(), register_num);
	}

	number_stack_elem res;
	res.is_variable = true;
	stringstream stream;
	stream << register_num++;
	res.variable = "%" + stream.str();
	number_stack.push(res);
}

void pop_and_not_print(stack<number_stack_elem> &number_stack, stack<return_token> &operator_stack) {
	number_stack_elem x1, x2;
	return_token op = operator_stack.top();
	operator_stack.pop();
	number_stack_elem res;
	res.is_variable = false;

	// 非运算是单目运算符，要单独考虑
	if (op.token == "Not") {
		x1 = number_stack.top();
		number_stack.pop();
		res.token.num = !x1.token.num;
		number_stack.push(res);
		return;
	}

	if (!number_stack.empty()) {
		x2 = number_stack.top();
		number_stack.pop();
	} else exit(-1);
	if (!number_stack.empty()) {
		x1 = number_stack.top();
		number_stack.pop();
	} else exit(-1);

	if (op.token == "Plus")
		res.token.num = x1.token.num + x2.token.num;
	else if (op.token == "Minus")
		res.token.num = x1.token.num - x2.token.num;
	else if (op.token == "Mult")
		res.token.num = x1.token.num * x2.token.num;
	else if (op.token == "Div")
		res.token.num = x1.token.num / x2.token.num;
	else if (op.token == "Mod")
		res.token.num = x1.token.num % x2.token.num;
	else if (op.token == "LogicAnd")
		res.token.num = x1.token.num && x2.token.num;
	else if (op.token == "LogicOr")
		res.token.num = x1.token.num || x2.token.num;
	else {
		if (op.token == "Eq")
			res.token.num = x1.token.num == x2.token.num;
		else if (op.token == "NotEq")
			res.token.num = x1.token.num != x2.token.num;
		else if (op.token == "Le")
			res.token.num = x1.token.num <= x2.token.num;
		else if (op.token == "Lt")
			res.token.num = x1.token.num < x2.token.num;
		else if (op.token == "Ge")
			res.token.num = x1.token.num >= x2.token.num;
		else if (op.token == "Gt")
			res.token.num = x1.token.num > x2.token.num;
		else exit(-1);
	}
	number_stack.push(res);
}

number_stack_elem calcAntiPoland(FILE *file, bool is_const_define, bool is_global_define) {
	bool last_word_is_operator = true;
	bool next_word_can_operator = true;
	bool have_logic_and = false;                             // 标记当前语句串是否有 &&
	int logic_and_block_num = logic_and_code_block_num;      // 标记当前是第几个 && 语句
	int logic_and_num = 1;                                   // 标记在当前 && 语句中的第几块
	stack<number_stack_elem> number_stack;
	stack<return_token> operator_stack;
	while (true) {
		// 如果是操作数则直接入栈，并且设置标志位为 false
		if (word.type == "Number") {
			number_stack_elem x;
			x.is_variable = false;
			x.token = word;
			number_stack.push(x);
			last_word_is_operator = false;
			next_word_can_operator = true;
		} else if (word.type == "Symbol") {
			// 如果操作符是分号或逗号，则证明运算结束，按照逆波兰表达式的方式进行运算然后输出
			if (word.token == "Semicolon" || word.token == "Comma" || word.token == "]" || word.token == "RBrace" ||
				word.token == "LogicOr" || word.token == "RPar") {
				if (!is_global_define) {
					while (!operator_stack.empty())
						pop_and_print(number_stack, operator_stack);
				} else {
					while (!operator_stack.empty())
						pop_and_not_print(number_stack, operator_stack);
				}
				if (have_logic_and) {
					number_stack_elem elem = number_stack.top();
					number_stack.pop();
					if (elem.is_function) {
						// TODO sth to deal with the func
					}
					fprintf(output, "%%%d = icmp ne i32 0, ", register_num);
					print_number_stack_elem(elem);
					fprintf(output, "\nbr i1 %%%d, label %%LOGIC_AND_TRUE_%d, label %%LOGIC_AND_FALSE_%d\n",
							register_num++, logic_and_block_num, logic_and_block_num);
					fprintf(output, "\n\nLOGIC_AND_TRUE_%d:\n", logic_and_block_num);
					fprintf(output, "store i32 1, i32* %%logic_and_val_%d\n", logic_and_block_num);
					fprintf(output, "br label %%LOGIC_AND_FINAL_%d\n", logic_and_block_num);

					fprintf(output, "\n\nLOGIC_AND_FALSE_%d:\n", logic_and_block_num);
					fprintf(output, "store i32 0, i32* %%logic_and_val_%d\n", logic_and_block_num);
					fprintf(output, "br label %%LOGIC_AND_FINAL_%d\n", logic_and_block_num);

					fprintf(output, "\n\nLOGIC_AND_FINAL_%d:\n", logic_and_block_num);
					fprintf(output, "%%%d = load i32, i32* %%logic_and_val_%d\n", register_num, logic_and_block_num);
					stringstream stream;
					stream << register_num++;
					elem.is_variable = true;
					elem.is_function = false;
					elem.variable = "%" + stream.str();
					number_stack.push(elem);
				}
				break;
			}

			// 如果操作符是逻辑运算符号，则计算符号之前的表达式，然后将逻辑运算符入栈
			if (is_cond_symbol(word)) {
				if (is_global_define) {
					while (!operator_stack.empty() && priority(operator_stack.top()) <= priority(word))
						pop_and_not_print(number_stack, operator_stack);
				} else {
					while (!operator_stack.empty() && priority(operator_stack.top()) <= priority(word))
						pop_and_print(number_stack, operator_stack);
					// 添加 逻辑与 运算
					if (word.token == "LogicAnd") {
						number_stack_elem elem = number_stack.top();
						number_stack.pop();
						if (!have_logic_and) {
							logic_and_code_block_num++;
							fprintf(output, "%%logic_and_val_%d = alloca i32\n", logic_and_block_num);
							fprintf(output, "br label %%LOGIC_AND_JUDGE_%d_%d\n", logic_and_block_num, logic_and_num);
							have_logic_and = true;
							fprintf(output, "\n\nLOGIC_AND_JUDGE_%d_%d:\t; 第 %d 个 逻辑与 的 第 %d 个判断\n",
									logic_and_block_num,
									logic_and_num, logic_and_block_num, logic_and_num);
						}
						logic_and_num++;
						if (elem.is_function) {
							// TODO deal with func
						}
						fprintf(output, "%%%d = icmp ne i32 0, ", register_num);
						print_number_stack_elem(elem);
						fprintf(output, "\nbr i1 %%%d, label %%LOGIC_AND_JUDGE_%d_%d, label %%LOGIC_AND_FALSE_%d\n",
								register_num++, logic_and_block_num, logic_and_num, logic_and_block_num);
						fprintf(output, "\n\nLOGIC_AND_JUDGE_%d_%d:\t; 第 %d 个 逻辑与 的 第 %d 个判断\n", logic_and_block_num,
								logic_and_num, logic_and_block_num, logic_and_num);
						word = get_symbol(input);
						next_word_can_operator = true;
						last_word_is_operator = true;
						continue;
					}
				}
				operator_stack.push(word);
				next_word_can_operator = true;
			}

				// "非" 运算符直接入栈
			else if (word.token == "Not") {
				operator_stack.push(word);
				next_word_can_operator = true;
			}

				// 左括号则直接入栈
			else if (word.token == "LPar") {
//				operator_stack.push(word);
//				next_word_can_operator = true;
				word = get_symbol(input);
				number_stack_elem res = calcAntiPoland(input, is_const_define, is_global_define);
				number_stack.push(res);
				last_word_is_operator = false;
				word = get_symbol(input);
				continue;
			}
				// 右括号则运算到前面的一个左括号，然后设置标志位为 true
			else if (word.token == "RPar") {
				if (!next_word_can_operator)
					exit(-1);
				if (!is_global_define) {
					while (!operator_stack.empty() && operator_stack.top().token != "LPar")
						pop_and_print(number_stack, operator_stack);
				} else {
					while (!operator_stack.empty() && operator_stack.top().token != "LPar")
						pop_and_not_print(number_stack, operator_stack);
				}
				if (operator_stack.empty())
					break;
				if (operator_stack.top().token != "LPar")
					exit(-1);
				operator_stack.pop();
				word = get_symbol(file);
				last_word_is_operator = false;
				continue;
			} else {
				if (!next_word_can_operator)
					exit(-1);

				if (word.token == "Div" || word.token == "Mod")
					next_word_can_operator = false;
				else next_word_can_operator = true;

				// 如果上一个输入的 token 也是操作符，证明需要添加 0
				if (last_word_is_operator) {
					number_stack_elem add_elem;
					add_elem.is_variable = false;
					add_elem.token.num = 0;
					number_stack.push(add_elem);
				}
					// 如果上一个输入的是操作数，证明下面这个是操作符，不需要添加 0，需要比较算符的优先顺序
				else {
					if (!is_global_define) {
						while (!operator_stack.empty() && priority(operator_stack.top()) <= priority(word))
							pop_and_print(number_stack, operator_stack);
					} else {
						while (!operator_stack.empty() && priority(operator_stack.top()) <= priority(word))
							pop_and_not_print(number_stack, operator_stack);
					}
				}
				operator_stack.push(word);
			}
			last_word_is_operator = true;
		} else if (word.type == IDENT) {
			// 判断是否为函数调用或变量调用，如果不是就报错了
			if (is_variable_list_contains_in_all_layer(word)) {
				// 全局变量定义
				if (is_global_define) {
					if (!is_variable_const(word)) {
						printf("\n'%s' is not a const variable value!\n", word.token.c_str());
						exit_();
					}
					variable_list_elem elem = get_variable(word);
					if (!elem.is_global)
						exit_();
					number_stack_elem x;
					x.is_variable = false;
					x.token.num = elem.global_variable_value;
					number_stack.push(x);
				}
					// 局部变量定义
				else {
					// 常量定义必须要求所有的变量均为常量
					if (is_const_define) {
						if (!is_variable_const(word)) {
							printf("\n'%s' is not a const variable value!\n", word.token.c_str());
							exit(-1);
						}
					}

					// lab 7 要求对数组元素进行计算
					variable_list_elem elem = get_variable(word);
					// 处理数组元素
					if (elem.is_array) {
						number_stack_elem array_dimension_value[10];
						int offset = register_num;
						fprintf(output, "%%%d = add i32 0, 0\t\t\t; 定义临时变量偏移量 0，用来计算数组元素的位置\n", register_num++);
						int i;
						for (i = 1; i <= elem.dimension; i++) {
							word = get_symbol(input);
							if (word.type != SYMBOL || word.token != "[")
								break;
							word = get_symbol(input);
							array_dimension_value[i] = calcAntiPoland(file, is_const_define, is_global_define);
							if (word.type != SYMBOL || word.token != "]")
								exit_();
							if (i != elem.dimension) {
								int number = 1; // 计算数组下一维度的元素数量
								for (int j = i + 1; j <= elem.dimension; j++)
									number *= elem.dimension_num[j];
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
						if (elem.variable_type == "i32")
							fprintf(output, "%%%d = getelementptr i32, i32* %s, i32 %%%d\t; 获取数组元素对应的指针\n",
									register_num++, elem.saved_pointer.c_str(), offset);
						else
							fprintf(output, "%%%d = getelementptr %s, %s* %s, i32 0, i32 %%%d\t; 获取数组元素对应的指针\n",
									register_num++, elem.variable_type.c_str(), elem.variable_type.c_str(),
									elem.saved_pointer.c_str(), offset);
						number_stack_elem x;
						x.is_variable = true;
						stringstream stream;
						if (i == elem.dimension + 1) {
							fprintf(output, "%%%d = load i32, i32* %%%d\t; 加载数组元素的值\n", register_num, register_num - 1);
							stream << register_num++;
						} else stream << register_num - 1;
						x.array_dimension = elem.dimension - (i - 1);
						x.variable = "%" + stream.str();
						number_stack.push(x);
						if (word.token == "RPar" || word.token == "Comma")
							continue;
					}
						// 处理函数
					else if (elem.is_function) {
						number_stack_elem params[10];
						word = get_symbol(input);
						if (word.type != SYMBOL || word.token != "LPar")
							exit_();
						for (int i = 1; i <= elem.function_param_num; i++) {
							word = get_symbol(input);
							params[i] = calcAntiPoland(input);
						}
						if (elem.function_param_num == 0)
							word = get_symbol(input);
						if (word.type != SYMBOL || word.token != "RPar")
							exit_();
						assert(elem.function_return_type == INT);
						fprintf(output, "%%%d = call i32 @%s (", register_num, elem.token.token.c_str());
						for (int i = 1; i <= elem.function_param_num; i++) {
							if (elem.function_param_type[i] == "i32*") {
								if (elem.function_param_dimension[i] != params[i].array_dimension)
									exit_();
							}
							fprintf(output, "%s ", elem.function_param_type[i].c_str());
							print_number_stack_elem(params[i]);
							fprintf(output, ", ");
						}
						if (elem.function_param_num != 0)
							fseek(output, -2, SEEK_CUR);
						fprintf(output, ")\n");
						number_stack_elem res;
						stringstream stream;
						stream << register_num++;
						res.is_variable = true;
						res.is_function = false;
						res.variable = "%" + stream.str();
						number_stack.push(res);
					}
						// 处理变量
					else {
						number_stack_elem x;
						x.is_variable = true;
						x.variable = get_register(word);
						number_stack.push(x);
					}
				}
				last_word_is_operator = false;
				next_word_can_operator = true;
			}
				// 可能调用了 getint() 函数
			else if (word.token == "getint") {
				fprintf(output, "%%%d = call i32 @getint()\n", register_num);
				number_stack_elem x;
				x.is_variable = true;
				stringstream stream;
				stream << register_num++;
				x.variable = "%" + stream.str();
				number_stack.push(x);

				word = get_symbol(file);

				if (word.type != SYMBOL || word.token != "LPar")
					exit(-1);
				word = get_symbol(file);

				if (word.type != SYMBOL || word.token != "RPar")
					exit(-1);
				last_word_is_operator = false;
				next_word_can_operator = true;
			}
				// 可能调用了 getch() 函数
			else if (word.token == "getch") {
				fprintf(output, "%%%d = call i32 @getch()\n", register_num);
				number_stack_elem x;
				x.is_variable = true;
				stringstream stream;
				stream << register_num++;
				x.variable = "%" + stream.str();
				number_stack.push(x);

				word = get_symbol(file);

				if (word.type != SYMBOL || word.token != "LPar")
					exit(-1);
				word = get_symbol(file);

				if (word.type != SYMBOL || word.token != "RPar")
					exit(-1);
				last_word_is_operator = false;
				next_word_can_operator = true;
			}
				// 可能调用了 putint() 函数
			else if (word.token == "putint") {
				word = get_symbol(file);
				if (word.type != SYMBOL || word.token != "LPar")
					exit(-1);

				number_stack_elem res = calcAntiPoland(file);
				fprintf(output, "call void @putint(i32 ");
				if (res.is_variable)
					fprintf(output, "%s)\n", res.variable.c_str());
				else
					fprintf(output, "%d\n)", res.token.num);

				if (word.type != SYMBOL || word.token != "RPar")
					exit(-1);

				word = get_symbol(file);
			}
				// 也可能调用了 putch() 函数
			else if (word.token == "putch") {
				word = get_symbol(file);
				if (word.type != SYMBOL || word.token != "LPar")
					exit(-1);

				number_stack_elem res = calcAntiPoland(file);
				fprintf(output, "call void @putch(i32 ");
				if (res.is_variable)
					fprintf(output, "%s)\n", res.variable.c_str());
				else
					fprintf(output, "%d\n)", res.token.num);

				if (word.type != SYMBOL || word.token != "RPar")
					exit(-1);

				word = get_symbol(file);
			}
				// 也可能调用了 getarray() 函数
			else if (word.token == "getarray") {
				word = get_symbol(input);
				if (word.type != SYMBOL || word.token != "LPar")
					exit_();
				word = get_symbol(input);
				if (!is_variable_list_contains_in_all_layer(word)) {
					fprintf(output, "%s has never been defined!\n", word.token.c_str());
					exit_();
				}
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

					int layer_number = 1;
					for (int j = i + 1; j <= array.dimension; j++)
						layer_number *= array.dimension_num[j];
					fprintf(output, "%%%d = mul i32 %d, ", register_num++, layer_number);
					print_number_stack_elem(param);
					fprintf(output, "\n");

					fprintf(output, "%%%d = add i32 %%%d, %%%d\n", register_num, register_num - 1, offset_register);
					offset_register = register_num++;
				}
				if (word.type != SYMBOL || word.token != "RPar")
					exit_();
				if (array.variable_type == "i32")
					fprintf(output, "%%%d = getelementptr i32, i32* %s, i32 %%%d\t; 获取数组元素对应的指针\n", register_num++,
							array.saved_pointer.c_str(), offset_register);
				else
					fprintf(output, "%%%d = getelementptr %s, %s* %s, i32 0, i32 %%%d\t; 获取数组元素对应的指针\n", register_num++,
							array.variable_type.c_str(), array.variable_type.c_str(), array.saved_pointer.c_str(),
							offset_register);
				fprintf(output, "%%%d = call i32 @getarray(i32* %%%d)\n", register_num, register_num - 1);
				number_stack_elem res;
				res.is_variable = true;
				res.is_function = false;
				stringstream stream;
				stream << register_num++;
				res.variable = "%" + stream.str();
				number_stack.push(res);
			} else {
				printf("%s has never been defined!\n", word.token.c_str());
				exit(-1);
			}
		}
		word = get_symbol(file);
	}
	//word = get_symbol(file);

	return number_stack.top();
}

void print_number_stack_elem(const number_stack_elem &x) {
	if (!x.is_variable)
		fprintf(output, "%d", x.token.num);
	else
		fprintf(output, "%s", x.variable.c_str());
}