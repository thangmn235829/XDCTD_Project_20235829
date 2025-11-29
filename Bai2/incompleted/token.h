/* Token.h */
#ifndef __TOKEN_H__
#define __TOKEN_H__

#define MAX_IDENT_LEN 15
#define MAX_STRING_LENGTH 255 // Độ dài tối đa của xâu ký tự
#define KEYWORDS_COUNT 24 // Cập nhật: 20 cơ bản + REPEAT, UNTIL, BYTE, STRING

typedef enum {
  TK_NONE, TK_IDENT, TK_NUMBER, TK_CHAR, TK_EOF,
  // Thêm loại Token mới
  TK_STRING 
} TokenType;

typedef enum {
  KW_PROGRAM, KW_CONST, KW_TYPE, KW_VAR, KW_INTEGER, KW_CHAR, KW_ARRAY,
  KW_OF, KW_FUNCTION, KW_PROCEDURE, KW_BEGIN, KW_END, KW_CALL,
  KW_IF, KW_THEN, KW_ELSE, KW_WHILE, KW_DO, KW_FOR, KW_TO,
  KW_REPEAT, KW_UNTIL,
  // Thêm Keywords mới
  KW_BYTE, KW_STRING
} KeywordType;

typedef enum {
  SB_SEMICOLON, SB_COLON, SB_PERIOD, SB_COMMA, SB_ASSIGN, SB_EQ, SB_NEQ,
  SB_LT, SB_LE, SB_GT, SB_GE, SB_PLUS, SB_MINUS, SB_TIMES, SB_SLASH,
  SB_LPAR, SB_RPAR, SB_LSEL, SB_RSEL, 
  // Thêm Separator mới
  SB_MOD, SB_EXPONENT 
} SeparatorType;

typedef struct {
  // Cập nhật kích thước tối đa cho string (MAX_STRING_LENGTH + 1 cho ký tự NULL)
  char string[MAX_STRING_LENGTH + 1]; 
  int value;
  int lineNo;
  int colNo;
  TokenType tokenType;
} Token;

Token* makeToken(TokenType tokenType, int lineNo, int colNo);
char *tokenToString(TokenType tokenType);
TokenType checkKeyword(char *string);
void printToken(Token *token);

#endif
