//
// Created by 谢铭轩 on 2021/9/21.
//

#include "../headers/judgeLetter.h"
#include "cctype"

bool judgeLetter::isSpace(char c) {
    return c == ' ';
}

bool judgeLetter::isNewline(char c) {
    return c == '\r' || c == '\n';
}

bool judgeLetter::isTab(char c) {
    return c == '\t';
}

bool judgeLetter::isLetter(char c) {
    return isalpha(c);
}

bool judgeLetter::isDigit(char c) {
    return isdigit(c);
}

bool judgeLetter::isColon(char c) {
    return c == ':';
}

bool judgeLetter::isComma(char c) {
    return c == ',';
}

bool judgeLetter::isSemi(char c) {
    return c == ';';
}

bool judgeLetter::isPlus(char c) {
    return c == '+';
}

bool judgeLetter::isMinus(char c) {
    return c == '-';
}

bool judgeLetter::isEqual(char c) {
    return c == '=';
}

bool judgeLetter::isDiv(char c) {
    return c == '/';
}

bool judgeLetter::isStar(char c) {
    return c == '*';
}

bool judgeLetter::isLBracket(char c) {
    return c == '(';
}

bool judgeLetter::isRBracket(char c) {
    return c == ')';
}

bool judgeLetter::isQuot2(char c) {
    return c == '\"';
}

bool judgeLetter::isQuot(char c) {
    return c == '\'';
}

bool judgeLetter::isAntiDiv(char c) {
    return c == '\\';
}

bool judgeLetter::isLBracketMid(char c) {
    return c == '[';
}

bool judgeLetter::isRBracketMid(char c) {
    return c == ']';
}

bool judgeLetter::isLBracketLarge(char c) {
    return c == '{';
}

bool judgeLetter::isRBracketLarge(char c) {
    return c == '}';
}

bool judgeLetter::isDot(char c) {
    return c == '.';
}

bool judgeLetter::isUnderline(char c) {
    return c == '_';
}

bool judgeLetter::isExcl(char c) {
    return c == '!';
}

bool judgeLetter::isVert(char c) {
    return c == '|';
}

bool judgeLetter::isGt(char c) {
    return c == '>';
}

bool judgeLetter::isLt(char c) {
    return c == '<';
}

bool judgeLetter::isOctalDigit(char c) {
    if (isDigit(c)) {
        if (c == '8' || c == '9')
            return false;
        return true;
    }
    return false;
}

bool judgeLetter::isHexadecimalDigit(char c) {
    if (c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' || c == 'f' ||
        c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E' || c == 'F')
        return true;
    return isDigit(c);
}

bool judgeLetter::isMod(char c) {
    return c == '%';
}
