//
// Created by 谢铭轩 on 2021/11/1.
//

#include "../headers/antiPolandExp.h"

struct queue_elem {
    bool is_number;
    union queue_elem {
        int number;
        char operator;
    };
};

return_token word;
queue<queue_elem> anti_poland_exp;
stack<int> number_stack;
void pop_number_stack();
stack<char> operator_stack;

int priority(char);

int calcAntiPoland(FILE *);

int priority(const return_token &c) {
    if (c.token == "Plus" || c.token == "Minus")
        return 1;
    if (c.token == "Mult" || c.token == "Div" || c.token == "Mod")
        return 2;
    return -1;
}

void pop_number_stack() {
    int x1, x2;
    if(!number_stack.empty())
        x2 = number_stack.pop();
    else x2 = 0;
}

int calcAntiPoland(FILE *file) {
    word = getSymbol(file);
    while (word) {
        if (word.type == "Symbol" && word.token == "Semicolon")
            break;

        if (word.type == "Number") {
            int x = atoi(word.token);
            number_stack.push(x);
        } else if (word.type == "Symbol") {
            if (word.token == "LPar")
                operator_stack.push('(');
            else if (word.token == "RPar") {
                while (operator_stack.top() != '(')
            }
        }

        word = getSymbol(file);
    }
}