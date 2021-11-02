//
// Created by 谢铭轩 on 2021/11/1.
//

#include "../headers/antiPolandExp.h"
#include "sstream"

typedef struct {
    bool is_variable;
    return_token token;
    string variable;
} number_stack_elem;

extern return_token word;
extern FILE *input;
extern FILE *output;
int exp_num = 0;
stack<number_stack_elem> number_stack;
stack<return_token> operator_stack;

int priority(const return_token &c) {
    if (c.token == "Plus" || c.token == "Minus")
        return 1;
    if (c.token == "Mult" || c.token == "Div" || c.token == "Mod")
        return 2;
    return -1;
}

void pop_and_print() {
    number_stack_elem x1, x2;
    if (!number_stack.empty()) {
        x2 = number_stack.top();
        number_stack.pop();
    } else exit(-1);
    if (!number_stack.empty()) {
        x1 = number_stack.top();
        number_stack.pop();
    } else exit(-1);

    return_token op = operator_stack.top();
    operator_stack.pop();
    fprintf(output, "\t%%x%d = ", exp_num);
    if (op.token == "Plus")
        fprintf(output, "add i32 ");
    else if (op.token == "Minus")
        fprintf(output, "sub i32 ");
    else if (op.token == "Mult")
        fprintf(output, "mul i32 ");
    else if (op.token == "Div")
        fprintf(output, "sdiv i32 ");
    else if (op.token == "Mod")
        fprintf(output, "mod i32 ");
    else exit(-1);

    if (!x1.is_variable)
        fprintf(output, "%d, ", x1.token.num);
    else
        fprintf(output, "%s, ", x1.variable.c_str());

    if (!x2.is_variable)
        fprintf(output, "%d\n", x2.token.num);
    else
        fprintf(output, "%s\n", x2.variable.c_str());
    number_stack_elem res;
    res.is_variable = true;
    stringstream stream;
    stream << exp_num++;
    res.variable = "%x" + stream.str();
    number_stack.push(res);
}

string calcAntiPoland(FILE *file) {
    word = getSymbol(file);
    bool last_word_is_operator = true;
    bool next_word_can_operator = true;
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
            // 如果操作符是分号，则证明运算结束，按照逆波兰表达式的方式进行运算然后输出
            if (word.token == "Semicolon") {
                while (!operator_stack.empty())
                    pop_and_print();
                break;
            }

            // 左括号则直接入栈
            if (word.token == "LPar") {
                operator_stack.push(word);
                next_word_can_operator = true;
            }
                // 右括号则运算到前面的一个左括号，然后设置标志位为 true
            else if (word.token == "RPar") {
                if (!next_word_can_operator)
                    exit(-1);
                while (!operator_stack.empty() && operator_stack.top().token != "LPar")
                    pop_and_print();
                if (operator_stack.empty() || operator_stack.top().token != "LPar")
                    exit(-1);
                operator_stack.pop();
                word = getSymbol(file);
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
                    while (!operator_stack.empty() && priority(operator_stack.top()) >= priority(word))
                        pop_and_print();
                }
                operator_stack.push(word);
            }
            last_word_is_operator = true;
        }
        word = getSymbol(file);
    }
    word = getSymbol(file);
    if (number_stack.top().is_variable) return number_stack.top().variable;
    return number_stack.top().token.token;
}