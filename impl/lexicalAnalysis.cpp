//
// Created by 谢铭轩 on 2021/9/21.
//

#include "../headers/lexicalAnalysis.h"

char c;

vector<char> token;

void clearToken();

void catToken();

int transNum(int);

void clearToken() {
	token.clear();
}


void catToken() {
	token.push_back(c);
}

int transNum(int base) {
	char *err;
	string tokenStr;
	for (char i: token) {
		tokenStr += i;
	}
	return strtol(tokenStr.c_str(), &err, base);
}

return_token getSymbol(FILE *file) {
	//fseek(file, 0, SEEK_SET);
	clearToken();
	return_token returnToken;
	returnToken.type = "Symbol";
	while ((c = fgetc(file)) != EOF) {

		// 读取字符，如果是空字符则跳过
		if (judgeLetter::isSpace(c) || judgeLetter::isNewline(c) || judgeLetter::isTab(c)) {
			if (judgeLetter::isSpace(c))
				printf(" ");
			else if (judgeLetter::isNewline(c))
				printf("\n");
			else printf("\t");
			continue;
		}

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
				returnToken.token = "If";
				//printf("If\n");
			else if (strcmp(tokenStr.c_str(), "else") == 0)
				returnToken.token = "Else";
				//printf("Else\n");
			else if (strcmp(tokenStr.c_str(), "while") == 0)
				returnToken.token = "While";
				//printf("While\n");
			else if (strcmp(tokenStr.c_str(), "break") == 0)
				returnToken.token = "Break";
				//printf("Break\n");
			else if (strcmp(tokenStr.c_str(), "continue") == 0)
				returnToken.token = "Continue";
				//printf("Continue\n");
			else if (strcmp(tokenStr.c_str(), "return") == 0)
				returnToken.token = "Return";
				//printf("Return\n");
			else
				returnToken.token = tokenStr;
			returnToken.type = "Ident";
			//printf("Ident(%s)\n", tokenStr.c_str());
			clearToken();
			fseek(file, -1, SEEK_CUR);
			printf("%s", tokenStr.c_str());
			return returnToken;
		}

			// 读取字符，如果是数字输出对应数
		else if (judgeLetter::isDigit(c)) {
			int base = 10;
			catToken();
			if (c == '0') {
				if ((c = fgetc(file)) != EOF) {
					catToken();
					if (c == 'x' || c == 'X') {
						base = 16;
						while ((c = fgetc(file)) != EOF) {
							if (judgeLetter::isHexadecimalDigit(c)) {
								catToken();
								continue;
							}
							break;
						}
					} else if (isdigit(c)) {
						base = 8;
						while ((c = fgetc(file)) != EOF) {
							if (judgeLetter::isOctalDigit(c)) {
								catToken();
								continue;
							}
							break;
						}
					}
				}
			} else {
				while ((c = fgetc(file)) != EOF) {
					if (judgeLetter::isDigit(c)) {
						catToken();
						continue;
					}
					break;
				}
			}
			// 拼接结束后输出转化成对数字，清空 token 然后把文件指针前移一位
			returnToken.type = "Number";
			returnToken.num = transNum(base);
			string num = "";
			for (char i: token) num += i;
			printf("%s", num.c_str());
			clearToken();
			fseek(file, -1, SEEK_CUR);
			return returnToken;
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
						fseek(file, -1, SEEK_CUR);
					}
				}
			} else {
				fseek(file, -1, SEEK_CUR);
				returnToken.token = "Div";
				return returnToken;
			}
			continue;
		}

			// 如果是 = 则判断下面的符号是什么
		else if (judgeLetter::isEqual(c)) {
			c = fgetc(file);
			if (judgeLetter::isEqual(c)) {
				printf("==");
				returnToken.token = "Eq";
			} else {
				printf("=");
				returnToken.token = "Assign";
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
		else if (judgeLetter::isComma(c))
			returnToken.token = "Comma";
		else if (judgeLetter::isSemi(c))
			//printf("Semicolon\n");
			returnToken.token = "Semicolon";
		else if (judgeLetter::isLBracket(c))
			//printf("LPar\n");
			returnToken.token = "LPar";
		else if (judgeLetter::isRBracket(c))
			//printf("RPar\n");
			returnToken.token = "RPar";
		else if (judgeLetter::isLBracketLarge(c))
			//printf("LBrace\n");
			returnToken.token = "LBrace";
		else if (judgeLetter::isRBracketLarge(c))
			//printf("RBrace\n");
			returnToken.token = "RBrace";
		else if (judgeLetter::isPlus(c))
			//printf("Plus\n");
			returnToken.token = "Plus";
		else if (judgeLetter::isMinus(c))
			//printf("Minus\n");
			returnToken.token = "Minus";
		else if (judgeLetter::isStar(c))
			//printf("Mult\n");
			returnToken.token = "Mult";
		else if (judgeLetter::isMod(c))
			//printf("Mod\n");
			returnToken.token = "Mod";
		else if (judgeLetter::isLt(c))
			//printf("Lt\n");
			returnToken.token = "Lt";
		else if (judgeLetter::isGt(c))
			//printf("Gt\n");
			returnToken.token = "Gt";

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
			returnToken.type = "Error";
			return returnToken;
		}
		if (judgeLetter::isColon(c) || judgeLetter::isComma(c) || judgeLetter::isLBracket(c) ||
			judgeLetter::isRBracket(c) || judgeLetter::isPlus(c) || judgeLetter::isMinus(c) ||
			judgeLetter::isStar(c) || judgeLetter::isSemi(c) || judgeLetter::isEqual(c) ||
			judgeLetter::isLBracketMid(c) || judgeLetter::isRBracketMid(c) ||
			judgeLetter::isLBracketLarge(c) || judgeLetter::isRBracketLarge(c) ||
			judgeLetter::isDot(c) || judgeLetter::isUnderline(c) || judgeLetter::isExcl(c) ||
			judgeLetter::isVert(c)) {
			printf("%c", c);
		}
		return returnToken;
	}
	if (judgeLetter::isColon(c) || judgeLetter::isComma(c) || judgeLetter::isLBracket(c) ||
		judgeLetter::isRBracket(c) || judgeLetter::isPlus(c) || judgeLetter::isMinus(c) ||
		judgeLetter::isStar(c) || judgeLetter::isSemi(c) || judgeLetter::isEqual(c) ||
		judgeLetter::isLBracketMid(c) || judgeLetter::isRBracketMid(c) ||
		judgeLetter::isLBracketLarge(c) || judgeLetter::isRBracketLarge(c) ||
		judgeLetter::isDot(c) || judgeLetter::isUnderline(c) || judgeLetter::isExcl(c) ||
		judgeLetter::isVert(c)) {
		printf("%c", c);
	}
	return returnToken;
}
