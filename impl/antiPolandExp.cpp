//
// Created by 谢铭轩 on 2021/11/1.
//

#include "../headers/antiPolandExp.h"
#include "sstream"

extern return_token word;
extern FILE *input;
extern FILE *output;
int register_num = 1;

void print_number_stack_elem(const number_stack_elem &);

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
	else if (op.token == "LogicAnd")
		fprintf(output, "and i32 ");
	else if (op.token == "LogicOr")
		fprintf(output, "or i32 ");
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
		else exit(-1);
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

number_stack_elem calcAntiPoland(FILE *file, bool is_const_define) {
	bool last_word_is_operator = true;
	bool next_word_can_operator = true;
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
			if (word.token == "Semicolon" || word.token == "Comma") {
				while (!operator_stack.empty())
					pop_and_print(number_stack, operator_stack);
				break;
			}

			// 如果操作符是逻辑运算符号，则计算符号之前的表达式，然后将逻辑运算符入栈
			if (is_cond_symbol(word)) {
				while (!operator_stack.empty() && priority(operator_stack.top()) <= priority(word))
					pop_and_print(number_stack, operator_stack);
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
				operator_stack.push(word);
				next_word_can_operator = true;
			}
				// 右括号则运算到前面的一个左括号，然后设置标志位为 true
			else if (word.token == "RPar") {
				if (!next_word_can_operator)
					exit(-1);
				while (!operator_stack.empty() && operator_stack.top().token != "LPar")
					pop_and_print(number_stack, operator_stack);
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

				if (word.token == "Mult" || word.token == "Div" || word.token == "Mod")
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
					while (!operator_stack.empty() && priority(operator_stack.top()) <= priority(word))
						pop_and_print(number_stack, operator_stack);
				}
				operator_stack.push(word);
			}
			last_word_is_operator = true;
		} else if (word.type == IDENT) {
			// 判断是否为函数调用或变量调用，如果不是就报错了
			if (list_contains(word)) {
				if (is_const_define)
					if (!is_variable_const(word)) {
						printf("\n'%s' is not a const variable value!\n", word.token.c_str());
						exit(-1);
					}
				number_stack_elem x;
				x.is_variable = true;
				x.variable = get_register(word);
				number_stack.push(x);
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
			} else {
				printf("%s has never been defined!\n", word.token.c_str());
				exit(-1);
			}
		}
		word = get_symbol(file);
	}
	//word = get_symbol(file);

	return number_stack.top();
//	if (number_stack.top().is_variable) return number_stack.top().variable;
//	if(number_stack.top().token.token != "")
//		return number_stack.top().token.token;
//
//	stringstream stream;
//	stream << number_stack.top().token.num;
//	return stream.str();
}

void print_number_stack_elem(const number_stack_elem &x) {
	if (!x.is_variable)
		fprintf(output, "%d", x.token.num);
	else
		fprintf(output, "%s", x.variable.c_str());
}