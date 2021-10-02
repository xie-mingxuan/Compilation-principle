//
// Created by 谢铭轩 on 2021/9/21.
//

#include "../headers/lexicalAnalysis.h"

void lexicalAnalysis::clearToken() {
    token.clear();
}

void lexicalAnalysis::catToken() {
    token.push_back(c);
}

int lexicalAnalysis::transNum() {
    int res = 0;
    for (char i: token) {
        res *= 10;
        int i_int = i - '0';
        res += i_int;
    }
    return res;
}

int lexicalAnalysis::getSymbol(FILE *file) {
    fseek(file, 0, SEEK_SET);
    clearToken();
    while ((c = fgetc(file)) != EOF) {

        // 读取字符，如果是空字符则跳过
        if (judgeLetter::isSpace(c) || judgeLetter::isNewline(c) || judgeLetter::isTab(c));

            // 读取字符，如果是字母则拼接字符串
        else if (judgeLetter::isLetter(c) || judgeLetter::isUnderline(c)) {
            catToken();
            while ((c = fgetc(file)) != EOF) {
                if (judgeLetter::isLetter(c) || judgeLetter::isDigit(c) || judgeLetter::isUnderline(c)) {
                    catToken();
                    continue;
                }
                break;
            }
            // 拼接结束后输出字符串，清空 token 然后把文件指针前移一位
            string tokenStr;
            for (char i: token) {
                tokenStr += i;
            }

            if (strcmp(tokenStr.c_str(), "if") == 0)
                printf("If\n");
            else if (strcmp(tokenStr.c_str(), "else") == 0)
                printf("Else\n");
            else if (strcmp(tokenStr.c_str(), "while") == 0)
                printf("While\n");
            else if (strcmp(tokenStr.c_str(), "break") == 0)
                printf("Break\n");
            else if (strcmp(tokenStr.c_str(), "continue") == 0)
                printf("Continue\n");
            else if (strcmp(tokenStr.c_str(), "return") == 0)
                printf("Return\n");
            else printf("Ident(%s)\n", tokenStr.c_str());
            clearToken();
            fseek(file, -1, SEEK_CUR);
        }

            // 读取字符，如果是数字输出对应数
        else if (judgeLetter::isDigit(c)) {
            catToken();
            while ((c = fgetc(file)) != EOF) {
                if (judgeLetter::isDigit(c)) {
                    catToken();
                    continue;
                }
                break;
            }
            // 拼接结束后输出转化成对数字，清空 token 然后把文件指针前移一位
            num = transNum();
            string numStr;
            for (char i: token)
                numStr += i;
            printf("Number(%s)\n", numStr.c_str());
            clearToken();
            fseek(file, -1, SEEK_CUR);
        }

            // 读取字符，如果是正斜杠判断是除号还是注释
        else if (judgeLetter::isDiv(c)) {
            char nextChar = fgetc(file);
            // 如果是单行注释则读到行末
            if (judgeLetter::isDiv(nextChar)) {
                while (c = fgetc(file)) {
                    if (c == EOF || judgeLetter::isNewline(c))
                        break;
                }
                fseek(file, -1, SEEK_CUR);
                continue;
            }
                // 如果是多行注释则读到下一个 */
            else if (judgeLetter::isStar(nextChar)) {
                while ((c = fgetc(file)) != EOF) {
                    if (judgeLetter::isStar(c)) {
                        c = fgetc(file);
                        if (judgeLetter::isDiv(c))
                            break;
                    }
                }
            } else {
                printf("Div\n");
                fseek(file, -1, SEEK_CUR);
            }
        }

            // 如果是 = 则判断下面的符号是什么
        else if (judgeLetter::isEqual(c)) {
            c = fgetc(file);
            if (judgeLetter::isEqual(c)) {
                printf("Eq\n");
            } else {
                printf("Assign\n");
                fseek(file, -1, SEEK_CUR);
            }
        }

//            // 如果是 + — * ( ) : ; = 就直接输出
//        else if (judgeLetter::isColon(c) || judgeLetter::isComma(c) || judgeLetter::isLBracket(c) ||
//                 judgeLetter::isRBracket(c) || judgeLetter::isPlus(c) || judgeLetter::isMinus(c) ||
//                 judgeLetter::isStar(c) || judgeLetter::isSemi(c) || judgeLetter::isEqual(c) ||
//                 judgeLetter::isLBracketMid(c) || judgeLetter::isRBracketMid(c) ||
//                 judgeLetter::isLBracketLarge(c) || judgeLetter::isRBracketLarge(c) ||
//                 judgeLetter::isDot(c) || judgeLetter::isUnderline(c) || judgeLetter::isExcl(c) ||
//                 judgeLetter::isVert(c)) {
//            printf("%c\n", c);
//        }
        else if (judgeLetter::isSemi(c))
            printf("Semicolon\n");
        else if (judgeLetter::isLBracket(c))
            printf("LPar\n");
        else if (judgeLetter::isRBracket(c))
            printf("RPar\n");
        else if (judgeLetter::isLBracketLarge(c))
            printf("LBrace\n");
        else if (judgeLetter::isRBracketLarge(c))
            printf("RBrace\n");
        else if (judgeLetter::isPlus(c))
            printf("Plus\n");
        else if (judgeLetter::isStar(c))
            printf("Mult\n");
        else if (judgeLetter::isDiv(c))
            printf("Div\n");
        else if (judgeLetter::isLt(c))
            printf("Lt\n");
        else if (judgeLetter::isGt(c))
            printf("Gt\n");

//            // 如果是 " 则找到下一个引号然后输出
//        else if (judgeLetter::isQuot(c) || judgeLetter::isQuot2(c)) {
//            if (judgeLetter::isQuot(c)) {
//                while ((c = fgetc(file))) {
//                    if (c == EOF || judgeLetter::isNewline(c)) {
//                        printf("单引号没有对应！\n");
//                    } else {
//                        // 如果是转义字符则直接输入下一个字符
//                        if (judgeLetter::isAntiDiv(c)) {
//                            token.push_back('\\');
//                            token.push_back(fgetc(file));
//                            continue;
//                        }
//
//                        // 如果是单引号则返回
//                        if (judgeLetter::isQuot(c)) {
//                            printf("'%c'\n", token[0]);
//                            clearToken();
//                            break;
//                        }
//
//                        // 如果是其他字符则先判断是否缓冲区是否为空，然后将其加入缓冲区
//                        if (token.empty()) {
//                            catToken();
//                        } else {
//                            printf("单引号中不能出现两个字符！\n");
//                        }
//                    }
//                }
//            } else {
//                while (c = fgetc(file)) {
//                    if (c == EOF || judgeLetter::isNewline(c)) {
//                        printf("双引号没有对应！\n");
//                    } else {
//                        // 如果是转义字符则直接记录下一个字符
//                        if (judgeLetter::isAntiDiv(c)) {
//                            token.push_back('\\');
//                            token.push_back(fgetc(file));
//                            continue;
//                        }
//
//                        // 如果是双引号则结束
//                        if (judgeLetter::isQuot2(c)) {
//                            printf("\"");
//                            for (char i: token)
//                                printf("%c", i);
//                            printf("\"\n");
//                            clearToken();
//                            break;
//                        }
//
//                        catToken();
//                    }
//                }
//            }
//        }

            // 如果是其他字符则输出错误
        else {
            printf("Err\n");
            return -1;
        }
    }
    return 0;
}
