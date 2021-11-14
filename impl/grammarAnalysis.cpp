//
// Created by 谢铭轩 on 2021/10/14.
//

#include "../headers/grammarAnalysis.h"

return_token word;
FILE *input;
FILE *output;
string express;
list<variable_list_elem> variable_list; // 这是所有定义的变量

void Decl(FILE *);

void ConstDecl(FILE *);

void BType(FILE *);

void ConstDef(FILE *);

number_stack_elem ConstInitVal(FILE *);

number_stack_elem ConstExp(FILE *);

void VarDecl(FILE *);

void VarDef(FILE *);

number_stack_elem InitVal(FILE *);

void FuncDef(FILE *);

void FuncType(FILE *);

void Block(FILE *);

void BlockItem(FILE *);

void Stmt(FILE *);

number_stack_elem Exp(FILE *);

void LVal(FILE *);

void PrimaryExp(FILE *);

void AddExp(FILE *);

void MulExp(FILE *);

void UnaryExp(FILE *);

void FuncRParams(FILE *);

void Ident(FILE *);

void UnaryOp(FILE *);

void init();


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
	word = getSymbol(file);
	fprintf(output, "(");

	if (word.type != SYMBOL || word.token != "RPar")
		exit_();
	word = getSymbol(file);
	fprintf(output, ")");

	Block(file);

	word = getSymbol(file);
}

void FuncType(FILE *file) {
	if (word.type != IDENT || word.token != "int")
		exit_();

	word = getSymbol(file);
	fprintf(output, "define i32 ");
}

void Ident(FILE *file) {
	if (word.type != IDENT || word.token != "main")
		exit_();

	word = getSymbol(file);
	fprintf(output, "@main");
}

void Block(FILE *file) {
	if (word.type != SYMBOL || word.token != "LBrace")
		exit_();
	word = getSymbol(file);
	fprintf(output, "{\n");

	while (word.type != SYMBOL || word.token != "RBrace") {
		BlockItem(file);
	}

	if (word.type != SYMBOL || word.token != "RBrace")
		exit_();
	word = getSymbol(file);
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
	word = getSymbol(file);

	if (word.type != IDENT || word.token != "int")
		exit_();
	word = getSymbol(file);

	ConstDef(file);

	while (word.type == SYMBOL && word.token == "Comma") {
		word = getSymbol(file);
		ConstDef(file);
	}

	if (word.type != SYMBOL || word.token != "Semicolon")
		exit_();

	word = getSymbol(file);
}

void ConstDef(FILE *file) {
	if (word.type != IDENT)
		exit_();
	fprintf(output, "%%%s = alloca i32\n", word.token.c_str());

	// 如果声明的变量已经被声明过了，就要退出；否则将其加入符号表
	if (list_contains(word))
		exit_();
	variable_list_elem elem;
	elem.token = word;
	elem.is_const = true;
	variable_list.push_back(elem);

	string i = word.token;

	word = getSymbol(file);

	if (word.type != SYMBOL || word.token != "Assign")
		exit_();
	word = getSymbol(file);

	number_stack_elem res = ConstInitVal(file);
	if (res.is_variable)
		fprintf(output, "store i32 %%%s, i32* %s", i.c_str(), res.variable.c_str());
	else
		fprintf(output, "store i32 %%%s, i32 %d", i.c_str(), res.token.num);
}

number_stack_elem ConstInitVal(FILE *file) {
	return ConstExp(file);
}

/**
 * 这个函数用来计算表达式的值，最后返回表达式的值或对应的储存元素
 */
number_stack_elem ConstExp(FILE *file) {
	return calcAntiPoland(file);
}

void VarDecl(FILE *file) {
	if (word.type != IDENT || word.token != "int")
		exit_();
	word = getSymbol(file);
	VarDef(file);

	while (word.type == SYMBOL && word.token == "Comma") {
		word = getSymbol(file);
		VarDef(file);
	}

	if (word.type != SYMBOL && word.token != "Semicolon")
		exit_();

	word = getSymbol(file);
}

void VarDef(FILE *file) {
	if (word.type != IDENT)
		exit_();
	fprintf(output, "%%%s = alloca i32\n", word.token.c_str());

	// 如果声明的变量已经被声明过了，就要退出；否则将其加入符号表
	if (list_contains(word))
		exit_();
	variable_list_elem elem;
	elem.token = word;
	variable_list.push_back(elem);

	string i = word.token;

	word = getSymbol(file);

	if (word.type != SYMBOL || word.token != "Assign")
		return;

	word = getSymbol(file);
	number_stack_elem res = InitVal(file);
	if (res.is_variable)
		fprintf(output, "store i32 %%%s, i32* %s\n", i.c_str(), res.variable.c_str());
	else
		fprintf(output, "store i32 %%%s, i32 %d\n", i.c_str(), res.token.num);
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
			word = getSymbol(file);
			number_stack_elem res = calcAntiPoland(file);
			if (res.is_variable)
				fprintf(output, "ret i32 %s\n", res.variable.c_str());
			else fprintf(output, "ret i32 %d\n", res.token.num);

			if (word.type != SYMBOL || word.token != "Semicolon")
				exit_();
			word = getSymbol(file);
			return;
		} else if (word.token == "putch") {
			word = getSymbol(file);
			if (word.type != SYMBOL || word.token != "LPar")
				exit(-1);

			number_stack_elem res = calcAntiPoland(file);
			if (res.is_variable)
				fprintf(output, "call void @putch(i32 %s)\n", res.variable.c_str());
			else
				fprintf(output, "call void @putch(i32 %d)\n", res.token.num);

			if (word.type != SYMBOL || word.token != "Semicolon")
				exit(-1);

			word = getSymbol(file);
			return;
		} else if (word.token == "putint") {
			word = getSymbol(file);
			if (word.type != SYMBOL || word.token != "LPar")
				exit(-1);

			number_stack_elem res = calcAntiPoland(file);
			if (res.is_variable)
				fprintf(output, "call void @putint(i32 %s)\n", res.variable.c_str());
			else
				fprintf(output, "call void @putint(i32 %d)\n", res.token.num);

			if (word.type != SYMBOL || word.token != "Semicolon")
				exit(-1);

			word = getSymbol(file);
			return;
		}

		if (!list_contains(word)) {
			printf("%s has never been defined!\n", word.token.c_str());
			exit_();
		}

		// 无意义语句直接跳过
		return_token x = word;
		word = getSymbol(file);
		if (word.type != SYMBOL && word.token != "Assign") {
			while (word.type != SYMBOL || word.token != "Semicolon")
				word = getSymbol(file);
			word = getSymbol(file);
			return;
		}

		// 赋值语句先判断是否是常量，然后再进行赋值
		if (is_variable_const(x)) {
			printf("%s is a const, cannot be assigned!\n", x.token.c_str());
			exit_();
		}
		word = getSymbol(file);
		number_stack_elem res = calcAntiPoland(file);
		if (res.is_variable)
			fprintf(output, "%%%s = %s\n", x.token.c_str(), res.variable.c_str());
		else
			fprintf(output, "%%%s = %d\n", x.token.c_str(), res.token.num);

		if (word.type != SYMBOL || word.token != "Semicolon")
			exit_();
		word = getSymbol(file);
		return;
	}
	while (word.type != SYMBOL || word.token != "Semicolon")
		word = getSymbol(file);
	word = getSymbol(file);
}

void CompUnit(FILE *in, FILE *out) {
	input = in;
	output = out;
	word = getSymbol(in);
	init();
	FuncDef(in);
	word = getSymbol(in);
	if (word.type == "Error")
		exit_();
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