//
// Created by 谢铭轩 on 2021/9/21.
//

#ifndef C_JUDGELETTER_H
#define C_JUDGELETTER_H


class judgeLetter {

public:
    static bool isSpace(char c);

    static bool isNewline(char c);

    static bool isTab(char c);

    static bool isLetter(char c);

    static bool isDigit(char c);

    static bool isColon(char c);

    static bool isComma(char c);

    static bool isSemi(char c);

    static bool isPlus(char c);

    static bool isMinus(char c);

    static bool isEqual(char c);

    static bool isDiv(char c);

    static bool isAntiDiv(char c);

    static bool isStar(char c);

    static bool isLBracket(char c);

    static bool isRBracket(char c);

    static bool isLBracketMid(char c);

    static bool isRBracketMid(char c);

    static bool isLBracketLarge(char c);

    static bool isRBracketLarge(char c);

    static bool isDot(char c);

    static bool isUnderline(char c);

    static bool isExcl(char c);

    static bool isVert(char c);

    static bool isQuot(char c);

    static bool isQuot2(char c);

    static bool isGt(char c);

    static bool isLt(char c);

    static bool isOctalDigit(char c);

    static bool isHexadecimalDigit(char c);

};


#endif //C_JUDGELETTER_H
